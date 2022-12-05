
// int lua_bsge_new_mesh(lua_State* L) {
// }

// int lua_bsge_use_texture(lua_State* L) {
// 	// this, texture
// 	meshData* bsgemesh = (meshData*)lua_touserdata(L, 1);
// 	bsgeImage* texture = (bsgeImage*)lua_touserdata(L, 2);
// 	bsgemesh->texture = texture->id;
// 	printf("attach_texture\n");

// 	return 0;
// }

// const luaL_Reg bsge_lua_mesh_methods[] = {
// 	{"use_texture", lua_bsge_use_texture},
// 	{NULL, NULL},
// };

// void lua_bsge_init_mesh(lua_State* L) {
// 	// {__index = { ...bsge_lua_mesh_methods }}
// 	luaL_newmetatable(L, "Mesh");
// 	lua_newtable(L);
// 	luaL_setfuncs(L, bsge_lua_mesh_methods, 0);
// 	lua_setfield(L, -2, "__index");
// }
#include "mesh.h"
#include "string"

#include <vector>

int mesh_load(lua_State* L) {
	meshData* bsgemesh = (meshData*)lua_touserdata(L, 1);
	const char* path = lua_tostring(L, 2);

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

		// printf("%f %f %f %f %f %f %f %f\n", vertex.position.x, vertex.position.y, vertex.position.z, vertex.normal.x, vertex.normal.y, vertex.normal.z, vertex.texCoords.x, vertex.texCoords.y);
		vertices.push_back(vertex);
	}

	printf("faces: %i\n", mesh->mNumFaces);

	// https://learnopengl.com/Model-Loading/Model
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		// printf("\n");
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			// printf("%i ", face.mIndices[j]);
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

int mesh_render(lua_State* L) {

	meshData* bsgemesh = (meshData*)lua_touserdata(L, 1);

	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::scale(trans, glm::vec3(0.1f, 0.1f, 0.1f));
	trans = glm::rotate(trans, glm::radians((float)glfwGetTime() * 90.0f), glm::vec3(0.5, 1, 0.2));

	glUseProgram(context_window->default_shader);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bsgemesh->ebo);
	glBindVertexArray(bsgemesh->vao);
	glBindTexture(GL_TEXTURE_2D, bsgemesh->texture);

	glUniformMatrix4fv(glGetUniformLocation(context_window->default_shader, "transform"), 1, GL_FALSE, glm::value_ptr(trans));
	glDrawElements(GL_TRIANGLES, bsgemesh->indices_count, GL_UNSIGNED_INT, 0);
	glUseProgram(0);

	return 0;
}

int mesh_index(lua_State* L) {
	// this, index
	const char* index = lua_tostring(L, 2);

	if (strcmp(index, "load") == 0) {
		lua_pushcclosure(L, mesh_load, 0);
		return 1;
	}
	else if (strcmp(index, "render") == 0) {
		lua_pushcclosure(L, mesh_render, 0);
		return 1;
	}

	return 0;
}

int mesh_newindex(lua_State* L) {
	const char* index = lua_tostring(L, 2);

	if (strcmp(index, "texture") == 0) {
		meshData* bsgemesh = (meshData*)lua_touserdata(L, 1);
		bsgeImage* texture = (bsgeImage*)lua_touserdata(L, 3);

		bsgemesh->texture = texture->id;
		printf("texture set\n");

		return 1;
	}

	return 0;
}

const luaL_Reg bsge_lua_mesh_metatable_access[] = {
	{"__index", mesh_index},
	{"__newindex", mesh_newindex},

	{NULL, NULL},
};

int lua_bsge_instance_mesh(lua_State* L) {
	meshData* bsgemesh = (meshData*)lua_newuserdata(L, sizeof(meshData));
	luaL_getmetatable(L, "Mesh");
	lua_setmetatable(L, -2);

	return 1;
}

void lua_bsge_init_mesh(lua_State* L) {
	luaL_newmetatable(L, "Mesh");
	luaL_setfuncs(L, bsge_lua_mesh_metatable_access, 0);

	// todo: replace with a universal constructor
	lua_register(L, "_temp_Mesh", lua_bsge_instance_mesh);
}
