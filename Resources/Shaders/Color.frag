#version 450 core

in vec4 fs_Color;

layout(location = 0) out vec4 color;

void main()
{
    color = fs_Color;
}
