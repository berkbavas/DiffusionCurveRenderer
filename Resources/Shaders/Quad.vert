#version 450 core

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTextureCoords;

out vec2 fsTextureCoords;

void main()
{
    gl_Position = vec4(aPosition, 0, 1);
    fsTextureCoords = aTextureCoords;
}
