#version 330 core
layout (location = 0) in vec3 vert_pos;
layout (location = 2) in vec2 vert_tex_coord;

// output a color to the fragment shader
out vec3 color; 
out vec2 tex_coord;

uniform mat4 camera_transform;
uniform mat4 projection;
uniform mat4 transform;

void main() {
    gl_Position = projection * camera_transform * transform * vec4(vert_pos, 1.0);
    // color = vert_color;
    tex_coord = vert_tex_coord;
}