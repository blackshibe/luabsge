#version 330 core

in vec2 uv;
out vec4 FragColor;

uniform float time;

void main() {
    FragColor = vec4(uv.x, uv.y, 0.0, 1.0);
}