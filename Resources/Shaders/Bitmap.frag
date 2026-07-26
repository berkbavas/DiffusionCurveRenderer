#version 450 core

uniform sampler2D uSourceTexture;

in vec2 fsTextureCoords;

out vec4 oColor;

void main()
{
    oColor = texture(uSourceTexture, fsTextureCoords);
}
