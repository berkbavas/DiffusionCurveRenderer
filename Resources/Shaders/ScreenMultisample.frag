#version 450 core

in vec2 fsTextureCoords;

uniform int numberOfSamples;
uniform sampler2DMS multisampleTexture;

out vec4 outColor;

vec4 multisample(sampler2DMS sampler, ivec2 coord)
{
    vec4 color = vec4(0.0);
    int numberOfColoredTexels = 0;

    for (int i = 0; i < numberOfSamples; i++)
    {
        vec4 texelColor = texelFetch(sampler, coord, i);

        if (texelColor.a > 0.1)
        {
            color += texelColor;
            numberOfColoredTexels++;
        }
    }

    color /= float(numberOfColoredTexels);

    return color;
}

void main()
{
    ivec2 texSize = textureSize(multisampleTexture);
    ivec2 texCoord = ivec2(fsTextureCoords * texSize);
    outColor = multisample(multisampleTexture, texCoord);
}