/*--
Original Copyright (c) 2000 Microsoft Corporation - Xbox SDK

Module Name:

    fastmath.cpp
--*/

#include "fastmath.h"
#include <algorithm>

namespace FastMath
{
    //----------------------------------------------------------------------------
    // atan
    float Atan(float x)
    {
        return std::atanf(x);
    }

    //----------------------------------------------------------------------------
    // atan2
    float Atan2(float y, float x)
    {
        return std::atan2f(y, x);
    }

    //----------------------------------------------------------------------------
    // acos
    float Acos(float x)
    {
        return std::acosf(x);
    }

    //----------------------------------------------------------------------------
    // asin
    float Asin(float x)
    {
        return std::asinf(x);
    }

    //----------------------------------------------------------------------------
    // log - natural logarithm
    float Log(float x)
    {
        return std::logf(x);
    }

    //----------------------------------------------------------------------------
    // log10
    float Log10(float x)
    {
        return std::log10f(x);
    }

    //----------------------------------------------------------------------------
    // exp
    float Exp(float x)
    {
        return std::expf(x);
    }

    //----------------------------------------------------------------------------
    // Compute both sin and cos together for efficiency
    void SinCos(float x, SinCosPair* result)
    {
        // DirectXMath doesn't have a combined sincos function
        // So we'll compute them separately
        result->fSin = Sin(x);
        result->fCos = Cos(x);
    }

    //----------------------------------------------------------------------------
    // sin
    float Sin(float x)
    {
        return DirectX::XMScalarSin(x);
    }

    //----------------------------------------------------------------------------
    // cos
    float Cos(float x)
    {
        return DirectX::XMScalarCos(x);
    }

    //----------------------------------------------------------------------------
    // tan
    float Tan(float x)
    {
        return std::tanf(x);
    }

    //----------------------------------------------------------------------------
    // pow
    float Pow(float x, float y)
    {
        return std::powf(x, y);
    }

    //----------------------------------------------------------------------------
    // hypot - compute sqrt(x*x + y*y) without intermediate overflow
    float Hypot(float x, float y)
    {
        return std::hypotf(x, y);
    }

    //----------------------------------------------------------------------------
    // ceil
    float Ceil(float x)
    {
        return std::ceilf(x);
    }

    //----------------------------------------------------------------------------
    // floor
    float Floor(float x)
    {
        return std::floorf(x);
    }

    //----------------------------------------------------------------------------
    // tanh
    float Tanh(float x)
    {
        return std::tanhf(x);
    }

    //----------------------------------------------------------------------------
    // cosh
    float Cosh(float x)
    {
        return std::coshf(x);
    }

    //----------------------------------------------------------------------------
    // sinh
    float Sinh(float x)
    {
        return std::sinhf(x);
    }
}