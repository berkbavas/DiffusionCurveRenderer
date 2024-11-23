#version 450 core

uniform sampler2D colorTexture;

in vec2 fsTextureCoords;

layout(location = 0) out vec4 outColor;

void main()
{
    float uStep = 1.0f / textureSize(colorTexture, 0).x;
    float vStep = 1.0f / textureSize(colorTexture, 0).y;

    vec2 nw = vec2(fsTextureCoords.s - uStep, fsTextureCoords.t + vStep);
    vec2 n = vec2(fsTextureCoords.s, fsTextureCoords.t + vStep);
    vec2 ne = vec2(fsTextureCoords.s + uStep, fsTextureCoords.t + vStep);

    vec2 w = vec2(fsTextureCoords.s - uStep, fsTextureCoords.t);
    vec2 c = vec2(fsTextureCoords.s, fsTextureCoords.t);
    vec2 e = vec2(fsTextureCoords.s + uStep, fsTextureCoords.t);

    vec2 sw = vec2(fsTextureCoords.s - uStep, fsTextureCoords.t - vStep);
    vec2 s = vec2(fsTextureCoords.s, fsTextureCoords.t - vStep);
    vec2 se = vec2(fsTextureCoords.s + uStep, fsTextureCoords.t - vStep);

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

    vec4 colors[9];

    for (int i = 0; i < 9; i++)
    {
        colors[i] = texture(colorTexture, vectors[i]);
    }


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
    {
        outColor = color / colorTotalWeight;
    }
    else
    {
        outColor = vec4(0, 0, 0, 0);
    }

}
