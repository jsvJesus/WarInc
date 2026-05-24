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
#include "PsShare.h"
#include "OGLRenderer.h"
#include "PsString.h"

#if defined(RENDERER_ENABLE_OPENGL)

#include <RendererDesc.h>

#include <RendererVertexBufferDesc.h>
#include "OGLRendererVertexBuffer.h"

#include <RendererIndexBufferDesc.h>
#include "OGLRendererIndexBuffer.h"

#include <RendererInstanceBufferDesc.h>
#include "OGLRendererInstanceBuffer.h"

#include <RendererMeshDesc.h>
#include <RendererMeshContext.h>
#include "OGLRendererMesh.h"

#include <RendererMaterialDesc.h>
#include <RendererMaterialInstance.h>
#include "OGLRendererMaterial.h"

#include <RendererLightDesc.h>
#include <RendererDirectionalLightDesc.h>
#include "OGLRendererDirectionalLight.h"
#include <RendererSpotLightDesc.h>
#include "OGLRendererSpotLight.h"

#include <RendererTexture2DDesc.h>
#include "OGLRendererTexture2D.h"

#include <SamplePlatform.h>

#include <PxMat34Legacy.h>
#include <RendererProjection.h>

using physx::pubfnd2::PxU32;

#if defined(RENDERER_ENABLE_CG)
	void setColorParameter(CGparameter param, const RendererColor &color)
	{
		const float inv255 = 1.0f / 255.0f;
		const float fcolor[3] = { color.r*inv255, color.g*inv255, color.b*inv255 };
		cgSetParameter3fv(param, fcolor);
	}
#endif

#if defined(RENDERER_ENABLE_CG)
	static void CGENTRY onCgError(CGcontext cgContext, CGerror err, void * /*data*/)
	{
		const char *errstr = cgGetErrorString(err);
		if(cgContext)
		{
			errstr = cgGetLastListing(cgContext);
		}
	#if defined(RENDERER_WINDOWS)
		MessageBoxA(0, errstr, "Cg Error", MB_OK);
	#endif
	}
	
	OGLRenderer::CGEnvironment::CGEnvironment(void)
	{
		memset(this, 0, sizeof(*this));
	}
	
	OGLRenderer::CGEnvironment::CGEnvironment(CGcontext cgContext)
	{
		modelMatrix         = cgCreateParameter(cgContext, CG_FLOAT4x4);
		viewMatrix          = cgCreateParameter(cgContext, CG_FLOAT4x4);
		projMatrix          = cgCreateParameter(cgContext, CG_FLOAT4x4);
		modelViewMatrix     = cgCreateParameter(cgContext, CG_FLOAT4x4);
		modelViewProjMatrix = cgCreateParameter(cgContext, CG_FLOAT4x4);
		
		boneMatrices        = cgCreateParameterArray(cgContext, CG_FLOAT4x4, RENDERER_MAX_BONES);

		eyePosition         = cgCreateParameter(cgContext, CG_FLOAT3);
		eyeDirection        = cgCreateParameter(cgContext, CG_FLOAT3);
		
		ambientColor        = cgCreateParameter(cgContext, CG_FLOAT3);
		
		lightColor          = cgCreateParameter(cgContext, CG_FLOAT3);
		lightIntensity      = cgCreateParameter(cgContext, CG_FLOAT);
		lightDirection      = cgCreateParameter(cgContext, CG_FLOAT3);
		lightPosition       = cgCreateParameter(cgContext, CG_FLOAT3);
		lightInnerRadius    = cgCreateParameter(cgContext, CG_FLOAT);
		lightOuterRadius    = cgCreateParameter(cgContext, CG_FLOAT);
		lightInnerCone      = cgCreateParameter(cgContext, CG_FLOAT);
		lightOuterCone      = cgCreateParameter(cgContext, CG_FLOAT);
	}
#endif

