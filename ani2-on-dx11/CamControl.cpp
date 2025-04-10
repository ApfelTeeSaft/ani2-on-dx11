///////////////////////////////////////////////////////////////////////////////
// File: CamControl.cpp - Modern Windows 11 port
//
// Copyright 2001 Pipeworks Software (Original)
// Modern Port Copyright (c) 2023
///////////////////////////////////////////////////////////////////////////////
#include "CamControl.h"
#include <random>
#include <algorithm>

using namespace DirectX;

// The camera path data has been preserved from the original Xbox codebase
// but converted to use XMFLOAT3 for position and lookat points
CamControlNodeData CameraController::m_CameraListData[] =
{
    // Top, pan down, pull out.
    {  0,   +00, +00,       {+11.4f,   -32.1f,   +33.0f},       {+0.0f,    +0.0f,    +0.0f} },
    { 20,   +00, +00,       {+13.4f,   -37.7f,   +25.6f},       {+0.0f,    +0.0f,    +0.0f} },
    { 40,   +00, +00,       {+15.6f,   -43.9f,    +8.8f},       {+0.0f,    +0.0f,    +0.0f} },
    { 60,   +00, +00,       {+16.0f,   -45.0f,   -12.8f},       {+0.0f,    +0.0f,    +0.0f} },
    { 90,   +00, +00,       {+18.2f,   -51.2f,   -29.6f},       {+0.0f,    +0.0f,    +0.0f} },

    // Start low near tube, pause, quickly go up and to the right, pause, pull out.
    {  00,   +0, +0,       {-55.4f,   +19.7f,   -31.5f},       {+0.0f,    +0.0f,    +0.0f} },
    {  30,   +0, +0,       {-55.4f,   +19.7f,   -31.5f},       {+0.0f,    +0.0f,    +0.0f} },
    {  45,   +0, +0,       {-39.5f,    -0.6f,    -7.8f},       {+0.0f,    +0.0f,    +0.0f} },
    {  60,   +0, +0,        {-4.3f,   -35.5f,   +16.6f},       {+0.0f,    +0.0f,    +0.0f} },
    {  70,   +0, +0,       {+31.1f,   -32.6f,   +17.6f},       {+0.0f,    +0.0f,    +0.0f} },
    {  80,   +0, +0,       {+57.7f,    -7.2f,    +3.3f},       {+0.0f,    +0.0f,    +0.0f} },
    {  95,   +0, +0,       {+70.9f,    +1.8f,    +3.1f},       {+0.0f,    +0.0f,    +0.0f} },

    // Rotate to left, fairly close.
    {  00,   +0, +0,       {+34.7f,   +25.9f,   +12.3f},       {+0.0f,    +0.0f,    +0.0f} },
    {  25,   +0, +0,       {+42.3f,    +9.3f,   +12.3f},       {+0.0f,    +0.0f,    +0.0f} },
    {  50,   +0, +0,       {+42.4f,    -8.8f,   +12.3f},       {+0.0f,    +0.0f,    +0.0f} },
    {  75,   +0, +0,       {+34.4f,   -26.3f,   +12.3f},       {+0.0f,    +0.0f,    +0.0f} },
    {  95,   +0, +0,       {+30.7f,   -48.1f,   +14.3f},       {+0.0f,    +0.0f,    +0.0f} },

    // Start out low, pause, rotate up slightly and pull out.
    {  0,   +00, +00,       {-50.1f,    -0.3f,   -51.5f},       {+0.0f,    +0.0f,    +0.0f} },
    {  25,  +00, +00,       {-50.1f,    -0.3f,   -51.5f},       {+0.0f,    +0.0f,    +0.0f} },
    {  75,  +00, +00,       {-50.1f,    -0.3f,   -51.5f},       {+0.0f,    +0.0f,    +0.0f} },
    {  95,  +00, +00,       {-62.2f,    -0.4f,   -12.0f},       {+0.0f,    +0.0f,    +0.0f} },
};

// Define the number of camera nodes
#define NUM_CC_NODES  (sizeof(CameraController::m_CameraListData)/sizeof(CamControlNodeData))

// Static array to store camera list nodes
CamControlNode CameraController::m_CameraList[NUM_CC_NODES];

