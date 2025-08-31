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
int lua_base_print(lua_State *L) {
	int arguments = lua_gettop(L);
	int i;

	for (i = 1; i <= arguments; i++) { /* for each argument */
		size_t l;
		const char *s = luaL_tolstring(L, i, &l); /* convert it to string */
		if (i > 1)								  /* not the first element? */
			lua_writestring("\t", 1);			  /* add a tab before it */
		lua_writestring(s, l);					  /* print it */
		lua_pop(L, 1);							  /* pop result */
	}
	lua_writeline();
	return 0;
}

int lua_warn(lua_State *L) {
	lua_writestring(ANSI_BOLD_YELLOW, 7);
	lua_writestring("[Lua] ", 6);
	lua_base_print(L);
	lua_writestring(ANSI_NC, 5);

	return 0;
}

int lua_print(lua_State *L) {
	lua_writestring(ANSI_BLUE, 7);
	lua_writestring("[Lua] ", 6);
	lua_base_print(L);
	lua_writestring(ANSI_NC, 5);

	return 0;
}

int lua_now(lua_State *L) {
	lua_pushnumber(L, now());
	return 1;
}

int lua_make_userdata(lua_State *L) {
	lua_newuserdata(L, 1);
	return 1;
}

int bsge_lua_instantiate(lua_State *L) {
	lua_newtable(L);
	return 0;
}

BSGEWindow *context_window;
int bsge_lua_init_state(BSGEWindow *window, sol::state &lua) {
	context_window = window;
	lua_State *L = lua.lua_state();

	lua.open_libraries();
	lua.set_function("now", lua_now);
	lua.set_function("print", lua_print);
	lua.set_function("warn", lua_warn);
	lua.set_function("make_userdata", lua_make_userdata);

	printf("[lua.cpp] init classes\n");
	lua_bsge_init_template(lua);
	lua_bsge_init_mesh(lua);
	lua_bsge_init_signal(L);
	// lua_bsge_init_color(L);
	lua_bsge_init_shader(lua);
	lua_bsge_init_textlabel(lua);
	lua_bsge_init_font(lua);
	lua_bsge_init_image(lua);
	lua_bsge_init_camera(lua);
	lua_bsge_init_vfx(lua);
	lua_bsge_init_gizmo(lua);
	lua_bsge_init_framebuffer(lua);
	lua_bsge_init_physics(lua);

	// ECS
	lua_bsge_init_object(lua);

	lua_bsge_init_sphere_tbo(lua);
	lua_bsge_init_mesh_tbo(lua);

	printf("[lua.cpp] init modules\n");
	lua_bsge_init_rendering(lua);
	lua_bsge_init_glm_bindings(lua);
	lua_bsge_init_imgui_bindings(lua);
	lua_bsge_init_opengl_constants(lua);

	// World = {...}
	printf("[lua.cpp] init World\n");
	lua_newtable(L);

	lua_bsge_connect_rendering(window, lua);
	lua_bsge_connect_window(window, lua);
	lua_setglobal(L, "World");
	
	lua_bsge_init_input(lua);
	lua_bsge_init_emscripten(lua);

	printf("[lua.cpp] init Scene\n");
	lua["Scene"] = BSGEObject();

#ifdef USE_EMSCRIPTEN
	lua["PLATFORM"] = "WEB";
#else
	lua["PLATFORM"] = "NATIVE";
#endif

	printf("[lua.cpp] start\n");

	return 0;
}