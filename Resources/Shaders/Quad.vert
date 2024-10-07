#version 450 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 textureCoords;

out vec2 fs_TextureCoords;

void main()
{
    gl_Position = vec4(position, 0, 1);
    fs_TextureCoords = textureCoords;
}
