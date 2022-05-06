#include "mesh.h"

#include "string"
#include <vector>

int lua_bsge_new_mesh(lua_State* L) {
	const char* path = lua_tostring(L, 1);
	printf("[mesh.cpp] loading mesh from %s\n", path);

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	aiNode* root_node = scene->mRootNode;

	// currently only single mesh loading is allowed
	if (root_node->mNumMeshes > 1) {
		// breaks
		return 25;
	}

	aiMesh* mesh = scene->mMeshes[0];
	std::vector<meshVertexData> vertices;
	std::vector<unsigned int> indices;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		meshVertexData vertex;
		glm::vec3 position;
		position.x = mesh->mVertices[i].x;
		position.y = mesh->mVertices[i].y;
		position.z = mesh->mVertices[i].z;
		vertex.position = position;

		glm::vec3 normal;
		normal.x = mesh->mNormals[i].x;
		normal.y = mesh->mNormals[i].y;
		normal.z = mesh->mNormals[i].z;
		vertex.normal = normal;

		glm::vec2 texture_coordinate;
		if (mesh->mTextureCoords[0]) {
			texture_coordinate.x = mesh->mTextureCoords[0][i].x;
			texture_coordinate.y = mesh->mTextureCoords[0][i].y;
		}
		else {
			texture_coordinate = glm::vec2(0.0f, 0.0f);
		}
		vertex.texCoords = texture_coordinate;

		printf("%f %f %f %f %f %f %f %f\n", vertex.position.x, vertex.position.y, vertex.position.z, vertex.normal.x, vertex.normal.y, vertex.normal.z, vertex.texCoords.x, vertex.texCoords.y);
		vertices.push_back(vertex);
	}

	printf("faces: %i\n", mesh->mNumFaces);

	// https://learnopengl.com/Model-Loading/Model
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		printf("\n");
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			printf("%i ", face.mIndices[j]);
			indices.push_back(face.mIndices[j]);

		}
	}

	printf("\n");

	unsigned int VBO;
	unsigned int VAO;
	unsigned int EBO;

	// MODEL CODE
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenVertexArrays(1, &VAO);
	printf("1\n");

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBindVertexArray(VAO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(meshVertexData), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// 1. then set the vertex attributes pointers

	// position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

	// normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

	// texture coordinates
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	meshData* bsgemesh = (meshData*)lua_newuserdata(L, sizeof(meshData));
	bsgemesh->ebo = EBO;
	bsgemesh->vao = VAO;
	bsgemesh->vbo = VBO;
	bsgemesh->indices_count = indices.size();
	bsgemesh->texture = 0;

	printf("indices: %i\n", bsgemesh->indices_count);

	luaL_getmetatable(L, "Mesh");
	lua_setmetatable(L, -2);

	return 1;
}

int lua_bsge_use_texture(lua_State* L) {
	// this, texture
	meshData* bsgemesh = (meshData*)lua_touserdata(L, 1);
	bsgeImage* texture = (bsgeImage*)lua_touserdata(L, 2);
	bsgemesh->texture = texture->id;
	printf("attach_texture\n");

	return 0;
}

const luaL_Reg bsge_lua_mesh_methods[] = {
	{"use_texture", lua_bsge_use_texture},
	{NULL, NULL},
};

void lua_bsge_init_mesh(lua_State* L) {
	// {__index = { ...bsge_lua_mesh_methods }}
	luaL_newmetatable(L, "Mesh");
	lua_newtable(L);
	luaL_setfuncs(L, bsge_lua_mesh_methods, 0);
	lua_setfield(L, -2, "__index");
}
