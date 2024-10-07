#version 450 core

in vec4 fs_Color;
in vec4 fs_Blur;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 blur;

void main()
{
    color = fs_Color;
    blur = fs_Blur;
}
