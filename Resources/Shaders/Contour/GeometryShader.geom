#version 430 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 projectionMatrix;
uniform float thickness;
uniform vec4 color;
uniform float ticksDelta;
uniform int controlPointsCount;

in float gs_Tick[];
out vec4 fs_Color;

layout(std430, binding = 1) buffer ControlPointsBuffer
{
    vec2 controlPoints[];
};


float customPow(float x, float y)
{
    if(x == 0 && y == 0)
        return 1;

    return pow(x, y);
}

float factorial(int n)
{
    float result = 1;

    for(int i = 2; i <= n; ++i)
        result *= i;

    return result;
}


float choose(int n, int k)
{
    return factorial(n) / (factorial(k) * factorial(n - k));
}


vec2 valueAt(float t)
{
    vec2 value = vec2(0, 0);
    int degree = controlPointsCount - 1;

    for(int i = 0; i <= degree; ++i)
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
    vec2 tangent = vec2(0,0);
    int degree = controlPointsCount - 1;

    for(int i = 0; i <= degree - 1; i++)
    {
        float c0 = choose(degree - 1, i);
        float c1 = customPow(t, i);
        float c2 = customPow(1 - t, degree - 1 - i);
        tangent += degree * c0 * c1 * c2 * (controlPoints[i+1] - controlPoints[i]);
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

    float t0 = gs_Tick[0];
    float t1 = t0 + ticksDelta;

    vec2 valueAt_t0 = valueAt(t0);
    vec2 valueAt_t1 = valueAt(t1);

    vec2 normalAt_t0 = normalAt(t0);
    vec2 normalAt_t1 = normalAt(t1);

    gl_Position = projectionMatrix * vec4(valueAt_t0 + 0.5f * thickness * normalAt_t0, 0, 1);
    fs_Color = color;
    EmitVertex();

    gl_Position = projectionMatrix * vec4(valueAt_t0 - 0.5f * thickness * normalAt_t0, 0, 1);
    fs_Color = color;
    EmitVertex();

    gl_Position = projectionMatrix * vec4(valueAt_t1 + 0.5f * thickness * normalAt_t1, 0, 1);
    fs_Color = color;
    EmitVertex();

    gl_Position = projectionMatrix * vec4(valueAt_t1 - 0.5f * thickness * normalAt_t1, 0, 1);
    fs_Color = color;
    EmitVertex();

    EndPrimitive();
}

