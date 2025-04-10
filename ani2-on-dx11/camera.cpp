//
//	camera.cpp - Modern Windows 11 port
//
///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2001, Pipeworks Software Inc. (Original)
//  Modern Port Copyright (c) 2023
//				All rights reserved

#include "camera.h"

using namespace DirectX;

///////////////////////////////////////////////////////////////////////////////
Camera::Camera()
{
    m_bClipPlanesValid = false;
    m_bViewProjMatrixValid = false;
    m_ClipPlanes.resize(6); // 6 frustum planes
    Init();
}

///////////////////////////////////////////////////////////////////////////////
Camera::~Camera()
{
    Uninit();
}

///////////////////////////////////////////////////////////////////////////////
void Camera::Init()
{
    m_bClipPlanesValid = false;
    m_bViewProjMatrixValid = false;

    // Initialize with identity matrices
    m_mCameraToWorld = XMMatrixIdentity();
    m_mWorldToCamera = XMMatrixIdentity();
    m_mProjection = XMMatrixIdentity();

    // Default camera values
    m_vPosition = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    m_vLookAt = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
    m_vUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    m_fNearPlane = 0.1f;
    m_fFarPlane = 1000.0f;
    m_fAspectRatio = 16.0f / 9.0f;
    m_fFieldOfView = XM_PIDIV4; // 45 degrees
}

///////////////////////////////////////////////////////////////////////////////
void Camera::Uninit()
{
    // Clean up any resources
}

///////////////////////////////////////////////////////////////////////////////
void Camera::LookAt(const XMFLOAT3& camPos, const XMFLOAT3& lookPt, const XMFLOAT3& up)
{
    LookAt(XMLoadFloat3(&camPos), XMLoadFloat3(&lookPt), XMLoadFloat3(&up));
}

///////////////////////////////////////////////////////////////////////////////
void Camera::LookAt(FXMVECTOR camPos, FXMVECTOR lookPt, FXMVECTOR up)
{
    m_vPosition = camPos;
    m_vLookAt = lookPt;
    m_vUp = up;

    // Create the view matrix (world to camera space)
    m_mWorldToCamera = XMMatrixLookAtLH(camPos, lookPt, up);

    // Create the inverse (camera to world space)
    m_mCameraToWorld = XMMatrixInverse(nullptr, m_mWorldToCamera);

    m_bClipPlanesValid = false;
    m_bViewProjMatrixValid = false;
}

///////////////////////////////////////////////////////////////////////////////
void Camera::SetProjection(float fovInRadiansY, float aspectRatio, float nearPlane, float farPlane)
{
    m_fFieldOfView = fovInRadiansY;
    m_fAspectRatio = aspectRatio;
    m_fNearPlane = nearPlane;
    m_fFarPlane = farPlane;

    // Create the projection matrix
    m_mProjection = XMMatrixPerspectiveFovLH(fovInRadiansY, aspectRatio, nearPlane, farPlane);

    m_bClipPlanesValid = false;
    m_bViewProjMatrixValid = false;
}

///////////////////////////////////////////////////////////////////////////////
void Camera::Translate(FXMVECTOR relativeVector)
{
    // Get the current position from the matrix
    XMVECTOR position = m_mCameraToWorld.r[3];

    // Add the translation
    position = XMVectorAdd(position, relativeVector);

    // Update the position in the matrix
    m_mCameraToWorld.r[3] = position;

    // Update the inverse matrix
    m_mWorldToCamera = XMMatrixInverse(nullptr, m_mCameraToWorld);

    // Update the camera position vector
    m_vPosition = position;

    m_bClipPlanesValid = false;
    m_bViewProjMatrixValid = false;
}

///////////////////////////////////////////////////////////////////////////////
void Camera::SetCameraToWorldMatrix(FXMMATRIX ctw)
{
    m_mCameraToWorld = ctw;
    m_mWorldToCamera = XMMatrixInverse(nullptr, m_mCameraToWorld);

    // Extract the position from the matrix
    m_vPosition = m_mCameraToWorld.r[3];

    m_bClipPlanesValid = false;
    m_bViewProjMatrixValid = false;
}

///////////////////////////////////////////////////////////////////////////////
void Camera::SetWorldToCameraMatrix(FXMMATRIX wtc)
{
    m_mWorldToCamera = wtc;
    m_mCameraToWorld = XMMatrixInverse(nullptr, m_mWorldToCamera);

    // Extract the position from the inverse matrix
    m_vPosition = m_mCameraToWorld.r[3];

    m_bClipPlanesValid = false;
    m_bViewProjMatrixValid = false;
}

