/*
 * Copyright 2009-2011 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and
 * international Copyright laws.  Users and possessors of this source code
 * are hereby granted a nonexclusive, royalty-free license to use this code
 * in individual and commercial software.
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION,  ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOURCE CODE.
 *
 * U.S. Government End Users.   This source code is a "commercial item" as
 * that term is defined at  48 C.F.R. 2.101 (OCT 1995), consisting  of
 * "commercial computer  software"  and "commercial computer software
 * documentation" as such terms are  used in 48 C.F.R. 12.212 (SEPT 1995)
 * and is provided to the U.S. Government only as a commercial end item.
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
 * source code with only those rights set forth herein.
 *
 * Any use of this source code in individual and commercial software must
 * include, in the user documentation and internal comments to the code,
 * the above Disclaimer and U.S. Government End Users Notice.
 */
#include <SampleApplication.h>
#include <SampleCommandLine.h>
#include <SampleAssetManager.h>
#include <RendererDesc.h>
#include "PsString.h"
#include "AgPerfMonEventSrcAPI.h"
#include "psfile.h"
#include <RendererMemoryMacros.h>
#include <SamplePlatform.h>

#include "PsShare.h"
#include "PsTime.h"

//#define RENDERER_USE_OPENGL_ON_WINDONWS	1

#ifdef AGPERFMON
#define DEFINE_EVENT(name) #name,
const char *AgAppPerfUtils::mEventNames[] = {
#include "AgPerfMonEventDefs.h"
	""
};

AgAppPerfUtils* gAppPerfUtils = NULL;
#undef DEFINE_EVENT
#endif

char gShadersDir[1024];
char gAssetDir[1024];

#if defined(SMOOTH_CAM)
const float g_smoothCamBaseVel  = 6.0f;
const float g_smoothCamShiftMul = 4.0f;
const float g_smoothCamPosLerp  = 0.4f;

const float g_smoothCamRotSpeed = 0.005f;
const float g_smoothCamRotLerp  = 0.4f;
#endif

using physx::pubfnd2::PxMat33Legacy;
using physx::pubfnd2::PxMat34Legacy;
using physx::pubfnd2::PxAtan2;
using physx::pubfnd2::PxAsin;
using physx::pubfnd2::PxPi;
using physx::pubfnd2::PxVec3;
using physx::pubfnd2::PxF32;
using physx::pubfnd2::PxU32;
using physx::pubfnd2::PxI32;

static PxMat33Legacy EulerToMat33(const PxVec3 &e)
{
	float c1 = cosf(e.z);
	float s1 = sinf(e.z);
	float c2 = cosf(e.y);
	float s2 = sinf(e.y);
	float c3 = cosf(e.x);
	float s3 = sinf(e.x);
	PxMat33Legacy m(PxVec3(c1*c2,              -s1*c2,             s2),
			  PxVec3((s1*c3)+(c1*s2*s3), (c1*c3)-(s1*s2*s3),-c2*s3),
			  PxVec3((s1*s3)-(c1*s2*c3), (c1*s3)+(s1*s2*c3), c2*c3));
	m.setTransposed(m);
	return m;
}

static PxVec3 Mat33ToEuler(const PxMat33Legacy &m)
{
	const PxF32 epsilon = 0.99999f;
	PxVec3 e, x, y, z;
	m.getColumn(0, x);
	m.getColumn(1, y);
	m.getColumn(2, z);
	if(x.z > epsilon)
	{
		e.x = PxAtan2(z.y, y.y);
		e.y = PxPi * 0.5f;
		e.z = 0;
	}
	else if(x.z < -epsilon)
	{
		e.x = PxAtan2(z.y, y.y);
		e.y = -PxPi * 0.5f;
		e.z = 0;
	}
	else
	{
		e.x = PxAtan2(-y.z, z.z);
		e.y = PxAsin(x.z);
		e.z = PxAtan2(-x.y, x.x);
	}
	return e;
}

