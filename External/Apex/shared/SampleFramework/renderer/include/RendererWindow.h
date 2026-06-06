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
#ifndef RENDERER_WINDOW_H
#define RENDERER_WINDOW_H

#include <RendererConfig.h>
#include "Px.h"
#include "PxSimpleTypes.h"

class SamplePlatform;

class RendererWindow
{
	public:
		typedef enum MouseButton
		{
			MOUSE_LEFT = 0,
			MOUSE_RIGHT,
			MOUSE_CENTER,
			
			NUM_MOUSE_BUTTONS,
		};
		
		typedef enum KeyCode
		{
			KEY_UNKNOWN = 0,

			KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G,
			KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N,
			KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U,
			KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,

			KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, 
			KEY_7, KEY_8, KEY_9,

			KEY_SPACE, KEY_RETURN, KEY_SHIFT, KEY_CONTROL, KEY_ESCAPE, KEY_COMMA, 
			KEY_NUMPAD0, KEY_NUMPAD1, KEY_NUMPAD2, KEY_NUMPAD3, KEY_NUMPAD4, KEY_NUMPAD5, KEY_NUMPAD6, KEY_NUMPAD7, KEY_NUMPAD8, KEY_NUMPAD9,
			KEY_MULTIPLY, KEY_ADD, KEY_SEPARATOR, KEY_SUBTRACT, KEY_DECIMAL, KEY_DIVIDE,

			KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
			KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,

			KEY_TAB, KEY_PRIOR, KEY_NEXT,
			KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,

			NUM_KEY_CODES,
		};

		typedef enum GamepadAxis
		{
			AXIS_UNKNOWN = 0,
			AXIS_RIGHT_HORIZONTAL,
			AXIS_RIGHT_VERTICAL,
			AXIS_LEFT_HORIZONTAL,
			AXIS_LEFT_VERTICAL,

			NUM_GAMEPAD_AXIS,
		};

		bool keyCodeToASCII( KeyCode code, char& c )
		{
			if( code >= KEY_A && code <= KEY_Z )
			{
				c = (char)code + 'A' - 1;
			}
			else if( code >= KEY_0 && code <= KEY_9 )
			{
				c = (char)code + '0' - 1;
			}
			else if( code == KEY_SPACE )
			{
				c = ' ';
			}
			else
			{
				return false;
			}
			return true;
		}
		
	public:
		RendererWindow(void);
		virtual ~RendererWindow(void);
		
		bool open(physx::PxU32 width, physx::PxU32 height, const char *title, bool fullscreen=false);
		void close(void);
		
		bool isOpen(void) const;
		
		// update the window's state... handle messages, etc.
		void update(void);
		
		// get/set the size of the window...
		void getSize(physx::PxU32 &width, physx::PxU32 &height) const;
		void setSize(physx::PxU32 width, physx::PxU32 height);
		
		// get/set the window's title...
		void getTitle(char *title, physx::PxU32 maxLength) const;
		void setTitle(const char *title);
		
	public:
		// called just AFTER the window opens.
		virtual void onOpen(void) {}
		
		// called just BEFORE the window closes. return 'true' to confirm the window closure.
		virtual bool onClose(void) { return true; }
		
		// called just AFTER the window is resized.
		virtual void onResize(physx::PxU32 /*width*/, physx::PxU32 /*height*/) {}
		
		virtual void onMouseMove(physx::PxU32 /*x*/, physx::PxU32 /*y*/) {}
		
		virtual void onMouseDown(physx::PxU32 /*x*/, physx::PxU32 /*y*/, MouseButton /*button*/) {}
		virtual void onMouseUp(  physx::PxU32 /*x*/, physx::PxU32 /*y*/, MouseButton /*button*/) {}
		
		virtual void onKeyDown(KeyCode /*keyCode*/) {}
		virtual void onKeyUp(  KeyCode /*keyCode*/) {}
		
		// called when the window's contents needs to be redrawn.
		virtual void onDraw(void) = 0;

	protected:
		SamplePlatform*				m_platform;
	private:
		bool						m_isOpen;
};

#endif
