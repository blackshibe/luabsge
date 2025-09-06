#include "vfx.h"
#include "image.h"
#include <glm/gtc/type_ptr.hpp>

void lua_bsge_init_vfx(sol::state &lua) {
    auto load_shader = [](sol::state &lua, VFXEffectStruct* effect, const char* vertex_path, const char* fragment_path) {
        return effect->load_shader(lua, vertex_path, fragment_path);
    };
    
    auto load_fragment_shader = [](VFXEffectStruct* effect, const char* fragment_path) {
        return effect->load_fragment_shader(fragment_path);
    };
    
    auto render = [](VFXEffectStruct* effect) {
        effect->render();
    };
    
    auto set_uniform_float = [](VFXEffectStruct* effect, const char* name, float value) {
        effect->set_uniform_float(name, value);
    };
    
    auto set_uniform_vec2 = [](VFXEffectStruct* effect, const char* name, float x, float y) {
        effect->set_uniform_vec2(name, x, y);
    };
    
    auto set_uniform_vec3 = [](VFXEffectStruct* effect, const char* name, float x, float y, float z) {
        effect->set_uniform_vec3(name, x, y, z);
    };
    
    auto set_uniform_vec4 = [](VFXEffectStruct* effect, const char* name, float x, float y, float z, float w) {
        effect->set_uniform_vec4(name, x, y, z, w);
    };
    
    auto set_uniform_int = [](VFXEffectStruct* effect, const char* name, int value) {
        effect->set_uniform_int(name, value);
    };
    
    auto set_uniform_texture = [](VFXEffectStruct* effect, const char* name, bsgeImage* image) {
        if (image && image->id != 0) {
            effect->set_uniform_texture(name, image->id, 0);
        }
    };
    
    auto set_uniform_mat4 = [](VFXEffectStruct* effect, const char* name, const glm::mat4& matrix) {
        effect->set_uniform_mat4(name, glm::value_ptr(matrix));
    };
    
    auto bind = [](VFXEffectStruct* effect) {
        effect->bind();
    };
    
    auto unbind = [](VFXEffectStruct* effect) {
        effect->unbind();
    };
    
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