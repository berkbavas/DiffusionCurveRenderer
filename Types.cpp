#include "Types.h"

RenderMode operator+(const RenderMode &lhs, const RenderMode &rhs)
{
    return RenderMode(int(lhs) ^ int(rhs));
}

bool operator&(const RenderMode &lhs, const RenderMode &rhs)
{
    return (int(lhs) & int(rhs));
}
