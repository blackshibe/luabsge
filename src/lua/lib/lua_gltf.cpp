#include "lua_gltf.h"
#include "assimp/camera.h"
#include "assimp/light.h"
#include "assimp/material.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include "assimp/texture.h"
#include "ecs/scene_camera.h"
#include "ecs/scene_light.h"
#include "ecs/scene_material.h"
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
		std::vector<int> mesh_texture_map;
		std::vector<glm::vec3> mesh_color_map;
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
			int local_mesh = node->mMeshes[0];
			registry->emplace<Ecs::MeshComponent>(root.entity, map.mesh_indices_map[local_mesh]);

			int mesh_map_index = map.mesh_texture_map[local_mesh];
			if (mesh_map_index != -1)
				registry->emplace<Ecs::PBRMaterialComponent>(root.entity, map.mesh_texture_map[local_mesh]);
			else
				registry->emplace<Ecs::BaseColorMaterialComponent>(root.entity, map.mesh_color_map[local_mesh]);
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

	MeshGeometry read_mesh_from_import(const aiScene *scene, int image_index, int current_engine_image_count, int &texture_index, glm::vec3 &base_color) {
		aiMesh *mesh = scene->mMeshes[image_index];
		MeshGeometry geometry(scene->mMeshes[image_index]->mName.data);

		output.info("importing %s", geometry.name.data());

		texture_index = -1;
		base_color = glm::vec3(1.0f);
		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

		aiColor4D color;
		if (material->Get(AI_MATKEY_BASE_COLOR, color) == AI_SUCCESS || material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
			base_color = glm::vec3(color.r, color.g, color.b);
		}

		aiString path;
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
			const aiTexture *embedded = scene->GetEmbeddedTexture(path.C_Str());
			if (embedded) {
				for (unsigned int i = 0; i < scene->mNumTextures; i++) {
					if (scene->mTextures[i] == embedded) {
						texture_index = current_engine_image_count + i;
						break;
					}
				}
			}
		}

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

		int current_engine_image_count = engine->resources.images.size();

		if (scene->HasTextures()) {
			for (unsigned int i = 0; i < scene->mNumTextures; i++) {
				ImageData image = read_texture_from_import(scene->mTextures[i]);
				engine->resources.images.push_back(image);

				output.info("imported texture '%s' (%ix%i) to index %i", image.name.data(), image.width, image.height, (int)engine->resources.images.size() - 1);
			}
		};

		if (scene->HasMeshes()) {
			for (int i = 0; i < scene->mNumMeshes; i++) {
				int texture_index = -1;
				glm::vec3 base_color = glm::vec3(1.0f);
				MeshGeometry geometry = read_mesh_from_import(scene, i, current_engine_image_count, texture_index, base_color);
				engine->resources.meshes.push_back(geometry);
				map.mesh_indices_map.push_back(engine->resources.meshes.size() - 1);
				map.mesh_texture_map.push_back(texture_index);
				map.mesh_color_map.push_back(base_color);

				output.info("imported mesh '%s' to index %i (texture %i)", geometry.name.data(), map.mesh_indices_map[map.mesh_indices_map.size() - 1], texture_index);
				output.info("mesh '%s' base color (%.3f, %.3f, %.3f)", geometry.name.data(), base_color.r, base_color.g, base_color.b);
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

		if (scene->HasLights()) {
			output.info("scene has lights");

			for (unsigned int i = 0; i < scene->mNumLights; i++) {
				aiLight *light = scene->mLights[i];
				if (light->mType != aiLightSource_DIRECTIONAL && light->mType != aiLightSource_POINT) continue;

				aiNode *light_node = scene->mRootNode->FindNode(light->mName);

				aiMatrix4x4 transform_matrix;
				for (aiNode *node = light_node; node != nullptr; node = node->mParent) {
					transform_matrix = node->mTransformation * transform_matrix;
				}

				glm::vec3 color(light->mColorDiffuse.r, light->mColorDiffuse.g, light->mColorDiffuse.b);

				Lua::instance::Instance light_instance(light->mName.data);
				light_instance.set_parent(root);
				light_instance.set_transform(transform_matrix);

				if (light->mType == aiLightSource_DIRECTIONAL) {
					// rotate the light's local direction into world space (rotation part only)
					aiVector3D d = light->mDirection;
					aiVector3D direction(
					    transform_matrix.a1 * d.x + transform_matrix.a2 * d.y + transform_matrix.a3 * d.z,
					    transform_matrix.b1 * d.x + transform_matrix.b2 * d.y + transform_matrix.b3 * d.z,
					    transform_matrix.c1 * d.x + transform_matrix.c2 * d.y + transform_matrix.c3 * d.z);
					direction.Normalize();

					registry->emplace<Ecs::DirectionalLightComponent>(
					    light_instance.entity,
					    glm::vec3(direction.x, direction.y, direction.z),
					    color);

					output.info("imported directional light %s", light->mName.data);
				} else {
					// transform the light's local position into world space (full transform)
					aiVector3D p = light->mPosition;
					aiVector3D position(
					    transform_matrix.a1 * p.x + transform_matrix.a2 * p.y + transform_matrix.a3 * p.z + transform_matrix.a4,
					    transform_matrix.b1 * p.x + transform_matrix.b2 * p.y + transform_matrix.b3 * p.z + transform_matrix.b4,
					    transform_matrix.c1 * p.x + transform_matrix.c2 * p.y + transform_matrix.c3 * p.z + transform_matrix.c4);

					registry->emplace<Ecs::PointLightComponent>(
					    light_instance.entity,
					    glm::vec3(position.x, position.y, position.z),
					    color);

					output.info("imported point light %s", light->mName.data);
				}
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