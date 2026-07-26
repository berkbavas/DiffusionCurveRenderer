#version 450 core

in vec2 fsTextureCoords;

uniform int uNumberOfSamples;
uniform sampler2DMS uMultisampleTexture;

out vec4 oColor;

vec4 Multisample(sampler2DMS sampler, ivec2 Coord)
{
    vec4 Color = vec4(0.0);
    int NumberOfColoredTexels = 0;

    for (int I = 0; I < uNumberOfSamples; I++)
    {
        vec4 TexelColor = texelFetch(sampler, Coord, I);

        if (TexelColor.a > 0.1)
        {
            Color += TexelColor;
            NumberOfColoredTexels++;
        }
    }

    Color /= float(NumberOfColoredTexels);

    return Color;
}

void main()
{
    ivec2 TexSize = textureSize(uMultisampleTexture, 0);
    ivec2 TexCoord = ivec2(fsTextureCoords * TexSize);
    oColor = Multisample(uMultisampleTexture, TexCoord);
}
