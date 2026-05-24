
#ifndef SAMPLE_APPLICATION_H
#define SAMPLE_APPLICATION_H
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
#include <Renderer.h>
#include <RendererWindow.h>

#include "Px.h"
#include "PxSimpleTypes.h"
#include "PxVec3.h"
#include "PxMat34Legacy.h"
#include "PsTime.h"

#include <SampleAssetManager.h>

class SampleCommandLine;

#define SMOOTH_CAM

template <typename T, bool AssumeOrthonormal = true>
class Transform
{
public:
	Transform() : m_dirty(true) {}

	void         setForwardTransform( const T& t )
	{ 
		m_fwd = t;
		m_dirty = true;
	}

	void         setInverseTransform( const T& t )
	{ 
		m_inv = t;
		m_inv.getInverse( m_fwd );
		m_dirty = false;
	}

	const T&	getForwardTransform() const
	{ 
		return m_fwd;
	}

	const T&	getInverseTransform() const
	{
		if(AssumeOrthonormal && m_dirty)
		{
			m_fwd.getInverseRT( m_inv );
			m_dirty = false;
		}
		if(!AssumeOrthonormal && m_dirty)
		{
			m_fwd.getInverse( m_inv );
			m_dirty = false;
		}
		return m_inv;
	}

private:
			T    m_fwd;
	mutable T    m_inv;
	mutable bool m_dirty;
};

// PT: TODO: share this with PxApplication
struct GamepadControls
{
	enum Button {
		DIGI_UP, DIGI_DOWN, DIGI_LEFT, DIGI_RIGHT,
		START, SELECT,
		LEFT_STICK, RIGHT_STICK,
		NORTH, SOUTH, WEST, EAST,
		LEFT_SHOULDER_TOP, RIGHT_SHOULDER_TOP,
		LEFT_SHOULDER_BOT, RIGHT_SHOULDER_BOT,
		NUM_PAD_BUTTONS
	} Button;

	enum Axis {
		RIGHT_STICK_X, RIGHT_STICK_Y,
		LEFT_STICK_X, LEFT_STICK_Y,
		NUM_PAD_AXES
	} Axis;
};

class SampleApplication : public RendererWindow
{
	public:
		SampleApplication(const SampleCommandLine& cmdline, const char* assetPathPrefix="media", MouseButton camMoveButton = MOUSE_LEFT);
		virtual								~SampleApplication(void);
		
				Renderer*					getRenderer(void)								{ return m_renderer; }
				SampleAssetManager*			getAssetManager(void)							{ return m_assetManager; }
				inline const char*			getAssetPathPrefix(void)				const	{ return m_assetPathPrefix; }
				const physx::PxMat34Legacy&	getEyeTransform(void)					const	{ return m_worldToView.getInverseTransform(); }
				void						setEyeTransform(const physx::PxMat34Legacy& eyeTransform);
				void						setEyeTransform(const physx::PxVec3& pos, const physx::PxVec3& rot);
				void						setViewTransform(const physx::PxMat34Legacy& viewTransform);
				const physx::PxMat34Legacy&	getViewTransform(void)					const;
				bool						isKeyDown(KeyCode keyCode)				const	{ return m_keyState[keyCode];			}
				bool						isMouseButtonDown(MouseButton button)	const	{ return m_mouseButtonState[button];	}
	
	public:
		virtual	void						onInit(void) = 0;
		virtual	void						onShutdown(void) = 0;
		virtual	void						onTickPreRender(float dtime) = 0;
		virtual	void						onRender(void) = 0;
		virtual	void						onTickPostRender(float dtime) = 0;
		
	public:
		virtual	void						onOpen(void);
		virtual	bool						onClose(void);
		
		virtual	void						onDraw(void);
		
		virtual	void						onMouseMove(physx::PxU32 x, physx::PxU32 y);
		virtual	void						onMouseDown(physx::PxU32 /*x*/, physx::PxU32 /*y*/, MouseButton button);
		virtual	void						onMouseUp(physx::PxU32 /*x*/, physx::PxU32 /*y*/, MouseButton button);


		
		virtual	void						onKeyDown(KeyCode keyCode);
		virtual	void						onKeyUp(KeyCode keyCode);

		// PT: we need those on PC as well, not just on consoles
		virtual	void						onGamepadButton(physx::PxU32 button, bool buttonDown);
		virtual	void						onGamepadAxis(physx::PxU32 axis, physx::PxReal absolutePosition);
		void								rotateCamera(physx::PxF32 dx, physx::PxF32 dy);
		virtual	void						doInput(void);

	protected:
		const SampleCommandLine&			m_cmdline;

				physx::PxF32				m_sceneSize;
		
				Renderer*					m_renderer;
				
				char						m_assetPathPrefix[256];
				SampleAssetManager*			m_assetManager;
				
				physx::PxVec3				m_eyeRot;
				Transform<physx::PxMat34Legacy>	m_worldToView;
				
				physx::PxU32				m_mouseX, m_mouseY;
				bool						m_mouseButtonState[NUM_MOUSE_BUTTONS];
				bool						m_keyState[NUM_KEY_CODES];
				physx::PxF32				m_gamepadAxisState[NUM_GAMEPAD_AXIS];

				MouseButton					m_camMoveButton;
				
    #if defined(SMOOTH_CAM)
				physx::PxVec3				m_targetEyeRot;
				physx::PxVec3				m_targetEyePos;
    #endif
private:
				physx::PxU64				m_timeCounter;
				physx::Time					m_time;

	// get rid of stupid C4512
				void						operator=(const SampleApplication&);
};

#endif
