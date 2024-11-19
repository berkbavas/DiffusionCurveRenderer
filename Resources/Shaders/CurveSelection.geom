#version 450 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in float gs_Point[];

uniform mat4 projection;
uniform vec2 controlPoints[32];
uniform float thickness;
uniform float zoom;
uniform float delta;
uniform int numberOfControlPoints;

float customPow(float x, float y)
{
    if (x == 0 && y == 0)
    {
        return 1;
    }

    return pow(x, y);
}

float factorial(int n)
{
    float result = 1;

    for (int i = 2; i <= n; ++i)
    {
        result *= float(i);
    }

    return result;
}

float choose(int n, int k)
{
    return factorial(n) / (factorial(k) * factorial(n - k));
}

vec2 valueAt(float t)
{
    vec2 value = vec2(0, 0);
    int degree = numberOfControlPoints - 1;

    for (int i = 0; i <= degree; ++i)
    {
        float c0 = choose(degree, i);
        float c1 = customPow(t, i);
        float c2 = customPow(1 - t, degree - i);

        value += c0 * c1 * c2 * controlPoints[i];
    }

    return value;
}

vec2 tangentAt(float t)
{
    vec2 tangent = vec2(0, 0);
    int degree = numberOfControlPoints - 1;

    for (int i = 0; i <= degree - 1; i++)
    {
        float c0 = choose(degree - 1, i);
        float c1 = customPow(t, i);
        float c2 = customPow(1 - t, degree - 1 - i);
        tangent += degree * c0 * c1 * c2 * (controlPoints[i + 1] - controlPoints[i]);
    }

    return normalize(tangent);
}

vec2 normalAt(float t)
{
    vec2 tangent = tangentAt(t);
    return normalize(vec2(-tangent.y, tangent.x));
}

void main()
{
    float t0 = gs_Point[0];
    float t1 = t0 + delta;

    vec2 v0 = valueAt(t0);
    vec2 v1 = valueAt(t1);

    vec2 n0 = normalAt(t0);
    vec2 n1 = normalAt(t1);

    float width = thickness * zoom;

    gl_Position = projection * vec4(v0 + 0.5 * width * n0, 0, 1);
    EmitVertex();

    gl_Position = projection * vec4(v0 - 0.5 * width * n0, 0, 1);
    EmitVertex();

    gl_Position = projection * vec4(v1 + 0.5 * width * n1, 0, 1);
    EmitVertex();

    gl_Position = projection * vec4(v1 - 0.5 * width * n1, 0, 1);
    EmitVertex();

    EndPrimitive();
}
