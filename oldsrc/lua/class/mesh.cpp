#include "mesh.h"

#include "../lua_state.h"

meshGeometry mesh_load_geometry(const char *path) {
	printf("[mesh.cpp] loading mesh from %s\n", path);
	meshGeometry geometry;
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		printf("[mesh.cpp] ERROR: Failed to load mesh: %s\n", importer.GetErrorString());
		return geometry;
	}

	if (scene->mNumMeshes == 0) {
		printf("[mesh.cpp] ERROR: No meshes found in file: %s\n", path);
		return geometry;
	}

	aiNode *root_node = scene->mRootNode;

	if (root_node->mNumMeshes > 1) {
		printf("[mesh.cpp] WARNING: Multiple meshes found, using first mesh only\n");
	}

	aiMesh *mesh = scene->mMeshes[0];

	return geometry;
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
