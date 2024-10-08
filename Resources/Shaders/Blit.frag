#version 450 core

uniform sampler2D sourceTexture;

in vec2 fs_TextureCoords;

out vec4 out_Color;

void main()
{
    out_Color = texture(sourceTexture, fs_TextureCoords);
}
