#version 430 core

in vec2 fs_TextureCoords;

uniform sampler2D sourceTexture;
uniform sampler2D targetTexture;

out vec4 out_Color;

void main()
{
    vec4 color = texture(targetTexture, fs_TextureCoords);

    if(color.a > 0.1)
    {
        out_Color = color;
    }
    else
    {
        out_Color = texture(sourceTexture, fs_TextureCoords);
    }
}
