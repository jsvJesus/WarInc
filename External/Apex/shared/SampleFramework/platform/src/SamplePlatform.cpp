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

#include <SamplePlatform.h>
#include <SampleApplication.h>

SamplePlatform* SamplePlatform::m_platform = NULL;

SamplePlatform* SamplePlatform::platform()
{
	RENDERER_ASSERT(SamplePlatform::m_platform, "SamplePlatform was not initialized!");
	if(SamplePlatform::m_platform) 
	{
		return SamplePlatform::m_platform;
	}
	return NULL;
}

void SamplePlatform::setPlatform(SamplePlatform* ptr)
{
	SamplePlatform::m_platform = ptr;
}

void* SamplePlatform::initializeD3D9()
{
	return NULL;
}

bool SamplePlatform::isD3D9ok()
{
	return true;
}

void* SamplePlatform::compileProgram(void * context, 
										const char *programPath, 
										physx::pubfnd2::PxU64 profile, 
										const char *entry, 
										const char **args)

{
	return NULL;
}

SamplePlatform::SamplePlatform(RendererWindow* _app) : m_app(_app)
{
	m_sf_app = static_cast<SampleApplication*>(m_app);
}

SamplePlatform::~SamplePlatform()
{
}

bool SamplePlatform::hasFocus() const
{
	return true;
}

void SamplePlatform::setFocus(bool b)
{
}

void SamplePlatform::update()
{
}

void SamplePlatform::setWindowSize(physx::pubfnd2::PxU32 width, 
									physx::pubfnd2::PxU32 height)
{
}

void SamplePlatform::getTitle(char *title, physx::pubfnd2::PxU32 maxLength) const
{
}

void SamplePlatform::setTitle(const char *title)
{
}

void SamplePlatform::recenterCursor(physx::pubfnd2::PxReal& deltaMouseX, 
									physx::pubfnd2::PxReal& deltaMouseY)
{
}


bool SamplePlatform::openWindow(physx::pubfnd2::PxU32& width, 
								physx::pubfnd2::PxU32& height,
								const char* title,
								bool fullscreen) 
{
	return true;
}


bool SamplePlatform::isOpen()
{
	return true;
}

bool SamplePlatform::closeWindow() 
{
	return true;
}

void SamplePlatform::setCWDToEXE(void) 
{
}

void SamplePlatform::popPathSpec(char *path)
{
	char *ls = 0;
	while(*path)
	{
		if(*path == '\\' || *path == '/') ls = path;
		path++;
	}
	if(ls) *ls = 0;
}

void SamplePlatform::preRendererSetup()
{
}

void SamplePlatform::postRendererSetup()
{
}

void SamplePlatform::setupRendererDescription(RendererDesc& renDesc) 
{
	renDesc.driver = Renderer::DRIVER_OPENGL;
}

void SamplePlatform::doInput()
{
}

void SamplePlatform::processGamepads()
{
}

void SamplePlatform::postRendererRelease()
{
}

void SamplePlatform::initializeOGLDisplay(const RendererDesc& desc,
									   physx::pubfnd2::PxU32& width, 
									   physx::pubfnd2::PxU32& height)
{
}

void SamplePlatform::showMessage(const char* title, const char* message)
{
	printf("%s: %s\n", title, message);
}

void SamplePlatform::initializeCGRuntimeCompiler()
{
}

void SamplePlatform::getWindowSize(physx::pubfnd2::PxU32& width, physx::pubfnd2::PxU32& height)
{
}
	
void SamplePlatform::freeDisplay() 
{
}

bool SamplePlatform::makeContextCurrent()
{
	return true;
}

void SamplePlatform::swapBuffers()
{
}

bool SamplePlatform::isContextValid()
{
	return true;
}

void SamplePlatform::postInitializeOGLDisplay()
{
}

physx::pubfnd2::PxU32 SamplePlatform::initializeD3D9Display(void * presentParameters, 
																char* m_deviceName, 
																physx::pubfnd2::PxU32& width, 
																physx::pubfnd2::PxU32& height,
																void * m_d3dDevice_out)
{
	return 0;
}

physx::pubfnd2::PxU64 SamplePlatform::getWindowHandle()
{
	return 0;
}

physx::pubfnd2::PxU32 SamplePlatform::D3D9Present()
{
	return 0;
}

void SamplePlatform::D3D9BlockUntilNotBusy(void * resource)
{
}

void SamplePlatform::D3D9DeviceBlockUntilIdle()
{
}

physx::pubfnd2::PxU64 SamplePlatform::getD3D9TextureFormat(RendererTexture2D::Format format)
{
	return 0;
}

