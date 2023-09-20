#include "lua.h"

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

int bsge_lua_instantiate(lua_State* L) {
	lua_newtable(L);
	return 0;
}

BSGEWindow* context_window;
int bsge_lua_init_state(BSGEWindow* _window, lua_State* L) {
	context_window = _window;

	luaL_openlibs(L);
	lua_register(L, "now", lua_now);
	lua_register(L, "print", lua_print);
	lua_register(L, "warn", lua_warn);
	lua_register(L, "make_userdata", lua_make_userdata);

	printf("[lua.cpp] init classes\n");
	lua_bsge_init_template(L);
	lua_bsge_init_mesh(L);
	lua_bsge_init_signal(L);
	lua_bsge_init_vector3(L);
	lua_bsge_init_vector2(L);
	lua_bsge_init_color(L);
	lua_bsge_init_textlabel(L);
	lua_bsge_init_font(L);
	lua_bsge_init_image(L);
	lua_bsge_init_camera(L);

	printf("[lua.cpp] init modules\n");
	lua_bsge_init_rendering(L);
	lua_bsge_init_window(L);
	lua_bsge_init_glm_bindings(L);

	// World = {...}
	printf("[lua.cpp] init World\n");
	lua_newtable(L);

	lua_bsge_connect_rendering(*_window, L);
	lua_bsge_connect_window(*_window, L);
	// lua_bsge_connect_ui(*_window, L);
	// lua_bsge_connect_meshloader(*_window, L);

	lua_setglobal(L, "World");

	printf("[lua.cpp] start\n");

	return 0;
}