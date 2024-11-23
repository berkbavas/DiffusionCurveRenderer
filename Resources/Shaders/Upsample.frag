#version 450 core

in vec2 fsTextureCoords;

uniform sampler2D colorSourceTexture;
uniform sampler2D colorTargetTexture;

layout(location = 0) out vec4 outColor;

void main()
{
    // Color
    vec4 color = texture(colorTargetTexture, fsTextureCoords);

    if (color.a > 0.1f)
    {
        outColor = color;
    }
    else
    {
        outColor = texture(colorSourceTexture, fsTextureCoords);
    }
}
