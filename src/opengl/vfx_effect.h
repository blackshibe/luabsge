#pragma once

#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include <string>
#include <unordered_map>
#include <chrono>
#include "../lua/config.h"

struct VFXEffectStruct {
    GLuint shader_program;
    GLuint quad_vao;
    GLuint quad_vbo;
    
    // Cached uniform locations for performance
    std::unordered_map<std::string, GLint> uniform_locations;
    
    // Effect properties
    bool is_valid;
    std::string name;
    
    VFXEffectStruct();
    ~VFXEffectStruct();
    
    // Load shader from files using existing shader.cpp functions
    bool load_shader(sol::state &lua, const char* vertex_path, const char* fragment_path);
    bool load_fragment_shader(const char* fragment_path);
    
    // Render fullscreen quad
    void render();
    
    // Uniform setters with caching
    void set_uniform_float(const char* name, float value);
    void set_uniform_vec2(const char* name, float x, float y);
    void set_uniform_vec3(const char* name, float x, float y, float z);
    void set_uniform_vec4(const char* name, float x, float y, float z, float w);
    void set_uniform_int(const char* name, int value);
    void set_uniform_texture(const char* name, GLuint texture_id, int texture_unit = 0);
    void set_uniform_mat4(const char* name, const float* matrix);
    
    // Time and resolution uniforms (common for VFX)
    void set_time(float time);
    
    // Utility functions
    void bind();
    void unbind();
    GLint get_uniform_location(const char* name);
    
private:
    void create_fullscreen_quad();
    void cleanup();
    
    // Default fullscreen vertex shader source
    static const char* default_vertex_shader;
};