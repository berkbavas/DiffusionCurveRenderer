#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 7) out;

uniform mat4 projectionMatrix;
uniform vec2 point;
uniform float innerRadius;
uniform float outerRadius;
uniform vec4 innerColor;
uniform vec4 outerColor;
uniform float ticksDelta;

out vec4 fs_Color;

in float gs_Tick[];

void main()
{
    float t0 = gs_Tick[0];
    float t1 = t0 + ticksDelta;

    vec2 v1 = vec2(cos(t0), sin(t0));
    vec2 v2 = vec2(cos(t1), sin(t1));

    // Inner triangle (3 vertices)
    {
        gl_Position = projectionMatrix * vec4(point, 0, 1);
        fs_Color = innerColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(point + innerRadius * v1, 0, 1);
        fs_Color = innerColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(point + innerRadius * v2, 0, 1);
        fs_Color = innerColor;
        EmitVertex();
        EndPrimitive();
    }


    // Outer triangles (4 vertices)
    {
        gl_Position = projectionMatrix * vec4(point + innerRadius * v1, 0, 1);
        fs_Color = outerColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(point + outerRadius * v1, 0, 1);
        fs_Color = outerColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(point + innerRadius * v2, 0, 1);
        fs_Color = outerColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(point + outerRadius * v2, 0, 1);
        fs_Color = outerColor;
        EmitVertex();

        EndPrimitive();
    }
}

