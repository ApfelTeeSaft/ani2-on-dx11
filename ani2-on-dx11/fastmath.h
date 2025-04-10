/*--
Original Copyright (c) 1999 - 2000 Microsoft Corporation - Xbox SDK Framework

Module Name:

    FastMath.h

Abstract:

    General math support including fast replacements for the standard math
    library. Modern version uses DirectXMath and SIMD intrinsics.

Revision History:

    Original Xbox implementation by Microsoft
    Modern port to Windows 11 with DirectXMath
--*/

#pragma once

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <cmath>
#include <immintrin.h>
#include <cfloat>

//------------------------------------------------------------------------------
// Constants
constexpr float PI = 3.14159265358979323846f;             // Pi
constexpr float PI_MUL_2 = 6.28318530717958623200f;             // 2 * Pi
constexpr float PI_DIV_2 = 1.57079632679489655800f;             // Pi / 2
constexpr float PI_DIV_4 = 0.78539816339744827900f;             // Pi / 4
constexpr float INV_PI = 0.31830988618379069122f;             // 1 / Pi
constexpr float DEGTORAD = 0.01745329251994329547f;             // Degrees to Radians
constexpr float RADTODEG = 57.29577951308232286465f;            // Radians to Degrees
constexpr float FLOAT_SMALL = 1.0e-6f;                             // Small number for floats
constexpr float FLOAT_HUGE = 1.0e+38f;                            // Huge number for floats
constexpr float FLOAT_EPSILON = 1.0e-5f;                             // Tolerance for floats
constexpr float FLOAT_INFINITY = FLT_MAX;                             // Infinity value for float

//----------------------------------------------------------------------------
// Define a C++ structure to hold a pair of floats for sin/cos operations
struct SinCosPair
{
    float   fCos;
    float   fSin;
};

//----------------------------------------------------------------------------
// Fast math replacement functions, optimized using SIMD instructions
namespace FastMath
{
    // Trigonometric functions
    float Atan(float x);
    float Atan2(float y, float x);
    float Acos(float x);
    float Asin(float x);
    void SinCos(float x, SinCosPair* result);
    float Sin(float x);
    float Cos(float x);
    float Tan(float x);

    // Logarithmic and exponential functions
    float Log(float x);
    float Log10(float x);
    float Exp(float x);
    float Pow(float x, float y);

    // Square root and inverse square root
    float Sqrt(float x);
    float InverseSqrt(float x);

    // Absolute value
    float Abs(float x);

    // Additional functions not in original but useful
    float Hypot(float x, float y);
    float Ceil(float x);
    float Floor(float x);
    float Tanh(float x);
    float Cosh(float x);
    float Sinh(float x);

    // Vector operations using DirectXMath
    DirectX::XMVECTOR VectorAdd(DirectX::XMVECTOR v1, DirectX::XMVECTOR v2);
    DirectX::XMVECTOR VectorSubtract(DirectX::XMVECTOR v1, DirectX::XMVECTOR v2);
    DirectX::XMVECTOR VectorMultiply(DirectX::XMVECTOR v1, DirectX::XMVECTOR v2);
    DirectX::XMVECTOR VectorDivide(DirectX::XMVECTOR v1, DirectX::XMVECTOR v2);
    DirectX::XMVECTOR VectorCross(DirectX::XMVECTOR v1, DirectX::XMVECTOR v2);
    float VectorDot(DirectX::XMVECTOR v1, DirectX::XMVECTOR v2);
    DirectX::XMVECTOR VectorNormalize(DirectX::XMVECTOR v);
    float VectorLength(DirectX::XMVECTOR v);
    float VectorLengthSquared(DirectX::XMVECTOR v);
}

// Inline implementations for the most critical functions

inline float FastMath::Abs(float x)
{
    return std::fabsf(x);
}

inline float FastMath::Sqrt(float x)
{
    return DirectX::XMVectorGetX(DirectX::XMVectorSqrt(DirectX::XMVectorReplicate(x)));
}

inline float FastMath::InverseSqrt(float x)
{
    return DirectX::XMVectorGetX(DirectX::XMVectorReciprocalSqrt(DirectX::XMVectorReplicate(x)));
}

inline DirectX::XMVECTOR FastMath::VectorAdd(DirectX::XMVECTOR v1, DirectX::XMVECTOR v2)
{
    return DirectX::XMVectorAdd(v1, v2);
}

inline DirectX::XMVECTOR FastMath::VectorSubtract(DirectX::XMVECTOR v1, DirectX::XMVECTOR v2)
{
    return DirectX::XMVectorSubtract(v1, v2);
}

inline DirectX::XMVECTOR FastMath::VectorMultiply(DirectX::XMVECTOR v1, DirectX::XMVECTOR v2)
{
    return DirectX::XMVectorMultiply(v1, v2);
}

inline DirectX::XMVECTOR FastMath::VectorDivide(DirectX::XMVECTOR v1, DirectX::XMVECTOR v2)
{
    return DirectX::XMVectorDivide(v1, v2);
}

inline DirectX::XMVECTOR FastMath::VectorCross(DirectX::XMVECTOR v1, DirectX::XMVECTOR v2)
{
    return DirectX::XMVector3Cross(v1, v2);
}

inline float FastMath::VectorDot(DirectX::XMVECTOR v1, DirectX::XMVECTOR v2)
{
    return DirectX::XMVectorGetX(DirectX::XMVector3Dot(v1, v2));
}

inline DirectX::XMVECTOR FastMath::VectorNormalize(DirectX::XMVECTOR v)
{
    return DirectX::XMVector3Normalize(v);
}

inline float FastMath::VectorLength(DirectX::XMVECTOR v)
{
    return DirectX::XMVectorGetX(DirectX::XMVector3Length(v));
}

inline float FastMath::VectorLengthSquared(DirectX::XMVECTOR v)
{
    return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(v));
}