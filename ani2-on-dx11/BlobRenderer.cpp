///////////////////////////////////////////////////////////////////////////////
// File: BlobRenderer.cpp - Modern Windows 11 port
//
// Original Copyright 2001 Pipeworks Software
// Modern Port Copyright (c) 2023
///////////////////////////////////////////////////////////////////////////////
#include "BlobRenderer.h"
#include "RenderObject.h"
#include <algorithm>
#include <random>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// Random number helpers
namespace {
    std::random_device g_rd;
    std::mt19937 g_gen(g_rd());
    std::uniform_real_distribution<float> g_dist01(0.0f, 1.0f);
    std::uniform_real_distribution<float> g_dist11(-1.0f, 1.0f);

    float FRand01() {
        return g_dist01(g_gen);
    }

    float FRand11() {
        return g_dist11(g_gen);
    }
}

///////////////////////////////////////////////////////////////////////////////
// BlobSource methods
///////////////////////////////////////////////////////////////////////////////
float BlobSource::Calculate(int checkAffiliation, const XMFLOAT3& pos) const
{
    // Calculate squared distance
    float dx = position.x - pos.x;
    float dy = position.y - pos.y;
    float dz = position.z - pos.z;
    float distSq = dx * dx + dy * dy + dz * dz;

    // Check if point is within consideration radius
    if (distSq > considerationRadiusSq)
        return 0.0f;

    // Calculate falloff based on distance
    float falloff = 1.0f - (distSq / considerationRadiusSq);

    // Different affiliations repel each other
    float affiliationFactor = (checkAffiliation == affiliation) ? 1.0f : -1.0f;

    // Final field contribution
    return affiliationFactor * strength * falloff * falloff;
}

///////////////////////////////////////////////////////////////////////////////
// BlobRenderer methods
///////////////////////////////////////////////////////////////////////////////
BlobRenderer::BlobRenderer()
    : m_pSources(nullptr)
    , m_NumBlobs(0)
    , m_Threshold(1.0f)
    , m_NumVertices(0)
    , m_NumIndices(0)
    , m_FieldX(0)
    , m_FieldY(0)
    , m_FieldZ(0)
    , m_FieldXY(0)
{
    m_LowerLeftCorner = XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_FieldToWorld = XMFLOAT3(1.0f, 1.0f, 1.0f);
    m_WorldToField = XMFLOAT3(1.0f, 1.0f, 1.0f);
}

///////////////////////////////////////////////////////////////////////////////
BlobRenderer::~BlobRenderer()
{
    Destroy();
}

///////////////////////////////////////////////////////////////////////////////
void BlobRenderer::Destroy()
{
    m_pVertexBuffer.Reset();
    m_pIndexBuffer.Reset();
    m_pVertexShader.Reset();
    m_pPixelShader.Reset();
    m_pInputLayout.Reset();
    m_pConstantBuffer.Reset();

    m_Field.clear();
    m_VertexIndices.clear();

    m_pSources = nullptr;
    m_NumBlobs = 0;
    m_NumVertices = 0;
    m_NumIndices = 0;
    m_FieldX = m_FieldY = m_FieldZ = m_FieldXY = 0;
}

///////////////////////////////////////////////////////////////////////////////
void BlobRenderer::Initialize(ID3D11Device* pDevice, const BlobSource* pBlobSources, int numBlobs,
    float xySpacing, float zSpacing,
    const XMFLOAT3& center, const XMFLOAT3& halfDim)
{
    m_pSources = pBlobSources;
    m_NumBlobs = numBlobs;

    // Calculate field dimensions
    m_FieldX = static_cast<int>((halfDim.x * 2.0f / xySpacing) + 2.0f);
    m_FieldY = static_cast<int>((halfDim.y * 2.0f / xySpacing) + 2.0f);
    m_FieldZ = static_cast<int>((halfDim.z * 2.0f / zSpacing) + 2.0f);
    m_FieldXY = m_FieldX * m_FieldY;

    // Allocate field and vertex index arrays
    m_Field.resize(m_FieldXY * m_FieldZ);
    m_VertexIndices.resize((m_FieldXY + 1) * 6);

    // Calculate corner of the field and conversion factors
    m_LowerLeftCorner.x = center.x - halfDim.x;
    m_LowerLeftCorner.y = center.y - halfDim.y;
    m_LowerLeftCorner.z = center.z - halfDim.z;

    m_FieldToWorld.x = xySpacing;
    m_FieldToWorld.y = xySpacing;
    m_FieldToWorld.z = zSpacing;

    m_WorldToField.x = 1.0f / xySpacing;
    m_WorldToField.y = 1.0f / xySpacing;
    m_WorldToField.z = 1.0f / zSpacing;

    // Create shader resources
    CreateShaderResources(pDevice);
}

