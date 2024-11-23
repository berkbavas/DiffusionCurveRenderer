#version 450 core

layout(location = 0) in float point;

out float gsPoint;

void main()
{
    gsPoint = point;
}
