///////////////////////////////////////////////////////////////////////////////
// File: BlobRenderer.h - Modern Windows 11 port
//
// Original Copyright 2001 Pipeworks Software
// Modern Port Copyright (c) 2023
//
// This is an implementation of metaballs (implicit surface rendering) 
// using marching cubes/tetrahedra techniques.
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <wrl/client.h>

// Forward declarations
class RenderObject;

///////////////////////////////////////////////////////////////////////////////
// BlobVertex - Vertex structure for blob rendering
///////////////////////////////////////////////////////////////////////////////
struct BlobVertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;    // not normalized, will do it in the GPU shader
};

///////////////////////////////////////////////////////////////////////////////
// BlobSource - Defines a metaball (implicit surface source)
///////////////////////////////////////////////////////////////////////////////
class BlobSource
{
public:
    DirectX::XMFLOAT3  position;
    float               strength;
    float               considerationRadius;
    float               considerationRadiusSq;  // squared for faster distance checks
    int                 affiliation;            // affiliation of -1 is never rendered, but repulses

    // Calculate field contribution at a point
    float Calculate(int affiliation, const DirectX::XMFLOAT3& pos) const;
};

///////////////////////////////////////////////////////////////////////////////
// BlobRenderer - Renders metaballs/implicit surfaces
///////////////////////////////////////////////////////////////////////////////
class BlobRenderer : public RenderObject
{
protected:
    const BlobSource* m_pSources;
    int                         m_NumBlobs;

    float                       m_Threshold;  // Threshold for surface generation

    UINT                        m_NumVertices;
    UINT                        m_NumIndices;

    // D3D11 buffers and resources
    Microsoft::WRL::ComPtr<ID3D11Buffer>             m_pVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>             m_pIndexBuffer;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>       m_pVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>        m_pPixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>        m_pInputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>             m_pConstantBuffer;

    // Field data for implicit surface generation
    std::vector<float>          m_Field;
    std::vector<int>            m_VertexIndices;  // Circular FIFO which stores one layer of vertex indices
    int                         m_FieldX, m_FieldY, m_FieldZ;  // Field dimensions
    int                         m_FieldXY;  // Precomputed m_FieldX * m_FieldY

    DirectX::XMFLOAT3           m_LowerLeftCorner;  // Corner of the field in world space
    DirectX::XMFLOAT3           m_FieldToWorld;     // Conversion scale from field to world
    DirectX::XMFLOAT3           m_WorldToField;     // Conversion scale from world to field

    // Helper methods for field and coordinate conversion
    int GetFieldCoords(const DirectX::XMFLOAT3& pos, int* p_x, int* p_y, int* p_z, DirectX::XMFLOAT3* p_remainder = nullptr) const;
    void GetWorldPos(DirectX::XMFLOAT3* pos, int x, int y, int z) const;

public:
    BlobRenderer();
    ~BlobRenderer();

    // RenderObject interface
    virtual bool IsVisible() override { return true; }
    virtual void Destroy() override;
    virtual void Render(ID3D11DeviceContext* pContext) override;
    virtual void AdvanceTime(float elapsedTime, float dt) override;

    // Initialize the blob renderer
    void Initialize(ID3D11Device* pDevice, const BlobSource* pBlobSources, int numBlobs,
        float xySpacing, float zSpacing,
        const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& halfDim);

    // Getters/setters
    float GetThreshold() const { return m_Threshold; }
    void SetThreshold(float threshold) { m_Threshold = threshold; }
};

///////////////////////////////////////////////////////////////////////////////
// TestBlobRenderer - Test implementation with predefined blobs
///////////////////////////////////////////////////////////////////////////////
class TestBlobRenderer : public BlobRenderer
{
protected:
    static constexpr int NUM_BLOBS = 6;
    BlobSource m_Sources[NUM_BLOBS];

public:
    TestBlobRenderer();
    ~TestBlobRenderer();

    virtual void CreateTestBlobs(ID3D11Device* pDevice);
    virtual void AdvanceTime(float elapsedTime, float dt) override;
};