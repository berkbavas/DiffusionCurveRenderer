#version 430 core
layout (location = 0) in float vs_Tick;

out float gs_Tick;

void main()
{
    gs_Tick = vs_Tick;
}
