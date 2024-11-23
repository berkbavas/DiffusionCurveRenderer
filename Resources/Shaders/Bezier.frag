#version 450 core

in vec4 fsColor;

uniform int useGlobalColor;
uniform vec4 globalColor;

out vec4 outColor;

void main()
{
    if (useGlobalColor == 1)
    {
        outColor = globalColor;
    }
    else
    {
        outColor = fsColor;
    }
}
