#version 430 core

in vec2 fs_TextureCoords;

uniform sampler2D sourceTexture;

out vec4 out_Color;

void main()
{

    float uStep = 1.0f / textureSize(sourceTexture, 0).x;
    float vStep =  1.0f / textureSize(sourceTexture, 0).y;

    // nw n ne
    // w  c e
    // sw s se

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

    vec4 colors[9];

    for(int i = 0; i < 9; i++)
    {
        colors[i] = texture(sourceTexture, vectors[i]);
    }


    float totalWeight = 0;
    vec4 color = vec4(0,0,0,0);

    for(int i = 0; i < 9; i++)
    {
        if(colors[i].a > 0.1)
        {
            color += weights[i] * colors[i];
            totalWeight += weights[i];
        }
    }

    if(totalWeight > 0)
    {
        out_Color = color / totalWeight;
    }
    else
    {
        out_Color = vec4(0,0,0,0);
    }

}
