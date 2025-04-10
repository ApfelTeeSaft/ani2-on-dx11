//
//	camera.h - Modern Windows 11 port
//
///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2001, Pipeworks Software Inc. (Original)
//  Modern Port Copyright (c) 2023
//				All rights reserved
#pragma once

#include <DirectXMath.h>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
class Camera
{
private:
    // Frustum planes for culling
    std::vector<DirectX::XMVECTOR> m_ClipPlanes;
    bool         m_bClipPlanesValid;
    float        m_fNearPlane;
    float        m_fFarPlane;
    float        m_fAspectRatio;
    float        m_fFieldOfView;

    bool         m_bViewProjMatrixValid;

    DirectX::XMVECTOR m_vPosition;
    DirectX::XMVECTOR m_vLookAt;
    DirectX::XMVECTOR m_vUp;

    DirectX::XMMATRIX m_mViewProjMatrix;

public:
    // Matrices
    DirectX::XMMATRIX m_mCameraToWorld;  // Was matCTW
    DirectX::XMMATRIX m_mWorldToCamera;  // Was matWTC
    DirectX::XMMATRIX m_mProjection;     // Was matProj

    Camera();
    ~Camera();

    void Init();
    void Uninit();

    // Sets up the camera view
    void LookAt(const DirectX::XMFLOAT3& camPos, const DirectX::XMFLOAT3& lookPt, const DirectX::XMFLOAT3& up);
    void LookAt(DirectX::FXMVECTOR camPos, DirectX::FXMVECTOR lookPt, DirectX::FXMVECTOR up);

    // Sets up projection matrix
    void SetProjection(float fovInRadiansY, float aspectRatio, float nearPlane, float farPlane);

    // Movement functions
    void Translate(DirectX::FXMVECTOR relativeVector);

    // Matrix setters directly
    void SetCameraToWorldMatrix(DirectX::FXMMATRIX ctw);
    void SetWorldToCameraMatrix(DirectX::FXMMATRIX wtc);

    // Visibility checking
    bool SphereVisibilityCheck(DirectX::FXMVECTOR position, float radius);
    void UpdateClipPlanes();

    // Helper functions
    float GetPixelScaleForZ(float z) const;
    DirectX::XMMATRIX GetViewProjectionMatrix();

    // Accessors
    void GetCameraPosition(DirectX::XMFLOAT3* pPos) const;
    void GetCameraLookAt(DirectX::XMFLOAT3* pLook) const;
    DirectX::XMVECTOR GetCameraPositionVector() const { return m_vPosition; }
    DirectX::XMVECTOR GetCameraLookAtVector() const { return m_vLookAt; }
    float GetFarPlane() const { return m_fFarPlane; }
    float GetNearPlane() const { return m_fNearPlane; }
    float GetAspectRatio() const { return m_fAspectRatio; }
    float GetFieldOfView() const { return m_fFieldOfView; }
};