///////////////////////////////////////////////////////////////////////////////
void BlobRenderer::CreateShaderResources(ID3D11Device* pDevice)
{
    // Define vertex input layout
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    // Create vertex shader
    const char* vsCode = R"(
        cbuffer Constants : register(b0)
        {
            matrix World;
            matrix View;
            matrix Projection;
        };

        struct VS_INPUT
        {
            float3 Position : POSITION;
            float3 Normal : NORMAL;
        };

        struct VS_OUTPUT
        {
            float4 Position : SV_POSITION;
            float3 Normal : NORMAL;
            float3 WorldPos : TEXCOORD0;
        };

        VS_OUTPUT main(VS_INPUT input)
        {
            VS_OUTPUT output;
            
            // Transform position to world space
            float4 worldPos = mul(float4(input.Position, 1.0f), World);
            
            // Transform to clip space
            output.Position = mul(worldPos, mul(View, Projection));
            
            // Transform normal to world space
            output.Normal = normalize(mul(input.Normal, (float3x3)World));
            
            // Pass world position to pixel shader
            output.WorldPos = worldPos.xyz;
            
            return output;
        }
    )";

    // Create pixel shader
    const char* psCode = R"(
        struct PS_INPUT
        {
            float4 Position : SV_POSITION;
            float3 Normal : NORMAL;
            float3 WorldPos : TEXCOORD0;
        };

        float4 main(PS_INPUT input) : SV_TARGET
        {
            // Simple lighting calculation
            float3 lightDir = normalize(float3(1.0f, -1.0f, 1.0f));
            float3 normal = normalize(input.Normal);
            
            // Diffuse lighting
            float diffuse = max(0.0f, dot(normal, -lightDir));
            
            // Ambient lighting
            float ambient = 0.3f;
            
            // Final color
            float3 color = float3(0.2f, 0.6f, 0.8f); // Blue blob color
            float lighting = ambient + diffuse * 0.7f;
            
            return float4(color * lighting, 1.0f);
        }
    )";

    // TODO: Compile shaders using D3DCompile and create shader objects
    // This would normally be done with D3DCompileFromFile or D3DCompile for runtime compilation
    // or by loading pre-compiled shader objects

    // Create constant buffer
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.ByteWidth = sizeof(XMMATRIX) * 3; // World, View, Projection matrices
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    pDevice->CreateBuffer(&cbDesc, nullptr, &m_pConstantBuffer);

    // Create dynamic vertex and index buffers that will be filled during rendering
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DYNAMIC;
    vbDesc.ByteWidth = sizeof(BlobVertex) * 65536; // Max vertices
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    pDevice->CreateBuffer(&vbDesc, nullptr, &m_pVertexBuffer);

    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.Usage = D3D11_USAGE_DYNAMIC;
    ibDesc.ByteWidth = sizeof(UINT) * 65536 * 3; // Max indices for triangles
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    pDevice->CreateBuffer(&ibDesc, nullptr, &m_pIndexBuffer);
}

