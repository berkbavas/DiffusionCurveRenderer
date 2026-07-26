#version 450 core

uniform int uUseGlobalColor;
uniform vec4 uGlobalColor;

in vec4 fsColor;

out vec4 oColor;

void main()
{
    if (uUseGlobalColor == 1)
    {
        oColor = uGlobalColor;
    }
    else
    {
        oColor = fsColor;
    }
}
