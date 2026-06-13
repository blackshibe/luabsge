#pragma once


#include <GLFW/glfw3.h>
#include <GLFW/glfw3.h>


const char* load_file(const char* filename);
bool compile_shader(GLuint *shader, int type, const char *src);
bool bsge_compile_shader(sol::state &lua, GLuint* _shader, const char* vert_path, const char* frag_path);