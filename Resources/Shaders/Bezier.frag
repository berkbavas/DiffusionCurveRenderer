#version 450 core

in vec4 fs_Color;

uniform int useGlobalColor;
uniform vec4 globalColor;

out vec4 out_Color;

void main()
{
    if (useGlobalColor == 1)
    {
        out_Color = globalColor;
    }
    else
    {
        out_Color = fs_Color;
    }
}
