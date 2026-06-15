#include "lua_gltf.h"
#include "assimp/camera.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include "assimp/texture.h"
#include "ecs/scene_camera.h"
#include "ecs/scene_mesh.h"
#include "engine/engine.h"
#include "entt/entity/entity.hpp"
#include "lua_instance.h"
#include "resource/image/image.h"
#include "resource/mesh/mesh.h"
#include "util/output.h"
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "include/stbi_image.h"

static Output output;
static Assimp::Importer importer;

namespace Lua::gltf {

	struct ImportedResourceMap {
		std::vector<int> mesh_indices_map;
	};

	Lua::instance::Instance create_nodes_from_import(ImportedResourceMap map, entt::entity parent, aiNode *node) {
		output.info("creating child %s", node->mName.data);
		Lua::instance::Instance root(node);

		root.set_transform(node->mTransformation);

		if (parent != entt::null) {
			root.set_parent(parent);
		}

		if (node->mNumMeshes > 0) {
			for (int i = 0; i < node->mNumMeshes; i++) {
				output.info("node %s has mesh mapped to %i", node->mName.data, map.mesh_indices_map[node->mMeshes[i]]);
			}

			// attach the node's first mesh so the renderer draws it
			registry->emplace<Ecs::MeshComponent>(root.entity, map.mesh_indices_map[node->mMeshes[0]]);
		}

		for (int i = 0; i < node->mNumChildren; i++) {
			aiNode *child = node->mChildren[i];

			// only import mesh children, lights and cameras are separate
			if (child->mNumMeshes == 0) continue;

			create_nodes_from_import(map, root.entity, child);
		}

		return root;
	}

	ImageData read_texture_from_import(const aiTexture *texture) {
		ImageData image(texture->mFilename.length > 0 ? texture->mFilename.data : "texture");

		if (texture->mHeight == 0) {
			int channels = 0;
			stbi_uc *data = stbi_load_from_memory(
			    reinterpret_cast<const stbi_uc *>(texture->pcData),
			    texture->mWidth,
			    &image.width,
			    &image.height,
			    &channels,
			    STBI_rgb_alpha);

			if (data) {
				image.pixels.assign(data, data + image.width * image.height * 4);
				stbi_image_free(data);
			} else {
				output.warn("failed to decode embedded texture %s", image.name.data());
			}
		} else {
			image.width = texture->mWidth;
			image.height = texture->mHeight;
			image.pixels.resize((size_t)image.width * image.height * 4);

			for (int i = 0; i < image.width * image.height; i++) {
				const aiTexel &texel = texture->pcData[i];
				image.pixels[i * 4 + 0] = texel.r;
				image.pixels[i * 4 + 1] = texel.g;
				image.pixels[i * 4 + 2] = texel.b;
				image.pixels[i * 4 + 3] = texel.a;
			}
		}

		return image;
	}

	MeshGeometry read_mesh_from_import(const aiScene *scene, int i) {
		aiMesh *mesh = scene->mMeshes[i];
		MeshGeometry geometry(scene->mMeshes[i]->mName.data);

		output.info("importing %s", geometry.name.data());

		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			MeshVertex vertex;

			glm::vec3 position;
			position.x = mesh->mVertices[i].x;
			position.y = mesh->mVertices[i].y;
			position.z = mesh->mVertices[i].z;

			glm::vec3 normal;
			normal.x = mesh->mNormals[i].x;
			normal.y = mesh->mNormals[i].y;
			normal.z = mesh->mNormals[i].z;

			vertex.position = position;
			vertex.normal = normal;
			if (mesh->mTextureCoords[0]) {
				vertex.uv_x = mesh->mTextureCoords[0][i].x;
				vertex.uv_y = mesh->mTextureCoords[0][i].y;
			}

			geometry.vertices.push_back(vertex);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				geometry.indices.push_back(face.mIndices[j]);
			}
		}

		return geometry;
	}

	Lua::instance::Instance import_scene(const char *path) {
		output.warn("loading glb: %s", path);

		const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		ImportedResourceMap map = {};

		if (scene->HasTextures()) {
			for (unsigned int i = 0; i < scene->mNumTextures; i++) {
				ImageData image = read_texture_from_import(scene->mTextures[i]);
				engine->resources.images.push_back(image);

				output.info("imported texture '%s' (%ix%i) to index %i", image.name.data(), image.width, image.height, (int)engine->resources.images.size() - 1);
			}
		};

		if (scene->HasMeshes()) {
			for (int i = 0; i < scene->mNumMeshes; i++) {
				MeshGeometry geometry = read_mesh_from_import(scene, i);
				engine->resources.meshes.push_back(geometry);
				map.mesh_indices_map.push_back(engine->resources.meshes.size() - 1);

				output.info("imported mesh '%s' to index %i", geometry.name.data(), map.mesh_indices_map[map.mesh_indices_map.size() - 1]);
			}
		};

		Lua::instance::Instance root = create_nodes_from_import(map, entt::null, scene->mRootNode);

		if (scene->HasCameras()) {
			output.info("scene has cameras");

			for (int i = 0; i < scene->mNumCameras; i++) {
				aiCamera *camera = scene->mCameras[i];

				aiNode *camera_node = scene->mRootNode->FindNode(camera->mName);

				aiMatrix4x4 transform_matrix;
				// TODO solve for every node on-render
				for (aiNode *node = camera_node; node != nullptr; node = node->mParent) {
					transform_matrix = node->mTransformation * transform_matrix;
				}

				Lua::instance::Instance camera_instance(camera->mName.data);
				camera_instance.set_parent(root);
				camera_instance.set_transform(transform_matrix);

				bool isOrthographic = camera->mOrthographicWidth != 0;

				if (isOrthographic) {
					registry->emplace<Ecs::CameraComponent>(
					    camera_instance.entity,
					    Ecs::CameraPerspectiveType::Orthographic,
					    camera->mHorizontalFOV);
				} else {
					registry->emplace<Ecs::CameraComponent>(
					    camera_instance.entity,
					    Ecs::CameraPerspectiveType::Perspective,
					    camera->mHorizontalFOV);
				}

				output.info("imported camera %s", camera->mName.data);
			}
		};

		return root;
	}

	void init(EngineInstance *engine_ref, sol::state &lua) {
		registry = &engine_ref->registry;
		engine = engine_ref;

		auto gltf_namespace = lua["GLTF"].get_or_create<sol::table>();

		gltf_namespace["import"] = import_scene;
	};
}