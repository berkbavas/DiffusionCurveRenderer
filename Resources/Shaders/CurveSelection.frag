#version 450 core

uniform int uCurveIndex;
uniform int uCurveType;

layout(location = 0) out ivec4 oCurveInfo;

void main()
{
    oCurveInfo = ivec4(uCurveIndex, uCurveType, 0, 1);
}
