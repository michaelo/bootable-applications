#define M_PI 3.14159265358979323846264338327950288
#define M_PI_2 1.57079632679489661923
#define M_PI_M_2 6.283185307179586476925286766559

int _fltused = 0;

double cos(double x)
{
    asm("fldl %0;"
        "fcos;"
        "fstpl %0"
        : "+m"(x)
    );
    return x;
}

double sin(double x)
{
    asm("fldl %0;"
        "fsin;"
        "fstpl %0"
        : "+m"(x)
    );
    return x;
}

double tan(double x)
{
    return (sin(x) / cos(x));
}

double sqrt(double x)
{
    asm("fldl %0;"
        "fsqrt;"
        "fstpl %0"
        : "+m"(x)
    );
    return x;
}
