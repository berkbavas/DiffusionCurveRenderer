#version 450 core

in vec2 fsTextureCoords;

uniform sampler2D uColorSourceTexture;
uniform sampler2D uColorTargetTexture;

layout(location = 0) out vec4 oColor;

void main()
{
    vec4 Color = texture(uColorTargetTexture, fsTextureCoords);

    if (Color.a > 0.1f)
    {
        oColor = Color;
    }
    else
    {
        oColor = texture(uColorSourceTexture, fsTextureCoords);
    }
}
