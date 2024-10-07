#version 450 core

in vec2 fs_TextureCoords;

uniform sampler2D sourceTexture;

out vec4 out_Color;

void main()
{
    out_Color = texture(sourceTexture, fs_TextureCoords);
}
