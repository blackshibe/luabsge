#version 100
precision mediump float;
attribute vec3 position;
attribute vec3 color;

varying vec3 vertex_color;

uniform mat4 projection;
uniform mat4 transform;
uniform mat4 camera_position;

void main() {
    gl_Position = projection * transform * vec4(position, 1.0);
    vertex_color = color;
}