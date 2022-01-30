#version 430 core

in vec2 fs_TextureCoords;
uniform sampler2D sourceTexture;
uniform float pixelRatio;

out vec4 out_Color;

void main()
{
    vec2 coords = vec2(fs_TextureCoords.s / pixelRatio, 1 - (1 - fs_TextureCoords.t) / pixelRatio);
    out_Color = texture(sourceTexture, coords );
}
