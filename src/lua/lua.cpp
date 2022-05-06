#include "lua.h"

#include <chrono>
#include "../include/colors.h"
#include "module/rendering.h"
#include "module/ui.h"
#include "module/meshloader.h"

#include "class/signal.h"
#include "class/vector3.h"
#include "class/vector2.h"
#include "class/color.h"
#include "class/mesh.h"

using namespace std::chrono;

int64_t now() {
	int64_t ms = duration_cast<milliseconds>(
		system_clock::now().time_since_epoch())
		.count();
	return ms;
}



// adds a prefix to Lua print
// lbaselib.c
int lua_base_print(lua_State* L) {
	int arguments = lua_gettop(L);
	int i;

	for (i = 1; i <= arguments; i++) {  /* for each argument */
		size_t l;
		const char* s = luaL_tolstring(L, i, &l);  /* convert it to string */
		if (i > 1)  /* not the first element? */
			lua_writestring("\t", 1);  /* add a tab before it */
		lua_writestring(s, l);  /* print it */
		lua_pop(L, 1);  /* pop result */
	}
	lua_writeline();
	return 0;
}

int lua_warn(lua_State* L) {
	lua_writestring(ANSI_BOLD_YELLOW, 7);
	lua_writestring("[Lua] ", 6);
	lua_base_print(L);
	lua_writestring(ANSI_NC, 5);

	return 0;
}

int lua_print(lua_State* L) {
	lua_writestring(ANSI_BLUE, 7);
	lua_writestring("[Lua] ", 6);
	lua_base_print(L);
	lua_writestring(ANSI_NC, 5);

	return 0;
}

int lua_now(lua_State* L) {
	lua_pushnumber(L, now());
	return 1;
}

int lua_make_userdata(lua_State* L) {
	lua_newuserdata(L, 1);
	return 1;
}

int lua_render(lua_State* L) {
	// 	glm::vec3(0.0f,  0.0f,  0.0f),
	// glm::vec3(2.0f,  5.0f, -15.0f),
	// glm::vec3(-1.5f, -2.2f, -2.5f),
	// glm::vec3(-3.8f, -2.0f, -12.3f),
	// glm::vec3(2.4f, -0.4f, -3.5f),
	// glm::vec3(-1.7f,  3.0f, -7.5f),
	// glm::vec3(1.3f, -2.0f, -2.5f),
	// glm::vec3(1.5f,  2.0f, -2.5f),
	// glm::vec3(1.5f,  0.2f, -1.5f),
	// glm::vec3(-1.3f,  1.0f, -1.5f)

	meshData* bsgemesh = (meshData*)lua_touserdata(L, -1);

	glBindTexture(GL_TEXTURE_2D, bsgemesh->texture);

	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::rotate(trans, glm::radians((float)glfwGetTime() * 90.0f), glm::vec3(0.5, 1, 0.2));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bsgemesh->ebo);
	glBindVertexArray(bsgemesh->vao);

	glUseProgram(context_window->default_shader);
	glUniformMatrix4fv(glGetUniformLocation(context_window->default_shader, "transform"), 1, GL_FALSE, glm::value_ptr(trans));
	glDrawElements(GL_TRIANGLES, bsgemesh->indices_count, GL_UNSIGNED_INT, 0);

}

BSGEWindow* context_window;
int bsge_lua_init_state(BSGEWindow* _window, lua_State* L) {
	context_window = _window;

	luaL_openlibs(L);
	lua_register(L, "now", lua_now);
	lua_register(L, "print", lua_print);
	lua_register(L, "warn", lua_warn);
	lua_register(L, "make_userdata", lua_make_userdata);
	lua_register(L, "render", lua_render);

	printf("[lua.cpp] init classes\n");
	lua_bsge_init_mesh(L);
	lua_bsge_init_signal(L);
	lua_bsge_init_vector3(L);
	lua_bsge_init_vector2(L);
	lua_bsge_init_color(L);
	lua_bsge_init_font(L);

	printf("[lua.cpp] init modules\n");
	lua_bsge_init_rendering(L);

	// World = {...}
	printf("[lua.cpp] init World\n");
	lua_newtable(L);
	lua_bsge_connect_rendering(*_window, L);
	lua_bsge_connect_ui(*_window, L);
	lua_bsge_connect_meshloader(*_window, L);

	lua_setglobal(L, "World");

	printf("[lua.cpp] start\n");

	return 0;
}