///////////////////////////////////////////////////////////////////////////////
CameraController::CameraController()
{
    m_numNodes = 0;
    m_numPaths = 0;
    m_curPathNum = -1;
    m_curStartNode = 0;
    m_curNumNodes = 0;
    m_curVariableNodes = 0;
    m_fCameraLookatInterpStart = 0.0f;
    m_fOOCameraLookatInterpDelta = 0.0f;

    // Initialize the matrices
    m_xfSlash = XMMatrixIdentity();
}

///////////////////////////////////////////////////////////////////////////////
CameraController::~CameraController()
{
    Uninit();
}

///////////////////////////////////////////////////////////////////////////////
void CameraController::Init()
{
    m_numNodes = NUM_CC_NODES;
    m_numPaths = 0;
    m_curPathNum = -1;

    // Initialize camera list from the static data
    for (int i = 0; i < m_numNodes; i++)
    {
        // Count paths (new path starts when time is 0)
        if (m_CameraListData[i].ucTime == 0)
            m_numPaths++;

        // Convert time from percentage to seconds
        m_CameraList[i].fTime = FINISH_START_TIME * ((float)m_CameraListData[i].ucTime) * 0.01f;

        // Copy position and lookat
        m_CameraList[i].ptPosition = m_CameraListData[i].ptPosition;
        m_CameraList[i].vecLookAt = m_CameraListData[i].vecLookAt;

        // Convert tension and bias from -100..100 to -1..1
        m_CameraList[i].tension = ((float)m_CameraListData[i].scTension) * 0.01f;
        m_CameraList[i].bias = ((float)m_CameraListData[i].scBias) * 0.01f;
    }

    // Pick a random path to start
    PickPath(-1);
}

///////////////////////////////////////////////////////////////////////////////
void CameraController::Uninit()
{
    // No special cleanup needed
}

///////////////////////////////////////////////////////////////////////////////
// Get a node (either from camera list or finish nodes)
CamControlNode* CameraController::GetNode(int i)
{
    return (i < m_curVariableNodes) ? &m_CameraList[i + m_curStartNode] : &m_FinishNodes[i - m_curVariableNodes];
}

///////////////////////////////////////////////////////////////////////////////
// Get a node (const version)
const CamControlNode* CameraController::GetNode(int i) const
{
    return (i < m_curVariableNodes) ? &m_CameraList[i + m_curStartNode] : &m_FinishNodes[i - m_curVariableNodes];
}

