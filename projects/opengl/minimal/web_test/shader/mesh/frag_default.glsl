precision mediump float;

uniform sampler2D img_texture;

varying vec2 tex_coord;
varying vec4 img_color;

void main() {
    gl_FragColor = texture2D(img_texture, tex_coord) * img_color;
} 