void PxToGL(GLfloat *gl44, const physx::PxMat34Legacy &nx34)
{
	nx34.getColumnMajor44(gl44);
}

void RenToGL(GLfloat *gl44, const RendererProjection &proj)
{
	proj.getColumnMajor44(gl44);
}

OGLRenderer::OGLRenderer(const RendererDesc &desc) :
	Renderer(DRIVER_OPENGL), m_platform(SamplePlatform::platform())
{
	m_useShadersForTextRendering	= false;
	m_displayWidth  = 0;
	m_displayHeight = 0;
	m_currMaterial  = 0;
	m_viewMatrix.id();
	
	m_platform->initializeOGLDisplay(desc, m_displayWidth, m_displayHeight);
	m_platform->postInitializeOGLDisplay();

	checkResize();

#if defined(RENDERER_ENABLE_CG)
	m_cgContext = cgCreateContext();
	RENDERER_ASSERT(m_cgContext, "Unable to obtain Cg Context.");

#if defined(RENDERER_PS3)
	cgRTCgcInit();
#endif
	if(m_cgContext)
	{
		m_cgEnv = CGEnvironment(m_cgContext);

#if !defined(RENDERER_PS3)
		cgSetErrorHandler(onCgError, this);
		cgSetParameterSettingMode(m_cgContext, CG_DEFERRED_PARAMETER_SETTING);
#endif
	#if defined(RENDERER_DEBUG)
		cgGLSetDebugMode(CG_TRUE);
	#else
		cgGLSetDebugMode(CG_FALSE);
	#endif
	}
#endif
	
	physx::string::strncpy_s(m_deviceName, sizeof(m_deviceName), (const char*)glGetString(GL_RENDERER), sizeof(m_deviceName));
	for(char *c=m_deviceName; *c; *c++)
	{
		if(*c == '/')
		{
			*c = 0;
			break;
		}
	}
	
    RendererColor& clearColor = getClearColor();
    glClearColor(clearColor.r/255.0f, clearColor.g/255.0f, clearColor.b/255.0f, clearColor.a/255.0f);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	glEnable(GL_DEPTH_TEST);
}

OGLRenderer::~OGLRenderer(void)
{
#if defined(RENDERER_ENABLE_CG)
	if(m_cgContext)
	{
		cgDestroyContext(m_cgContext);
	}
#endif
	m_platform->freeDisplay();
}

bool OGLRenderer::begin(void)
{
	bool ok = false;
	ok = m_platform->makeContextCurrent();
	return ok;
}

void OGLRenderer::end(void)
{
	
}

void OGLRenderer::checkResize(void)
{
	PxU32 width  = m_displayWidth;
	PxU32 height = m_displayHeight;
	m_platform->getWindowSize(width, height);
	if(width != m_displayWidth || height != m_displayHeight)
	{
		m_displayWidth  = width;
		m_displayHeight = height;
		glViewport(0, 0, (GLsizei)m_displayWidth, (GLsizei)m_displayHeight);
		glScissor( 0, 0, (GLsizei)m_displayWidth, (GLsizei)m_displayHeight);
	}
}

// clears the offscreen buffers.
void OGLRenderer::clearBuffers(void)
{
	if(begin())
	{
		GLbitfield glbuf = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
		glDepthMask(1);
		glClear(glbuf);
        RendererColor& clearColor = getClearColor();
        glClearColor(clearColor.r/255.0f, clearColor.g/255.0f, clearColor.b/255.0f, clearColor.a/255.0f);
	}
	end();
}

// presents the current color buffer to the screen.
// returns true on device reset and if buffers need to be rewritten.
bool OGLRenderer::swapBuffers(void)
{
	RENDERER_PERFZONE(OGLRendererSwapBuffers);
	if(begin())
	{
		m_platform->swapBuffers();			
		checkResize();
	}
	end();
	return false;
}