SampleApplication::SampleApplication(const SampleCommandLine &cmdline, const char *assetPathPrefix, MouseButton camMoveButton) :
	m_cmdline(cmdline)
{
	m_platform->setCWDToEXE();

	if (assetPathPrefix)
	{
		if (!SampleAssetManager::searchForPath(assetPathPrefix, m_assetPathPrefix, sizeof(m_assetPathPrefix)/sizeof(m_assetPathPrefix[0]), 20))
		{
			RENDERER_ASSERT(false, "assetPathPrefix could not be found in any of the parent directories!");
			m_assetPathPrefix[0] = 0;
		}
	}
	else
	{
		RENDERER_ASSERT(assetPathPrefix, "assetPathPrefix must not be NULL (try \"media\" instead)");
		m_assetPathPrefix[0] = 0;
	}

	m_renderer     = 0;
	m_sceneSize    = 1.0f;
	m_assetManager = 0;
	m_mouseX       = 0;
	m_mouseY       = 0;
	m_timeCounter  = 0;
	memset(m_mouseButtonState, 0, sizeof(m_mouseButtonState));
	memset(m_keyState,         0, sizeof(m_keyState));
	memset(m_gamepadAxisState, 0, sizeof(m_gamepadAxisState));
	m_camMoveButton = camMoveButton;
	
#ifdef AGPERFMON
#ifndef WIN32
#define MAX_IP_LENGTH 32
	FILE *agInFile = NULL;
	char ipBuffer[MAX_IP_LENGTH];
	//shdfnd::fopen_s(&agInFile, "/app_home/APEX/1.0/common/perfmonConfig.txt", "r");

	if (agInFile!=NULL)
	{
		// obtain file size:
		fseek (agInFile , 0 , SEEK_END);
		int lSize = ftell (agInFile);
		if (lSize > MAX_IP_LENGTH)
			lSize = MAX_IP_LENGTH;

		rewind (agInFile);

		// copy the file into the buffer
		fread (ipBuffer,1,lSize,agInFile);
		ipBuffer[lSize] = '\0';

		// terminate
		fclose (agInFile);
		AgAppPerfUtils::SetIPAddress(ipBuffer);
	}
	else
		AgAppPerfUtils::SetIPAddress("10.20.200.134");
#endif //!WIN32
	gAppPerfUtils = new AgAppPerfUtils;
#endif //AGPERFMON
}

SampleApplication::~SampleApplication(void)
{
	RENDERER_ASSERT(!m_renderer, "Renderer was not released prior to window closure.");
	RENDERER_ASSERT(!m_assetManager, "Asset Manager was not released prior to window closure.");

#ifdef AGPERFMON
	if(gAppPerfUtils)
	{
		delete gAppPerfUtils;
		gAppPerfUtils = 0;
	}
#endif
}

void SampleApplication::setEyeTransform(const PxMat34Legacy &eyeTransform)
{
	m_worldToView.setInverseTransform(eyeTransform);
	m_eyeRot = Mat33ToEuler(eyeTransform.M);
#if defined(SMOOTH_CAM)
	m_targetEyePos = m_worldToView.getInverseTransform().t;
	m_targetEyeRot = m_eyeRot;
#endif
}

void SampleApplication::setEyeTransform(const PxVec3 &pos, const PxVec3 &rot)
{
	PxMat34Legacy eye;

	eye.id();
	eye.t  = pos;
	m_eyeRot = rot;
	eye.M = EulerToMat33(m_eyeRot);
#if defined(SMOOTH_CAM)
	m_targetEyePos = eye.t;
	m_targetEyeRot = m_eyeRot;
#endif

	m_worldToView.setInverseTransform(eye);
}

void SampleApplication::setViewTransform(const PxMat34Legacy &viewTransform)
{
	m_worldToView.setForwardTransform(viewTransform);
	m_eyeRot = Mat33ToEuler( m_worldToView.getInverseTransform().M );
#if defined(SMOOTH_CAM)
	m_targetEyePos = m_worldToView.getInverseTransform().t;
	m_targetEyeRot = m_eyeRot;
#endif
}

const PxMat34Legacy& SampleApplication::getViewTransform() const
{
	return m_worldToView.getForwardTransform();
}

