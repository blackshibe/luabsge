precision mediump float;

attribute vec3 position;
attribute vec2 tex_coord;

varying vec2 uv;

void main() {
    uv = tex_coord;
    gl_Position = vec4(position, 1.0);
}