void OGLRenderer::getWindowSize(physx::PxU32 &width, physx::PxU32 &height) const
{
	width = m_displayWidth;
	height = m_displayHeight;
}


RendererVertexBuffer *OGLRenderer::createVertexBuffer(const RendererVertexBufferDesc &desc)
{
	RENDERER_PERFZONE(OGLRenderer_createVertexBuffer);
	OGLRendererVertexBuffer *vb = 0;
	RENDERER_ASSERT(desc.isValid(), "Invalid Vertex Buffer Descriptor.");
	if(desc.isValid())
	{
		vb = new OGLRendererVertexBuffer(desc, m_deferredVBUnlock);
	}
	return vb;
}

RendererIndexBuffer *OGLRenderer::createIndexBuffer(const RendererIndexBufferDesc &desc)
{
	RENDERER_PERFZONE(OGLRenderer_createIndexBuffer);
	OGLRendererIndexBuffer *ib = 0;
	RENDERER_ASSERT(desc.isValid(), "Invalid Index Buffer Descriptor.");
	if(desc.isValid())
	{
		ib = new OGLRendererIndexBuffer(desc);
	}
	return ib;
}

RendererInstanceBuffer *OGLRenderer::createInstanceBuffer(const RendererInstanceBufferDesc &desc)
{
	RENDERER_PERFZONE(OGLRenderer_createInstanceBuffer);
	OGLRendererInstanceBuffer *ib = 0;
	RENDERER_ASSERT(desc.isValid(), "Invalid Instance Buffer Descriptor.");
	if(desc.isValid())
	{
		ib = new OGLRendererInstanceBuffer(desc);
	}
	return ib;
}

RendererTexture2D *OGLRenderer::createTexture2D(const RendererTexture2DDesc &desc)
{
	RENDERER_PERFZONE(OGLRenderer_createTexture2D);
	OGLRendererTexture2D *texture = 0;
	RENDERER_ASSERT(desc.isValid(), "Invalid Texture 2D Descriptor.");
	if(desc.isValid())
	{
		texture = new OGLRendererTexture2D(desc);
	}
	return texture;
}

RendererTarget *OGLRenderer::createTarget(const RendererTargetDesc &desc)
{
	RENDERER_PERFZONE(OGLRenderer_createTarget);
	RENDERER_ASSERT(0, "Not Implemented.");
	return 0;
}

RendererMaterial *OGLRenderer::createMaterial(const RendererMaterialDesc &desc)
{
	RENDERER_PERFZONE(OGLRenderer_createMaterial);
	OGLRendererMaterial *mat = 0;
	RENDERER_ASSERT(desc.isValid(), "Invalid Material Descriptor.");
	if(desc.isValid())
	{
		mat = new OGLRendererMaterial(*this, desc);
	}
	return mat;
}

RendererMesh *OGLRenderer::createMesh(const RendererMeshDesc &desc)
{
	RENDERER_PERFZONE(OGLRenderer_createMesh);
	OGLRendererMesh *mesh = 0;
	RENDERER_ASSERT(desc.isValid(), "Invalid Mesh Descriptor.");
	if(desc.isValid())
	{
		mesh = new OGLRendererMesh(*this, desc);
	}
	return mesh;
}

RendererLight *OGLRenderer::createLight(const RendererLightDesc &desc)
{
	RENDERER_PERFZONE(OGLRenderer_createLight);
	RendererLight *light = 0;
	RENDERER_ASSERT(desc.isValid(), "Invalid Light Descriptor.");
	if(desc.isValid())
	{
		switch(desc.type)
		{
			case RendererLight::TYPE_DIRECTIONAL:
				light = new OGLRendererDirectionalLight(*static_cast<const RendererDirectionalLightDesc*>(&desc), *this);
				break;
			case RendererLight::TYPE_SPOT:
				light = new OGLRendererSpotLight(*static_cast<const RendererSpotLightDesc*>(&desc), *this);
				break;
		}
	}
	return light;
}

