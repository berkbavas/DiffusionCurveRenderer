#version 450 core

uniform sampler2D uColorTexture;

in vec2 fsTextureCoords;

layout(location = 0) out vec4 oColor;

void main()
{
    float uStep = 1.0f / textureSize(uColorTexture, 0).x;
    float vStep = 1.0f / textureSize(uColorTexture, 0).y;

    vec2 nw = vec2(fsTextureCoords.s - uStep, fsTextureCoords.t + vStep);
    vec2 n = vec2(fsTextureCoords.s, fsTextureCoords.t + vStep);
    vec2 ne = vec2(fsTextureCoords.s + uStep, fsTextureCoords.t + vStep);

    vec2 w = vec2(fsTextureCoords.s - uStep, fsTextureCoords.t);
    vec2 c = vec2(fsTextureCoords.s, fsTextureCoords.t);
    vec2 e = vec2(fsTextureCoords.s + uStep, fsTextureCoords.t);

    vec2 sw = vec2(fsTextureCoords.s - uStep, fsTextureCoords.t - vStep);
    vec2 s = vec2(fsTextureCoords.s, fsTextureCoords.t - vStep);
    vec2 se = vec2(fsTextureCoords.s + uStep, fsTextureCoords.t - vStep);

    vec2 Vectors[9];
    Vectors[0] = nw;
    Vectors[1] = n;
    Vectors[2] = ne;
    Vectors[3] = w;
    Vectors[4] = c;
    Vectors[5] = e;
    Vectors[6] = sw;
    Vectors[7] = s;
    Vectors[8] = se;

    float Weights[9];
    Weights[0] = 1; // nw
    Weights[1] = 2; // n
    Weights[2] = 1; // ne
    Weights[3] = 2; // w
    Weights[4] = 4; // c
    Weights[5] = 2; // e
    Weights[6] = 1; // sw
    Weights[7] = 2; // s
    Weights[8] = 1; // se

    vec4 Colors[9];

    for (int i = 0; i < 9; i++)
    {
        Colors[i] = texture(uColorTexture, Vectors[i]);
    }

    float TotalWeight = 0;
    vec4 Color = vec4(0);
    for (int i = 0; i < 9; i++)
    {
        if (Colors[i].a > 0.1f)
        {
            Color += Weights[i] * Colors[i];
            TotalWeight += Weights[i];
        }
    }

    if (TotalWeight > 0)
    {
        oColor = Color / TotalWeight;
    }
    else
    {
        oColor = vec4(0, 0, 0, 0);
    }
}
