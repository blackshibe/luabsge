#include "vfx_effect.h"
#include "shader.h"
#include <stdio.h>
#include <cstring>

using namespace std::chrono;

// duplicate of now() that i cba to #include
int64_t start = duration_cast<milliseconds>(
					 system_clock::now().time_since_epoch())
					 .count();

int64_t get_time() {
	int64_t ms = duration_cast<milliseconds>(
					 system_clock::now().time_since_epoch())
					 .count();
	return ms - start;
}

const char* VFXEffectStruct::default_vertex_shader = R"(
#version 330 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec2 tex_coord;

out vec2 uv;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    uv = tex_coord;
}
)";

VFXEffectStruct::VFXEffectStruct() 
    : shader_program(0), quad_vao(0), quad_vbo(0), is_valid(false) {
    create_fullscreen_quad();
}

VFXEffectStruct::~VFXEffectStruct() {
    cleanup();
}

void VFXEffectStruct::create_fullscreen_quad() {
    // Fullscreen quad vertices with texture coordinates
    float quad_vertices[] = {
        // positions   // tex coords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f
    };

    glGenVertexArrays(1, &quad_vao);
    glGenBuffers(1, &quad_vbo);
    
    glBindVertexArray(quad_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    // Texture coordinate attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glBindVertexArray(0);
}

bool VFXEffectStruct::load_shader(sol::state &lua, const char* vertex_path, const char* fragment_path) {
    cleanup();
    
    bool success = bsge_compile_shader(lua, &shader_program, vertex_path, fragment_path);
    if (!success) {
        printf("[VFXEffect] Failed to compile shader from %s and %s\n", vertex_path, fragment_path);
        is_valid = false;
        return false;
    }
    
    uniform_locations.clear();
    is_valid = true;
    return true;
}

bool VFXEffectStruct::load_fragment_shader(const char* fragment_path) {
    cleanup();
    
    const char* fragment_source = load_file(fragment_path);
    if (!fragment_source) {
        printf("[VFXEffect] Failed to load fragment shader: %s\n", fragment_path);
        is_valid = false;
        return false;
    }
    
    GLuint vertex_shader, fragment_shader;
    if (!compile_shader(&vertex_shader, GL_VERTEX_SHADER, default_vertex_shader)) {
        printf("[VFXEffect] Failed to compile default vertex shader\n");
        free((void*)fragment_source);
        is_valid = false;
        return false;
    }
    
    if (!compile_shader(&fragment_shader, GL_FRAGMENT_SHADER, fragment_source)) {
        printf("[VFXEffect] Failed to compile fragment shader: %s\n", fragment_path);
        free((void*)fragment_source);
        glDeleteShader(vertex_shader);
        is_valid = false;
        return false;
    }
    
    free((void*)fragment_source);
    
    // Link shader program
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    
    // Check for linking errors
    int success;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(shader_program, 512, NULL, info_log);
        printf("[VFXEffect] Shader program linking failed:\n%s\n", info_log);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(shader_program);
        shader_program = 0;
        is_valid = false;
        return false;
    }
    
    // Clean up individual shaders
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    uniform_locations.clear();
    is_valid = true;
    return true;
}

void VFXEffectStruct::render() {
    if (!is_valid) {
        printf("[VFXEffect] Warning: Attempting to render invalid effect\n");
        return;
    }

    set_time(get_time());
    bind();

    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void VFXEffectStruct::bind() {
    if (is_valid) {
        glUseProgram(shader_program);
    }
}

void VFXEffectStruct::unbind() {
    glUseProgram(0);
}

GLint VFXEffectStruct::get_uniform_location(const char* name) {
    if (!is_valid) return -1;
    
    std::string name_str(name);
    auto it = uniform_locations.find(name_str);
    
    if (it != uniform_locations.end()) {
        return it->second;
    }
    
    // Cache the location
    GLint location = glGetUniformLocation(shader_program, name);
    uniform_locations[name_str] = location;
    
    return location;
}

void VFXEffectStruct::set_uniform_float(const char* name, float value) {
    if (!is_valid) return;
    
    GLint location = get_uniform_location(name);
    if (location != -1) {
        bind();
        glUniform1f(location, value);
    }
}

void VFXEffectStruct::set_uniform_vec2(const char* name, float x, float y) {
    if (!is_valid) return;
    
    GLint location = get_uniform_location(name);
    if (location != -1) {
        bind();
        glUniform2f(location, x, y);
    }
}

void VFXEffectStruct::set_uniform_vec3(const char* name, float x, float y, float z) {
    if (!is_valid) return;
    
    GLint location = get_uniform_location(name);
    if (location != -1) {
        bind();
        glUniform3f(location, x, y, z);
    }
}

void VFXEffectStruct::set_uniform_vec4(const char* name, float x, float y, float z, float w) {
    if (!is_valid) return;
    
    GLint location = get_uniform_location(name);
    if (location != -1) {
        bind();
        glUniform4f(location, x, y, z, w);
    }
}

void VFXEffectStruct::set_uniform_int(const char* name, int value) {
    if (!is_valid) return;
    
    GLint location = get_uniform_location(name);
    if (location != -1) {
        bind();
        glUniform1i(location, value);
    }
}

void VFXEffectStruct::set_uniform_texture(const char* name, GLuint texture_id, int texture_unit) {
    if (!is_valid) return;
    
    GLint location = get_uniform_location(name);
    if (location != -1) {
        bind();
        glActiveTexture(GL_TEXTURE0 + texture_unit);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glUniform1i(location, texture_unit);
    }
}

void VFXEffectStruct::set_uniform_mat4(const char* name, const float* matrix) {
    if (!is_valid) return;
    
    GLint location = get_uniform_location(name);
    if (location != -1) {
        bind();
        glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
    }
}

void VFXEffectStruct::set_time(float time) {
    set_uniform_float("time", time);
}

void VFXEffectStruct::cleanup() {
    if (shader_program != 0) {
        glDeleteProgram(shader_program);
        shader_program = 0;
    }
    uniform_locations.clear();
    is_valid = false;
}