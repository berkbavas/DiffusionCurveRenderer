#version 450 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in float gsPoint[];

uniform mat4 uProjection;
uniform vec4 uColor;
uniform vec2 uControlPoints[32];
uniform float uThickness;
uniform float uDelta;
uniform int uNumberOfControlPoints;

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
    const int Degree = uNumberOfControlPoints - 1;

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
    const int Degree = uNumberOfControlPoints - 1;

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

void main()
{
    const float T0 = gsPoint[0];
    const float T1 = T0 + uDelta;

    const vec2 V0 = ValueAt(T0);
    const vec2 V1 = ValueAt(T1);

    const vec2 N0 = NormalAt(T0);
    const vec2 N1 = NormalAt(T1);

    const float Width = uThickness;

    gl_Position = uProjection * vec4(V0 + 0.5 * Width * N0, 0, 1);
    fsColor = uColor;
    EmitVertex();

    gl_Position = uProjection * vec4(V0 - 0.5 * Width * N0, 0, 1);
    fsColor = uColor;
    EmitVertex();

    gl_Position = uProjection * vec4(V1 + 0.5 * Width * N1, 0, 1);
    fsColor = uColor;
    EmitVertex();

    gl_Position = uProjection * vec4(V1 - 0.5 * Width * N1, 0, 1);
    fsColor = uColor;
    EmitVertex();

    EndPrimitive();
}
