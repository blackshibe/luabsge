#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 vertex_color;

uniform mat4 projection;
uniform mat4 transform;
uniform mat4 camera_position;

void main() {
    gl_Position = projection * transform * vec4(position, 1.0);
    vertex_color = color;
}