void OGLRenderer::bindViewProj(const physx::PxMat34Legacy &eye, const RendererProjection &proj)
{
	physx::PxMat34Legacy inveye;
	eye.getInverseRT(inveye);
	
	// load the projection matrix...
	GLfloat glproj[16];
	RenToGL(glproj, proj);
	
	// load the view matrix...
	m_viewMatrix = inveye;
	
#if defined(RENDERER_ENABLE_CG)
	GLfloat glview[16];
	PxToGL(glview, inveye);
	const physx::PxVec3 &eyePosition  =  eye.t;
	const physx::PxVec3  eyeDirection = -eye.M.getColumn(2);
	if(m_cgEnv.viewMatrix)	 cgGLSetMatrixParameterfc(m_cgEnv.viewMatrix,    glview);
	if(m_cgEnv.projMatrix)   cgGLSetMatrixParameterfc(m_cgEnv.projMatrix,    glproj);
	if(m_cgEnv.eyePosition)  cgGLSetParameter3fv(     m_cgEnv.eyePosition,  &eyePosition.x);
	if(m_cgEnv.eyeDirection) cgGLSetParameter3fv(     m_cgEnv.eyeDirection, &eyeDirection.x);
#endif
}

void OGLRenderer::bindAmbientState(const RendererColor &ambientColor)
{
#if defined(RENDERER_ENABLE_CG)
	setColorParameter(m_cgEnv.ambientColor, ambientColor);
#endif
}

void OGLRenderer::bindDeferredState(void)
{
	RENDERER_ASSERT(0, "Not implemented!");
}

void OGLRenderer::bindMeshContext(const RendererMeshContext &context)
{
	RENDERER_PERFZONE(OGLRendererBindMeshContext);
	physx::PxMat34Legacy model;
	physx::PxMat34Legacy modelView;

	if(context.transform) model = *context.transform;
	else                  model.id();
	modelView.multiply(m_viewMatrix, model);
	
	GLfloat glmodelview[16];
	PxToGL(glmodelview, modelView);
	glLoadMatrixf(glmodelview);

    switch(context.cullMode)
    {
    case RendererMeshContext::CLOCKWISE: 
		glFrontFace( GL_CW );
		glCullFace( GL_BACK );
		glEnable( GL_CULL_FACE );
		break;
    case RendererMeshContext::COUNTER_CLOCKWISE: 
		glFrontFace( GL_CCW );
		glCullFace( GL_BACK );
		glEnable( GL_CULL_FACE );
		break;
    case RendererMeshContext::NONE: 
		glDisable( GL_CULL_FACE );
		break;
    default:
        RENDERER_ASSERT(0, "Invalid Cull Mode");
    }

#if defined(RENDERER_ENABLE_CG)
	GLfloat glmodel[16];
	PxToGL(glmodel, model);
	if(m_cgEnv.modelMatrix)     cgGLSetMatrixParameterfc(m_cgEnv.modelMatrix,     glmodel);
	if(m_cgEnv.modelViewMatrix) cgGLSetMatrixParameterfc(m_cgEnv.modelViewMatrix, glmodelview);
	
	RENDERER_ASSERT(context.numBones <= RENDERER_MAX_BONES, "Too many bones.");
	if(context.boneMatrices && context.numBones>0 && context.numBones <= RENDERER_MAX_BONES)
	{
	#if 0
		for(physx::PxU32 i=0; i<context.numBones; i++)
		{
			GLfloat glbonematrix[16];
			PxToGL(glbonematrix, context.boneMatrices[i]);
			cgSetMatrixParameterfc(cgGetArrayParameter(m_cgEnv.boneMatrices, i), glbonematrix);
		}
	#else
		GLfloat glbonematrix[16*RENDERER_MAX_BONES];
		for(physx::PxU32 i=0; i<context.numBones; i++)
		{
			PxToGL(glbonematrix+(16*i), context.boneMatrices[i]);			
		} 

		#if defined(RENDERER_PS3)
			OGLRendererMaterial* pMaterial = (OGLRendererMaterial*)context.material;
			if( pMaterial )
			{
				CGprogram program = pMaterial->GetVertexProgramPS3();
				cgGLSetMatrixParameterArrayfc(cgGetNamedParameter(program, "g_" "boneMatrices"),
					0, context.numBones, glbonematrix);
			}
		#else
			{
			RENDERER_PERFZONE(cgGLSetMatrixParameter);
			cgGLSetMatrixParameterArrayfc(m_cgEnv.boneMatrices, 0, context.numBones, glbonematrix);
			}
		#endif
	#endif
	}
	
#endif
}

