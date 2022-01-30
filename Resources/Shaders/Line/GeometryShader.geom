#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 projectionMatrix;

uniform vec2 startingPoint;
uniform vec2 endPoint;
uniform vec4 color;
uniform int lineStyle;
uniform float thickness;

uniform float rareTicksDelta;
uniform float denseTicksDelta;

uniform float lineLength;
uniform float dashLength;
uniform float gapLength;


out vec4 fs_Color;

in float gs_Tick[];

vec2 tangent = normalize(endPoint - startingPoint);
vec2 normal = vec2(-tangent.y, tangent.x);
float dashAndGapLength = dashLength + gapLength;

vec2 valueAt(float t)
{
    return mix(startingPoint, endPoint, t);
}

void main()
{
    if(lineStyle == 0)
    {
        float t0 = gs_Tick[0];
        float t1 = t0 + rareTicksDelta;

        gl_Position = projectionMatrix * vec4(valueAt(t0) + 0.5 * thickness * normal, 0, 1);
        fs_Color = color;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(valueAt(t0) - 0.5 * thickness * normal, 0, 1);
        fs_Color = color;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(valueAt(t1) + 0.5 * thickness * normal, 0, 1);
        fs_Color = color;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(valueAt(t1) - 0.5 * thickness * normal, 0, 1);
        fs_Color = color;
        EmitVertex();

        EndPrimitive();
    }
    else
    {
        float t0 = gs_Tick[0];

        if(int(t0 * lineLength) % int(dashAndGapLength) > dashLength)
            return;

        float t1 = t0 + denseTicksDelta;

        gl_Position = projectionMatrix * vec4(valueAt(t0) + 0.5 * thickness * normal, 0, 1);
        fs_Color = color;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(valueAt(t0) - 0.5 * thickness * normal, 0, 1);
        fs_Color = color;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(valueAt(t1) + 0.5 * thickness * normal, 0, 1);
        fs_Color = color;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(valueAt(t1) - 0.5 * thickness * normal, 0, 1);
        fs_Color = color;
        EmitVertex();

        EndPrimitive();
    }

}

