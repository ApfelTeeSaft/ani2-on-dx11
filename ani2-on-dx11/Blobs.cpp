///////////////////////////////////////////////////////////////////////////////
// File: Blobs.cpp - Modern Windows 11 port
//
// Original Copyright 2001 Pipeworks Software
// Modern Port Copyright (c) 2023
///////////////////////////////////////////////////////////////////////////////
#include "Blobs.h"
#include <algorithm>
#include <random>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// Initialize static members
const LavaLampInterior* LLBlob::spLL = nullptr;
std::mt19937 LavaLampInterior::m_RandomGenerator(std::random_device{}());

// Random number generation
float LavaLampInterior::FRand01()
{
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(m_RandomGenerator);
}

float LavaLampInterior::FRand11()
{
    static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    return dist(m_RandomGenerator);
}

///////////////////////////////////////////////////////////////////////////////
// LavaLampInterior methods
///////////////////////////////////////////////////////////////////////////////
LavaLampInterior::LavaLampInterior()
{
    m_NumConicSections = 0;
}

///////////////////////////////////////////////////////////////////////////////
LavaLampInterior::~LavaLampInterior()
{
    Destroy();
}

///////////////////////////////////////////////////////////////////////////////
void LavaLampInterior::Destroy()
{
    // Clean up blob resources
    for (int i = 0; i < NUM_LLBLOBS; i++)
    {
        m_Blobs[i].Destroy();
    }

    // Release shader resources
    m_pVertexShader.Reset();
    m_pPixelShader.Reset();
    m_pInputLayout.Reset();
    m_pConstantBuffer.Reset();
    m_pNormCubemap.Reset();
}

///////////////////////////////////////////////////////////////////////////////
void LavaLampInterior::InitializeShaders(ID3D11Device* pDevice)
{
    // Define vertex shader code
    const char* vsCode = R"(
        cbuffer Constants : register(b0)
        {
            matrix World;
            matrix ViewProj;
            float4 Constants; // x=0, y=1, z=2, w=0.5
        };

        struct VS_INPUT
        {
            float3 Position : POSITION;
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
            
            // Transform position
            float4 worldPos = mul(float4(input.Position, 1.0f), World);
            output.Position = mul(worldPos, ViewProj);
            
            // For a sphere, normal is the same as position (from center)
            output.Normal = normalize(input.Position);
            
            // Pass world position
            output.WorldPos = worldPos.xyz;
            
            return output;
        }
    )";

    // Define pixel shader code
    const char* psCode = R"(
        TextureCube NormalCubeMap : register(t0);
        SamplerState LinearSampler : register(s0);

        cbuffer LightData : register(b0)
        {
            float3 LightDir1;
            float3 LightDir2;
            float4 BlobColor;
            float4 AmbientColor;
        }

        struct PS_INPUT
        {
            float4 Position : SV_POSITION;
            float3 Normal : NORMAL;
            float3 WorldPos : TEXCOORD0;
        };

        float4 main(PS_INPUT input) : SV_TARGET
        {
            // Get normal from cubemap for more interesting lighting
            float3 normal = NormalCubeMap.Sample(LinearSampler, input.Normal).xyz * 2.0f - 1.0f;
            normal = normalize(normal);
            
            // Calculate lighting
            float light1 = max(0.0f, dot(normal, -LightDir1));
            float light2 = max(0.0f, dot(normal, -LightDir2));
            
            // Combine lights
            float4 diffuse = BlobColor * (light1 + light2);
            float4 final = diffuse + AmbientColor;
            
            return float4(final.rgb, BlobColor.a);
        }
    )";

    // TODO: Compile shaders using D3DCompile
    // In a real implementation, we would compile these shaders using D3DCompileFromFile
    // or load pre-compiled shader objects

    // Create normal cubemap for blob lighting
    // This would typically be loaded from a file or generated procedurally
}