///////////////////////////////////////////////////////////////////////////////
void BlobRenderer::Render(ID3D11DeviceContext* pContext)
{
    if (!m_pSources || m_NumBlobs == 0)
        return;

    // Sort blobs by affiliation
    int nextAffiliation = m_pSources[0].affiliation;
    for (int i = 1; i < m_NumBlobs; i++)
    {
        nextAffiliation = std::min(nextAffiliation, m_pSources[i].affiliation);
    }

    // Process each affiliation group separately
    for (int affiliation = nextAffiliation; affiliation < m_NumBlobs; affiliation = nextAffiliation)
    {
        nextAffiliation = m_NumBlobs;
        XMFLOAT3 ptMin, ptMax;
        bool bUnset = true;

        // For this affiliation, find the bounding box
        for (int i = 0; i < m_NumBlobs; i++)
        {
            const BlobSource& bs = m_pSources[i];

            // Track the next affiliation
            int aff = bs.affiliation;
            if (aff > affiliation)
                nextAffiliation = std::min(aff, nextAffiliation);

            if (aff == affiliation)
            {
                if (bUnset)
                {
                    bUnset = false;
                    ptMin = ptMax = bs.position;
                    ptMin.x -= bs.considerationRadius;
                    ptMin.y -= bs.considerationRadius;
                    ptMin.z -= bs.considerationRadius;
                    ptMax.x += bs.considerationRadius;
                    ptMax.y += bs.considerationRadius;
                    ptMax.z += bs.considerationRadius;
                }
                else
                {
                    ptMin.x = std::min(ptMin.x, bs.position.x - bs.considerationRadius);
                    ptMin.y = std::min(ptMin.y, bs.position.y - bs.considerationRadius);
                    ptMin.z = std::min(ptMin.z, bs.position.z - bs.considerationRadius);

                    ptMax.x = std::max(ptMax.x, bs.position.x + bs.considerationRadius);
                    ptMax.y = std::max(ptMax.y, bs.position.y + bs.considerationRadius);
                    ptMax.z = std::max(ptMax.z, bs.position.z + bs.considerationRadius);
                }
            }
        }

        // Get field coordinates for the bounding box
        int sx, sy, sz, ex, ey, ez;
        GetFieldCoords(ptMin, &sx, &sy, &sz);
        GetFieldCoords(ptMax, &ex, &ey, &ez);
        ex++; ey++; ez++;

        // Calculate field dimensions for this region
        int lenX = ex - sx + 1;
        int lenY = ey - sy + 1;
        int lenZ = ez - sz + 1;
        int lenXY = lenX * lenY;
        int lenXYZ = lenXY * lenZ;

        // Skip if region is too large for our field
        if (lenX * lenY * lenZ > m_FieldX * m_FieldY * m_FieldZ)
            continue;

        // Clear the field
        std::fill(m_Field.begin(), m_Field.begin() + lenXYZ, 0.0f);

        // Populate the field with blob contributions
        for (int i = 0; i < m_NumBlobs; i++)
        {
            const BlobSource& bs = m_pSources[i];
            XMFLOAT3 ptStart, ptEnd;

            // Calculate bounding box for this blob
            ptStart.x = std::max(ptMin.x, bs.position.x - bs.considerationRadius);
            ptStart.y = std::max(ptMin.y, bs.position.y - bs.considerationRadius);
            ptStart.z = std::max(ptMin.z, bs.position.z - bs.considerationRadius);

            ptEnd.x = std::min(ptMax.x, bs.position.x + bs.considerationRadius);
            ptEnd.y = std::min(ptMax.y, bs.position.y + bs.considerationRadius);
            ptEnd.z = std::min(ptMax.z, bs.position.z + bs.considerationRadius);

            // Convert to field coordinates
            int bsx, bsy, bsz, bex, bey, bez;
            GetFieldCoords(ptStart, &bsx, &bsy, &bsz);
            GetFieldCoords(ptEnd, &bex, &bey, &bez);
            bex++; bey++; bez++;

            // Add blob contribution to field
            if (bsx <= bex && bsy <= bey && bsz <= bez)
            {
                XMFLOAT3 pos, posll;
                GetWorldPos(&posll, bsx, bsy, bsz);

                pos.z = posll.z;
                for (int w = bsz; w <= bez; w++, pos.z += m_FieldToWorld.z)
                {
                    pos.y = posll.y;
                    for (int v = bsy; v <= bey; v++, pos.y += m_FieldToWorld.y)
                    {
                        pos.x = posll.x;
                        float* pField = &m_Field[w * lenXY + v * lenX + bsx - sx];

                        for (int u = bsx; u <= bex; u++, pos.x += m_FieldToWorld.x)
                        {
                            *(pField++) += bs.Calculate(affiliation, pos);
                        }
                    }
                }
            }
        }

        // Generate isosurface using marching cubes/tetrahedra
        // TODO: Implement the marching cubes or marching tetrahedra algorithm here

        // This would:
        // 1. Analyze each voxel in the field
        // 2. Determine which corners are inside/outside the isosurface (based on threshold)
        // 3. Generate triangles for the isosurface
        // 4. Store vertices and indices
        // 5. Update vertex and index buffers

        // Once the geometry is generated, set up the pipeline state and draw
        if (m_NumVertices > 0 && m_NumIndices > 0)
        {
            // Set vertex buffer
            UINT stride = sizeof(BlobVertex);
            UINT offset = 0;
            pContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);

            // Set index buffer
            pContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

            // Set input layout
            pContext->IASetInputLayout(m_pInputLayout.Get());

            // Set primitive topology
            pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Set shaders
            pContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
            pContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

            // Update constant buffer with transformation matrices
            // TODO: Get actual matrices from the camera system
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            if (SUCCEEDED(pContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
            {
                // Define transformation matrices
                XMMATRIX world = XMMatrixIdentity();
                XMMATRIX view = XMMatrixLookAtLH(
                    XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f),
                    XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
                    XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f)
                );
                XMMATRIX projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, 16.0f / 9.0f, 0.1f, 100.0f);

                // Write to constant buffer
                XMMATRIX* pMatrices = (XMMATRIX*)mappedResource.pData;
                pMatrices[0] = XMMatrixTranspose(world);
                pMatrices[1] = XMMatrixTranspose(view);
                pMatrices[2] = XMMatrixTranspose(projection);

                pContext->Unmap(m_pConstantBuffer.Get(), 0);
            }

            // Set constant buffer
            pContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());

            // Draw indexed
            pContext->DrawIndexed(m_NumIndices, 0, 0);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void BlobRenderer::AdvanceTime(float elapsedTime, float dt)
{
    // Nothing to do here in the base class
    // Derived classes might implement blob animation
}

