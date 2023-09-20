// LuaBSGE - Default fragment shader for text, without sampling.

#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main() {   
    color = vec4(textColor.rgb, 1.0);
}