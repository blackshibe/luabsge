// LuaBSGE - Default fragment shader for text.

#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main() {   
	if (texture(text, TexCoords).r == 0) discard;
    color = vec4(textColor.rgb, texture(text, TexCoords).r);
}