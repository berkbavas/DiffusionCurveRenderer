#version 450 core

uniform int curveIndex;
uniform int curveType;

layout(location = 0) out ivec4 curveInfo;

void main()
{
    curveInfo = ivec4(curveIndex, curveType, 0, 1);
}
