///////////////////////////////////////////////////////////////////////////////
// File: RenderObject.h - Modern Windows 11 port
//
// Original Copyright 2001 Pipeworks Software
// Modern Port Copyright (c) 2023
//
// Base class for all renderable objects in the application
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

///////////////////////////////////////////////////////////////////////////////
// RenderObject - Abstract base class for all renderable objects
///////////////////////////////////////////////////////////////////////////////
class RenderObject
{
public:
    RenderObject() = default;
    virtual ~RenderObject() = default;

    // Check if object is visible
    virtual bool IsVisible() = 0;

    // Clean up resources
    virtual void Destroy() = 0;

    // Render the object using the provided device context
    virtual void Render(ID3D11DeviceContext* pContext) = 0;

    // Update object animation/state
    // elapsedTime: total time elapsed since start
    // dt: delta time since last update
    virtual void AdvanceTime(float elapsedTime, float dt) = 0;
};