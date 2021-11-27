#if !defined( MATH_H )
#define MATH_H

#include <math.h>

// #define ALMOST_ZERO_DELTA 0.00001f


// real32 abs(real32 x)
// {
//     return x>=0.0f?x:-x;
// }

// TODO: handle correctly negative range rounding 
internal inline int32
RoundReal32ToInt32(real32 val)
{
    if (val < 0) {
        return (int32) (val - 0.5f);
    } else 
    {
        return (int32) (val + 0.5f);
    }
}

struct v2ui
{
    uint32 x;
    uint32 y;
};

struct v2
{
    real32 x;
    real32 y;
};

inline v2
operator+(v2 a, v2 b)
{
    v2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

inline v2
operator-(v2 a, v2 b)
{

    v2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

inline int32 
operator==(v2 a, v2 b)
{
    real32 result;
    result = ((a.x == b.x) && (a.y == b.y)) ? 1:0;
    return result;
}

inline int32 
operator!=(v2 a, v2 b)
{
    return !(a == b);
}

inline v2
operator*(real32 a, v2 b)
{

    v2 result;
    result.x = a * b.x;
    result.y = a * b.y;
    return result;
}

inline v2
operator*(v2 b, real32 a)
{

    v2 result;
    result.x = a * b.x;
    result.y = a * b.y;
    return result;
}

inline v2 &
operator*=(v2 &b, real32 a)
{
    b = a * b;
    return b;
}


inline v2
operator-(v2 a)
{
    v2 result;
    result.x = -a.x;
    result.y = -a.y;
    return result;
}

inline real32
product(v2 a, v2 b)
{
    return a.x*b.x + a.y*b.y;
}

inline real32
norm(v2 a)
{
    return sqrt(product(a, a));
}

inline void
normalize(v2 * a)
{
    *a = (1 / norm(*a)) * (*a);
}
#endif
