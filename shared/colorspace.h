#ifndef COLORSPACE_H
#define COLORSPACE_H

#include "shared/color.h"

//Based on https://stackoverflow.com/a/14733008/2087791

typedef struct Color_HSVA {
    unsigned char h;
    unsigned char s;
    unsigned char v;
    unsigned char a;
} Color_HSVA;

Color_BGRA HsvToRgb(Color_HSVA hsva)
{
    Color_BGRA bgra;
    unsigned char region, remainder, p, q, t;
    
    if (hsva.s == 0)
    {
        bgra.Red = hsva.v;
        bgra.Green = hsva.v;
        bgra.Blue = hsva.v;
        bgra.Reserved = hsva.a;
        return bgra;
    }
    
    region = hsva.h / 43;
    remainder = (hsva.h - (region * 43)) * 6; 
    
    p = (hsva.v * (255 - hsva.s)) >> 8;
    q = (hsva.v * (255 - ((hsva.s * remainder) >> 8))) >> 8;
    t = (hsva.v * (255 - ((hsva.s * (255 - remainder)) >> 8))) >> 8;
    
    switch (region)
    {
        case 0:
            bgra.Red = hsva.v; bgra.Green = t; bgra.Blue = p;
            break;
        case 1:
            bgra.Red = q; bgra.Green = hsva.v; bgra.Blue = p;
            break;
        case 2:
            bgra.Red = p; bgra.Green = hsva.v; bgra.Blue = t;
            break;
        case 3:
            bgra.Red = p; bgra.Green = q; bgra.Blue = hsva.v;
            break;
        case 4:
            bgra.Red = t; bgra.Green = p; bgra.Blue = hsva.v;
            break;
        default:
            bgra.Red = hsva.v; bgra.Green = p; bgra.Blue = q;
            break;
    }
    
    return bgra;
}

Color_HSVA RgbToHsv(Color_BGRA bgra)
{
    Color_HSVA hsva;
    hsva.a = bgra.Reserved;

    unsigned char rgbMin, rgbMax;

    rgbMin = bgra.Red < bgra.Green ? (bgra.Red < bgra.Blue ? bgra.Red : bgra.Blue) : (bgra.Green < bgra.Blue ? bgra.Green : bgra.Blue);
    rgbMax = bgra.Red > bgra.Green ? (bgra.Red > bgra.Blue ? bgra.Red : bgra.Blue) : (bgra.Green > bgra.Blue ? bgra.Green : bgra.Blue);
    
    hsva.v = rgbMax;
    if (hsva.v == 0)
    {
        hsva.h = 0;
        hsva.s = 0;
        return hsva;
    }

    hsva.s = 255 * (long)(rgbMax - rgbMin) / hsva.v;
    if (hsva.s == 0)
    {
        hsva.h = 0;
        return hsva;
    }

    if (rgbMax == bgra.Red)
        hsva.h = 0 + 43 * (bgra.Green - bgra.Blue) / (rgbMax - rgbMin);
    else if (rgbMax == bgra.Green)
        hsva.h = 85 + 43 * (bgra.Blue - bgra.Red) / (rgbMax - rgbMin);
    else
        hsva.h = 171 + 43 * (bgra.Red - bgra.Green) / (rgbMax - rgbMin);

    return hsva;
}

#endif