///////////////////////////////////////////////////////////////////////////////
void LavaLampInterior::Create(ID3D11Device* pDevice)
{
    // Set up the reference to this object for all blobs
    LLBlob::spLL = this;

    // Define the conic sections that make up the lava lamp shape
    m_ConicSectionCenterX = +0.04f;
    m_ConicSectionCenterY = -0.082f;

    m_NumConicSections = 2;
    m_ConicSectionBotZ[0] = -0.47f;
    m_ConicSectionBotZ[1] = -0.25f;
    m_ConicSectionBotZ[2] = +0.35f;

    m_ConicSectionRadius[0] = 0.11f;
    m_ConicSectionRadius[1] = 0.25f;
    m_ConicSectionRadius[2] = 0.12f;

    // Calculate normals for each conic section
    for (int i = 0; i < m_NumConicSections; i++)
    {
        // Calculate slope (dr/dz)
        m_ConicSectionSlope[i] = (m_ConicSectionRadius[i + 1] - m_ConicSectionRadius[i]) /
            (m_ConicSectionBotZ[i + 1] - m_ConicSectionBotZ[i]);

        // Normalize to get normal vector components
        float norm = 1.0f / std::sqrt(1.0f + m_ConicSectionSlope[i] * m_ConicSectionSlope[i]);
        m_ConicSectionNormalR[i] = norm * -1.0f;
        m_ConicSectionNormalZ[i] = norm * m_ConicSectionSlope[i];
    }

    // Initialize shaders
    InitializeShaders(pDevice);

    // Create blobs
    float bot = m_ConicSectionBotZ[0];
    float sx = m_ConicSectionCenterX - 0.1f;
    float sy = m_ConicSectionCenterY - 0.1f;

    for (int i = 0; i < NUM_LLBLOBS; i++)
    {
        // Create each blob with a position and color
        XMFLOAT3 pos = {
            m_ConicSectionCenterX,
            m_ConicSectionCenterY,
            m_ConicSectionBotZ[m_NumConicSections >> 1]
        };

        // Calculate colors based on species
        XMFLOAT4 color = {
            (i & 0x04) ? 0.0f : 1.0f,
            (i & 0x02) ? 0.0f : 1.0f,
            (i & 0x01) ? 0.0f : 1.0f,
            1.0f
        };

        // Base color for all blobs
        XMFLOAT4 baseColor = { 0.724f, 0.732f, 0.556f, 1.0f };

        // Blend species color with base color (mostly base)
        m_Blobs[i].Create(pos, baseColor);
        m_Blobs[i].SetSpecies(i);
    }
}

///////////////////////////////////////////////////////////////////////////////
void LavaLampInterior::AdvanceTime(float elapsedTime, float dt)
{
    // Clamp dt to avoid instability in physics
    dt = std::min(dt, 0.1f);
    dt = std::max(dt, 0.0f);

    // Update each blob
    for (int i = 0; i < NUM_LLBLOBS; i++)
    {
        m_Blobs[i].AdvanceTime(elapsedTime, dt);
    }

    // Update group affiliations
    RecomputeSpecies();
}

///////////////////////////////////////////////////////////////////////////////
float LavaLampInterior::GetTemperature(float z) const
{
    // Temperature gradient - hotter at the bottom, cooler at the top
    float dz = -0.5f + (z - m_ConicSectionBotZ[0]) /
        (m_ConicSectionBotZ[m_NumConicSections] - m_ConicSectionBotZ[0]);
    dz *= 2.6f;
    dz *= dz * dz;  // Cubic falloff

    return std::max(0.0f, std::min(1.0f, 0.5f - dz));
}

