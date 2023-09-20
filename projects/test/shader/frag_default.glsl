// LuaBSGE - Default fragment shader for 3D meshes.

#version 330 core
out vec4 FragColor;

in vec2 tex_coord;

uniform float time;
uniform sampler2D img_texture;

void main()
{
    // FragColor = vec4(color.rgb, 1.0f); //  + vec4(time, 0.0f, 0.0f, 0.0f);
    FragColor = texture(img_texture, tex_coord);
} 