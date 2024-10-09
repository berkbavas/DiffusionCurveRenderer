#version 450 core

in vec2 fs_TextureCoords;

uniform sampler2D colorSourceTexture;
uniform sampler2D colorTargetTexture;

layout(location = 0) out vec4 outColor;

void main()
{
    // Color
    vec4 color = texture(colorTargetTexture, fs_TextureCoords);

    if (color.a > 0.1f)
        outColor = color;
    else
        outColor = texture(colorSourceTexture, fs_TextureCoords);
}
