#version 430 core
layout (location = 0) in vec2 vs_Position;
layout (location = 1) in vec2 vs_TextureCoords;

out vec2 fs_TextureCoords;

void main()
{
    gl_Position = vec4(vs_Position, 0.0, 1.0);
    fs_TextureCoords = vs_TextureCoords;
}