///////////////////////////////////////////////////////////////////////////////
bool Camera::SphereVisibilityCheck(FXMVECTOR pos, float radius)
{
    // Quick near/far plane check
    XMVECTOR centerToEye = XMVectorSubtract(pos, m_vPosition);
    float distanceAlongView = XMVectorGetX(XMVector3Dot(centerToEye, m_mWorldToCamera.r[2]));

    if (distanceAlongView + radius < m_fNearPlane || distanceAlongView - radius > m_fFarPlane)
    {
        return false;
    }

    // Check against the frustum planes
    if (!m_bClipPlanesValid)
    {
        UpdateClipPlanes();
    }

    // Test against each frustum plane
    for (const auto& plane : m_ClipPlanes)
    {
        // Calculate signed distance from sphere center to plane
        float distance = XMVectorGetX(XMVector3Dot(pos, plane)) - XMVectorGetW(plane);

        // If the sphere is completely behind any plane, it's outside the frustum
        if (distance < -radius)
        {
            return false;
        }
    }

    // If we get here, the sphere is visible
    return true;
}

///////////////////////////////////////////////////////////////////////////////
void Camera::UpdateClipPlanes()
{
    // Get the view-projection matrix
    XMMATRIX viewProj = GetViewProjectionMatrix();

    // Extract the 6 planes from the view-projection matrix
    // Left, right, top, bottom, near, far
    m_ClipPlanes[0] = XMVectorSet(viewProj.r[0].m128_f32[3] + viewProj.r[0].m128_f32[0],
        viewProj.r[1].m128_f32[3] + viewProj.r[1].m128_f32[0],
        viewProj.r[2].m128_f32[3] + viewProj.r[2].m128_f32[0],
        viewProj.r[3].m128_f32[3] + viewProj.r[3].m128_f32[0]);

    m_ClipPlanes[1] = XMVectorSet(viewProj.r[0].m128_f32[3] - viewProj.r[0].m128_f32[0],
        viewProj.r[1].m128_f32[3] - viewProj.r[1].m128_f32[0],
        viewProj.r[2].m128_f32[3] - viewProj.r[2].m128_f32[0],
        viewProj.r[3].m128_f32[3] - viewProj.r[3].m128_f32[0]);

    m_ClipPlanes[2] = XMVectorSet(viewProj.r[0].m128_f32[3] - viewProj.r[0].m128_f32[1],
        viewProj.r[1].m128_f32[3] - viewProj.r[1].m128_f32[1],
        viewProj.r[2].m128_f32[3] - viewProj.r[2].m128_f32[1],
        viewProj.r[3].m128_f32[3] - viewProj.r[3].m128_f32[1]);

    m_ClipPlanes[3] = XMVectorSet(viewProj.r[0].m128_f32[3] + viewProj.r[0].m128_f32[1],
        viewProj.r[1].m128_f32[3] + viewProj.r[1].m128_f32[1],
        viewProj.r[2].m128_f32[3] + viewProj.r[2].m128_f32[1],
        viewProj.r[3].m128_f32[3] + viewProj.r[3].m128_f32[1]);

    m_ClipPlanes[4] = XMVectorSet(viewProj.r[0].m128_f32[2],
        viewProj.r[1].m128_f32[2],
        viewProj.r[2].m128_f32[2],
        viewProj.r[3].m128_f32[2]);

    m_ClipPlanes[5] = XMVectorSet(viewProj.r[0].m128_f32[3] - viewProj.r[0].m128_f32[2],
        viewProj.r[1].m128_f32[3] - viewProj.r[1].m128_f32[2],
        viewProj.r[2].m128_f32[3] - viewProj.r[2].m128_f32[2],
        viewProj.r[3].m128_f32[3] - viewProj.r[3].m128_f32[2]);

    // Normalize all the planes
    for (auto& plane : m_ClipPlanes)
    {
        XMVECTOR normal = XMVectorSet(
            XMVectorGetX(plane),
            XMVectorGetY(plane),
            XMVectorGetZ(plane),
            0.0f
        );

        float normalLength = XMVectorGetX(XMVector3Length(normal));
        float invLength = 1.0f / normalLength;

        plane = XMVectorScale(plane, invLength);
    }

    m_bClipPlanesValid = true;
}

///////////////////////////////////////////////////////////////////////////////
float Camera::GetPixelScaleForZ(float z) const
{
    // Calculate the scale of a pixel at distance z
    float tanHalfFov = tanf(m_fFieldOfView * 0.5f);
    return z * tanHalfFov * 2.0f / (m_fAspectRatio * 720.0f); // Assuming 720p height
}

///////////////////////////////////////////////////////////////////////////////
XMMATRIX Camera::GetViewProjectionMatrix()
{
    if (!m_bViewProjMatrixValid)
    {
        m_mViewProjMatrix = XMMatrixMultiply(m_mWorldToCamera, m_mProjection);
        m_bViewProjMatrixValid = true;
    }

    return m_mViewProjMatrix;
}

///////////////////////////////////////////////////////////////////////////////
void Camera::GetCameraPosition(XMFLOAT3* pPos) const
{
    XMStoreFloat3(pPos, m_vPosition);
}

///////////////////////////////////////////////////////////////////////////////
void Camera::GetCameraLookAt(XMFLOAT3* pLook) const
{
    XMStoreFloat3(pLook, m_vLookAt);
}