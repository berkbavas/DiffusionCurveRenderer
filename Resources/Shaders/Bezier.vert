#version 450 core

layout(location = 0) in float aPoint;

out float gsPoint;

void main()
{
    gsPoint = aPoint;
}
