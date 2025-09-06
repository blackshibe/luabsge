#include "vfx.h"
#include "image.h"
#include <glm/gtc/type_ptr.hpp>

void lua_bsge_init_vfx(sol::state &lua) {
    lua.new_usertype<VFXEffectStruct>("VFXEffect",
        sol::constructors<VFXEffectStruct()>(),
        "load_shader", &VFXEffectStruct::load_shader,
        "load_fragment_shader", &VFXEffectStruct::load_fragment_shader,
        "render", &VFXEffectStruct::render,
        "set_uniform_float", &VFXEffectStruct::set_uniform_float,
        "set_uniform_vec2", &VFXEffectStruct::set_uniform_vec2,
        "set_uniform_vec3", &VFXEffectStruct::set_uniform_vec3,
        "set_uniform_vec4", &VFXEffectStruct::set_uniform_vec4,
        "set_uniform_int", &VFXEffectStruct::set_uniform_int,
        "set_uniform_texture", &VFXEffectStruct::set_uniform_texture,
        "set_uniform_mat4", &VFXEffectStruct::set_uniform_mat4,
        "bind", &VFXEffectStruct::bind,
        "unbind", &VFXEffectStruct::unbind,
        "is_valid", &VFXEffectStruct::is_valid
    );
}