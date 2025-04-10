///////////////////////////////////////////////////////////////////////////////
// File: CamControl.h - Modern Windows 11 port
//
// Copyright 2001 Pipeworks Software (Original)
// Modern Port Copyright (c) 2023
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <DirectXMath.h>
#include <vector>
#include "defines.h"

///////////////////////////////////////////////////////////////////////////////
// CamControlNodeData - data for camera path nodes
///////////////////////////////////////////////////////////////////////////////
struct CamControlNodeData
{
    unsigned char   ucTime;              // Time when the camera arrives here (0 indicates start of new path, 100 is start of finalization)
    signed char     scTension, scBias;   // Tension and bias of point, from -100 to +100
    DirectX::XMFLOAT3 ptPosition;        // Position of node
    DirectX::XMFLOAT3 vecLookAt;         // Position the camera looks at
};

///////////////////////////////////////////////////////////////////////////////
// CamControlNode - runtime node with additional data for interpolation
///////////////////////////////////////////////////////////////////////////////
struct CamControlNode
{
    float               fTime;          // Time when the camera arrives here
    DirectX::XMFLOAT3   ptPosition;     // Position of node
    DirectX::XMFLOAT3   vecVelocity;    // Velocity at this point (for Hermite interpolation)
    DirectX::XMFLOAT3   vecLookAt;      // Position the camera looks at
    DirectX::XMFLOAT3   vecLookAtW;     // Velocity of what the camera looks at
    float               tension, bias;   // Interpolation parameters
};

///////////////////////////////////////////////////////////////////////////////
// CameraController - Manages camera paths and interpolation
//
// The camera system uses a series of paths with Hermite interpolation to create
// smooth camera movement. Each path has a series of nodes with position, lookat
// point, and interpolation parameters.
//
// The finish nodes are:
//   0: before the beginning of the slash, anything before this doesn't need to render the slash
//   1: after exiting slash
//   2: translated down, but still looking at center (but it is eclipsed by slash geometry) 
//   3: partly rotated to final position, looking at slash center now
//   4: final position
//   5: final position, with high time value (the endcap)
///////////////////////////////////////////////////////////////////////////////
class CameraController
{
private:
    static CamControlNodeData    m_CameraListData[];
    static CamControlNode        m_CameraList[];

    static constexpr int NUM_FINISH_NODES = 8;
    CamControlNode               m_FinishNodes[NUM_FINISH_NODES];
    DirectX::XMMATRIX            m_xfSlash;
    DirectX::XMFLOAT3            m_ptSlashCenter;
    DirectX::XMFLOAT3            m_ptFinalLookAt;

    int                         m_numNodes;
    int                         m_numPaths;

    int                         m_curPathNum;
    int                         m_curStartNode;
    int                         m_curNumNodes;
    int                         m_curVariableNodes;

    float                       m_fCameraLookatInterpStart;
    float                       m_fOOCameraLookatInterpDelta;

    // Helper method to get a node (either from camera list or finish nodes)
    CamControlNode* GetNode(int i);
    const CamControlNode* GetNode(int i) const;

public:
    CameraController();
    ~CameraController();

    void Init();
    void Uninit();

    // Called when button is pressed in debug mode
    void ButtonPressed();

    // Select a camera path (negative indicates random)
    void PickPath(int path = -1);

    // Get camera position and lookat point at time t
    void GetPosition(float t, DirectX::XMFLOAT3* p_pos, DirectX::XMFLOAT3* p_look, bool* pb_render_geom, bool* pb_render_slash);

    // Get slash transformation matrix
    const DirectX::XMMATRIX& GetSlashTransform() const { return m_xfSlash; }
};