///////////////////////////////////////////////////////////////////////////////
bool LavaLampInterior::CollideWithCaps(LLBlob* pllb, float x, float y, float z, float radius) const
{
    // Check collision with bottom cap
    if (z - radius < m_ConicSectionBotZ[0])
    {
        XMFLOAT3 pos = { x, y, m_ConicSectionBotZ[0] + radius + 0.001f };
        XMFLOAT3 norm = { 0.0f, 0.0f, 1.0f };  // Normal pointing up
        pllb->Collided(pos, norm);
        return true;
    }

    // Check collision with top cap
    if (z + radius > m_ConicSectionBotZ[m_NumConicSections])
    {
        XMFLOAT3 pos = { x, y, m_ConicSectionBotZ[m_NumConicSections] - radius - 0.001f };
        XMFLOAT3 norm = { 0.0f, 0.0f, -1.0f };  // Normal pointing down
        pllb->Collided(pos, norm);
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
void LavaLampInterior::Collide(LLBlob* pllb, float x, float y, float z, float radius, float dt) const
{
    // First check collision with top and bottom caps
    if (CollideWithCaps(pllb, x, y, z, radius))
    {
        // Get updated position after collision
        z = pllb->GetPosition().z;
    }

    // Adjust for lamp center
    x -= m_ConicSectionCenterX;
    y -= m_ConicSectionCenterY;

    // Calculate radial distance from center axis
    float r = std::sqrt(x * x + y * y);

    // Check collision with conic sections
    bool hitWall = false;

    for (int i = 0; i < m_NumConicSections; i++)
    {
        // Skip sections that don't overlap with blob's position
        if (m_ConicSectionBotZ[i + 1] < z - radius) continue;
        if (m_ConicSectionBotZ[i] > z + radius) break;

        // Sphere overlaps the section - calculate distance to surface
        float dz = z - m_ConicSectionBotZ[i];
        float dr = r - m_ConicSectionRadius[i];

        // Calculate overlap with surface
        float overlap = radius - (dr * m_ConicSectionNormalR[i] + dz * m_ConicSectionNormalZ[i]);
        if (overlap < 0.0f) continue;

        // Calculate distance along surface to nearest collision point
        float s = dr * m_ConicSectionNormalZ[i] - dz * m_ConicSectionNormalR[i];
        if (s < 0.0f) continue;

        float height = m_ConicSectionBotZ[i + 1] - m_ConicSectionBotZ[i];
        if (s * s > height * height * (1.0f + m_ConicSectionSlope[i] * m_ConicSectionSlope[i])) continue;

        // Calculate collision normal
        float nz = m_ConicSectionNormalZ[i];
        float oo_r = 1.0f / std::max(0.001f, r);
        float nx = x * oo_r * m_ConicSectionNormalR[i];
        float ny = y * oo_r * m_ConicSectionNormalR[i];

        // Adjust position by overlap
        x += nx * overlap;
        y += ny * overlap;
        z += nz * overlap;

        // Recompute radius
        r = std::sqrt(x * x + y * y);

        // Create collision response
        XMFLOAT3 pos = {
            m_ConicSectionCenterX + x,
            m_ConicSectionCenterY + y,
            z
        };
        XMFLOAT3 norm = { nx, ny, nz };

        pllb->Collided(pos, norm);
        hitWall = true;
    }

    // Check the section corners if not already hit wall
    if (!hitWall)
    {
        for (int i = 0; i < m_NumConicSections; i++)
        {
            // Skip sections that don't overlap with blob's position
            if (m_ConicSectionBotZ[i + 1] < z - radius) continue;
            if (m_ConicSectionBotZ[i] > z + radius) break;

            float dz = z - m_ConicSectionBotZ[i];
            float dr = r - m_ConicSectionRadius[i];

            // Check if point is within sphere
            if (dz * dz + dr * dr > radius * radius) continue;

            float dist = std::sqrt(dz * dz + dr * dr);
            float overlap = radius - dist;

            // Calculate normal
            float f_norm = 1.0f / std::max(0.001f, dist);
            float nz = -dz * f_norm;
            float nr = -dr * f_norm;

            float oo_r = 1.0f / std::max(0.001f, r);
            float nx = x * oo_r * nr;
            float ny = y * oo_r * nr;

            // Adjust position by overlap
            x += nx * overlap;
            y += ny * overlap;
            z += nz * overlap;

            // Recompute radius
            r = std::sqrt(x * x + y * y);

            // Create collision response
            XMFLOAT3 pos = {
                m_ConicSectionCenterX + x,
                m_ConicSectionCenterY + y,
                z
            };
            XMFLOAT3 norm = { nx, ny, nz };

            pllb->Collided(pos, norm);
            hitWall = true;
        }
    }

    // Check for collisions with other blobs
    for (int i = 0; i < NUM_LLBLOBS; i++)
    {
        // Skip self
        if (&m_Blobs[i] == pllb) continue;

        // Calculate vector between blobs
        XMFLOAT3 pos1 = pllb->GetPosition();
        XMFLOAT3 pos2 = m_Blobs[i].GetPosition();
        XMFLOAT3 delta = {
            pos2.x - pos1.x,
            pos2.y - pos1.y,
            pos2.z - pos1.z
        };

        // Calculate squared distance
        float distSq = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;

        // Combined radius
        float rad = m_Blobs[i].GetRadius() + pllb->GetRadius();

        // Check for overlap
        if (distSq > rad * rad) continue;

        // Blobs are interacting
        pllb->InteractWithBlob(&m_Blobs[i], dt);
    }

    // Double-check caps to ensure validity
    if (CollideWithCaps(pllb, x + m_ConicSectionCenterX, y + m_ConicSectionCenterY, z, radius))
    {
        // Should never happen with convex hull
        z = pllb->GetPosition().z;
    }
}

///////////////////////////////////////////////////////////////////////////////
float LavaLampInterior::GetRadius(float z) const
{
    // Return 0 if below the bottom
    if (z < m_ConicSectionBotZ[0]) return 0.0f;

    // Find which section contains this z-value
    for (int i = 0; i < m_NumConicSections; i++)
    {
        if (m_ConicSectionBotZ[i + 1] < z) continue;

        // Interpolate radius within this section
        float diff = m_ConicSectionBotZ[i + 1] - m_ConicSectionBotZ[i];
        float s = (z - m_ConicSectionBotZ[i]) / diff;

        return m_ConicSectionRadius[i] + s * (m_ConicSectionRadius[i + 1] - m_ConicSectionRadius[i]);
    }

    // Above the top
    return 0.0f;
}

///////////////////////////////////////////////////////////////////////////////
void LavaLampInterior::RecomputeSpecies()
{
    // Store previous species assignments
    int prevSpecies[NUM_LLBLOBS];
    for (int i = 0; i < NUM_LLBLOBS; i++)
    {
        prevSpecies[i] = m_Blobs[i].GetSpecies();
        m_Blobs[i].SetSpecies(i);  // Reset to unique species
    }

    // Check for blob proximity to merge species
    for (int i = 0; i < NUM_LLBLOBS - 1; i++)
    {
        for (int j = i + 1; j < NUM_LLBLOBS; j++)
        {
            // Calculate distance between blobs
            XMFLOAT3 pos1 = m_Blobs[i].GetPosition();
            XMFLOAT3 pos2 = m_Blobs[j].GetPosition();
            XMFLOAT3 diff = {
                pos1.x - pos2.x,
                pos1.y - pos2.y,
                pos1.z - pos2.z
            };

            float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

            if (prevSpecies[i] == prevSpecies[j])
            {
                // Were connected previously
                float radii = m_Blobs[i].GetRadius() + m_Blobs[j].GetRadius();
                float radiiSq = radii * radii;

                if (distSq < 0.9f * 0.9f * radiiSq)
                {
                    // Still touching - maintain same species
                    m_Blobs[j].SetSpecies(m_Blobs[i].GetSpecies());
                }
            }
            else
            {
                // Were not connected
                float checkRad = std::max(m_Blobs[i].GetRadius(), m_Blobs[j].GetRadius());
                checkRad += 0.7f * std::min(m_Blobs[i].GetRadius(), m_Blobs[j].GetRadius());

                if (distSq < checkRad * checkRad)
                {
                    // Connect them only if they are in the top or bottom
                    bool closeToEnd = false;

                    // Check blob i
                    float z = m_Blobs[i].GetPosition().z;
                    float r = m_Blobs[i].GetRadius();
                    closeToEnd = closeToEnd || (z - m_ConicSectionBotZ[0] < 1.5f * r);
                    closeToEnd = closeToEnd || (m_ConicSectionBotZ[m_NumConicSections] - z < 1.5f * r);

                    // Check blob j
                    z = m_Blobs[j].GetPosition().z;
                    r = m_Blobs[j].GetRadius();
                    closeToEnd = closeToEnd || (z - m_ConicSectionBotZ[0] < 1.5f * r);
                    closeToEnd = closeToEnd || (m_ConicSectionBotZ[m_NumConicSections] - z < 1.5f * r);

                    if (closeToEnd)
                    {
                        m_Blobs[j].SetSpecies(m_Blobs[i].GetSpecies());
                    }
                }
            }
        }
    }

    // Ensure consistent species references
    for (int i = 0; i < NUM_LLBLOBS; i++)
    {
        m_Blobs[i].SetSpecies(m_Blobs[m_Blobs[i].GetSpecies()].GetSpecies());
    }

    // Update blob colors based on species
    for (int i = 0; i < NUM_LLBLOBS; i++)
    {
        int s = m_Blobs[i].GetSpecies();

        // Create color based on species
        XMFLOAT4 color = {
            (s & 0x04) ? 0.0f : 1.0f,
            (s & 0x02) ? 0.0f : 1.0f,
            (s & 0x01) ? 0.0f : 1.0f,
            1.0f
        };

        // Base color
        XMFLOAT4 baseColor = { 0.724f, 0.732f, 0.556f, 1.0f };

        // Blend with base color (95% base, 5% species)
        XMFLOAT4 finalColor = {
            color.x * 0.05f + baseColor.x * 0.95f,
            color.y * 0.05f + baseColor.y * 0.95f,
            color.z * 0.05f + baseColor.z * 0.95f,
            color.w * 0.05f + baseColor.w * 0.95f
        };

        m_Blobs[i].SetColor(finalColor);
    }
}

///////////////////////////////////////////////////////////////////////////////
void LavaLampInterior::Render(ID3D11DeviceContext* pContext)
{
    // Set rendering state
    // Enable alpha blending
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    ID3D11BlendState* pPrevBlendState = nullptr;
    UINT sampleMask = 0xffffffff;
    pContext->OMGetBlendState(&pPrevBlendState, blendFactor, &sampleMask);

    // Use additive blending for glow effect
    ID3D11BlendState* pBlendState = nullptr;
    // TODO: Create or use blend state

    pContext->OMSetBlendState(pBlendState, blendFactor, sampleMask);

    // Set depth state
    ID3D11DepthStencilState* pPrevDepthState = nullptr;
    UINT stencilRef = 0;
    pContext->OMGetDepthStencilState(&pPrevDepthState, &stencilRef);

    // Allow transparent blobs to be visible through each other
    ID3D11DepthStencilState* pDepthState = nullptr;
    // TODO: Create or use depth state

    pContext->OMSetDepthStencilState(pDepthState, 0);

    // Set rasterizer state
    ID3D11RasterizerState* pPrevRasterState = nullptr;
    pContext->RSGetState(&pPrevRasterState);

    // Use backface culling
    ID3D11RasterizerState* pRasterState = nullptr;
    // TODO: Create or use rasterizer state

    pContext->RSSetState(pRasterState);

    // Set shaders
    pContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    pContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

    // Set texture
    ID3D11ShaderResourceView* pSRVs[] = { m_pNormCubemap.Get() };
    pContext->PSSetShaderResources(0, 1, pSRVs);

    // Render each blob
    for (int i = 0; i < NUM_LLBLOBS; i++)
    {
        m_Blobs[i].Render(pContext);
    }

    // Restore previous render states
    pContext->OMSetBlendState(pPrevBlendState, blendFactor, sampleMask);
    if (pPrevBlendState) pPrevBlendState->Release();

    pContext->OMSetDepthStencilState(pPrevDepthState, stencilRef);
    if (pPrevDepthState) pPrevDepthState->Release();

    pContext->RSSetState(pPrevRasterState);
    if (pPrevRasterState) pPrevRasterState->Release();

    // Clear resources
    ID3D11ShaderResourceView* pNullSRVs[] = { nullptr };
    pContext->PSSetShaderResources(0, 1, pNullSRVs);
}

///////////////////////////////////////////////////////////////////////////////
// LLBlob methods
///////////////////////////////////////////////////////////////////////////////
LLBlob::LLBlob()
    : m_NumVertices(0)
    , m_NumIndices(0)
    , m_Temperature(0.5f)
    , m_DeformationInertia(0.3f)
{
    m_Acceleration = { 0.0f, 0.0f, 0.0f };
    m_Velocity = { 0.0f, 0.0f, 0.0f };
    m_Position = { 0.0f, 0.0f, 0.0f };
    m_Scale = { 0.9f, 0.9f, 0.9f };
    m_BlobColor = { 1.0f, 1.0f, 1.0f, 1.0f };
}

///////////////////////////////////////////////////////////////////////////////
void LLBlob::Destroy()
{
    m_pVertexBuffer.Reset();
    m_pIndexBuffer.Reset();
}

///////////////////////////////////////////////////////////////////////////////
void LLBlob::CalculateFacePoint(XMFLOAT3* pPos, int face, int u, int v)
{
    // Cube faces from -1 to +1
    float fu = (u == m_Subdivisions) ? +1.0f : -1.0f + m_DivisionStep * ((float)u);
    float fv = (v == m_Subdivisions) ? +1.0f : -1.0f + m_DivisionStep * ((float)v);

    // Calculate point on each face
    switch (face)
    {
    case 0: *pPos = { -1.0f, -fu, +fv }; break;
    case 1: *pPos = { +fv, -1.0f, -fu }; break;
    case 2: *pPos = { -fu, +fv, -1.0f }; break;
    case 3: *pPos = { +1.0f, +fu, +fv }; break;
    case 4: *pPos = { +fv, +1.0f, +fu }; break;
    case 5: *pPos = { +fu, +fv, +1.0f }; break;
    }
}

///////////////////////////////////////////////////////////////////////////////
void LLBlob::Create(XMFLOAT3 pos, XMFLOAT4 color)
{
    // Store position and color
    m_Position = pos;
    m_BlobColor = color;
    m_Scale = { 0.9f, 0.9f, 0.9f };
    m_DeformationInertia = spLL->FRand11() * 0.1f;

    // Generate random radius
    m_Radius = spLL->FRand01();
    m_Radius = 0.5f * (m_Radius * m_Radius + spLL->FRand01());
    m_Radius = 0.03f + 0.05f * m_Radius;
    m_TemperatureAbsorbance = 0.05f / m_Radius;

    // Set initial temperature
    m_Temperature = 0.5f + 0.2f * spLL->FRand11();

    // Set subdivision level for sphere generation
    m_Subdivisions = 4;  // 4x4 grid of quads per face
    m_DivisionStep = 2.0f / m_Subdivisions;

    // Calculate vertex and index counts
    m_NumVertices = 6 * (m_Subdivisions + 1) * (m_Subdivisions + 1);
    m_NumIndices = 6 * (m_Subdivisions) * (m_Subdivisions) * 2 * 3;

    // TODO: Create vertex and index buffers using Direct3D 11
    // This would create a cube and then normalize vertices to form a sphere
}

///////////////////////////////////////////////////////////////////////////////
void LLBlob::AdvanceTime(float elapsedTime, float dt)
{
    // Calculate ambient temperature
    float ambientTemp = spLL->GetTemperature(m_Position.z);

    // Adjust blob temperature toward ambient
    float scale = 0.002f * m_TemperatureAbsorbance * m_TemperatureAbsorbance * dt;
    m_Temperature += scale * (ambientTemp - m_Temperature);

    // Apply buoyancy force - blobs lighter than water rise, heavier sink
    m_Velocity.z += dt * 1.0f * (m_Temperature - 0.5f);

    // Add small random forces
    m_Acceleration.x += spLL->FRand11() * dt;
    m_Acceleration.y += spLL->FRand11() * dt;
    m_Acceleration.z += spLL->FRand11() * dt;

    // Clamp acceleration
    m_Acceleration.x = std::min(+0.05f, std::max(-0.05f, m_Acceleration.x));
    m_Acceleration.y = std::min(+0.05f, std::max(-0.05f, m_Acceleration.y));
    m_Acceleration.z = std::min(+0.05f, std::max(-0.05f, m_Acceleration.z));

    // Scale acceleration if too large
    float accelMagSq = m_Acceleration.x * m_Acceleration.x +
        m_Acceleration.y * m_Acceleration.y +
        m_Acceleration.z * m_Acceleration.z;

    if (accelMagSq > 1.0f)
    {
        m_Acceleration.x *= 0.96f;
        m_Acceleration.y *= 0.96f;
        m_Acceleration.z *= 0.96f;
    }

    // Apply acceleration to velocity
    m_Velocity.x += m_Acceleration.x * dt;
    m_Velocity.y += m_Acceleration.y * dt;
    m_Velocity.z += m_Acceleration.z * dt;

    // Apply drag force
    float velMagSq = m_Velocity.x * m_Velocity.x +
        m_Velocity.y * m_Velocity.y +
        m_Velocity.z * m_Velocity.z;

    float drag = 1.0f - dt * 120.0f * velMagSq;
    m_Velocity.x *= drag;
    m_Velocity.y *= drag;
    m_Velocity.z *= drag;

    // Update position
    m_Position.x += m_Velocity.x * dt;
    m_Position.y += m_Velocity.y * dt;
    m_Position.z += m_Velocity.z * dt;

    // Check for collisions
    spLL->Collide(this, m_Position.x, m_Position.y, m_Position.z, m_Radius, dt);

    // Adjust blob shape (wobble effect)
    m_Scale.x += m_DeformationInertia * dt * m_TemperatureAbsorbance;
    m_Scale.y += m_DeformationInertia * dt * m_TemperatureAbsorbance;

    // Maintain volume by adjusting z-scale
    m_Scale.z = 0.9f - (m_Scale.x - 0.9f) * (0.9f + 0.9f) * 0.9f / (0.9f * 0.9f);

    // Apply spring force to return to spherical shape
    float accel;
    if (m_DeformationInertia > 0.0f)
    {
        accel = 0.91f - m_Scale.x;
    }
    else
    {
        accel = 0.89f - m_Scale.x;
    }

    // Update deformation
    m_DeformationInertia += 20.0f * accel * dt;
    m_DeformationInertia = std::max(-0.3f, std::min(+0.3f, m_DeformationInertia));
}

///////////////////////////////////////////////////////////////////////////////
void LLBlob::Collided(XMFLOAT3 pos, XMFLOAT3 normal)
{
    // Check if collision is valid
    XMFLOAT3 diff = {
        pos.x - m_Position.x,
        pos.y - m_Position.y,
        pos.z - m_Position.z
    };

    float diffMagSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
    if (diffMagSq > 0.5f * m_Radius * m_Radius)
    {
        // Collision point too far from center - ignore
        return;
    }

    // Calculate reflection of velocity and acceleration
    float dotAccel = m_Acceleration.x * normal.x +
        m_Acceleration.y * normal.y +
        m_Acceleration.z * normal.z;

    if (dotAccel < 0.0f)
    {
        // Reflect acceleration
        m_Acceleration.x -= normal.x * dotAccel;
        m_Acceleration.y -= normal.y * dotAccel;
        m_Acceleration.z -= normal.z * dotAccel;
    }

    float dotVel = m_Velocity.x * normal.x +
        m_Velocity.y * normal.y +
        m_Velocity.z * normal.z;

    if (dotVel < 0.0f)
    {
        // Reflect velocity
        m_Velocity.x -= normal.x * dotVel;
        m_Velocity.y -= normal.y * dotVel;
        m_Velocity.z -= normal.z * dotVel;
    }

    // Set new position
    m_Position = pos;
}

///////////////////////////////////////////////////////////////////////////////
void LLBlob::InteractWithBlob(const LLBlob* pllb, float dt)
{
    // Calculate mass of other blob
    float massB = pllb->m_Radius;
    massB *= massB * massB;  // Mass proportional to volume

    // Calculate vector between blobs
    XMFLOAT3 delta = {
        pllb->m_Position.x - m_Position.x,
        pllb->m_Position.y - m_Position.y,
        pllb->m_Position.z - m_Position.z
    };

    float distSq = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
    if (distSq < 0.000001f) return;  // Too close to calculate

    // Calculate relative velocity
    XMFLOAT3 deltaV = {
        pllb->m_Velocity.x - m_Velocity.x,
        pllb->m_Velocity.y - m_Velocity.y,
        pllb->m_Velocity.z - m_Velocity.z
    };

    // Project relative velocity onto separation vector
    float dotProduct = deltaV.x * delta.x + deltaV.y * delta.y + deltaV.z * delta.z;

    // Interaction depends on species
    if (GetSpecies() == pllb->GetSpecies())
    {
        // Same species - attract more when moving together, less when moving apart
        float extremeRad = (m_Radius + pllb->m_Radius);
        float attract = massB * dt * 10000.0f * ((dotProduct > 0.0f) ? 1.0f : 0.5f);

        // Apply attractive force
        m_Velocity.x += delta.x * attract;
        m_Velocity.y += delta.y * attract;
        m_Velocity.z += delta.z * attract;

        // Apply repulsion when very close
        extremeRad *= 0.6f;
        float extremeRadSq = extremeRad * extremeRad;
        float repel = ((1.0f / std::min(extremeRadSq * 0.04f, distSq)) - (1.0f / extremeRadSq)) *
            massB * dt * 5.0f;

        repel *= ((dotProduct > 0.0f) ? 0.3f : 1.0f);

        if (repel > 0.0f)
        {
            m_Velocity.x -= delta.x * repel;
            m_Velocity.y -= delta.y * repel;
            m_Velocity.z -= delta.z * repel;
        }
    }
    else
    {
        // Different species - repel
        float extremeRadSq = (m_Radius + pllb->m_Radius);
        extremeRadSq *= extremeRadSq;

        float repel = ((1.0f / std::min(extremeRadSq * 0.04f, distSq)) - (1.0f / extremeRadSq)) *
            massB * dt * 1.0f;

        repel *= ((dotProduct > 0.0f) ? 0.3f : 1.0f);

        m_Velocity.x -= delta.x * repel;
        m_Velocity.y -= delta.y * repel;
        m_Velocity.z -= delta.z * repel;
    }
}

///////////////////////////////////////////////////////////////////////////////
void LLBlob::Render(ID3D11DeviceContext* pContext)
{
    // Create world transformation matrix
    XMMATRIX world = XMMatrixScaling(m_Scale.x * m_Radius, m_Scale.y * m_Radius, m_Scale.z * m_Radius) *
        XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

    // Update constant buffer with world transform and blob color
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if (SUCCEEDED(pContext->Map(spLL->m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
    {
        // First slot in constant buffer is world matrix
        XMMATRIX* pMatrix = (XMMATRIX*)mappedResource.pData;
        *pMatrix = XMMatrixTranspose(world);

        // Update lighting parameters
        XMFLOAT3* pLight1 = (XMFLOAT3*)((char*)mappedResource.pData + sizeof(XMMATRIX) * 3);
        *pLight1 = { 0.5f, 0.6f, 0.5f };

        XMFLOAT3* pLight2 = (XMFLOAT3*)((char*)mappedResource.pData + sizeof(XMMATRIX) * 3 + sizeof(XMFLOAT3));
        *pLight2 = { 0.5f, 0.4f, 0.5f };

        // Update blob color
        XMFLOAT4* pColor = (XMFLOAT4*)((char*)mappedResource.pData + sizeof(XMMATRIX) * 3 + sizeof(XMFLOAT3) * 2);
        *pColor = m_BlobColor;

        // Update ambient light
        XMFLOAT4* pAmbient = (XMFLOAT4*)((char*)mappedResource.pData + sizeof(XMMATRIX) * 3 + sizeof(XMFLOAT3) * 2 + sizeof(XMFLOAT4));
        pAmbient->x = m_BlobColor.x * 0.6f;
        pAmbient->y = m_BlobColor.y * 0.6f;
        pAmbient->z = m_BlobColor.z * 0.6f;
        pAmbient->w = m_BlobColor.w;

        pContext->Unmap(spLL->m_pConstantBuffer.Get(), 0);
    }

    // Set constant buffer
    pContext->VSSetConstantBuffers(0, 1, spLL->m_pConstantBuffer.GetAddressOf());
    pContext->PSSetConstantBuffers(0, 1, spLL->m_pConstantBuffer.GetAddressOf());

    // Set vertex and index buffers
    UINT stride = sizeof(BlobVertex);
    UINT offset = 0;
    pContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    pContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

    // Draw blob
    pContext->DrawIndexed(m_NumIndices, 0, 0);
}