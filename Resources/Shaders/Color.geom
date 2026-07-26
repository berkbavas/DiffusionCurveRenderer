#version 450 core

layout(points) in;
layout(triangle_strip, max_vertices = 8) out;

uniform mat4 uProjection;
uniform float uDelta;
uniform float uDiffusionWidth;
uniform float uDiffusionGap;

uniform vec2 uControlPoints[32];
uniform int uControlPointsCount;

uniform vec4 uLeftColors[32];
uniform float uLeftColorPositions[32];
uniform int uLeftColorsCount;

uniform vec4 uRightColors[32];
uniform float uRightColorPositions[32];
uniform int uRightColorsCount;

in float gsPoint[];

out vec4 fsColor;

float CustomPow(float X, float Y)
{
    if (abs(X) < 1e-6 && abs(Y) < 1e-6)
    {
        return 1;
    }

    return pow(X, Y);
}

float Factorial(int N)
{
    float Result = 1;

    for (int I = 2; I <= N; ++I)
    {
        Result *= float(I);
    }

    return Result;
}

float Choose(int N, int K)
{
    return Factorial(N) / (Factorial(K) * Factorial(N - K));
}

vec2 ValueAt(float T)
{
    vec2 Value = vec2(0, 0);
    const int Degree = uControlPointsCount - 1;

    for (int I = 0; I <= Degree; ++I)
    {
        float C0 = Choose(Degree, I);
        float C1 = CustomPow(T, I);
        float C2 = CustomPow(1 - T, Degree - I);

        Value += C0 * C1 * C2 * uControlPoints[I];
    }

    return Value;
}

vec2 TangentAt(float T)
{
    vec2 Tangent = vec2(0, 0);
    const int Degree = uControlPointsCount - 1;

    for (int I = 0; I <= Degree - 1; I++)
    {
        float C0 = Choose(Degree - 1, I);
        float C1 = CustomPow(T, I);
        float C2 = CustomPow(1 - T, Degree - 1 - I);
        Tangent += Degree * C0 * C1 * C2 * (uControlPoints[I + 1] - uControlPoints[I]);
    }

    return normalize(Tangent);
}

vec2 NormalAt(float T)
{
    const vec2 Tangent = TangentAt(T);
    return normalize(vec2(-Tangent.y, Tangent.x));
}

vec4 LeftColorAt(float T)
{
    if (uLeftColorsCount == 0)
    {
        return vec4(0);
    }

    for (int i = 1; i < uLeftColorsCount; i++)
    {
        float t0 = uLeftColorPositions[i - 1];
        float t1 = uLeftColorPositions[i];

        if (t0 <= T && T <= t1)
        {
            return mix(uLeftColors[i - 1], uLeftColors[i], (T - t0) / (t1 - t0));
        }
    }

    if (T < uLeftColorPositions[0])
    {
        return uLeftColors[0];
    }

    if (uLeftColorPositions[uLeftColorsCount - 1] < T)
    {
        return uLeftColors[uLeftColorsCount - 1];
    }

    return vec4(0);
}

vec4 RightColorAt(float T)
{
    if (uRightColorsCount == 0)
    {
        return vec4(0);
    }

    for (int i = 1; i < uRightColorsCount; i++)
    {
        float t0 = uRightColorPositions[i - 1];
        float t1 = uRightColorPositions[i];

        if (t0 <= T && T <= t1)
        {
            return mix(uRightColors[i - 1], uRightColors[i], (T - t0) / (t1 - t0));
        }
    }

    if (T < uRightColorPositions[0])
    {
        return uRightColors[0];
    }

    if (uRightColorPositions[uRightColorsCount - 1] < T)
    {
        return uRightColors[uRightColorsCount - 1];
    }

    return vec4(0);
}

void main()
{
    const float T0 = gsPoint[0];
    const float T1 = T0 + uDelta;

    const vec2 V0 = ValueAt(T0);
    const vec2 V1 = ValueAt(T1);

    const vec2 N0 = NormalAt(T0);
    const vec2 N1 = NormalAt(T1);

    const vec4 L0 = LeftColorAt(T0);
    const vec4 L1 = LeftColorAt(T1);

    const vec4 R0 = RightColorAt(T0);
    const vec4 R1 = RightColorAt(T1);

    const float Width = uDiffusionWidth;
    const float Gap = uDiffusionGap;

    // Left side
    {
        gl_Position = uProjection * vec4(V0 - 0.5f * Gap * N0, 0, 1);
        fsColor = L0;
        EmitVertex();

        gl_Position = uProjection * vec4(V0 - 0.5f * Gap * N0 - 1.0f * Width * N0, 0, 1);
        fsColor = L0;
        EmitVertex();

        gl_Position = uProjection * vec4(V1 - 0.5f * Gap * N1, 0, 1);
        fsColor = L1;
        EmitVertex();

        gl_Position = uProjection * vec4(V1 - 0.5f * Gap * N1 - 1.0f * Width * N1, 0, 1);
        fsColor = L1;
        EmitVertex();

        EndPrimitive();
    }

    // Right side
    {
        gl_Position = uProjection * vec4(V0 + 0.5f * Gap * N0, 0, 1);
        fsColor = R0;
        EmitVertex();

        gl_Position = uProjection * vec4(V0 + 0.5f * Gap * N0 + 1.0f * Width * N0, 0, 1);
        fsColor = R0;
        EmitVertex();

        gl_Position = uProjection * vec4(V1 + 0.5f * Gap * N1, 0, 1);
        fsColor = R1;
        EmitVertex();

        gl_Position = uProjection * vec4(V1 + 0.5f * Gap * N1 + 1.0f * Width * N1, 0, 1);
        fsColor = R1;
        EmitVertex();

        EndPrimitive();
    }
}
