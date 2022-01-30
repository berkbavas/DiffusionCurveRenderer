#version 330 core

out vec4 out_Color;
in vec4 fs_Color;

void main()
{
    out_Color = fs_Color;
}
