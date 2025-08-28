#include "gizmo.h"
#include <map>
#include <vector>

bool initialized = false;
GLuint shader_program = 0;
unsigned int VAO = 0;
unsigned int VBO = 0;

std::map<float, std::vector<GizmoVertex>> vertices_by_width;
glm::mat4 projection_matrix;
glm::mat4 transform_matrix;

float line_width = 1.0f;
bool depth_test_enabled = true;

void init_gizmo(sol::state &lua) {
    if (!bsge_compile_shader(lua, &shader_program, "shader/gizmo/vertex_default.glsl", "shader/gizmo/frag_default.glsl")) {
        printf("[Gizmo] Failed to compile fragment shader\n");
        glDeleteShader(shader_program);
        return;
    }
  
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindVertexArray(VAO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    // position and color attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), (void *)offsetof(GizmoVertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), (void *)offsetof(GizmoVertex, color));
}

void lua_bsge_gizmo_begin_frame(glm::mat4 camera_projection, glm::mat4 camera_transform) {    
    projection_matrix = camera_projection;
    transform_matrix = camera_transform;
    vertices_by_width.clear();
}

void lua_bsge_gizmo_end_frame() {
    if (vertices_by_width.empty()) return;

    // Set depth test state
    if (depth_test_enabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }

	glUseProgram(shader_program);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "transform"), 1, GL_FALSE, glm::value_ptr(transform_matrix));

    // Render lines grouped by width
    for (auto& [width, vertices] : vertices_by_width) {
        if (vertices.empty()) continue;
        
        glLineWidth(width);
        glBufferData(GL_ARRAY_BUFFER, (GLsizei)(vertices.size() * sizeof(GizmoVertex)), vertices.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINES, 0, (GLsizei)vertices.size());
    }

	glUseProgram(0);
    // Restore default line width
    glLineWidth(1.0f);
}

void draw_line(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color) {
    vertices_by_width[line_width].push_back({start, color});
    vertices_by_width[line_width].push_back({end, color});
}

void draw_grid(float size, int divisions, const glm::vec3& color) {
    float step = size / divisions;
    float half_size = size * 0.5f;
    
    // Draw horizontal lines (along X axis)
    for (int i = 0; i <= divisions; i++) {
        float z = -half_size + i * step;
        draw_line(glm::vec3(-half_size, -0.01f, z), glm::vec3(half_size, -0.01, z), color);
    }
    
    // Draw vertical lines (along Z axis) 
    for (int i = 0; i <= divisions; i++) {
        float x = -half_size + i * step;
        draw_line(glm::vec3(x, -0.01f, -half_size), glm::vec3(x, -0.01, half_size), color);
    }
}

void set_line_width(float width) {
    line_width = width;
}

void set_depth_test(bool enabled) {
    depth_test_enabled = enabled;
}

void lua_bsge_init_gizmo(sol::state &lua) {
    lua_State *L = lua.lua_state();
    
    auto lua_draw_line = [&L](const glm::vec3& start, const glm::vec3& end, const glm::vec3& color) {
        draw_line(start, end, color);
        return 0;
    };

    sol::table gizmo = lua.create_table();
    lua["Gizmo"] = gizmo;

    gizmo["draw_line"] = lua_draw_line;
    gizmo["draw_grid"] = draw_grid;
    gizmo["set_line_width"] = set_line_width;
    gizmo["set_depth_test"] = set_depth_test;

    init_gizmo(lua);
}