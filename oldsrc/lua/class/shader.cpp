#include "shader.h"

struct ShaderStruct {
	GLuint id;
};

void lua_bsge_compile_shader(ShaderStruct *shader, int type, const char *path) {
	printf("[mesh.cpp] loading shader from %s\n", path);

    if (!compile_shader(&shader->id, type, load_file(path))) {
	    printf("[mesh.cpp] failed! %s\n", path);

        return;
    }

}

void lua_bsge_init_shader(sol::state &lua) {
	lua.new_usertype<ShaderStruct>("Shader",
									 "compile", &lua_bsge_compile_shader);
}
