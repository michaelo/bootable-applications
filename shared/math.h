#ifndef MATH_H
#define MATH_H

#define M_PI 3.14159265358979323846264338327950288
#define M_PI_2 1.57079632679489661923
#define M_PI_M_2 6.283185307179586476925286766559

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define CLAMP(x,low,high) (((x)>(high))?(high):(((x)<(low))?(low):(x)))

int _fltused = 0;

static float degToRad(float deg)
{
    return deg / (180.0 / M_PI);
}

static float radToDeg(float rad)
{
    return rad * (180.0 / M_PI);
}

static double cos(double x)
{
    asm("fldl %0;"
        "fcos;"
        "fstpl %0"
        : "+m"(x)
    );
    return x;
}

static double sin(double x)
{
    asm("fldl %0;"
        "fsin;"
        "fstpl %0"
        : "+m"(x)
    );
    return x;
}

static double tan(double x)
{
    return (sin(x) / cos(x));
}

static double sqrt(double x)
{
    asm("fldl %0;"
        "fsqrt;"
        "fstpl %0"
        : "+m"(x)
    );
    return x;
}

static int abs(int v)
{
    return v > 0 ? v : -v;
}

static double round(double x)
{
    if (x < 0.0)
        return (int)(x - 0.5);
    else
        return (int)(x + 0.5);
}

static float roundf(float x)
{
    if (x < 0.0f)
        return (int)(x - 0.5f);
    else
        return (int)(x + 0.5f);
}


#endif