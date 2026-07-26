#version 450 core

in vec2 fsTextureCoords;

uniform sampler2D uColorConstrainedTexture;
uniform sampler2D uColorTargetTexture;

layout(location = 0) out vec4 oColor;

void main()
{
    float uStep = 1.0f / textureSize(uColorConstrainedTexture, 0).x;
    float vStep = 1.0f / textureSize(uColorConstrainedTexture, 0).y;

    // nw n ne
    // w  c e
    // sw s se

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
    Weights[0] = 1;
    Weights[1] = 2; // n
    Weights[2] = 1;
    Weights[3] = 2; // w
    Weights[4] = 4; // c
    Weights[5] = 2; // e
    Weights[6] = 1;
    Weights[7] = 2; // s
    Weights[8] = 1;

    // Colors
    vec4 Color = texture(uColorConstrainedTexture, fsTextureCoords);

    if (Color.a > 0.1f)
    {
        oColor = Color;
    }
    else
    {
        vec4 Colors[9];

        for (int I = 0; I < 9; I++)
        {
            Colors[I] = texture(uColorTargetTexture, Vectors[I]);
        }

        float TotalWeight = 0;
        vec4 Color = vec4(0, 0, 0, 0);

        for (int I = 0; I < 9; I++)
        {
            if (Colors[I].a > 0)
            {
                Color += Weights[I] * Colors[I];
                TotalWeight += Weights[I];
            }
        }

        if (TotalWeight > 0)
        {
            oColor = Color / TotalWeight;
        }
        else
        {
            oColor = vec4(1, 1, 1, 1);
        }
    }
}
