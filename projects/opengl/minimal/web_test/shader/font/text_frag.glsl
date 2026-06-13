#version 100
precision mediump float;
varying vec2 TexCoords;

uniform sampler2D text;
uniform vec3 textColor;

void main() {   
	if (texture2D(text, TexCoords).r == 0.0) discard;
    gl_FragColor = vec4(textColor.rgb, texture2D(text, TexCoords).r);
}