void SampleApplication::onOpen(void)
{
	m_platform->preRendererSetup();

    memset(m_mouseButtonState, 0, sizeof(m_mouseButtonState));
	memset(m_keyState,         0, sizeof(m_keyState));

	char rendererdir[1024];
	physx::string::strcpy_s(rendererdir, sizeof(rendererdir), m_assetPathPrefix);
	physx::string::strcat_s(rendererdir, sizeof(rendererdir), "/SampleRenderer/3/");
	physx::string::strcpy_s(gAssetDir, sizeof(gAssetDir), rendererdir);

	physx::string::strcpy_s(gShadersDir, sizeof(gShadersDir), rendererdir);
	physx::string::strcat_s(gShadersDir, sizeof(gShadersDir), "shaders/");

	m_eyeRot = PxVec3(0,0,0);
	PxMat34Legacy eye;
	eye.id();
	eye.t.x = 2;
	eye.t.y = 2;
	eye.t.z = 16;
	m_worldToView.setInverseTransform(eye);
#if defined(SMOOTH_CAM)
	m_targetEyePos = eye.t;
	m_targetEyeRot = m_eyeRot;
#endif

	RendererDesc renDesc;
	
	// default renderer drivers for various platforms...
	m_platform->setupRendererDescription(renDesc);
#if defined RENDERER_USE_OPENGL_ON_WINDONWS
	renDesc.driver = SampleRenderer::Renderer::DRIVER_OPENGL;
#endif

	// check to see if the user wants to override the renderer driver...
	if(m_cmdline.hasSwitch("ogl"))        renDesc.driver = Renderer::DRIVER_OPENGL;
	else if(m_cmdline.hasSwitch("d3d9"))  renDesc.driver = Renderer::DRIVER_DIRECT3D9;
	else if(m_cmdline.hasSwitch("d3d10")) renDesc.driver = Renderer::DRIVER_DIRECT3D10;
	else if(m_cmdline.hasSwitch("gcm"))   renDesc.driver = Renderer::DRIVER_LIBGCM;

	m_renderer = Renderer::createRenderer(renDesc);
	m_platform->postRendererSetup();

	m_timeCounter = m_time.getCurrentCounterValue();

	m_assetManager = new SampleAssetManager(*m_renderer);
	m_assetManager->addSearchPath(m_assetPathPrefix);
	m_assetManager->addSearchPath(rendererdir);

	onInit();
}

bool SampleApplication::onClose(void)
{
	onShutdown();
	DELETESINGLE(m_assetManager);

	SAFE_RELEASE(m_renderer);
	m_platform->postRendererRelease();

	return true;
}

inline float SmoothStepPolynomial( float s )
{
	if( s <= 0 ) return 0;
	if( s >= 1 ) return 1;
	return s*s*(3-2*s);
}

template <typename T>
T SmoothStep( const T& start, const T& end, float s )
{
	float ss = SmoothStepPolynomial( s );
	return ss * (end - start) + start;
}

