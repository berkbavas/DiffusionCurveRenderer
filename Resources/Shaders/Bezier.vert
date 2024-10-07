#version 450 core

layout(location = 0) in float point;

out float gs_Point;

void main()
{
    gs_Point = point;
}
