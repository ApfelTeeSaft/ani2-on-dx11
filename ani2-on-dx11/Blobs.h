///////////////////////////////////////////////////////////////////////////////
// File: Blobs.h - Modern Windows 11 port
//
// Original Copyright 2001 Pipeworks Software
// Modern Port Copyright (c) 2023
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <memory>
#include <vector>
#include <random>
#include "RenderObject.h"

///////////////////////////////////////////////////////////////////////////////
// BlobVertex - Vertex structure for blob rendering
///////////////////////////////////////////////////////////////////////////////
struct BlobVertex
{
    DirectX::XMFLOAT3 position;
    // Normal is implicit for spheres (position from center)
};

///////////////////////////////////////////////////////////////////////////////
// Forward declaration
///////////////////////////////////////////////////////////////////////////////
class LavaLampInterior;

///////////////////////////////////////////////////////////////////////////////
// LLBlob - Individual blob in the lava lamp
///////////////////////////////////////////////////////////////////////////////
class LLBlob : public RenderObject
{
public:
    // Rendering resources
    UINT                                    m_NumVertices;
    UINT                                    m_NumIndices;
    Microsoft::WRL::ComPtr<ID3D11Buffer>    m_pVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>    m_pIndexBuffer;
    DirectX::XMFLOAT4                       m_BlobColor;

    // Position and appearance
    DirectX::XMFLOAT3                       m_Position;
    DirectX::XMFLOAT3                       m_Scale;
    float                                   m_DeformationInertia;
    float                                   m_Radius;

    // Physics and behavior
    float                                   m_Temperature;
    float                                   m_TemperatureAbsorbance;
    DirectX::XMFLOAT3                       m_Velocity;
    DirectX::XMFLOAT3                       m_Acceleration;

    // Mesh subdivisions
    int                                     m_Subdivisions;  // Number of quads in a direction for each face
    float                                   m_DivisionStep;  // Distance on cube face that a division spans

    // Group behavior
    int                                     m_Species;

    // Cube has dimensions from -1 to +1, this calculates points on each face
    void                               CalculateFacePoint(DirectX::XMFLOAT3* pPos, int face, int u, int v);

public:
    LLBlob();

    void Create(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 color);

    // RenderObject interface implementation
    virtual void Create(ID3D11Device* pDevice) {}
    virtual void Destroy() override;
    virtual bool IsVisible() override { return true; }
    virtual void Render(ID3D11DeviceContext* pContext) override;
    virtual void AdvanceTime(float elapsedTime, float dt) override;

    // Physics and collision
    void Collided(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 normal);
    void InteractWithBlob(const LLBlob* pllb, float dt);

    // Getters/setters
    DirectX::XMFLOAT3 GetPosition() const { return m_Position; }
    float GetRadius() const { return m_Radius; }
    float GetRadiusSq() const { return m_Radius * m_Radius; }

    int GetSpecies() const { return m_Species; }
    void SetSpecies(int s) { m_Species = s; }

    void SetColor(DirectX::XMFLOAT4 color) { m_BlobColor = color; }
    const DirectX::XMFLOAT4& GetColor() const { return m_BlobColor; }

    // Reference to the containing lava lamp
    static const LavaLampInterior* spLL;

    // Friend declaration to allow LLBlob to access LavaLampInterior's protected members
    friend class LavaLampInterior;
};

///////////////////////////////////////////////////////////////////////////////
// LavaLampInterior - Lava lamp container with multiple blobs
///////////////////////////////////////////////////////////////////////////////
class LavaLampInterior : public RenderObject
{
public:
    static constexpr int NUM_LLBLOBS = 64;
    LLBlob m_Blobs[NUM_LLBLOBS];

    // Rendering resources
    Microsoft::WRL::ComPtr<ID3D11VertexShader>    m_pVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>     m_pPixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>     m_pInputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>          m_pConstantBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pNormCubemap;

    // Lava lamp shape definition
    static constexpr int MAX_CONIC_SECTIONS = 32;
    int                         m_NumConicSections;
    float                       m_ConicSectionCenterX, m_ConicSectionCenterY;
    float                       m_ConicSectionBotZ[MAX_CONIC_SECTIONS + 1];
    float                       m_ConicSectionRadius[MAX_CONIC_SECTIONS + 1];
    float                       m_ConicSectionSlope[MAX_CONIC_SECTIONS];    // dr/dz
    float                       m_ConicSectionNormalR[MAX_CONIC_SECTIONS];  // -1, normalized
    float                       m_ConicSectionNormalZ[MAX_CONIC_SECTIONS];  // slope, normalized

    // Random number generator
    static std::mt19937         m_RandomGenerator;

    // Private methods
    void                        InitializeShaders(ID3D11Device* pDevice);

public:
    LavaLampInterior();
    ~LavaLampInterior();

    // RenderObject interface implementation
    virtual bool IsVisible() override { return true; }
    virtual void Create(ID3D11Device* pDevice);
    virtual void Destroy() override;
    virtual void Render(ID3D11DeviceContext* pContext) override;
    virtual void AdvanceTime(float elapsedTime, float dt) override;

    // Lava lamp container properties
    float GetBottom() const { return m_ConicSectionBotZ[0]; }
    float GetTop() const { return m_ConicSectionBotZ[m_NumConicSections]; }

    // Get radius at a specific height
    float GetRadius(float z) const;

    // Temperature gradient (1.0 at bottom, 0.0 at top)
    float GetTemperature(float z) const;

    // Collision detection for blobs
    void Collide(LLBlob* pllb, float x, float y, float z, float radius, float dt) const;
    bool CollideWithCaps(LLBlob* pllb, float x, float y, float z, float radius) const;

    // Group behavior
    void RecomputeSpecies();

    // Static random number helpers (0.0-1.0 and -1.0-1.0)
    static float FRand01();
    static float FRand11();

    // Make LLBlob a friend class to allow it to access protected members
    friend class LLBlob;
};