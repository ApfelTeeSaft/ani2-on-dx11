//
//	xbox_app.h
//
///////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2001, Pipeworks Software Inc.
//				All rights reserved
#ifndef __XBOX_APP_H__
#define __XBOX_APP_H__

#include "renderer.h"
#include "camera.h"
#include "scene_renderer.h"
#include "logo_renderer.h"
#include "VBlob.h"
#include "Shield.h"
#include "GreenFog.h"
#include "qrand.h"
#include "CamControl.h"
#include "PlacementDoodad.h"
#include <iostream>
#include <windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <xaudio2.h>
#include <Xinput.h>

#ifdef INCLUDE_INPUT
#include "XBInput.h"
#endif // INCLUDE_INPUT



#include "defines.h"




///////////////////////////////////////////////////////////////////////////////
class XBoxStartupApp
{
private:
	
	int				nWidth;
	int				nHeight;


	bool			bPaused;
	bool			bActive;
	bool			bLoop;
	bool			bCameraUnderControl;
	bool			bUserControlledIntensity;
	bool			bRenderGeom;
	bool			bRenderSlash;

	float			fTimeElapsed;
	float			fJogDeltaTime;
	float			fIIDT;
	float			fBaseBlobIntensity;
	float			fBlobIntensity;
	float			fSmoothedBlobIntensity;

	DWORD			dwLastTick;
	DWORD			dwLastFramecountTick;
#ifndef FINAL_BUILD
	int				numFrames, numFramesTooSlow;
	float			fFastestFrame, fSlowestFrame;
#endif


	enum { NUM_PULSES = 12 };
	D3DVECTOR		vPulses[NUM_PULSES];	// x=center time, y=time radius of effect, z=intensity
	void			initIntensityPulses();
	float			sumIntensityPulses(float et);


	void					process();
	void					drawFrame();

	// we still want to move around when paused.
	void					advanceTime(float dt, float cam_dt);

	ShieldMgr				shieldMgr;
	LogoRenderer			logoGeom;
	

	QRand					qrand;

#ifdef INCLUDE_INPUT
    XDEVICE_PREALLOC_TYPE  * inputDeviceTypes;
    DWORD                    dwNumInputDeviceTypes;
    XBGAMEPAD              * pGamepad;
	XBGAMEPAD                defaultGamepad;

	void processInput();
#endif // INCLUDE_INPUT

	float					 fCamRad,	fSavedCamRad;
	float					 fCamTheta,	fSavedCamTheta;
	float					 fCamPhi,	fSavedCamPhi;
	float					 fCamRadBlob;

#ifdef INCLUDE_PLACEMENT_DOODAD	
	bool					bPositionLookatMode;
	D3DVECTOR				ptLookatPosition;
	PlacementDoodad			placementDoodad;
public:
	const D3DVECTOR&		getLookatPoint()	const	{ return ptLookatPosition; }
protected:
#endif


public:

#ifndef STARTUPANIMATION
	float fInitTimes[64];	
	void startTimeRec(int i) { fInitTimes[i] = ((float)GetTickCount()) * 0.001f; }
	void endTimeRec(int i)   { fInitTimes[i] = (((float)GetTickCount()) * 0.001f) - fInitTimes[i]; }

#endif // STARTUPANIMATION

	Renderer				theRenderer;
	Camera					theCamera;
	
	D3DLIGHT8               blobLight;
	D3DLIGHT8               moodLight;

	LPDIRECT3DCUBETEXTURE8	pNormalCubeMapLoRes;
	LPDIRECT3DCUBETEXTURE8  pNormalCubeMapHiRes;

	LPDIRECT3DCUBETEXTURE8  pStaticReflectionCubeMap;
	LPDIRECT3DTEXTURE8      pRoughBumpMap;

	LPDIRECT3DTEXTURE8      pGlowMap;

	DWORD					dwPShader[st_NoTypes];
	DWORD					dwVShader[st_NoTypes];

	SceneRenderer           sceneGeom;
	VBlob					vblob;
	CameraController		camController;
	GreenFog				greenFog;

	int						nBootCount;

	int						appSpeedId;
	bool					bSceneShadowsOn;
	bool					bFogOn;
	bool				    bWideScreen;

	void Construct();
	void Destruct();

	bool	init(int width,int height);
	void	shutdown();
	int		run();
	void	pause();
	void	activate();

#ifndef BINARY_RESOURCE
	DWORD   loadVertexShader(const TCHAR *pName,const DWORD *pdwDecl);
	DWORD   loadPixelShader(const TCHAR *pName);
#else // BINARY_RESOURCE
	DWORD   loadVertexShader(const BYTE *pb,const DWORD *pdwDecl);
	DWORD   loadPixelShader(const BYTE *pb);
#endif // BINARY_RESOURCE

	float			getElapsedTime()			const	{ return fTimeElapsed; }
	float			getBaseBlobIntensity()		const	{ return fBaseBlobIntensity; }
	float			getBlobIntensity()			const	{ return fBlobIntensity; }
	float			getPulseIntensity()			const	{ return fBlobIntensity - fBaseBlobIntensity; }
	float			getSmoothedBlobIntensity()	const	{ return fSmoothedBlobIntensity; }
	float			getIIDT()					const	{ return fIIDT; } // IIDT is integration of (intensity * dt)

	
	float			getCameraRadiusFromBlob()	const { return fCamRadBlob; }
	float			getCameraRadius()	const	{ return fCamRad; }
	float			getCameraTheta()	const	{ return fCamTheta; }
	float			getCameraPhi()		const	{ return fCamPhi; }

	ShieldMgr*		getShieldMgr()				{ return &shieldMgr; }

	inline float	fRand01();
	inline float	fRand11();
	inline int		rand() { return qrand.Rand(); }

};
///////////////////////////////////////////////////////////////////////////////
#define XBSA_RAND_MAX  0x00010000
#define XBSA_RAND_MASK 0x0000FFFF
inline float XBoxStartupApp::fRand01()
{
	static float mul = 1.0f / ((float)XBSA_RAND_MAX);
	return ((float)(qrand.Rand()&XBSA_RAND_MASK)) * mul;
}
inline float XBoxStartupApp::fRand11()
{
	static float mul = 2.0f / ((float)XBSA_RAND_MAX);
	return (((float)(qrand.Rand()&XBSA_RAND_MASK)) * mul) - 1.0f;
}
///////////////////////////////////////////////////////////////////////////////

extern XBoxStartupApp gApp;

#endif // __XBOX_APP_H__