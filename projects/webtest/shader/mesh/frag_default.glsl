#version 100
precision mediump float;
varying vec2 tex_coord;

uniform float time;
uniform sampler2D img_texture;

void main()
{
    // gl_FragColor = vec4(color.rgb, 1.0); //  + vec4(time, 0.0, 0.0, 0.0);
    gl_FragColor = texture2D(img_texture, tex_coord);
} 