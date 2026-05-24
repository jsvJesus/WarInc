#ifndef SAMPLE_PLATFORM_H
#define SAMPLE_PLATFORM_H
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
#include <RendererDesc.h>
#include <RendererWindow.h>
#include <RendererTexture2D.h>

class SampleApplication;

class SamplePlatform 
{
protected:
	RendererWindow*						m_app;
	SampleApplication*					m_sf_app;
	static SamplePlatform*				m_platform;
public:
	// access
	static SamplePlatform*				platform();
	static void							setPlatform(SamplePlatform*);
	// creation
	explicit							SamplePlatform(RendererWindow* _app);
	virtual								~SamplePlatform() = 0;
	// System
	virtual void						setCWDToEXE(void);
	virtual void						popPathSpec(char *path);
	virtual bool						openWindow(physx::pubfnd2::PxU32& width, 
													physx::pubfnd2::PxU32& height,
													const char* title,
													bool fullscreen);
	virtual void						update();
	virtual bool						closeWindow();
	virtual bool						hasFocus() const;
	virtual void						setFocus(bool b);
	virtual bool						isOpen();
	virtual physx::pubfnd2::PxU64		getWindowHandle();
	virtual void						setWindowSize(physx::pubfnd2::PxU32 width, 
														physx::pubfnd2::PxU32 height);
	virtual void						getWindowSize(physx::pubfnd2::PxU32& width, physx::pubfnd2::PxU32& height);
	virtual void						getTitle(char *title, physx::pubfnd2::PxU32 maxLength) const;
	virtual void						setTitle(const char *title);
	virtual void						recenterCursor(physx::pubfnd2::PxReal& deltaMouseX, 
														physx::pubfnd2::PxReal& deltaMouseY);
	virtual void						showMessage(const char* title, const char* message);
	virtual void*						compileProgram(void * context, 
														const char *programPath, 
														physx::pubfnd2::PxU64 profile, 
														const char *entry, 
														const char **args);
	virtual void*						initializeD3D9();
	virtual bool						isD3D9ok();
	// Rendering
	virtual void						initializeCGRuntimeCompiler();
	virtual void						initializeOGLDisplay(const RendererDesc& desc,
															physx::pubfnd2::PxU32& width, 
															physx::pubfnd2::PxU32& height);
	virtual physx::pubfnd2::PxU32		initializeD3D9Display(void * presentParameters, 
															char* m_deviceName, 
															physx::pubfnd2::PxU32& width, 
															physx::pubfnd2::PxU32& height,
															void * m_d3dDevice_out);
	virtual physx::pubfnd2::PxU32		D3D9Present();
	virtual void						D3D9BlockUntilNotBusy(void * resource);
	virtual void						D3D9DeviceBlockUntilIdle();
	virtual physx::pubfnd2::PxU64		getD3D9TextureFormat(RendererTexture2D::Format format);
	virtual void						postInitializeOGLDisplay();
	virtual bool						makeContextCurrent();
	virtual bool						isContextValid();
	virtual void						freeDisplay();
	virtual void						swapBuffers();
	virtual void						postRendererRelease();
	virtual void						preRendererSetup();
	virtual void						postRendererSetup();
	virtual void						setupRendererDescription(RendererDesc& renDesc);
	// Input
	virtual void						doInput();
	virtual void						processGamepads();
};

SamplePlatform*		createPlatform(RendererWindow* _app);

#endif
