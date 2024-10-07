#version 450 core

in vec2 fs_TextureCoords;

uniform sampler2D colorSourceTexture;
uniform sampler2D colorTargetTexture;

uniform sampler2D blurSourceTexture;
uniform sampler2D blurTargetTexture;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outBlur;

void main()
{
    // Color
    vec4 color = texture(colorTargetTexture, fs_TextureCoords);

    if (color.a > 0.1f)
        outColor = color;
    else
        outColor = texture(colorSourceTexture, fs_TextureCoords);

    // Blur
    vec4 blur = texture(blurTargetTexture, fs_TextureCoords);

    if (blur.a > 0.1f)
        outBlur = blur;
    else
        outBlur = texture(blurSourceTexture, fs_TextureCoords);
}
