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
#ifndef SAMPLE_APEX_RENDERER_H
#define SAMPLE_APEX_RENDERER_H

#include <NxApex.h>

#include <vector>

#include <SampleAsset.h>

class Renderer;
class RendererMaterial;
class SampleAssetManager;

namespace physx
{
	namespace apex
	{
		class NxApexSDK;
		class NxModuleParticles;
		class NxModuleIofx;
		class NxModuleDestructible;
		class NxModuleTurbulence;
		class NxModuleExplosion;
		class NxModuleForceField;
		class NxModuleEmitter;
		class NxModuleClothing;
	}
}

// TODO: DISABLE ME!!!
#define WORK_AROUND_BROKEN_ASSET_PATHS 1
#define APEX_CALLBACK_NAME_STR_LEN 1024

class SampleApexRenderResourceManager : public physx::apex::NxUserRenderResourceManager
{
	public:
											SampleApexRenderResourceManager(Renderer &renderer);
		virtual								~SampleApexRenderResourceManager(void);
		
	public:
		virtual physx::apex::NxUserRenderVertexBuffer*		createVertexBuffer( const physx::apex::NxUserRenderVertexBufferDesc &desc );
		virtual void										releaseVertexBuffer( physx::apex::NxUserRenderVertexBuffer &buffer );

		virtual physx::apex::NxUserRenderIndexBuffer*		createIndexBuffer( const physx::apex::NxUserRenderIndexBufferDesc &desc );
		virtual void										releaseIndexBuffer( physx::apex::NxUserRenderIndexBuffer &buffer );

		virtual physx::apex::NxUserRenderBoneBuffer*		createBoneBuffer( const physx::apex::NxUserRenderBoneBufferDesc &desc );
		virtual void										releaseBoneBuffer( physx::apex::NxUserRenderBoneBuffer &buffer );

		virtual physx::apex::NxUserRenderInstanceBuffer*	createInstanceBuffer( const physx::apex::NxUserRenderInstanceBufferDesc &desc );
		virtual void										releaseInstanceBuffer( physx::apex::NxUserRenderInstanceBuffer &buffer );

		virtual physx::apex::NxUserRenderSpriteBuffer*		createSpriteBuffer( const physx::apex::NxUserRenderSpriteBufferDesc &desc );
		virtual void										releaseSpriteBuffer( physx::apex::NxUserRenderSpriteBuffer &buffer );

		virtual physx::apex::NxUserRenderResource*			createResource( const physx::apex::NxUserRenderResourceDesc &desc );
		virtual void										releaseResource( physx::apex::NxUserRenderResource &resource );
		
		virtual physx::PxU32								getMaxBonesForMaterial(void *material);
		
	private:
		Renderer& m_renderer;
		
		physx::PxU32	m_numVertexBuffers;
		physx::PxU32	m_numIndexBuffers;
		physx::PxU32	m_numBoneBuffers;
		physx::PxU32	m_numInstanceBuffers;
		physx::PxU32	m_numResources;
};

class SampleApexRenderer : public physx::apex::NxUserRenderer
{
	public:
		virtual void renderResource(const physx::apex::NxApexRenderContext &context);
};

class SampleApexResourceCallback : public physx::apex::NxResourceCallback
{
	public:
								SampleApexResourceCallback(Renderer &renderer, SampleAssetManager &assetManager);
		virtual				   ~SampleApexResourceCallback(void);
		
		void					addResourceSearchPath(const char *path);
		void					removeResourceSearchPath(const char *path);
		void					clearResourceSearchPaths();

		void					registerNxCollisionGroup(const char *name, NxCollisionGroup group);
		void					registerNxGroupsMask128(const char *name, NxGroupsMask& groupsMask);
		void					registerNxCollisionGroupsMask(const char *name, NxU32 groupsBitMask);
		void					registerPhysicalMaterial(const char *name, NxMaterialIndex physicalMaterial);
		void					setApexSupport(physx::apex::NxApexSDK &apexSDK, physx::apex::NxModuleParticles *apexParticlesModule = NULL);
		void					setIofxModule(physx::apex::NxModuleIofx &apexIofxModule);
		void					setEmitterModule(physx::apex::NxModuleEmitter &apexEmitterModule);
		void					setDestructibleModule(physx::apex::NxModuleDestructible &apexDestructibleModule);
		void					setTurbulenceModule(physx::apex::NxModuleTurbulence &apexTurbulenceModule );
		void					setExplosionModule(physx::apex::NxModuleExplosion &apexExplosionModule );
		void					setForceFieldModule(physx::apex::NxModuleForceField &apexForceFieldModule );
		void					setClothingModule(physx::apex::NxModuleClothing &apexClothingModule );
	
	private:
		void					fixupAssetName(char *outStr, size_t outStrCapacity, const char *inStr);	
		physx::PxFileBuf	   *findApexAsset(const char *assetName);
		SampleAsset			   *findSampleAsset(const char *assetName, SampleAsset::Type type);
		
		bool					xmlFileExtension(const char *assetName);
		const char *			getFileExtension(const char *assetName);
	#if WORK_AROUND_BROKEN_ASSET_PATHS
		const char			   *mapHackyPath(const char *path);
		void					fixBrokenExtension(const char *nameSpace, char *path, physx::PxU32 pathMaxLen);
	#endif
		
	public:
		virtual void*			requestResource(const char *nameSpace, const char *name);
		virtual void			releaseResource(const char *nameSpace, const char *name, void *resource);

		bool					doesFileExist(const char *filename, const char *ext);
		bool					doesFileExist(const char *filename);
	
	protected:
		Renderer							&m_renderer;
		SampleAssetManager					&m_assetManager;
		std::vector<char*>					 m_searchPaths;
		std::vector<NxGroupsMask>			 m_nxGroupsMasks;
		physx::apex::NxApexSDK				*m_apexSDK;
		physx::apex::NxModuleParticles		*m_apexParticlesModule;
		physx::apex::NxModuleIofx			*m_apexIofxModule;
		physx::apex::NxModuleDestructible	*m_apexDestructibleModule;
		physx::apex::NxModuleTurbulence		*m_apexTurbulenceModule;
		physx::apex::NxModuleExplosion		*m_apexExplosionModule;
		physx::apex::NxModuleForceField		*m_apexForceFieldModule;
		physx::apex::NxModuleEmitter		*m_apexEmitterModule;
		physx::apex::NxModuleClothing		*m_apexClothingModule;
		physx::PxU32						 m_numGets;
};

#endif
