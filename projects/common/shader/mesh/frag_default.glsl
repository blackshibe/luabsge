// LuaBSGE - Default fragment shader for 3D meshes.

#version 330 core
out vec4 FragColor;

in vec2 tex_coord;

uniform float time;
uniform sampler2D img_texture;
uniform vec4 img_color;

void main()
{
    FragColor = texture(img_texture, tex_coord) * img_color;
} 