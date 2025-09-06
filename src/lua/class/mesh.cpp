#include "mesh.h"


#include "../lua.h"


meshGeometry mesh_load_geometry(const char *path)
{
	printf("[mesh.cpp] loading mesh from %s\n", path);
	meshGeometry geometry;
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		printf("[mesh.cpp] ERROR: Failed to load mesh: %s\n", importer.GetErrorString());
		return geometry;
	}

	if (scene->mNumMeshes == 0)
	{
		printf("[mesh.cpp] ERROR: No meshes found in file: %s\n", path);
		return geometry;
	}

	aiNode *root_node = scene->mRootNode;

	if (root_node->mNumMeshes > 1)
	{
		printf("[mesh.cpp] WARNING: Multiple meshes found, using first mesh only\n");
	}

	aiMesh *mesh = scene->mMeshes[0];


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
		if (mesh->mTextureCoords[0])
		{
			texture_coordinate.x = mesh->mTextureCoords[0][i].x;
			texture_coordinate.y = mesh->mTextureCoords[0][i].y;
		}
		else
		{
			texture_coordinate = glm::vec2(0.0f, 0.0f);
		}
		vertex.texCoords = texture_coordinate;

		geometry.vertices.push_back(vertex);
	}


	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			geometry.indices.push_back(face.mIndices[j]);
		}
	}

	return geometry;
}


int mesh_load(bsgeMesh *bsgemesh, const char *path)
{
	printf("[mesh.cpp] loading mesh from %s\n", path);
	meshGeometry geometry = mesh_load_geometry(path);
	bsgemesh->geometry = geometry;
	
	unsigned int VBO;
	unsigned int VAO;
	unsigned int EBO;

	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenVertexArrays(1, &VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBindVertexArray(VAO);
	glBufferData(GL_ARRAY_BUFFER, geometry.vertices.size() * sizeof(meshVertexData), &geometry.vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.indices.size() * sizeof(unsigned int), &geometry.indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));

	bsgemesh->ebo = EBO;
	bsgemesh->vao = VAO;
	bsgemesh->vbo = VBO;
	bsgemesh->indices_count = geometry.indices.size();
	bsgemesh->texture = 0;

	return 0;
}

int mesh_render(sol::state &lua, glm::mat4 matrix, bsgeMesh &bsgemesh) {
	glUseProgram(context_window->default_shader);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bsgemesh.ebo);
	glBindVertexArray(bsgemesh.vao);
	glUniform4f(glGetUniformLocation(context_window->default_shader, "img_color"), bsgemesh.color.x, bsgemesh.color.y, bsgemesh.color.z, bsgemesh.color.w);

	camera_set_shader_projection_matrix(lua, context_window);

	if (bsgemesh.texture != 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bsgemesh.texture);
		glUniform1i(glGetUniformLocation(context_window->default_shader, "img_texture"), 0);
	}

	glUniformMatrix4fv(glGetUniformLocation(context_window->default_shader, "transform"), 1, GL_FALSE, glm::value_ptr(matrix));
	glDrawElements(GL_TRIANGLES, bsgemesh.indices_count, GL_UNSIGNED_INT, 0);
	glUseProgram(0);

	return 0;
}

void lua_bsge_init_mesh(sol::state &lua) {
	lua_State *L = lua.lua_state();

	auto set_texture = [](bsgeMesh *mesh, bsgeImage *image)
	{
		mesh->texture = image->id;
	};

	lua.new_usertype<bsgeMesh>("Mesh",
								sol::constructors<bsgeMesh(sol::this_state, const char*)>(),
							   "texture", sol::property([](bsgeMesh *mesh)
														{ return mesh->texture; }, set_texture)
								);
}
