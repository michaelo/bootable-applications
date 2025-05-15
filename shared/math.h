#ifndef MATH_H
#define MATH_H

#define M_PI 3.14159265358979323846264338327950288
#define M_PI_2 1.57079632679489661923
#define M_PI_M_2 6.283185307179586476925286766559

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define CLAMP(x,low,high) (((x)>(high))?(high):(((x)<(low))?(low):(x)))

int _fltused = 0;

static void useFloatingPointMath()
{
    *((int volatile *)&_fltused)=0; //prevent LTO from removing the marker symbol _fltused
}

static float degToRad(float deg)
{
    return deg / (180.0 / M_PI);
}

static float radToDeg(float rad)
{
    return rad * (180.0 / M_PI);
}

static float cos(float x)
{
    asm("flds %0;"
        "fcos;"
        "fstps %0"
        : "+m"(x)
    );
    return x;
}

static float sin(float x)
{
    asm("flds %0;"
        "fsin;"
        "fstps %0"
        : "+m"(x)
    );
    return x;
}

static float tan(float x)
{
    return (sin(x) / cos(x));
}

static float sqrt(float x)
{
    asm("flds %0;"
        "fsqrt;"
        "fstps %0"
        : "+m"(x)
    );
    return x;
}

static float fmod(float x, float y)
{
    if (y == 0.0f) {
        return 0.0f; // Handle division by zero gracefully
    }
    return x - (int)(x / y) * y;
}

static int abs(int v)
{
    return v > 0 ? v : -v;
}

static float round(float x)
{
    if (x < 0.0f)
        return (int)(x - 0.5f);
    else
        return (int)(x + 0.5f);
}


#endif