///////////////////////////////////////////////////////////////////////////////
int BlobRenderer::GetFieldCoords(const XMFLOAT3& pos, int* p_x, int* p_y, int* p_z, XMFLOAT3* p_remainder) const
{
    float fx = (pos.x - m_LowerLeftCorner.x) * m_WorldToField.x;
    float fy = (pos.y - m_LowerLeftCorner.y) * m_WorldToField.y;
    float fz = (pos.z - m_LowerLeftCorner.z) * m_WorldToField.z;

    *p_x = static_cast<int>(fx);
    *p_y = static_cast<int>(fy);
    *p_z = static_cast<int>(fz);

    if (p_remainder)
    {
        p_remainder->x = fx - static_cast<float>(*p_x);
        p_remainder->y = fy - static_cast<float>(*p_y);
        p_remainder->z = fz - static_cast<float>(*p_z);
    }

    // Return field index
    return (*p_z * m_FieldXY) + (*p_y * m_FieldX) + (*p_x);
}

///////////////////////////////////////////////////////////////////////////////
void BlobRenderer::GetWorldPos(XMFLOAT3* pos, int x, int y, int z) const
{
    pos->x = m_LowerLeftCorner.x + static_cast<float>(x) * m_FieldToWorld.x;
    pos->y = m_LowerLeftCorner.y + static_cast<float>(y) * m_FieldToWorld.y;
    pos->z = m_LowerLeftCorner.z + static_cast<float>(z) * m_FieldToWorld.z;
}

///////////////////////////////////////////////////////////////////////////////
// TestBlobRenderer methods
///////////////////////////////////////////////////////////////////////////////
TestBlobRenderer::TestBlobRenderer()
{
    // Initialize with default values
}

///////////////////////////////////////////////////////////////////////////////
TestBlobRenderer::~TestBlobRenderer()
{
    Destroy();
}

///////////////////////////////////////////////////////////////////////////////
void TestBlobRenderer::CreateTestBlobs(ID3D11Device* pDevice)
{
    // Create test blob sources
    for (int i = 0; i < NUM_BLOBS; i++)
    {
        // Random position near origin
        m_Sources[i].position = XMFLOAT3(
            FRand01() * 0.2f,
            FRand01() * 0.2f,
            FRand01() * 0.2f
        );

        // Set properties
        m_Sources[i].strength = 1.0f;
        m_Sources[i].considerationRadius = 0.09f;
        m_Sources[i].considerationRadiusSq = m_Sources[i].considerationRadius * m_Sources[i].considerationRadius;
        m_Sources[i].affiliation = 0;  // All blobs same affiliation for now
    }

    // Center and dimensions of the field
    XMFLOAT3 center = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 halfDim = { 1.0f, 1.0f, 1.0f };

    // Initialize the blob renderer
    Initialize(pDevice, m_Sources, NUM_BLOBS, 0.005f, 0.005f, center, halfDim);
}

///////////////////////////////////////////////////////////////////////////////
void TestBlobRenderer::AdvanceTime(float elapsedTime, float dt)
{
    // Animate the blobs
    for (int i = 0; i < NUM_BLOBS; i++)
    {
        // Simple animation: move around a little bit
        m_Sources[i].position.x += FRand11() * 0.002f * dt;
        m_Sources[i].position.y += FRand11() * 0.002f * dt;
        m_Sources[i].position.z += FRand11() * 0.002f * dt;

        // Constrain to prevent blobs from wandering too far
        m_Sources[i].position.x = std::max(-0.5f, std::min(0.5f, m_Sources[i].position.x));
        m_Sources[i].position.y = std::max(-0.5f, std::min(0.5f, m_Sources[i].position.y));
        m_Sources[i].position.z = std::max(-0.5f, std::min(0.5f, m_Sources[i].position.z));
    }
}