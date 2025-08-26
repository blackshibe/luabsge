#version 100
precision mediump float;
varying vec2 TexCoords;

uniform sampler2D text;
uniform vec3 textColor;

void main() {   
    gl_FragColor = vec4(textColor.rgb, 1.0);
}