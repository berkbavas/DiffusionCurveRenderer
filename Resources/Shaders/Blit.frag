#version 450 core

uniform sampler2D sourceTexture;

in vec2 fsTextureCoords;

out vec4 outColor;

void main()
{
    outColor = texture(sourceTexture, fsTextureCoords);
}