void OGLRenderer::beginMultiPass(void)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthFunc(GL_EQUAL);
}

void OGLRenderer::endMultiPass(void)
{
	glDisable(GL_BLEND);
	glDepthFunc(GL_LESS);
}

void OGLRenderer::renderDeferredLight(const RendererLight &/*light*/)
{
	RENDERER_ASSERT(0, "Not implemented!");
}

bool OGLRenderer::isOk(void) const
{
	bool ok = true;
	ok = m_platform->isContextValid();
	return ok;
}

///////////////////////////////////////////////////////////////////////////////

/*static DWORD gCullMode;
static DWORD gAlphaBlendEnable;
static DWORD gSrcBlend;
static DWORD gDstBlend;
static DWORD gFillMode;
static DWORD gZWrite;
static DWORD gLighting;
*/

#if defined(__CELLOS_LV2__)
	#define glOrtho glOrthof
#endif

void OGLRenderer::setupTextRenderStates()
{
/*	// PT: save render states. Let's just hope this isn't a pure device, else the Get method won't work
	m_d3dDevice->GetRenderState(D3DRS_CULLMODE, &gCullMode);
	m_d3dDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &gAlphaBlendEnable);
	m_d3dDevice->GetRenderState(D3DRS_SRCBLEND, &gSrcBlend);
	m_d3dDevice->GetRenderState(D3DRS_DESTBLEND, &gDstBlend);
	m_d3dDevice->GetRenderState(D3DRS_FILLMODE, &gFillMode);
	m_d3dDevice->GetRenderState(D3DRS_ZWRITEENABLE, &gZWrite);
	m_d3dDevice->GetRenderState(D3DRS_LIGHTING, &gLighting);

	// PT: setup render states for text rendering
	m_d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_d3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	m_d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);*/

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDepthMask(false);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
}

void OGLRenderer::resetTextRenderStates()
{
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDepthMask(true);
	glEnable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
}

void OGLRenderer::renderTextBuffer(const void* verts, physx::PxU32 nbVerts, const physx::PxU16* indices, physx::PxU32 nbIndices)
{
	const int stride = sizeof(TextVertex);
	char* data = (char*)verts;

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// pos
	glVertexPointer(3, GL_FLOAT, stride, data);
	data += 3*sizeof(float);

	// rhw
	physx::PxU32 width, height;
	getWindowSize(width, height);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, width, height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	data += sizeof(float);

	// Diffuse color
	glColorPointer(4, GL_UNSIGNED_BYTE, stride, data);
	data += sizeof(int);

	// Texture coordinates
	glTexCoordPointer(2, GL_FLOAT, stride, data);
	data += 2*sizeof(float);

	glDrawElements(GL_TRIANGLES, nbIndices, GL_UNSIGNED_SHORT, indices);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void OGLRenderer::setupScreenquadRenderStates()
{
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDepthMask(false);
	glDisable(GL_LIGHTING);
}

void OGLRenderer::resetScreenquadRenderStates()
{
	glEnable(GL_CULL_FACE);
	glDepthMask(true);
	glEnable(GL_LIGHTING);
}

#endif // #if defined(RENDERER_ENABLE_OPENGL)
