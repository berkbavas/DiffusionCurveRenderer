#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 20) out;

uniform mat4 projectionMatrix;
uniform vec4 fillColor;
uniform bool borderEnabled;
uniform float borderWidth;
uniform vec4 borderColor;
uniform float width;
uniform float height;
uniform vec2 topLeft;

out vec4 fs_Color;

in float gs_Tick[];

void main()
{
    vec2 topRight = topLeft + vec2(width, 0);
    vec2 bottomRight = topRight + vec2(0, height);
    vec2 bottomLeft = bottomRight - vec2(width, 0);

    float halfBorderWidth = 0.5 * borderWidth;

    if(!borderEnabled)
        halfBorderWidth = 0;

    vec2 northWest = vec2(-halfBorderWidth, -halfBorderWidth);
    vec2 northEast = vec2(halfBorderWidth, -halfBorderWidth);
    vec2 southWest = vec2(-halfBorderWidth, halfBorderWidth);
    vec2 southEast = vec2(halfBorderWidth, halfBorderWidth);

    {
        gl_Position = projectionMatrix * vec4(bottomLeft + northEast, 0, 1);
        fs_Color = fillColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(bottomRight + northWest, 0, 1);
        fs_Color = fillColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(topLeft + southEast, 0, 1);
        fs_Color = fillColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(topRight + southWest, 0, 1);
        fs_Color = fillColor;
        EmitVertex();

        EndPrimitive();
    }

    if(borderEnabled)
    {
    // Left
    {
        gl_Position = projectionMatrix * vec4(topLeft + northWest, 0, 1);
        fs_Color = borderColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(topLeft + northEast, 0, 1);
        fs_Color = borderColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(bottomLeft + southWest, 0, 1);
        fs_Color = borderColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(bottomLeft + southEast, 0, 1);
        fs_Color = borderColor;
        EmitVertex();

        EndPrimitive();
    }

    // Bottom
    {
        gl_Position = projectionMatrix * vec4(bottomRight + southEast, 0, 1);
        fs_Color = borderColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(bottomRight + northEast, 0, 1);
        fs_Color = borderColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(bottomLeft + southWest, 0, 1);
        fs_Color = borderColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(bottomLeft + northWest, 0, 1);
        fs_Color = borderColor;
        EmitVertex();

        EndPrimitive();
    }

    // Right
    {
        gl_Position = projectionMatrix * vec4(topRight + northWest, 0, 1);
        fs_Color = borderColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(topRight + northEast, 0, 1);
        fs_Color = borderColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(bottomRight + southWest, 0, 1);
        fs_Color = borderColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(bottomRight + southEast, 0, 1);
        fs_Color = borderColor;
        EmitVertex();

        EndPrimitive();
    }

    // Top
    {
        gl_Position = projectionMatrix * vec4(topRight + southEast, 0, 1);
        fs_Color = borderColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(topRight + northEast, 0, 1);
        fs_Color = borderColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(topLeft + southWest, 0, 1);
        fs_Color = borderColor;
        EmitVertex();

        gl_Position = projectionMatrix * vec4(topLeft + northWest, 0, 1);
        fs_Color = borderColor;
        EmitVertex();

        EndPrimitive();
    }
    }

}

