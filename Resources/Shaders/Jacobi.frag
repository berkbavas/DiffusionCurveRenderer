#version 450 core

in vec2 fsTextureCoords;

uniform sampler2D colorConstrainedTexture;
uniform sampler2D colorTargetTexture;

layout(location = 0) out vec4 outColor;

void main()
{
    float uStep = 1.0f / textureSize(colorConstrainedTexture, 0).x;
    float vStep = 1.0f / textureSize(colorConstrainedTexture, 0).y;

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
    weights[0] = 1;
    weights[1] = 2; // n
    weights[2] = 1;
    weights[3] = 2; // w
    weights[4] = 4; // c
    weights[5] = 2; // e
    weights[6] = 1;
    weights[7] = 2; // s
    weights[8] = 1;

    // Colors
    vec4 color = texture(colorConstrainedTexture, fsTextureCoords);

    if (color.a > 0.1f)
    {
        outColor = color;
    }
    else
    {
        vec4 colors[9];

        for (int i = 0; i < 9; i++)
        {
            colors[i] = texture(colorTargetTexture, vectors[i]);
        }


        float totalWeight = 0;
        vec4 color = vec4(0, 0, 0, 0);

        for (int i = 0; i < 9; i++)
        {
            if (colors[i].a > 0)
            {
                color += weights[i] * colors[i];
                totalWeight += weights[i];
            }
        }

        if (totalWeight > 0)
        {
            outColor = color / totalWeight;
        }
        else
        {
            outColor = vec4(1, 1, 1, 1);
        }

    }
}
