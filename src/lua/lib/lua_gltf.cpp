#include "lua_gltf.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include "engine/engine.h"
#include "entt/entity/entity.hpp"
#include "lua_instance.h"
#include "resource/mesh/mesh.h"
#include "util/output.h"
#include <vector>

static Output output;
static Assimp::Importer importer;

namespace Lua::gltf {

	struct ImportedResourceMap {
		std::vector<int> mesh_indices_map;
	};

	Lua::instance::Instance create_nodes_from_import(ImportedResourceMap map, entt::entity parent, aiNode *node) {
		Lua::instance::Instance root(node);

		if (parent != entt::null) {
			auto &instance = registry->get<Scene::ecs::Instance>(root.entity);
			instance.parent = parent;
		}

		if (node->mNumMeshes > 0) {
			for (int i = 0; i < node->mNumMeshes; i++) {
				output.info("node %s has mesh mapped to %i", node->mName.data, map.mesh_indices_map[node->mMeshes[i]]);
			}
		}

		for (int i = 0; i < node->mNumChildren; i++) {
			aiNode *child = node->mChildren[i];

			create_nodes_from_import(map, root.entity, child);
		}

		return root;
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

		output.info("scene has %i meshes", scene->mNumMeshes);
		output.info("scene has %i textures", scene->mNumTextures);
		output.info("scene has %i cameras", scene->mNumCameras);

		ImportedResourceMap map = {};

		if (scene->HasTextures()) {
			output.info("scene has textures");
		};

		if (scene->HasMeshes()) {
			for (int i = 0; i < scene->mNumMeshes; i++) {
				MeshGeometry geometry = read_mesh_from_import(scene, i);
				engine->resources.meshes.push_back(geometry);
				map.mesh_indices_map.push_back(engine->resources.meshes.size() - 1);

				output.info("imported mesh '%s' to index %i", geometry.name.data(), map.mesh_indices_map[map.mesh_indices_map.size() - 1]);
			}
		};

		return create_nodes_from_import(map, entt::null, scene->mRootNode);
	}

	void init(EngineInstance *engine_ref, sol::state &lua) {
		registry = &engine_ref->registry;
		engine = engine_ref;

		auto gltf_namespace = lua["GLTF"].get_or_create<sol::table>();

		gltf_namespace["import"] = import_scene;
	};
}