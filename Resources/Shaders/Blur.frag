#version 450 core

in vec2 fs_TextureCoords;

uniform sampler2D colorTexture;
uniform sampler2D blurTexture;

out vec4 outColor;

void main()
{
    vec2 coords = fs_TextureCoords;
    vec4 thisColor = texture(colorTexture, coords);
    float blurStrength = texture(blurTexture, coords).r;

    if (blurStrength < 0.001f)
        outColor = thisColor;
    else
    {
        // Blur
        float uStep = 1.0f / textureSize(colorTexture, 0).x;
        float vStep = 1.0f / textureSize(colorTexture, 0).y;

        // nw n ne
        // w  c e
        // sw s se

        vec2 nw = vec2(coords.s - uStep, coords.t + vStep);
        vec2 n = vec2(coords.s, coords.t + vStep);
        vec2 ne = vec2(coords.s + uStep, coords.t + vStep);

        vec2 w = vec2(coords.s - uStep, coords.t);
        vec2 c = vec2(coords.s, coords.t);
        vec2 e = vec2(coords.s + uStep, coords.t);

        vec2 sw = vec2(coords.s - uStep, coords.t - vStep);
        vec2 s = vec2(coords.s, coords.t - vStep);
        vec2 se = vec2(coords.s + uStep, coords.t - vStep);

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
        weights[0] = 1 / 16.0f; // nw
        weights[1] = 2 / 16.0f; // n
        weights[2] = 1 / 16.0f; // ne
        weights[3] = 2 / 16.0f; // w
        weights[4] = 4 / 16.0f; // c
        weights[5] = 2 / 16.0f; // e
        weights[6] = 1 / 16.0f; // sw
        weights[7] = 2 / 16.0f; // s
        weights[8] = 1 / 16.0f; // se

        vec4 totalColor = vec4(0);

        for (int i = 0; i < 9; i++)
            totalColor += weights[i] * texture(colorTexture, vectors[i]);

        outColor = thisColor * (1 - blurStrength) + blurStrength * totalColor;
    }
}