///////////////////////////////////////////////////////////////////////////////
// Pick a camera path (negative indicates random)
void CameraController::PickPath(int path)
{
    // If path is negative, pick a random path
    if (path < 0)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, m_numPaths - 1);
        path = dist(gen);
    }

    // Ensure path is in valid range
    if (path >= m_numPaths)
        path = path % m_numPaths;

    m_curPathNum = path;

    // Find the start node for this path
    int i;
    for (i = 0; i < m_numNodes; i++)
    {
        if (m_CameraList[i].fTime == 0.0f)
        {
            if (!path) break;
            path--;
        }
    }
    m_curStartNode = i;

    // Find the end node for this path
    for (i = m_curStartNode + 1; i < m_numNodes; i++)
    {
        if (m_CameraList[i].fTime == 0.0f) break;
    }
    m_curVariableNodes = i - m_curStartNode;
    m_curNumNodes = m_curVariableNodes + NUM_FINISH_NODES;

    // Calculate the finish nodes
    for (int j = 0; j < NUM_FINISH_NODES; j++)
    {
        m_FinishNodes[j].fTime = FINISH_START_TIME + FINISH_TRANSITION_TIME * ((float)j) / ((float)(NUM_FINISH_NODES - 1));
        m_FinishNodes[j].tension = +0.0f;
        m_FinishNodes[j].bias = 0.0f;
    }

    m_fCameraLookatInterpStart = m_FinishNodes[2].fTime;
    m_fOOCameraLookatInterpDelta = 1.0f / (m_FinishNodes[5].fTime - m_fCameraLookatInterpStart);

    // Get the last variable node
    const CamControlNode* plast = &m_CameraList[m_curStartNode + m_curVariableNodes - 1];
    CamControlNode* pthis = &m_FinishNodes[0];

    float start_time = plast->fTime;

    // Constants for slash animation
    const float slash_start_rad = -95.0f;
    const float slash_end_rad = 132.14f;

    const float cfYPositions[NUM_FINISH_NODES] = { +95.0f, +30.548f, -70.819f, -150.298f, -220.64f, -243.021f, -261.441f, -287.773f };
    const float cfZPositions[NUM_FINISH_NODES] = { 0.0f,   0.322f,   1.821f, 2.323f,    -11.926f,  -39.973f,  -60.774f,  -90.795f };
    const float camera_end_coord_y = cfYPositions[NUM_FINISH_NODES - 1];
    const float camera_end_coord_z = cfZPositions[NUM_FINISH_NODES - 1];

    const float lookat_offset = slash_end_rad * camera_end_coord_z / camera_end_coord_y;
    const float cfSlashDist = slash_end_rad - slash_start_rad;
    const float cfMinStartDist = 100.0f;

    // First finish node is at the entrance of the slash
    pthis->ptPosition = plast->ptPosition;

    // Calculate velocity from last two nodes
    XMFLOAT3 vel = { 0.0f, 0.0f, 0.0f };
    if (m_curVariableNodes >= 2)
    {
        // Calculate velocity from the last two nodes
        XMFLOAT3 p1 = GetNode(m_curVariableNodes - 1)->ptPosition;
        XMFLOAT3 p2 = GetNode(m_curVariableNodes - 2)->ptPosition;
        float dt = GetNode(m_curVariableNodes - 1)->fTime - GetNode(m_curVariableNodes - 2)->fTime;

        // v = (p1 - p2) / dt
        vel.x = (p1.x - p2.x) / dt;
        vel.y = (p1.y - p2.y) / dt;
        vel.z = (p1.z - p2.z) / dt;
    }

    // Add scaled velocity to position
    pthis->ptPosition.x += vel.x * (pthis->fTime - plast->fTime) * 0.7f;
    pthis->ptPosition.y += vel.y * (pthis->fTime - plast->fTime) * 0.7f;
    pthis->ptPosition.z += vel.z * (pthis->fTime - plast->fTime) * 0.7f;

    // Normalize the position
    XMVECTOR posVec = XMLoadFloat3(&pthis->ptPosition);
    float vel_adj_len = XMVectorGetX(XMVector3Length(posVec));
    posVec = XMVector3Normalize(posVec);

    // Store slash direction
    XMFLOAT3 slash_dir;
    XMStoreFloat3(&slash_dir, posVec);

    // Adjust position based on slash parameters
    float slash_y_offset = std::max(cfMinStartDist - slash_start_rad, vel_adj_len * 1.2f - slash_start_rad);

    // Scale position by radius + offset
    posVec = XMVectorScale(posVec, slash_y_offset + slash_start_rad);
    XMStoreFloat3(&pthis->ptPosition, posVec);

    // Set lookat point to origin
    pthis->vecLookAt = { 0.0f, 0.0f, 0.0f };

    // Calculate slash transform matrix
    XMVECTOR origin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    XMVECTOR up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

    // The shape has negative y going away from core.
    // Negative Y is normalized vector from slash_pos to origin
    XMVECTOR y_dir = XMLoadFloat3(&slash_dir);
    y_dir = XMVectorNegate(y_dir);

    // X is y_dir crossed with up and normalized
    XMVECTOR x_dir = XMVector3Cross(y_dir, up);
    x_dir = XMVector3Normalize(x_dir);

    // Z is x_dir cross y_dir
    XMVECTOR z_dir = XMVector3Cross(x_dir, y_dir);

    // Build the transformation matrix
    m_xfSlash = XMMatrixIdentity();

    // Set the rotation part of the matrix
    m_xfSlash.r[0] = XMVectorSetW(x_dir, 0.0f);
    m_xfSlash.r[1] = XMVectorSetW(y_dir, 0.0f);
    m_xfSlash.r[2] = XMVectorSetW(z_dir, 0.0f);

    // Set the translation part of the matrix
    XMVECTOR translation = XMVectorScale(y_dir, -slash_y_offset);
    m_xfSlash.r[3] = XMVectorSetW(translation, 1.0f);

    // Calculate slash center
    XMVECTOR slashCenter = XMVectorScale(y_dir, -(slash_end_rad + slash_y_offset));
    XMStoreFloat3(&m_ptSlashCenter, slashCenter);

    // Calculate positions for the rest of the finish nodes
    float y_basis = slash_end_rad;
    for (int j = 1; j < NUM_FINISH_NODES; j++)
    {
        plast = pthis++;

        // Position in slash space
        XMVECTOR pt_in_slash = XMVectorSet(0.0f, cfYPositions[j] + y_basis, cfZPositions[j], 1.0f);

        // Transform to world space
        XMVECTOR worldPos = XMVector3Transform(pt_in_slash, m_xfSlash);
        worldPos = XMVectorAdd(worldPos, XMLoadFloat3(&m_ptSlashCenter));

        // Store position
        XMStoreFloat3(&pthis->ptPosition, worldPos);

        // Set lookat point to origin
        pthis->vecLookAt = { 0.0f, 0.0f, 0.0f };
    }

    // Set the final lookat point
    XMVECTOR t = XMVectorSet(0.0f, slash_end_rad, 25.0f, 1.0f);
    XMVECTOR finalLookAt = XMVector3Transform(t, m_xfSlash);
    finalLookAt = XMVectorAdd(finalLookAt, XMLoadFloat3(&m_ptSlashCenter));
    XMStoreFloat3(&m_ptFinalLookAt, finalLookAt);

    // Set the "m" parameters implicitly for Hermite interpolation
    for (int j = 0; j < m_curNumNodes; j++)
    {
        CamControlNode* pthis = GetNode(j);

        // Initialize velocities to zero
        pthis->vecVelocity = { 0.0f, 0.0f, 0.0f };
        pthis->vecLookAtW = { 0.0f, 0.0f, 0.0f };

        if (j > 0)
        {
            // Calculate velocity contribution from previous node
            const CamControlNode* prev = GetNode(j - 1);

            float scale = (1.0f - pthis->tension) * (1.0f + pthis->bias) * 0.5f;

            // Position velocity
            pthis->vecVelocity.x += (pthis->ptPosition.x - prev->ptPosition.x) * scale;
            pthis->vecVelocity.y += (pthis->ptPosition.y - prev->ptPosition.y) * scale;
            pthis->vecVelocity.z += (pthis->ptPosition.z - prev->ptPosition.z) * scale;

            // Lookat velocity
            pthis->vecLookAtW.x += (pthis->vecLookAt.x - prev->vecLookAt.x) * scale;
            pthis->vecLookAtW.y += (pthis->vecLookAt.y - prev->vecLookAt.y) * scale;
            pthis->vecLookAtW.z += (pthis->vecLookAt.z - prev->vecLookAt.z) * scale;
        }

        if (j < m_curNumNodes - 1)
        {
            // Calculate velocity contribution from next node
            const CamControlNode* next = GetNode(j + 1);

            float scale = (1.0f - pthis->tension) * (1.0f - pthis->bias) * 0.5f;

            // Position velocity
            pthis->vecVelocity.x += (next->ptPosition.x - pthis->ptPosition.x) * scale;
            pthis->vecVelocity.y += (next->ptPosition.y - pthis->ptPosition.y) * scale;
            pthis->vecVelocity.z += (next->ptPosition.z - pthis->ptPosition.z) * scale;

            // Lookat velocity
            pthis->vecLookAtW.x += (next->vecLookAt.x - pthis->vecLookAt.x) * scale;
            pthis->vecLookAtW.y += (next->vecLookAt.y - pthis->vecLookAt.y) * scale;
            pthis->vecLookAtW.z += (next->vecLookAt.z - pthis->vecLookAt.z) * scale;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Get camera position and lookat point at time t
void CameraController::GetPosition(float t, XMFLOAT3* p_pos, XMFLOAT3* p_look, bool* pb_render_geom, bool* pb_render_slash)
{
    // If we're past the stop time, return the final position
    if (t > FINISH_STOP_TIME)
    {
        *p_pos = m_FinishNodes[NUM_FINISH_NODES - 1].ptPosition;
        *p_look = m_ptFinalLookAt;
        *pb_render_slash = true;
        *pb_render_geom = false;
        return;
    }

    // Find which nodes to interpolate between
    int i;
    for (i = 1; i < m_curNumNodes; i++)
    {
        if (GetNode(i)->fTime > t) break;
    }

    // Handle edge cases
    if (i == m_curNumNodes)
    {
        *p_pos = GetNode(m_curNumNodes - 1)->ptPosition;
        *p_look = m_ptFinalLookAt;
        *pb_render_slash = true;
        *pb_render_geom = false;
        return;
    }
    if (i == 0)
    {
        // Should never happen, but handle it safely
        p_pos->x = 0.0f;
        p_pos->y = -90.0f;
        p_pos->z = 0.0f;
        p_look->x = 0.0f;
        p_look->y = 0.0f;
        p_look->z = 0.0f;
        *pb_render_slash = true;
        *pb_render_geom = false;
        return;
    }

    // Get the nodes to interpolate between
    const CamControlNode* pprev = GetNode(i - 1);
    const CamControlNode* pnext = GetNode(i);

    // Calculate time deltas for interpolation
    float dtc = std::max(0.001f, pnext->fTime - pprev->fTime);
    float dtp = std::max(0.001f, (i >= 2) ? pprev->fTime - GetNode(i - 2)->fTime : dtc);
    float dtn = std::max(0.001f, (i < m_curNumNodes - 1) ? GetNode(i + 1)->fTime - pnext->fTime : dtc);

    // Calculate normalized time for interpolation
    float uts = std::min(1.0f, std::max(0.0f, (t - pprev->fTime) / dtc));
    float utss = uts * uts;
    float utsss = utss * uts;
    float frac = -2.0f * utsss + 3.0f * utss;
    float s = (t - pprev->fTime) / ((1.0f - frac) * dtp + frac * dtc);

    // Hermite interpolation coefficients
    float ss = s * s;
    float sss = ss * s;
    float cA = 2.0f * sss - 3.0f * ss + 1.0f;
    float cB = sss - 2.0f * ss + s;
    float cC = sss - ss;
    float cD = -2.0f * sss + 3.0f * ss;

    // Interpolate position
    p_pos->x = cA * pprev->ptPosition.x + cB * pprev->vecVelocity.x + cC * pnext->vecVelocity.x + cD * pnext->ptPosition.x;
    p_pos->y = cA * pprev->ptPosition.y + cB * pprev->vecVelocity.y + cC * pnext->vecVelocity.y + cD * pnext->ptPosition.y;
    p_pos->z = cA * pprev->ptPosition.z + cB * pprev->vecVelocity.z + cC * pnext->vecVelocity.z + cD * pnext->ptPosition.z;

    // Interpolate look-at position
    p_look->x = cA * pprev->vecLookAt.x + cB * pprev->vecLookAtW.x + cC * pnext->vecLookAtW.x + cD * pnext->vecLookAt.x;
    p_look->y = cA * pprev->vecLookAt.y + cB * pprev->vecLookAtW.y + cC * pnext->vecLookAtW.y + cD * pnext->vecLookAt.y;
    p_look->z = cA * pprev->vecLookAt.z + cB * pprev->vecLookAtW.z + cC * pnext->vecLookAtW.z + cD * pnext->vecLookAt.z;

    // Calculate lookat interpolation with final lookat point
    float sl = std::max(0.0f, std::min(1.0f, (t - m_fCameraLookatInterpStart) * m_fOOCameraLookatInterpDelta));

    // Smooth interpolation using cosine
    float interp = 0.5f * (1.0f - cosf(sl * XM_PI));

    // Blend between interpolated lookat and final lookat
    XMVECTOR lookVec = XMLoadFloat3(p_look);
    XMVECTOR finalLookVec = XMLoadFloat3(&m_ptFinalLookAt);

    lookVec = XMVectorScale(lookVec, 1.0f - interp);
    finalLookVec = XMVectorScale(finalLookVec, interp);
    lookVec = XMVectorAdd(lookVec, finalLookVec);

    XMStoreFloat3(p_look, lookVec);

    // Set render flags based on current position in animation
    *pb_render_slash = (i > m_curVariableNodes - 1);
    *pb_render_geom = (i < m_curNumNodes - 1);
}

///////////////////////////////////////////////////////////////////////////////
void CameraController::ButtonPressed()
{
#ifdef INCLUDE_PLACEMENT_DOODAD
    // Debug functionality to output camera position for placement tool
    XMFLOAT3 pos;
    XMFLOAT3 la;

    // TODO: Get current camera position from app
    //app.theCamera.GetCameraPosition(&pos);
    //la = app.GetLookatPoint();

    // Output formatted position for copy/paste into camera data array
    char buf[1024];
    sprintf_s(buf, 1024, "    {  0,   +00, +00,    {%+8.1ff,%+8.1ff,%+8.1ff},   {%+8.1ff,%+8.1ff,%+8.1ff} },\n",
        pos.x, pos.y, pos.z, la.x, la.y, la.z);

    // Output to debug window
    OutputDebugStringA(buf);
#endif
}