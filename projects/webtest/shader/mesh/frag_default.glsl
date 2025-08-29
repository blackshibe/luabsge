precision mediump float;

uniform sampler2D img_texture;

varying vec2 tex_coord;

void main() {
    gl_FragColor = texture2D(img_texture, tex_coord);
} 