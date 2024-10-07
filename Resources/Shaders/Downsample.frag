#version 450 core

uniform sampler2D colorTexture;
uniform sampler2D blurTexture;

in vec2 fs_TextureCoords;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outBlur;

void main()
{
    float uStep = 1.0f / textureSize(colorTexture, 0).x;
    float vStep = 1.0f / textureSize(colorTexture, 0).y;

    vec2 nw = vec2(fs_TextureCoords.s - uStep, fs_TextureCoords.t + vStep);
    vec2 n = vec2(fs_TextureCoords.s, fs_TextureCoords.t + vStep);
    vec2 ne = vec2(fs_TextureCoords.s + uStep, fs_TextureCoords.t + vStep);

    vec2 w = vec2(fs_TextureCoords.s - uStep, fs_TextureCoords.t);
    vec2 c = vec2(fs_TextureCoords.s, fs_TextureCoords.t);
    vec2 e = vec2(fs_TextureCoords.s + uStep, fs_TextureCoords.t);

    vec2 sw = vec2(fs_TextureCoords.s - uStep, fs_TextureCoords.t - vStep);
    vec2 s = vec2(fs_TextureCoords.s, fs_TextureCoords.t - vStep);
    vec2 se = vec2(fs_TextureCoords.s + uStep, fs_TextureCoords.t - vStep);

    vec2 vectors[9];
    vectors[0] = nw;
    vectors[1] = n;
    vectors[2] = ne;
    vectors[3] = w;
    vectors[4] = c;
    vectors[5] = e;
    vectors[6] = sw;
    vectors[7] = s;
    vectors[8] = se;

    float weights[9];
    weights[0] = 1; // nw
    weights[1] = 2; // n
    weights[2] = 1; // ne
    weights[3] = 2; // w
    weights[4] = 4; // c
    weights[5] = 2; // e
    weights[6] = 1; // sw
    weights[7] = 2; // s
    weights[8] = 1; // se

    // Colors
    {
        vec4 colors[9];

        for (int i = 0; i < 9; i++)
            colors[i] = texture(colorTexture, vectors[i]);

        float colorTotalWeight = 0;
        vec4 color = vec4(0);
        for (int i = 0; i < 9; i++)
        {
            if (colors[i].a > 0.1f)
            {
                color += weights[i] * colors[i];
                colorTotalWeight += weights[i];
            }
        }

        if (colorTotalWeight > 0)
            outColor = color / colorTotalWeight;
        else
            outColor = vec4(0, 0, 0, 0);
    }

    // Blur
    {
        vec4 blurs[9];

        for (int i = 0; i < 9; i++)
            blurs[i] = texture(blurTexture, vectors[i]);

        float blurTotalWeight = 0;
        vec4 blur = vec4(0);
        for (int i = 0; i < 9; i++)
        {
            if (blurs[i].a > 0.1f)
            {
                blur += weights[i] * blurs[i];
                blurTotalWeight += weights[i];
            }
        }

        if (blurTotalWeight > 0)
            outBlur = blur / blurTotalWeight;
        else
            outBlur = vec4(0, 0, 0, 0);
    }
}
