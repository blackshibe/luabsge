precision mediump float;

attribute vec3 vert_pos;
attribute vec3 vert_normal;
attribute vec2 vert_tex_coord;

// output a color to the fragment shader
varying vec2 tex_coord;

uniform mat4 camera_transform;
uniform mat4 projection;
uniform mat4 transform;

void main() {
    gl_Position = projection * camera_transform * transform * vec4(vert_pos, 1.0);
    
    tex_coord = vert_tex_coord;
}