#include <iostream>
#include "../glad/glad.h"
#include <GLFW/glfw3.h>

#include "../include/colors.h"

const char* load_file(const char* filename) {
	char* buffer = NULL;
	long length;
	FILE* file = fopen(filename, "r");

	if (file) {
		fseek(file, 0, SEEK_END);
		length = ftell(file) + 1;
		fseek(file, 0, SEEK_SET);
		buffer = (char*)malloc(length);
		if (buffer) fread(buffer, 1, length - 1, file);
		fclose(file);

		buffer[length - 1] = '\0';

	}

	return buffer;
}

bool compile_shader(GLuint* shader, int type, const char* src) {
	if (!src) {
		printf("%s[shader.cpp] no shader source\n%s", ANSI_RED, ANSI_NC);
		return false;
	}

	int success;
	char infolog[512];

	GLuint id = glCreateShader(type);
	glShaderSource(id, 1, &src, 0);
	glCompileShader(id);

	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(id, 512, 0, infolog);
		printf("%s[shader.cpp] failed to compile %s shader:\n%s\n%s",
			ANSI_RED,
			type == GL_VERTEX_SHADER ? "vertex" : "fragment",
			infolog,
			ANSI_NC
		);

		return false;
	}

	*shader = id;

	return true;
}

bool bsge_compile_shader(GLuint* _shader, const char* vert_path, const char* frag_path) {

	GLuint vert_shader;
	GLuint frag_shader;

	if (!compile_shader(&vert_shader, GL_VERTEX_SHADER, load_file(vert_path))) {
		return false;
	}

	if (!compile_shader(&frag_shader, GL_FRAGMENT_SHADER, load_file(frag_path))) {
		return false;
	}

	GLuint shader = glCreateProgram();
	glAttachShader(shader, vert_shader);
	glAttachShader(shader, frag_shader);
	glLinkProgram(shader);

	int success;
	char infolog[512];
	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader, 512, NULL, infolog);
		printf(infolog);
		return false;
	}

	*_shader = shader;

	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);

	return true;
}
