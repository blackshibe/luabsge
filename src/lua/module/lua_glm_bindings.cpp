#include <lua.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// glm has hopeless amounts of overloads and i cannot hope to understand this library at all

int lua_bsge_glm_mat4(lua_State* L) {
	float size = lua_tonumber(L, 1);

	glm::mat4* pointer = (glm::mat4*)lua_newuserdata(L, sizeof(glm::mat4));
	*pointer = glm::mat4(size);

	luaL_getmetatable(L, "Glm");
	lua_setmetatable(L, -2);

    return 1;
}

int lua_bsge_glm_vec3(lua_State* L) {
	float x = lua_tonumber(L, 1);
	float y = lua_tonumber(L, 2);
	float z = lua_tonumber(L, 3);

	glm::vec3* pointer = (glm::vec3*)lua_newuserdata(L, sizeof(glm::vec3));
	*pointer = glm::vec3(x, y, z);

	luaL_getmetatable(L, "Glm");
	lua_setmetatable(L, -2);

    return 1;
}

int lua_bsge_glm_mat4_translate_vec3(lua_State* L) {
    // printf("[lua_glm_bindings.cpp] mat4_translate_vec3\n");

	glm::mat4* arg1 = (glm::mat4*)lua_touserdata(L, 1);
	glm::vec3* arg2 = (glm::vec3*)lua_touserdata(L, 2);
	glm::mat4* pointer = (glm::mat4*)lua_newuserdata(L, sizeof(glm::mat4));
	*pointer = glm::translate(*arg1, *arg2);

	// psychological torture
	// printf("mat4: %i\n", arg1->length()); 
	// printf("vec3: %i\n", arg2->length()); 
	// printf("mat4 props: %f %f %f %f\n", arg1[0], arg1[1], arg1[2], arg1[3]); 
	// printf("vec3 props: %f %f %f\n", arg2->x, arg2->y, arg2->z); 

	luaL_getmetatable(L, "Glm");
	lua_setmetatable(L, -2);

    return 1;
}

static const luaL_Reg bsge_glm_methods[] = {
	{"mat4", lua_bsge_glm_mat4},
	{"vec3", lua_bsge_glm_vec3},
	{"mat4_translate_vec3", lua_bsge_glm_mat4_translate_vec3},

	{NULL, NULL},
};

int lua_bsge_init_glm_bindings(lua_State* L) {

	// {__index = { ...bsge_lua_vector_methods }}
	// global metatable
	luaL_newmetatable(L, "Glm");


	lua_newtable(L);
	luaL_setfuncs(L, bsge_glm_methods, 0);
	lua_setfield(L, -2, "__index");

	// the global that lets you create the class in the first place
	lua_newtable(L);
	luaL_setfuncs(L, bsge_glm_methods, 0);
	lua_setglobal(L, "glm");

	return 0;
}