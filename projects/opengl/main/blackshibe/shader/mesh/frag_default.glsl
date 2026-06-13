precision mediump float;

uniform sampler2D img_texture;
uniform vec4 img_color;

varying vec2 tex_coord;

void main() {
    gl_FragColor = texture2D(img_texture, tex_coord) * img_color;
} 