void SampleApplication::onDraw(void)
{	
	SAMPLE_PERF_SCOPE(ApexOnDraw);
#if USE_MEMORY_TRACKER
	TRACK_FRAME();
#endif

	physx::PxU64 qpc = m_time.getCurrentCounterValue();
	float dtime = static_cast<float>((double)(qpc - m_timeCounter) / (double)m_time.sCounterFreq.mDenominator);
	m_timeCounter = qpc;
	PX_ASSERT(dtime >= 0);
	if(dtime > 0)
	{
		{
			SAMPLE_PERF_SCOPE( ApexOnTickPreRender );
			onTickPreRender(dtime);
		}
		if(m_renderer)
		{
			SAMPLE_PERF_SCOPE( ApexOnRender );
			onRender();
		}
		{
			SAMPLE_PERF_SCOPE( ApexOnTickPostRender );
			onTickPostRender(dtime);
		}
		
		// update scene...

		PxMat34Legacy eye = m_worldToView.getInverseTransform();
		eye.M = EulerToMat33(m_eyeRot);
		PxVec3* targetParam;

#if defined(SMOOTH_CAM)
		const float eyeSpeed = m_sceneSize * g_smoothCamBaseVel * dtime * (isKeyDown(KEY_SHIFT) ? g_smoothCamShiftMul : 1.0f);
		targetParam = &m_targetEyePos;
#else
		const float eyeSpeed = m_sceneSize * 4.0f * dtime * (isKeyDown(KEY_SHIFT) ? 4.0f : 1.0f);
		targetParam = &eye.t;
#endif

		if(m_keyState[KEY_W]) *targetParam -= eye.M.getColumn(2) * eyeSpeed;
		if(m_keyState[KEY_A]) *targetParam -= eye.M.getColumn(0) * eyeSpeed;
		if(m_keyState[KEY_S]) *targetParam += eye.M.getColumn(2) * eyeSpeed;
		if(m_keyState[KEY_D]) *targetParam += eye.M.getColumn(0) * eyeSpeed;

		// move forward from gamepad
		*targetParam -= eye.M.getColumn(2) * eyeSpeed * m_gamepadAxisState[AXIS_RIGHT_VERTICAL] * 40.0f * dtime;
		// strafe from gamepad
		*targetParam += eye.M.getColumn(0) * eyeSpeed * m_gamepadAxisState[AXIS_RIGHT_HORIZONTAL] * 40.0f * dtime;

#if defined(SMOOTH_CAM)
		eye.t = eye.t + (m_targetEyePos - eye.t) * g_smoothCamPosLerp;
#endif

		// rotate from gamepad
		{
			const PxF32 rotationSpeed = 200.0f * dtime;
			PxF32 dx = m_gamepadAxisState[AXIS_LEFT_HORIZONTAL] * rotationSpeed;
			PxF32 dy = m_gamepadAxisState[AXIS_LEFT_VERTICAL] * rotationSpeed;
			rotateCamera(dx, dy);
		}

		m_worldToView.setInverseTransform(eye);
	}
}

void SampleApplication::onMouseMove(PxU32 x, PxU32 y)
{
	if(m_mouseButtonState[m_camMoveButton])
	{
		PxI32 dx = ((PxI32)x)-(PxI32)m_mouseX;
		PxI32 dy = ((PxI32)y)-(PxI32)m_mouseY;
		rotateCamera((PxF32)dx, (PxF32)dy);
	}
	m_mouseX = x;
	m_mouseY = y;
}

void SampleApplication::onMouseDown(PxU32 /*x*/, PxU32 /*y*/, MouseButton button)
{
	m_mouseButtonState[button] = true;
}

void SampleApplication::onMouseUp(PxU32 /*x*/, PxU32 /*y*/, MouseButton button)
{
	m_mouseButtonState[button] = false;
}

void SampleApplication::onKeyDown(KeyCode keyCode)
{
	m_keyState[keyCode] = true;
}

void SampleApplication::onKeyUp(KeyCode keyCode)
{
	m_keyState[keyCode] = false;
}

void SampleApplication::onGamepadButton(physx::PxU32 /*button*/, bool /*buttonDown*/)
{

}

void SampleApplication::onGamepadAxis(physx::PxU32 axis, physx::PxReal absolutePosition)
{
	// This will rotate the camera with the left stick and move it with the right stick.
	// PH: If this behaviour is unwanted, make sure not to forward the onGamepadAxis callback to SampleApplication but intercept yourself.
	m_gamepadAxisState[axis+1] = absolutePosition;
}

void SampleApplication::rotateCamera(PxF32 dx, PxF32 dy)
{
	const float eyeCap      = 1.5f;
#if defined(SMOOTH_CAM)
	m_targetEyeRot.x -= dy * g_smoothCamRotSpeed;
	m_targetEyeRot.y += dx * g_smoothCamRotSpeed;
	if(m_targetEyeRot.x >  eyeCap) m_targetEyeRot.x =  eyeCap;
	if(m_targetEyeRot.x < -eyeCap) m_targetEyeRot.x = -eyeCap;

	m_eyeRot= m_eyeRot + (m_targetEyeRot - m_eyeRot) * g_smoothCamRotLerp;
#else
	const float eyeRotSpeed = 0.005f;
	m_eyeRot.x -= dy * eyeRotSpeed;
	m_eyeRot.y += dx * eyeRotSpeed;
	if(m_eyeRot.x >  eyeCap) m_eyeRot.x =  eyeCap;
	if(m_eyeRot.x < -eyeCap) m_eyeRot.x = -eyeCap;
#endif
}

void SampleApplication::doInput()
{
	m_platform->doInput();
}