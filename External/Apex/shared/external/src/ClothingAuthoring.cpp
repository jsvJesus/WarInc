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
#include "ClothingAuthoring.h"

#if defined(PX_WINDOWS)

#include <cstdio>
#include <stdarg.h>
#define NOMINMAX
#include <windows.h>

// for std::sort
#include <algorithm>

// PhysX foundation
#include <PsMathUtils.h>
#include <PsTime.h>
#include <PxTaskManager.h>

// PhysX
#include <NxArray.h> // doh
#include <NxActorDesc.h>
#include <NxPlaneShapeDesc.h>
#include <NxRemoteDebugger.h>
#include <NxScene.h>

// APEX Framework
#include <NxApex.h>
#include <NxApexRenderDebug.h>
#include <NxParamUtils.h>
#include <NxApexCustomBufferIterator.h>
#include <NxFromPx.h>

// APEX Clothing
#include <NxClothingActor.h>
#include <NxClothingAssetAuthoring.h>
#include <NxClothingPreview.h>
#include <NxClothingIsoMesh.h>

// shared::external
#include "BestFit.h"
#include "MeshPainter.h"
#include "SkeletalAnim.h"
#include "TriangleMesh.h"

// shared::general
#include <AutoGeometry.h>

// samples::common
#include "UserAllocator.h"

#include <MeshImport.h>

#ifdef _DEBUG
#define VERIFY_PARAM(_A) PX_ASSERT(_A == NxParameterized::ERROR_NONE)
#else
#define VERIFY_PARAM(_A) _A
#endif


typedef physx::pubfnd2::PxU32 PxU32;
typedef physx::pubfnd2::PxVec3 PxVec3;
typedef physx::pubfnd2::PxBounds3 PxBounds3;

namespace SharedTools
{
	const unsigned int SIMPLIFICATOR_MAX_STEPS = 100000;

	class ProgressCallback : public physx::apex::IProgressListener
	{
	public:
		ProgressCallback(int totalWork, physx::apex::IProgressListener* parent) :
		  m_work(0), m_subtaskWork(0), m_totalWork( totalWork ), m_taskName( NULL ), m_parent( parent ) {}

		  void	setSubtaskWork( int subtaskWork, const char* taskName = NULL )
		  {
			  if (subtaskWork < 0)
				  subtaskWork = m_totalWork - m_work;

			  m_subtaskWork = subtaskWork;
			  assert(m_work + m_subtaskWork <= m_totalWork);
			  m_taskName = taskName;
			  setProgress(0, m_taskName);
		  }

		  void	completeSubtask()
		  {
			  setProgress(100, m_taskName);
			  m_work += m_subtaskWork;
			  m_subtaskWork = 0;
		  }

		  void	setProgress( int progress, const char* taskName = NULL )
		  {
			  assert(progress >= 0);
			  assert(progress <= 100);

			  if (taskName == NULL)
				  taskName = m_taskName;

			  if( m_parent != NULL )
			  {
				  m_parent->setProgress( m_totalWork > 0 ? (m_work*100+m_subtaskWork*progress)/m_totalWork : 100, taskName );
			  }
		  }

	protected:
		int m_work;
		int m_subtaskWork;
		int m_totalWork;
		const char* m_taskName;
		physx::apex::IProgressListener* m_parent;
	};



	ClothingAuthoring::ClothingAuthoring(NxPhysicsSDK* physicsSDK, physx::apex::NxApexSDK* apexSDK,
		physx::apex::NxModuleClothing* moduleClothing,
		physx::apex::NxResourceCallback* resourceCallback,
		physx::apex::NxUserRenderResourceManager* renderResourceManager,
		ClothingAuthoring::ErrorCallback* errorCallback ) :

		_mPhysicsSDK(physicsSDK),
		_mApexSDK(apexSDK),
		_mModuleClothing(moduleClothing),
		_mApexRenderDebug(NULL),
		_mResourceCallback(resourceCallback),
		_mRenderResourceManager(renderResourceManager),

		_mErrorCallback(errorCallback)
	{
		mSimulation.init();
		mMeshes.init();
		mAuthoringObjects.init();
		resetTempConfiguration();

		initConfiguration();
		prepareConfiguration();

		_mApexRenderDebug = _mApexSDK->createApexRenderDebug();

		mSimulation.taskManager = _mApexSDK->createTaskManager(2);

		physx::apex::NxApexSceneDesc sceneDesc;
		sceneDesc.taskManager = mSimulation.taskManager;
		mSimulation.apexScene = _mApexSDK->createScene( sceneDesc );

		if (mSimulation.apexScene == NULL)
		{
			if (_mErrorCallback != NULL)
			{
				_mErrorCallback->reportError("NxApexScene Creation", "Error: Unable to initialize APEX Scene.");
			}
		}
		else
		{
			const physx::PxU32 viewID = mSimulation.apexScene->allocViewMatrix(physx::apex::ViewMatrixType::LOOK_AT_RH);
			const physx::PxU32 projID = mSimulation.apexScene->allocProjMatrix(physx::apex::ProjMatrixType::USER_CUSTOMIZED);
		}

	}



	ClothingAuthoring::~ClothingAuthoring()
	{
		releasePhysX();

		resetTempConfiguration();

		mMeshes.clear(_mRenderResourceManager, _mResourceCallback, true);
	}


	void ClothingAuthoring::releasePhysX()
	{
		for (size_t i = 0; i < mSimulation.actors.size(); i++)
		{
			if (mSimulation.actors[i].actor != NULL)
			{
				mSimulation.actors[i].actor->release();
				mSimulation.actors[i].actor = NULL;
			}

			if (mSimulation.actors[i].preview != NULL)
			{
				mSimulation.actors[i].preview->release();
				mSimulation.actors[i].preview = NULL;
			}
		}
		mSimulation.actors.clear();

		if (mSimulation.userRecompute != NULL)
		{
			delete mSimulation.userRecompute;
			mSimulation.userRecompute = NULL;
		}

		for (size_t i = 0; i < mSimulation.assets.size(); i++)
		{
			mSimulation.assets[i]->release();
			mSimulation.assets[i] = NULL;
		}
		mSimulation.assets.clear();

		if (mSimulation.apexScene != NULL)
		{
			mSimulation.apexScene->release();
			mSimulation.apexScene = NULL;
		}

		if (mSimulation.taskManager != NULL)
		{
			mSimulation.taskManager->release();
			mSimulation.taskManager = NULL;
		}


		if (mSimulation.groundPlane != NULL)
		{
			assert(mSimulation.physxScene != NULL);
			mSimulation.physxScene->releaseActor(*mSimulation.groundPlane);
			mSimulation.groundPlane = NULL;
		}

		if (mSimulation.physxScene != NULL)
		{
			_mPhysicsSDK->releaseScene(*mSimulation.physxScene);
			mSimulation.physxScene = NULL;
		}



		mAuthoringObjects.clear();
		clearMaterialLibraries();

		if (_mApexRenderDebug != NULL)
		{
			_mApexRenderDebug->release();
			_mApexRenderDebug = NULL;
		}
	}



	int  ClothingAuthoring::getNumParameters() const
	{
		return (int)(mFloatConfiguration.size() + mIntConfiguration.size() + mBoolConfiguration.size());
	}



	bool ClothingAuthoring::getParameter(unsigned int i, std::string& name, std::string& type, std::string& val) const
	{
		if (i < mFloatConfiguration.size())
		{
			std::map<std::string, float*>::const_iterator it;
			for (it = mFloatConfiguration.begin(); i > 0 && it != mFloatConfiguration.end(); ++it, --i);

			assert(it != mFloatConfiguration.end());
			name = it->first;
			type = "float";
			char buf[16];
			sprintf_s(buf, 16, "%.9g", *it->second);
			val = buf;
			return true;
		}
		i -= (int)mFloatConfiguration.size();

		if (i < mIntConfiguration.size())
		{
			std::map<std::string, int*>::const_iterator it;
			for (it = mIntConfiguration.begin(); i > 0 && it != mIntConfiguration.end(); ++it, --i);

			assert(it != mIntConfiguration.end());
			name = it->first;
			type = "int";
			char buf[16];
			sprintf_s(buf, 16, "%d", *it->second);
			val = buf;
			return true;
		}
		i -= (int)mIntConfiguration.size();

		if (i < mBoolConfiguration.size())
		{
			std::map<std::string, bool*>::const_iterator it;
			for (it = mBoolConfiguration.begin(); i > 0 && it != mBoolConfiguration.end(); ++it, --i);

			assert(it != mBoolConfiguration.end());
			name = it->first;
			type = "bool";
			val = *it->second ? "true" : "false";
			return true;
		}

		return false;
	}



	bool ClothingAuthoring::setParameter(std::string& name, std::string& type, std::string& val)
	{
		if (type == "float")
		{
			std::map<std::string, float*>::iterator it = mFloatConfiguration.find(name);

			if (it != mFloatConfiguration.end())
			{
				*(it->second) = (float)atof(val.c_str());
				return true;
			}
			else
			{
				it = mFloatConfigurationOld.find(name);
				if (it != mFloatConfigurationOld.end())
				{
					*(it->second) = (float)atof(val.c_str());
					return true;
				}
			}
		}

		if (type == "int")
		{
			std::map<std::string, int*>::iterator it = mIntConfiguration.find(name);

			if (it != mIntConfiguration.end())
			{
				*(it->second) = atoi(val.c_str());
				return true;
			}
			else
			{
				it = mIntConfigurationOld.find(name);
				if (it != mIntConfigurationOld.end())
				{
					*(it->second) = atoi(val.c_str());
					return true;
				}
			}
		}

		if (type == "bool")
		{
			std::map<std::string, bool*>::iterator it = mBoolConfiguration.find(name);

			if (it != mBoolConfiguration.end())
			{
				*(it->second) = (val == "true");
				return true;
			}
			else
			{
				it = mBoolConfigurationOld.find(name);
				if (it != mBoolConfigurationOld.end())
				{
					*(it->second) = (val == "true");
					return true;
				}
			}
		}

		return false;
	}



	bool ClothingAuthoring::createDefaultMaterialLibrary()
	{
		if (!mState.materialLibraries.empty())
		{
			CurrentState::tMaterialLibraries::iterator found;

			// Assert that each library has one material!
			for (found = mState.materialLibraries.begin(); found != mState.materialLibraries.end(); ++found)
			{
				NxParameterized::Interface* materialLibrary = found->second;
				PX_ASSERT(materialLibrary != NULL);

				NxParameterized::Handle arrayHandle(materialLibrary);

				NxParameterized::ErrorType error = NxParameterized::ERROR_NONE;
				error = arrayHandle.getParameter("materials");
				PX_ASSERT(error == NxParameterized::ERROR_NONE);

				int numMaterials = 0;
				error = arrayHandle.getArraySize(numMaterials);
				PX_ASSERT(error == NxParameterized::ERROR_NONE);

				if (numMaterials == 0)
				{
					numMaterials = 1;
					addMaterialToLibrary(found->first.c_str(), "Default");
				}
			}

			// select one material
			found = mState.materialLibraries.find(mState.selectedMaterialLibrary);
			if (found == mState.materialLibraries.end())
			{
				found = mState.materialLibraries.begin();
				mState.selectedMaterialLibrary = found->first;
			}

			NxParameterized::Interface* materialLibrary = found->second;
			PX_ASSERT(materialLibrary != NULL);

			NxParameterized::Handle arrayHandle(materialLibrary);

			NxParameterized::ErrorType error = NxParameterized::ERROR_NONE;
			error = arrayHandle.getParameter("materials");
			PX_ASSERT(error == NxParameterized::ERROR_NONE);

			int numMaterials = 0;
			error = arrayHandle.getArraySize(numMaterials);
			PX_ASSERT(error == NxParameterized::ERROR_NONE);

			const char* firstMaterialName = NULL;
			for (int i = 0; i < numMaterials; i++)
			{
				error = arrayHandle.set(i);
				PX_ASSERT(error == NxParameterized::ERROR_NONE);

				NxParameterized::Handle nameHandle(materialLibrary);
				error = arrayHandle.getChildHandle(materialLibrary, "materialName", nameHandle);
				PX_ASSERT(error == NxParameterized::ERROR_NONE);

				const char* materialName = NULL;
				error = nameHandle.getParamString(materialName);

				if (firstMaterialName == NULL)
					firstMaterialName = materialName;

				if (mState.selectedMaterial == materialName)
					return false; // we found an existing material

				arrayHandle.popIndex();
			}

			PX_ASSERT(firstMaterialName != NULL);
			mState.selectedMaterial = firstMaterialName;

			return false;
		}

		mState.selectedMaterialLibrary = "Default";
		mState.selectedMaterial = "Default";

		addMaterialLibrary("Default", NULL);
		addMaterialToLibrary("Default", "Default");
		addMaterialToLibrary("Default", "HS Limited");
		addMaterialToLibrary("Default", "Low Gravity");

		mDirty.workspace = true;

		PX_ASSERT(mState.materialLibraries.begin()->first == "Default");
		NxParameterized::Interface* materialLibrary = mState.materialLibraries.begin()->second;

		NxParameterized::Handle arrayHandle(materialLibrary);

		NxParameterized::ErrorType error = NxParameterized::ERROR_NONE;
		error = arrayHandle.getParameter("materials");
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		int numMaterials = 0;
		error = arrayHandle.getArraySize(numMaterials);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);
		PX_ASSERT(numMaterials == 3);

		NxParameterized::Handle elementHandle(materialLibrary);

		for (int i = 0; i < numMaterials; i++)
		{
			arrayHandle.set(i);

			if (i == 2)
			{
				error = arrayHandle.getChildHandle(materialLibrary, "gravityScale", elementHandle);
				PX_ASSERT(error == NxParameterized::ERROR_NONE);
				error = elementHandle.setParamF32(0.2f);
				PX_ASSERT(error == NxParameterized::ERROR_NONE);
			}

			if (i == 1)
			{
				error = arrayHandle.getChildHandle(materialLibrary, "hardStretchLimitation", elementHandle);
				PX_ASSERT(error == NxParameterized::ERROR_NONE);
				error = elementHandle.setParamF32(1.03f);
				PX_ASSERT(error == NxParameterized::ERROR_NONE);
			}

			arrayHandle.popIndex();
		}

		return true;
	}



	bool ClothingAuthoring::addMaterialLibrary(const char* name, NxParameterized::Interface* newInterface)
	{
		assert(newInterface == NULL || strcmp(newInterface->className(), "ClothingMaterialLibraryParameters") == 0);

		CurrentState::tMaterialLibraries::iterator found = mState.materialLibraries.find(name);
		if (found != mState.materialLibraries.end())
		{
			if (newInterface == NULL)
				return true;

			if (_mErrorCallback != NULL)
			{
				_mErrorCallback->reportErrorPrintf("Material Loading", "Material \'%s\' already exists, didn't overwrite", name);
			}
			return false;
		}
		else
		{
			if (newInterface == NULL)
			{
				mState.materialLibraries[name] = _mApexSDK->getParameterizedTraits()->createNxParameterized("ClothingMaterialLibraryParameters");
			}
			else
			{
				mState.materialLibraries[name] = newInterface;
			}
		}
		return true;
	}



	void ClothingAuthoring::removeMaterialLibrary(const char* name)
	{
		CurrentState::tMaterialLibraries::iterator found = mState.materialLibraries.find(name);
		if (found != mState.materialLibraries.end())
		{
			found->second->destroy();
			mState.materialLibraries.erase(found);
		}

		createDefaultMaterialLibrary();
	}



	void ClothingAuthoring::addMaterialToLibrary(const char* libName, const char* matName)
	{
		// here starts the fun
		CurrentState::tMaterialLibraries::iterator found = mState.materialLibraries.find(libName);
		if (found == mState.materialLibraries.end())
			return;

		NxParameterized::ErrorType error = NxParameterized::ERROR_NONE;

		NxParameterized::Interface* materialLibrary = found->second;
		NxParameterized::Handle arrayHandle(materialLibrary);
		error = arrayHandle.getParameter("materials");
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		int oldSize = 0;
		error = arrayHandle.getArraySize(oldSize);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);
		error = arrayHandle.resizeArray(oldSize+1);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		error = arrayHandle.set(oldSize);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		NxParameterized::Handle elementHandle(materialLibrary);
		error = arrayHandle.getChildHandle(materialLibrary, "materialName", elementHandle);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);
		error = elementHandle.setParamString(matName);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		error = arrayHandle.getChildHandle(materialLibrary, "stretchingStiffness", elementHandle);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);
		error = elementHandle.setParamF32(1.0f);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		error = arrayHandle.getChildHandle(materialLibrary, "bendingStiffness", elementHandle);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);
		error = elementHandle.setParamF32(0.5f);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		error = arrayHandle.getChildHandle(materialLibrary, "orthoBending", elementHandle);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);
		error = elementHandle.setParamBool(false);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		error = arrayHandle.getChildHandle(materialLibrary, "compression.limit", elementHandle);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);
		error = elementHandle.setParamF32(1.0f);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		error = arrayHandle.getChildHandle(materialLibrary, "compression.stiffness", elementHandle);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);
		error = elementHandle.setParamF32(0.5f);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		error = arrayHandle.getChildHandle(materialLibrary, "damping", elementHandle);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);
		error = elementHandle.setParamF32(0.1f);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		error = arrayHandle.getChildHandle(materialLibrary, "comDamping", elementHandle);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);
		error = elementHandle.setParamBool(false);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		error = arrayHandle.getChildHandle(materialLibrary, "friction", elementHandle);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);
		error = elementHandle.setParamF32(0.25f);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		error = arrayHandle.getChildHandle(materialLibrary, "solverIterations", elementHandle);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);
		error = elementHandle.setParamU32(5);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		error = arrayHandle.getChildHandle(materialLibrary, "gravityScale", elementHandle);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);
		error = elementHandle.setParamF32(1.0f);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		error = arrayHandle.getChildHandle(materialLibrary, "hardStretchLimitation", elementHandle);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);
		error = elementHandle.setParamF32(0.0f);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		error = arrayHandle.getChildHandle(materialLibrary, "maxDistanceBias", elementHandle);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);
		error = elementHandle.setParamF32(0.0f);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		error = arrayHandle.getChildHandle(materialLibrary, "hierarchicalSolverIterations", elementHandle);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);
		error = elementHandle.setParamU32(0);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		arrayHandle.popIndex();
	}



	void ClothingAuthoring::removeMaterialFromLibrary(const char* libName, const char* matName)
	{
		// here the real fun starts
		CurrentState::tMaterialLibraries::iterator found = mState.materialLibraries.find(libName);
		if (found == mState.materialLibraries.end())
			return;

		NxParameterized::ErrorType error = NxParameterized::ERROR_NONE;

		NxParameterized::Interface* materialLibrary = found->second;
		NxParameterized::Handle arrayHandle(materialLibrary);
		error = arrayHandle.getParameter("materials");
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		int numMaterials = 0;
		error = arrayHandle.getArraySize(numMaterials);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);

		int deleteIndex = -1;

		for (int i = 0; i < numMaterials; i++)
		{
			error = arrayHandle.set(i);
			PX_ASSERT(error == NxParameterized::ERROR_NONE);

			NxParameterized::Handle stringHandle(materialLibrary);
			error = arrayHandle.getChildHandle(materialLibrary, "materialName", stringHandle);
			PX_ASSERT(error == NxParameterized::ERROR_NONE);
			const char* currentMaterialName = NULL;
			error = stringHandle.getParamString(currentMaterialName);
			PX_ASSERT(error == NxParameterized::ERROR_NONE);

			if (strcmp(currentMaterialName, matName) == 0)
			{
				PX_ASSERT(deleteIndex == -1); // should only find 1!!
				deleteIndex = i;
			}

			arrayHandle.popIndex();
		}

		if (deleteIndex == -1)
			return;


		// we should remove the deleteIndex item!

#if 1 // replace with last
		if (deleteIndex < numMaterials - 1)
		{
			error = arrayHandle.swapArrayElements(deleteIndex, numMaterials-1);
			PX_ASSERT(error == NxParameterized::ERROR_NONE);
		}
#else // erase
		for (int i = deleteIndex+1; i < numMaterials; i++)
		{
			error = arrayHandle.swapArrayElements(i-1, i);
			PX_ASSERT(error == NxParameterized::ERROR_NONE);
		}
#endif
		error = arrayHandle.resizeArray(numMaterials-1);
		PX_ASSERT(error == NxParameterized::ERROR_NONE);
	}



	void ClothingAuthoring::selectMaterial(const char* libName, const char* matName)
	{
		mDirty.workspace |= mState.selectedMaterialLibrary != libName;
		mState.selectedMaterialLibrary = libName;
		mDirty.workspace |= mState.selectedMaterial != matName;
		mState.selectedMaterial = matName;
	}



	void ClothingAuthoring::clearMaterialLibraries()
	{
		for (CurrentState::tMaterialLibraries::iterator it = mState.materialLibraries.begin(); it != mState.materialLibraries.end(); ++it)
		{
			if (it->second != NULL)
				it->second->destroy();
		}
		mState.materialLibraries.clear();
	}
	


	NxParameterized::Interface* ClothingAuthoring::getMaterialLibrary(unsigned int libraryIndex) const
	{
		NxParameterized::Interface* result = NULL;
		if (libraryIndex < mState.materialLibraries.size())
		{
			unsigned int i = 0;
			for (CurrentState::tMaterialLibraries::const_iterator it = mState.materialLibraries.begin(); it != mState.materialLibraries.end(); ++it, ++i)
			{
				if (libraryIndex == i)
				{
					result = it->second;
					break;
				}
			}
		}

		return result;
	}

	
	
	bool ClothingAuthoring::setMaterialLibrary(unsigned int libraryIndex, NxParameterized::Interface* data)
	{
		if (::strcmp(data->className(), "ClothingMaterialLibraryParameters") != 0)
			return false;

		if (libraryIndex < mState.materialLibraries.size())
		{
			unsigned int i = 0;
			for (CurrentState::tMaterialLibraries::iterator it = mState.materialLibraries.begin(); it != mState.materialLibraries.end(); ++it, ++i)
			{
				if (libraryIndex == i)
				{
					NxParameterized::Interface* oldData = it->second;
					it->second = data;

					// hopefully this is not used by any asset?
					oldData->destroy();
					return true;
				}
			}
		}
		return false;
	}



	const char* ClothingAuthoring::getMaterialLibraryName(unsigned int libraryIndex) const
	{
		const char* result = NULL;
		if (libraryIndex < mState.materialLibraries.size())
		{
			unsigned int i = 0;
			for (CurrentState::tMaterialLibraries::const_iterator it = mState.materialLibraries.begin(); it != mState.materialLibraries.end(); ++it, ++i)
			{
				if (libraryIndex == i)
				{
					result = it->first.c_str();
					break;
				}
			}
		}

		return result;
	}



	unsigned int ClothingAuthoring::getNumMaterials(unsigned int libraryIndex) const
	{
		NxParameterized::Interface* materialLibrary = getMaterialLibrary(libraryIndex);

		if (materialLibrary != NULL)
		{
			NxParameterized::ErrorType error = NxParameterized::ERROR_NONE;
			NxParameterized::Handle handle(materialLibrary);

			error = handle.getParameter("materials");
			PX_ASSERT(error == NxParameterized::ERROR_NONE);

			int numMaterials = 0;
			error = handle.getArraySize(numMaterials);
			PX_ASSERT(error == NxParameterized::ERROR_NONE);
			PX_ASSERT(numMaterials >= 0);

			return numMaterials;
		}

		return 0;
	}



	const char* ClothingAuthoring::getMaterialName(unsigned int libraryIndex, unsigned int materialIndex) const
	{
		NxParameterized::Interface* materialLibrary = getMaterialLibrary(libraryIndex);

		if (materialLibrary != NULL)
		{
			NxParameterized::ErrorType error = NxParameterized::ERROR_NONE;
			NxParameterized::Handle handle(materialLibrary);

			error = handle.getParameter("materials");
			PX_ASSERT(error == NxParameterized::ERROR_NONE);

			int numMaterials = 0;
			error = handle.getArraySize(numMaterials);
			PX_ASSERT(error == NxParameterized::ERROR_NONE);
			PX_ASSERT(numMaterials >= 0);

			if (materialIndex < (unsigned int)numMaterials)
			{
				error = handle.set(materialIndex);
				PX_ASSERT(error == NxParameterized::ERROR_NONE);

				NxParameterized::Handle nameHandle(materialLibrary);
				error = handle.getChildHandle(materialLibrary, "materialName", nameHandle);
				PX_ASSERT(error == NxParameterized::ERROR_NONE);

				const char* materialName = NULL;
				error = nameHandle.getParamString(materialName);
				PX_ASSERT(error == NxParameterized::ERROR_NONE);

				return materialName;
			}
		}

		return NULL;
	}



	physx::apex::NxClothingActor* ClothingAuthoring::getSimulationActor(size_t index)
	{
		if (index < mSimulation.actors.size())
			return mSimulation.actors[index].actor;

		return NULL;
	}



	physx::apex::NxApexRenderable* ClothingAuthoring::getSimulationRenderable(size_t index)
	{
		if (index < mSimulation.actors.size())
		{
			if (mSimulation.actors[index].actor != NULL)
				return mSimulation.actors[index].actor;
			else
				return mSimulation.actors[index].preview;
		}

		return NULL;
	}



	const physx::PxMat44& ClothingAuthoring::getSimulationActorPose(size_t index) const
	{
		assert(index < mSimulation.actors.size());
		return mSimulation.actors[index].currentPose;
	}



	void ClothingAuthoring::setActorCount(int count, bool addToCommandQueue)
	{
		if (addToCommandQueue)
		{
			char buf[64];
			sprintf_s(buf, 64, "setActorCount %d", count);
			addCommand(buf);
		}
		if (mSimulation.assets.empty())
		{
			mSimulation.actorCount = count;
		}
		else if (mSimulation.actorCount != count)
		{
			stopSimulation();
			mSimulation.actorCount = count;

			// create instances
			assert(mSimulation.assets.size() > 0);

			NxParameterized::Interface* actorDesc = mSimulation.assets[0]->getDefaultActorDesc();
			assert(actorDesc != NULL);

			NxParameterized::Interface* previewDesc = mSimulation.assets[0]->getDefaultAssetPreviewDesc();
			assert(previewDesc != NULL);


			NxParameterized::setParamBool(*actorDesc, "useHardwareCloth", mConfig.simulation.gpuSimulation);
			NxParameterized::setParamBool(*actorDesc, "flags.ParallelPhysXMeshSkinning", mConfig.apex.parallelPhysXMeshSkinning);
			NxParameterized::setParamBool(*actorDesc, "flags.ParallelMeshMeshSkinning", mConfig.apex.parallelMeshMeshSkinning);
			NxParameterized::setParamBool(*actorDesc, "flags.ParallelCpuSkinning", mConfig.apex.parallelCpuSkinning);
			NxParameterized::setParamBool(*actorDesc, "flags.RecomputeNormals", mConfig.apex.recomputeNormals);
			NxParameterized::setParamF32(*actorDesc, "maxDistanceBlendTime", mConfig.simulation.blendTime);
			NxParameterized::setParamBool(*actorDesc, "fallbackSkinning", mConfig.simulation.fallbackSkinning);
			NxParameterized::setParamBool(*actorDesc, "useInternalBoneOrder", false);
			NxParameterized::setParamBool(*actorDesc, "updateStateWithGlobalMatrices", mConfig.animation.useGlobalPoseMatrices);

			// LoD
			NxParameterized::setParamF32(*actorDesc, "lodWeights.maxDistance", 30.0f * mState.simulationValuesScale);
			NxParameterized::setParamF32(*actorDesc, "lodWeights.distanceWeight", 1.0f);
			NxParameterized::setParamF32(*actorDesc, "lodWeights.bias", 0.0f);
			NxParameterized::setParamF32(*actorDesc, "lodWeights.benefitsBias", 0.0f);

			// max distance scale
			NxParameterized::setParamF32(*actorDesc, "maxDistanceScale.Scale", mConfig.painting.maxDistanceScale);
			NxParameterized::setParamBool(*actorDesc, "maxDistanceScale.Multipliable", mConfig.painting.maxDistanceScaleMultipliable);



			// preview
			NxParameterized::setParamBool(*previewDesc, "useInternalBoneOrder", false);
			NxParameterized::setParamBool(*previewDesc, "updateStateWithGlobalMatrices", mConfig.animation.useGlobalPoseMatrices);

			const float width = std::max(mState.apexBounds.maximum.x - mState.apexBounds.minimum.x,
				std::max(mState.apexBounds.maximum.y - mState.apexBounds.minimum.y, mState.apexBounds.maximum.z - mState.apexBounds.minimum.z));

			const physx::PxMat44* skinningMatrices = NULL;
			int numSkinningMatrices = 0;

			if (mMeshes.skeleton != NULL)
			{
				if (mConfig.animation.useGlobalPoseMatrices)
				{
					numSkinningMatrices = (int)mMeshes.skeleton->getSkinningMatricesWorld().size();
					skinningMatrices = numSkinningMatrices > 0 ? &mMeshes.skeleton->getSkinningMatricesWorld()[0] : NULL;
				}
				else
				{
					numSkinningMatrices = (int)mMeshes.skeleton->getSkinningMatrices().size();
					skinningMatrices = numSkinningMatrices > 0 ? &mMeshes.skeleton->getSkinningMatrices()[0] : NULL;
				}
			}

			if (skinningMatrices != NULL)
			{
				NxParameterized::Handle actorHandle(actorDesc);
				VERIFY_PARAM(actorHandle.getParameter("boneMatrices"));
				VERIFY_PARAM(actorHandle.resizeArray(numSkinningMatrices));
				VERIFY_PARAM(actorHandle.setParamMat44Array(skinningMatrices, numSkinningMatrices));

				NxParameterized::Handle previewHandle(previewDesc);
				VERIFY_PARAM(previewHandle.getParameter("boneMatrices"));
				VERIFY_PARAM(previewHandle.resizeArray(numSkinningMatrices));
				VERIFY_PARAM(previewHandle.setParamMat44Array(skinningMatrices, numSkinningMatrices));
			}

			Simulation::ClothingActor clothingActor;

			int numActors = (int)(mSimulation.actors.size() / mSimulation.assets.size());
			assert(mSimulation.actors.size() % mSimulation.assets.size() == 0);

			if (mSimulation.actorCount > numActors)
			{
				for (int i = numActors; i < mSimulation.actorCount; i++)
				{
					physx::PxVec3 pos(0.0f, 0.0f, 0.0f);

					const float index = i + 1.0f;
					const float squareRoot = physx::PxSqrt(index);
					const float roundedSquareRoot = physx::PxFloor(squareRoot);
					const float square = roundedSquareRoot * roundedSquareRoot;
					if (physx::PxAbs(index-square) < 0.01f)
					{
						pos.x = width * 2.0f * (squareRoot - 1);
						pos.z = -pos.x;
					}
					else if (squareRoot - roundedSquareRoot < 0.5f)
					{
						pos.x = width * 2.0f * roundedSquareRoot;
						pos.z = -(i - square) * width * 2.0f;
					}
					else
					{
						pos.z = -width * 2.0f * roundedSquareRoot;
						pos.x = (i - square - roundedSquareRoot) * width * 2.0f;
					}

					if (mConfig.ui.zAxisUp)
					{
						const float t = pos.y;
						pos.y = pos.z;
						pos.z = t;
					}


					for (size_t j = 0; j < mSimulation.assets.size(); j++)
					{
						clothingActor.initPose.setPosition(pos);
						clothingActor.currentPose.setPosition(pos + mSimulation.CCTPosition);

						NxParameterized::setParamMat44(*actorDesc, "globalPose", clothingActor.currentPose);
						NxParameterized::setParamMat44(*previewDesc, "globalPose", clothingActor.currentPose);

						if (mConfig.simulation.usePreview)
						{
							physx::apex::NxApexAssetPreview* apexPreview = mSimulation.assets[j]->createApexAssetPreview(*previewDesc);
							clothingActor.preview = static_cast<physx::apex::NxClothingPreview*>(apexPreview);
						}
						else
						{
							physx::apex::NxApexActor* actor = mSimulation.assets[j]->createApexActor(*actorDesc, *mSimulation.apexScene);
							clothingActor.actor = static_cast<physx::apex::NxClothingActor*>(actor);

							clothingActor.actor->setUserRecompute(mSimulation.userRecompute);
						}

						mSimulation.actors.push_back(clothingActor);
					}
				}
			}
			else if (mSimulation.actorCount < numActors)
			{
				while (mSimulation.actors.size() > mSimulation.actorCount * mSimulation.assets.size())
				{
					Simulation::ClothingActor clothingActor = mSimulation.actors.back();
					mSimulation.actors.pop_back();

					if (clothingActor.actor != NULL)
					{
						clothingActor.actor->release();
						clothingActor.actor = NULL;
					}
					if (clothingActor.preview != NULL)
					{
						clothingActor.preview->release();
						clothingActor.preview = NULL;
					}
				}
			}
		}
	}



	void ClothingAuthoring::setMatrices(const physx::PxMat44& viewMatrix, const physx::PxMat44& projectionMatrix)
	{
		if (mSimulation.apexScene != NULL)
		{
			mSimulation.apexScene->setViewMatrix(viewMatrix, 0);
			mSimulation.apexScene->setProjMatrix(projectionMatrix, 0);
			mSimulation.apexScene->setUseViewProjMatrix(0, 0);
		}
	}



	void ClothingAuthoring::startCreateApexScene()
	{
		stopSimulation();

		if (mSimulation.apexScene == NULL)
			return;
		
		mConfig.simulation.budgetPercent = 100;

		for (size_t i = 0; i < mSimulation.assets.size(); i++)
		{
			mSimulation.assets[i]->release();
			mSimulation.assets[i] = NULL;
		}
		mSimulation.assets.clear();
		mSimulation.actors.clear(); // deleted when releasing all the assets :)

		if (mSimulation.physxScene != NULL)
		{
			mSimulation.apexScene->setPhysXScene(NULL);

			_mApexSDK->getPhysXSDK()->releaseScene(*mSimulation.physxScene);
			mSimulation.physxScene = NULL;
			mSimulation.groundPlane = NULL;
		}

		// turn on debug rendering
		_mApexSDK->getPhysXSDK()->setParameter(NX_VISUALIZATION_SCALE, 1.0f);

		{
			NxParameterized::Interface* debugRendering = mSimulation.apexScene->getDebugRenderParams();

			NxParameterized::setParamBool(*debugRendering, "VISUALIZATION_ENABLE", true);
			NxParameterized::setParamF32(*debugRendering, "VISUALIZATION_SCALE", 1.0f);
		}

		{
			NxSceneDesc sceneDesc;
			sceneDesc.maxTimestep = 1.0f / ClothingAuthoring::getSimulationFrequency();

			sceneDesc.flags &= ~NX_SF_SEQUENTIAL_PRIMARY;
			//sceneDesc.flags |= NX_SF_SEQUENTIAL_PRIMARY;

			mSimulation.physxScene = _mApexSDK->getPhysXSDK()->createScene(sceneDesc);
		}

		mSimulation.apexScene->setPhysXScene(mSimulation.physxScene);

		// connect PVD
		if (!_mApexSDK->getPhysXSDK()->getFoundationSDK().getRemoteDebugger()->isConnected())
		{
			_mApexSDK->getPhysXSDK()->getFoundationSDK().getRemoteDebugger()->connect("localhost");
		}

		if (mMeshes.inputMesh != NULL)
		{
			for (int i = 0; i < (int)mMeshes.inputMesh->getNumSubmeshes(); i++)
			{
				mMeshes.inputMesh->setSubMeshHasPhysics(i, mMeshes.inputMesh->getSubMesh(i)->selected);
			}
		}
	}



	bool ClothingAuthoring::addAssetToScene(physx::apex::NxClothingAsset* clothingAsset, bool remapBones, physx::apex::IProgressListener* progress)
	{
		if (mSimulation.apexScene == NULL)
			return false;

		if (clothingAsset != NULL)
		{
			assert(!mSimulation.clearAssets || mSimulation.assets.empty());
			mSimulation.clearAssets = false;
			mSimulation.assets.push_back(clothingAsset);

			if (remapBones && mMeshes.skeleton != NULL)
			{
				const std::vector<Samples::SkeletalBone>& bones = mMeshes.skeleton->getBones();
				for (unsigned int i = 0; i < (unsigned int)bones.size(); i++)
				{
					clothingAsset->remapBoneIndex(bones[i].name.c_str(), i);
				}
			}
			return true;
		}
		else
		{
			assert(mSimulation.assets.empty());
			mSimulation.clearAssets = true;
			physx::apex::NxClothingAssetAuthoring* authoringAsset = getClothingAsset(progress);
			if (authoringAsset != NULL)
			{
#if 1
				physx::apex::NxApexAsset* asset = _mApexSDK->createAsset(*authoringAsset, "TheSimulationAsset");
#else
				static bool flip = true;
				if (flip)
				{
					physx::PxMat44 mat = physx::PxMat44::identity();
					mat(1,1) = -1;
					authoringAsset->applyTransformation(mat, 1.0f, true, true);
				}
				// PH: test the releaseAndReturnNxParameterizedInterface() method
				PX_ASSERT(mAuthoringObjects.clothingAssetAuthoring == authoringAsset);
				NxParameterized::Interface* authoringInterface = authoringAsset->releaseAndReturnNxParameterizedInterface();

				physx::apex::NxApexAsset* asset = NULL;
				if (authoringInterface != NULL)
				{
					mAuthoringObjects.clothingAssetAuthoring = authoringAsset = NULL;
					asset = _mApexSDK->createAsset(authoringInterface, "TheSimulationAsset");
				}
#endif
				if (asset != NULL)
				{
					if (strcmp(asset->getObjTypeName(), NX_CLOTHING_AUTHORING_TYPE_NAME) == 0)
					{
						mSimulation.assets.push_back(static_cast<physx::apex::NxClothingAsset*>(asset));
						return true;
					}
					else
					{
						_mApexSDK->releaseAsset(*asset);
						return false;
					}
				}
			}
		}
		return false;
	}



	void ClothingAuthoring::finishCreateApexScene(bool recomputeScale)
	{
		if (mSimulation.apexScene == NULL)
			return;

		int oldActorCount = std::max(1, mSimulation.actorCount);
		mSimulation.actorCount = 0;

		float time = 0.0f;
		if (mMeshes.skeleton != NULL && !mMeshes.skeleton->getAnimations().empty())
		{
			Samples::SkeletalAnimation* anim = mMeshes.skeleton->getAnimations()[0];
			clampAnimation(time, false, false, anim->minTime, anim->maxTime);
		}
		setAnimationTimes(time);

		updateAnimation(0);

		mState.apexBounds.setEmpty();

		if (mMeshes.inputMesh != NULL)
			mMeshes.inputMesh->getBounds(mState.apexBounds);

		for (size_t i = 0; i < mSimulation.assets.size(); i++)
			mState.apexBounds.include(mSimulation.assets[i]->getBoundingBox());

		if (mState.gravityValueScale != 0.0f)
		{
			// loaded from a .ctp
			mState.simulationValuesScale = mState.gravityValueScale;
		}
		else if (recomputeScale)
		{
			// loaded without a .ctp, directly an .apx or .apb
			float scale = 0.5f * mState.apexBounds.minimum.distance(mState.apexBounds.maximum);
			if (scale >= 0.5f && scale <= 2.0f)
				scale = 1.0f;

			mState.simulationValuesScale = scale;

			if (mState.apexBounds.minimum.y < mState.apexBounds.minimum.z * 10)
				setZAxisUp(true);
			else if (mState.apexBounds.minimum.z < mState.apexBounds.minimum.y * 10)
				setZAxisUp(false);
		}

		setActorCount(oldActorCount, false); // creates the actors and computes mApexBounds

		mSimulation.CCTPosition.setZero();
		mSimulation.CCTDirection.setZero();

		mDirty.gravity = true;
		mDirty.groundPlane = true;

		mSimulation.paused = false;
		assert(mSimulation.running == false);
	}



	void ClothingAuthoring::startSimulation()
	{
		if (mSimulation.assets.empty() || mSimulation.running)
			return;

		if (getAndClearNeedsRestart())
			restartSimulation(); // using setActorCount twice

		if (mSimulation.stepsUntilPause > 0)
		{
			mSimulation.paused = false;
			if (mConfig.animation.selectedAnimation >= 0)
			{
				mSimulation.stepsUntilPause--;

				if (mSimulation.stepsUntilPause == 0)
				{
					setAnimation(-mConfig.animation.selectedAnimation);
					mSimulation.paused = true;
				}
			}
			else
			{
				mSimulation.stepsUntilPause = 0;
			}
		}

		if (mSimulation.paused)
			return;

		float deltaT = 1.0f / mConfig.simulation.frequency;
		if (mConfig.simulation.timingNoise != 0.0f)
		{
			const float randomValue = (::rand() / (float)RAND_MAX);
			deltaT += randomValue * deltaT * mConfig.simulation.timingNoise;
		}

		updateCCT(deltaT);

		if (mSimulation.apexScene != NULL)
		{
			unsigned int maxBudget = 0;

			for (size_t i = 0; i < mSimulation.actors.size(); i++)
			{
				if (mSimulation.actors[i].actor != NULL)
					maxBudget += mSimulation.actors[i].actor->getMaximumSimulationBudget();
			}

			mSimulation.apexScene->setLODResourceBudget((maxBudget * mConfig.simulation.budgetPercent) / 100.0f);
		}

		stepAnimationTimes(mConfig.animation.speed * deltaT / 100.f);

		int frameDelay = 0;
		for (size_t i = 0; i < mSimulation.actors.size(); i++)
		{
			if (mSimulation.actors[i].actor != NULL)
			{
				frameDelay = mSimulation.actors[i].actor->getFrameDelay();
				break;
			}
		}
		bool animationLooped = updateAnimation(frameDelay);


		const physx::PxMat44* skinningMatrices = NULL;
		unsigned int numSkinningMatrices = 0;

		if (mMeshes.skeleton != NULL)
		{
			if (mConfig.animation.useGlobalPoseMatrices)
			{
				numSkinningMatrices = (int)mMeshes.skeleton->getSkinningMatricesWorld().size();
				skinningMatrices = numSkinningMatrices > 0 ? &mMeshes.skeleton->getSkinningMatricesWorld()[0] : NULL;
			}
			else
			{
				numSkinningMatrices = (int)mMeshes.skeleton->getSkinningMatrices().size();
				skinningMatrices = numSkinningMatrices > 0 ? &mMeshes.skeleton->getSkinningMatrices()[0] : NULL;
			}
		}

		for (size_t i = 0; i < mSimulation.actors.size(); i++)
		{
			mSimulation.actors[i].currentPose.setPosition(mSimulation.actors[i].initPose.getPosition() + mSimulation.CCTPosition);

			if (mSimulation.actors[i].preview != NULL)
			{
				mSimulation.actors[i].preview->updateState(mSimulation.actors[i].currentPose, skinningMatrices, sizeof(physx::PxMat44), numSkinningMatrices);
			}
			else
			{
				assert(mSimulation.actors[i].actor != NULL);

				mSimulation.actors[i].actor->setGraphicalLOD(mConfig.simulation.graphicalLod);

				mSimulation.actors[i].actor->updateState(mSimulation.actors[i].currentPose, skinningMatrices,
					sizeof(physx::PxMat44), numSkinningMatrices,
					!animationLooped || ClothingAuthoring::getAnimationContinuous());

				NxParameterized::Interface* actorDesc = mSimulation.actors[i].actor->getActorDesc();
				NxParameterized::Handle handle(actorDesc);

				// update wind
				if (mConfig.simulation.windVelocity <= 0.0f)
				{
					mState.windOrigin.setZero();
					mState.windTarget.setZero();
					mState.drawWindTime = 0.0f;

					NxParameterized::setParamVec3(*actorDesc, "windParams.Velocity", physx::PxVec3(0.0f));
					NxParameterized::setParamF32(*actorDesc, "windParams.Adaption", 0.0f);
				}
				else
				{
					if (getZAxisUp())
					{
						mState.windOrigin.set(mConfig.simulation.windVelocity * mState.simulationValuesScale, 0.0f, 0.0f);
						physx::PxMat33Legacy rotation;
						rotation.setRotY(physx::degToRad((float)-mConfig.simulation.windElevation));
						mState.windOrigin = rotation * mState.windOrigin;
						rotation.setRotZ(physx::degToRad((float)mConfig.simulation.windDirection));
						mState.windOrigin = rotation * mState.windOrigin;
					}
					else
					{
						mState.windOrigin.set(mConfig.simulation.windVelocity * mState.simulationValuesScale, 0.0f, 0.0f);
						physx::PxMat33Legacy rotation;
						rotation.setRotZ(physx::degToRad((float)mConfig.simulation.windElevation));
						mState.windOrigin = rotation * mState.windOrigin;
						rotation.setRotY(physx::degToRad((float)mConfig.simulation.windDirection));
						mState.windOrigin = rotation * mState.windOrigin;
					}
					mState.windTarget = mSimulation.actors[i].currentPose.getPosition();
					mState.windOrigin += mState.windTarget;

					NxParameterized::setParamVec3(*actorDesc, "windParams.Velocity", mState.windTarget - mState.windOrigin);
					NxParameterized::setParamF32(*actorDesc, "windParams.Adaption", 1.0f);

					// average the target
					mState.windTarget = 0.8f * mState.windTarget + 0.2f * mState.windOrigin;

					mState.drawWindTime = physx::PxMax(0.0f, mState.drawWindTime - deltaT);
				}

				mSimulation.actors[i].actor->forcePhysicalLod((float)mConfig.simulation.lodOverwrite);

				if (mDirty.maxDistanceScale)
				{
					NxParameterized::setParamF32(*actorDesc, "maxDistanceScale.Scale", mConfig.painting.maxDistanceScale);
					NxParameterized::setParamBool(*actorDesc, "maxDistanceScale.Multipliable", mConfig.painting.maxDistanceScaleMultipliable);
				}

				if (mDirty.clothingActorFlags)
				{
					NxParameterized::setParamBool(*actorDesc, "flags.ParallelPhysXMeshSkinning", mConfig.apex.parallelPhysXMeshSkinning);
					NxParameterized::setParamBool(*actorDesc, "flags.ParallelMeshMeshSkinning", mConfig.apex.parallelMeshMeshSkinning);
					NxParameterized::setParamBool(*actorDesc, "flags.ParallelCpuSkinning", mConfig.apex.parallelCpuSkinning);
					NxParameterized::setParamBool(*actorDesc, "flags.RecomputeNormals", mConfig.apex.recomputeNormals);
				}

				if (mDirty.blendTime)
				{
					NxParameterized::setParamF32(*actorDesc, "maxDistanceBlendTime", mConfig.simulation.blendTime);
				}
			}
		}

		mDirty.maxDistanceScale = mDirty.clothingActorFlags = mDirty.blendTime = false;

		if (getAndClearGroundPlaneDirty() && mSimulation.physxScene != NULL)
		{
			if (mSimulation.groundPlane != NULL)
			{
				mSimulation.physxScene->releaseActor(*mSimulation.groundPlane);
				mSimulation.groundPlane = NULL;
			}

			if (mConfig.simulation.groundplaneEnabled)
			{
				NxPlaneShapeDesc planeShape;
				planeShape.normal.set(0.0f, mConfig.ui.zAxisUp ? 0.0f : 1.0f, mConfig.ui.zAxisUp ? 1.0f : 0.0f);
				planeShape.d = mConfig.simulation.groundplane * mState.simulationValuesScale * 0.1f;

				NxActorDesc actorDesc;
				actorDesc.shapes.pushBack(&planeShape);

				mSimulation.groundPlane = mSimulation.physxScene->createActor(actorDesc);
			}
		}

		if (getAndClearGravityDirty() && mSimulation.physxScene != NULL)
		{
			const float scale = mState.gravityValueScale != 0 ? mState.gravityValueScale : mState.simulationValuesScale;
			if (mConfig.ui.zAxisUp)
				mSimulation.physxScene->setGravity(NxVec3(0.0f, 0.0f, -scale * mConfig.simulation.gravity));
			else
				mSimulation.physxScene->setGravity(NxVec3(0.0f, -scale * mConfig.simulation.gravity, 0.0f));
		}


		// advance scene
		if (mSimulation.apexScene != NULL)
		{
#if 0
			// basically a variable timestep
			mSimulation.physxScene->setTiming(deltaT, 1, NX_TIMESTEP_FIXED);
#else
			// The UE3 way of stepping
			int NumSubSteps = (int)::ceil(deltaT * mConfig.simulation.frequency);
			NumSubSteps = physx::PxClamp(NumSubSteps, 1, 2);
			float MaxSubstep = physx::PxClamp(deltaT/NumSubSteps, 0.0025f, 1.0f / mConfig.simulation.frequency);
			mSimulation.physxScene->setTiming(MaxSubstep, NumSubSteps, NX_TIMESTEP_FIXED);
#endif

			mSimulation.apexScene->simulate(deltaT);	

			mSimulation.running = true;
			mState.currentFrameNumber++;
		}
	}



	void ClothingAuthoring::stopSimulation()
	{
		if (mSimulation.running)
		{
			unsigned int errorState = 0;
			mSimulation.apexScene->fetchResults(true, &errorState);
			assert(errorState == 0);
			mSimulation.running = false;
		}
	}



	void ClothingAuthoring::restartSimulation()
	{
#if 1
		int oldActorCount = mSimulation.actorCount;
		setActorCount(0, false);
		setActorCount(oldActorCount, false);
#else
		if (mSimulation.apexScene != NULL && mSimulation.physxScene != NULL)
		{
			mSimulation.apexScene->setPhysXScene(NULL);
			mSimulation.apexScene->setPhysXScene(mSimulation.physxScene);
		}
#endif

		restartSimulationCallback();
	}



	void ClothingAuthoring::updateCCT(float deltaT)
	{
		if (mConfig.simulation.CCTSpeed > 0.0f)
		{
			const float cctSpeed = mConfig.simulation.CCTSpeed * mState.simulationValuesScale * 3.0f;
			physx::PxVec3 cctTargetSpeedVec(0.0f, 0.0f, 0.0f);
			if (!mSimulation.CCTDirection.isZero())
				cctTargetSpeedVec = mSimulation.CCTDirection * (cctSpeed / mSimulation.CCTDirection.magnitude());

			mSimulation.CCTPosition += cctTargetSpeedVec * deltaT;
		}
	}



	void ClothingAuthoring::stepsUntilPause(int steps)
	{
		mSimulation.stepsUntilPause = steps + 1; // always need one more

		if (mConfig.animation.selectedAnimation < 0)
			setAnimation(-mConfig.animation.selectedAnimation);
	}



	int ClothingAuthoring::getMaxLodValue() const
	{
		if (mSimulation.actors.size() > 0)
		{
			if (mSimulation.actors[0].actor != NULL)
			{
				float min, max;
				bool intOnly;
				mSimulation.actors[0].actor->getPhysicalLodRange(min, max, intOnly);

				return (int)max;
			}
		}
		return -1;
	}



	bool ClothingAuthoring::loadInputMesh(const char* filename, bool allowConversion, bool silentOnError)
	{
		mMeshes.clear(_mRenderResourceManager, _mResourceCallback, false);

		if (filename == NULL)
			filename = mMeshes.inputMeshFilename.c_str();

		bool OK = false;

		const char* extension = filename;

		const char* lastDir = std::max(::strrchr(filename, '/'), ::strrchr(filename, '\\'));
		if (lastDir != NULL)
			extension = lastDir+1;

		while (!OK)
		{
			extension = ::strchr(extension, '.'); // first dot in string
			if (extension == NULL)
				return false;

			extension++; // go beyond the '.'

			if (mMeshes.inputMesh == NULL)
			{
				mMeshes.inputMesh = new Samples::TriangleMesh(0);
				mMeshes.inputMesh->setTextureUVOrigin(getTextureUvOrigin());
			}

			mMeshes.inputMesh->clear(_mRenderResourceManager, _mResourceCallback);

			if (::strcmp(extension, "mesh.xml") == 0)
			{
				OK = mMeshes.inputMesh->loadFromXML(filename, true);
				if (OK)
				{
					if (mMeshes.skeleton == NULL)
					{
						mMeshes.skeleton = new Samples::SkeletalAnim();
					}

					// check whether there is a corresponding skeleton
					std::string error;
					std::string skeletonFile = filename;
					if (!mMeshes.skeleton->loadFromXML(skeletonFile.substr(0, skeletonFile.size()-8) + "skeleton.xml", error))
					{
						// error.empty() means the file couldn't be found, not an error!
						if (!error.empty() && _mErrorCallback != NULL)
						{
							_mErrorCallback->reportErrorPrintf("Skeleton Import Failure", "%s", error.c_str());
						}

						delete mMeshes.skeleton;
						mMeshes.skeleton = NULL;
					}
				}
			}
			else if (::strcmp(extension, "obj") == 0)
			{
				OK = mMeshes.inputMesh->loadFromObjFile(filename, true);
				if (OK)
				{
					mMeshes.inputMesh->moveAboveGround(0.0f);
				}
			}
			else if (::strcmp(extension, "arm") == 0 ||
				::strcmp(extension, "apx") == 0 || ::strcmp(extension, "apb") == 0)
			{
				physx::apex::NxRenderMeshAssetAuthoring* renderMeshAssetAuthoring = NULL;
				
				if (::strcmp(extension, "arm") == 0)
				{
					physx::PxFileBuf* stream = _mApexSDK->createStream(filename, physx::apex::NxApexStreamFlags::OPEN_FOR_READ);
					if (stream != NULL)
					{
						if (stream->isOpen())
						{
							renderMeshAssetAuthoring = static_cast<physx::apex::NxRenderMeshAssetAuthoring*>(_mApexSDK->createAssetAuthoring(NX_RENDER_MESH_AUTHORING_TYPE_NAME, "The Render Mesh"));
							renderMeshAssetAuthoring->deserialize(*stream);
							OK = true;
						}
						stream->release();
					}
				}
				else
				{
					// try loading it as a render mesh asset
					NxParameterized::Serializer::DeserializedData deserializedData;
					loadParameterized(filename, NULL, deserializedData, silentOnError);

					for (unsigned int i = 0; i < deserializedData.size(); i++)
					{
						if (::strcmp(deserializedData[i]->className(), "RenderMeshAssetParameters") == 0 && renderMeshAssetAuthoring == NULL)
						{
							renderMeshAssetAuthoring = static_cast<physx::apex::NxRenderMeshAssetAuthoring*>(_mApexSDK->createAssetAuthoring(deserializedData[i], "The Render Mesh"));
							OK = true;
						}
						else
						{
							deserializedData[i]->destroy();
						}
					}
				}

				if (OK)
				{
					mMeshes.inputMesh->initFrom(*renderMeshAssetAuthoring, true);

					mMeshes.skeleton = new Samples::SkeletalAnim();
					if (!mMeshes.skeleton->initFrom(*renderMeshAssetAuthoring))
					{
						delete mMeshes.skeleton;
						mMeshes.skeleton = NULL;
					}

					for (size_t i = 0; i < mAuthoringObjects.renderMeshAssets.size(); i++)
					{
						mAuthoringObjects.renderMeshAssets[i]->release();
					}
					mAuthoringObjects.renderMeshAssets.clear();

					mAuthoringObjects.renderMeshAssets.push_back(renderMeshAssetAuthoring);

					mState.renderMeshAssetLocked = true;
				}
				else if (renderMeshAssetAuthoring != NULL)
				{
					renderMeshAssetAuthoring->release();
				}
			}
			else if (physx::gMeshImport)
			{
				if (::strcmp(extension, "ezm") == 0 && getTextureUvOrigin() == physx::apex::NxTextureUVOrigin::ORIGIN_TOP_LEFT)
				{
					setTextureUvOrigin(physx::apex::NxTextureUVOrigin::ORIGIN_BOTTOM_LEFT);
				}

				FILE *fph = fopen(filename, "rb");
				if ( fph )
				{
					fseek(fph,0L,SEEK_END);
					unsigned int len = ftell(fph);
					if ( len > 0 )
					{
						fseek(fph,0L,SEEK_SET);
						unsigned char *fileData = new unsigned char[len];
						fread(fileData,len,1,fph);
						physx::Time timer;
						physx::MeshSystemContainer *msc = physx::gMeshImport->createMeshSystemContainer(filename,fileData,len,0);
						if ( msc )
						{
							if (::strcmp(extension, "gr2") == 0)
							{
								physx::gMeshImport->scale(msc,10);
							}

							physx::Time::Second elapsed = timer.getElapsedSeconds();
							if (elapsed > 5.0f && allowConversion)
							{
								int seconds = (int)physx::PxFloor((float)elapsed);
								int miliseconds = (int)physx::PxFloor((float)((elapsed - seconds) * 1000.0));

								char temp[512];
								sprintf_s(temp, 512, "Loading of %s took %d.%.3ds\nDo you want to save it to EazyMesh (ezm)?", filename, seconds, miliseconds);
								elapsed = ::MessageBox(NULL, temp, "File Import Conversion", MB_YESNO) == IDYES ? 1.0 : 0.0;
							}
							else
							{
								elapsed = 0.0f;
							}

							if (elapsed == 1.0)
							{
								std::string newFileName = filename;
								physx::MeshSystem *ms = physx::gMeshImport->getMeshSystem(msc);
								physx::MeshSerializeFormat format = physx::MSF_EZMESH;
								if (::strcmp(extension, "ezm") == 0)
								{
									format = physx::MSF_PSK;
									newFileName = newFileName.substr(0, newFileName.size()-4) + "psk";
								}
								else
								{
									newFileName = newFileName.substr(0, newFileName.size()-4) + "ezm";
								}

								physx::MeshSerialize data(format);
								bool ok = physx::gMeshImport->serializeMeshSystem(ms,data);
								if ( ok && data.mBaseData )
								{
									FILE *fph = fopen(newFileName.c_str(), "wb");
									if ( fph )
									{
										fwrite(data.mBaseData, data.mBaseLen, 1, fph);
										fclose(fph);
									}
								}
							}

							OK = mMeshes.inputMesh->loadFromMeshImport(msc, true);
							if (OK)
							{
								if (mMeshes.skeleton == NULL)
									mMeshes.skeleton = new Samples::SkeletalAnim();

								std::string error;
								if (!mMeshes.skeleton->loadFromMeshImport(msc, error))
								{
									assert(!error.empty());
									if (!error.empty() && _mErrorCallback != NULL)
									{
										_mErrorCallback->reportErrorPrintf("SkeletalMesh Import Error", "%s", error.c_str());
									}

									delete mMeshes.skeleton;
									mMeshes.skeleton = NULL;
								}
							}
							physx::gMeshImport->releaseMeshSystemContainer(msc);
						}
						delete [] fileData;
					}
					fclose(fph);
				}
			}
		}

		if (!OK)
		{
			if (_mErrorCallback != NULL && !silentOnError)
				_mErrorCallback->reportErrorPrintf("Mesh Loading error", "%s unrecognized file type", filename);

			delete mMeshes.inputMesh;
			mMeshes.inputMesh = NULL;

			setAuthoringState(AuthoringState::None, true);

			return false;
		}

		mMeshes.inputMeshFilename = filename;
		mMeshes.inputMesh->loadMaterials(_mResourceCallback);

		mMeshes.inputMesh->selectSubMesh(-1, true); // select all

		if (mMeshes.skeleton != NULL)
		{
			mMeshes.skeletonBehind = new Samples::SkeletalAnim();
			mMeshes.skeletonBehind->loadFromParent(mMeshes.skeleton);
		}

		physx::PxBounds3 bounds;
		mMeshes.inputMesh->getBounds(bounds);

		// as if the character's height is about 2m
		mState.simulationValuesScale = 0.5f * bounds.minimum.distance(bounds.maximum);

		// round to 1.0 if near
		if (mState.simulationValuesScale >= 0.5f && mState.simulationValuesScale <= 2.0f)
			mState.simulationValuesScale = 1.0f;


		updatePainter();

		mMeshes.subdivideHistory.clear();
		mMeshes.renameHistory.clear();
		mState.needsRedraw = true;
		mDirty.maxDistancePainting = true;

		setAuthoringState(AuthoringState::MeshLoaded, true);

		return true;
	}



	bool ClothingAuthoring::saveInputMeshToXml(const char *filename)
	{
		if (mMeshes.inputMesh == NULL || mMeshes.inputMesh->getNumVertices() == 0)
		{
			if (_mErrorCallback != NULL)
				_mErrorCallback->reportErrorPrintf("saveInputMeshToXml Error", "Load a mesh first");

			return false;
		}

		bool OK;
		int l = (int)strlen(filename);


		OK = mMeshes.inputMesh->saveToXML(filename);

		if (OK && mMeshes.skeleton != NULL && !mMeshes.skeleton->getAnimations().empty())
		{
			std::string skeletonFile = filename;

			if (strstr(filename, ".mesh.xml") != NULL)
				skeletonFile = skeletonFile.substr(0, skeletonFile.size() - 8) + "skeleton.xml";
			else
				skeletonFile = skeletonFile.substr(0, skeletonFile.size() - 3) + "skeleton.xml";

			OK = mMeshes.skeleton->saveToXML(skeletonFile.c_str());
		}

		if (!OK) {
			if (_mErrorCallback != NULL)
				_mErrorCallback->reportErrorPrintf("SaveMeshToXML Error", "%s save error", filename);

			return false;
		}

		return true;
	}



	void ClothingAuthoring::selectSubMesh(int subMeshNr, bool on) 
	{
		if (mMeshes.inputMesh == NULL)
			return;

		bool dirtyChanged = false;
		if (subMeshNr == -1)
		{
			for (size_t i = 0; i < mMeshes.inputMesh->getNumSubmeshes(); i++)
			{
				const Samples::TriangleSubMesh* submesh = mMeshes.inputMesh->getSubMesh(i);
				if (submesh != NULL)
				{
					dirtyChanged |= submesh->selected != on;
				}
			}
		}
		else
		{
			const Samples::TriangleSubMesh* submesh = mMeshes.inputMesh->getSubMesh(subMeshNr);
			if (submesh != NULL)
			{
				dirtyChanged |= submesh->selected != on;
			}
		}

		mDirty.workspace |= dirtyChanged;
		mDirty.maxDistancePainting |= dirtyChanged;

		mMeshes.inputMesh->selectSubMesh(subMeshNr, on);

		setAuthoringState(AuthoringState::SubmeshSelectionChanged, false);

		// restrict painting:
		setPainterIndexBufferRange();
	}



	bool ClothingAuthoring::loadCustomPhysicsMesh(const char* filename)
	{
		if (filename == NULL)
		{
			if (mMeshes.customPhysicsMesh != NULL)
			{
				mMeshes.customPhysicsMesh->clear(_mRenderResourceManager, _mResourceCallback);
				delete mMeshes.customPhysicsMesh;
				mMeshes.customPhysicsMesh = NULL;
				mMeshes.customPhysicsMeshFilename.clear();
			}
			return true;
		}

		if (mMeshes.customPhysicsMesh == NULL)
			mMeshes.customPhysicsMesh = new Samples::TriangleMesh(0);

		const char* extension = filename;

		const char* lastDir = std::max(::strrchr(filename, '/'), ::strrchr(filename, '\\'));
		if (lastDir != NULL)
			extension = lastDir;

		bool OK = false;

		while (!OK)
		{
			extension = ::strchr(extension, '.'); // first dot in string
			if (extension == NULL)
				return false;

			extension++; // go beyond the '.'


			//mCustomPhysicsMeshFilename = filename;

			if (::strcmp(extension, "obj") == 0)
			{
				OK = mMeshes.customPhysicsMesh->loadFromObjFile(filename, false);
			}
			else if (::strcmp(extension, "mesh.xml") == 0)
			{
				OK = mMeshes.customPhysicsMesh->loadFromXML(filename, false);
			}
			else if (physx::gMeshImport)
			{
				FILE *fph = fopen(filename, "rb");
				if (fph != NULL)
				{
					fseek(fph, 0L, SEEK_END);
					size_t len = ftell(fph);
					if (len > 0)
					{
						fseek(fph, 0L, SEEK_SET);
						unsigned char *data = new unsigned char[len];
						fread(data, len, 1, fph);
						physx::MeshSystemContainer *msc = physx::gMeshImport->createMeshSystemContainer(filename,data,(physx::PxU32)len,0);
						if (msc != NULL)
						{
							if (::strcmp(extension, "gr2") == 0)
								physx::gMeshImport->scale(msc,10);

							OK = mMeshes.customPhysicsMesh->loadFromMeshImport(msc, true);
							physx::gMeshImport->releaseMeshSystemContainer(msc);
						}
					}
					fclose(fph);
				}
			}
		}

		if (!OK || mMeshes.customPhysicsMesh->getVertices().empty())
		{
			mMeshes.customPhysicsMesh->clear(_mRenderResourceManager, _mResourceCallback);
			delete mMeshes.customPhysicsMesh;
			mMeshes.customPhysicsMesh = NULL;

			mMeshes.customPhysicsMeshFilename.clear();
			return false;
		}

		mMeshes.customPhysicsMesh->loadMaterials(_mResourceCallback, true);
		mMeshes.customPhysicsMesh->setSubMeshColor(-1, 0xff00ff);

		mMeshes.customPhysicsMeshFilename = filename;
		return true;
	}



	void ClothingAuthoring::clearCustomPhysicsMesh()
	{
		if (mMeshes.customPhysicsMesh != NULL)
		{
			mMeshes.customPhysicsMesh->clear(_mRenderResourceManager, _mResourceCallback);
			delete mMeshes.customPhysicsMesh;
			mMeshes.customPhysicsMesh = NULL;

			mMeshes.customPhysicsMeshFilename.clear();
		}
	}



	int ClothingAuthoring::subdivideSubmesh(int subMeshNumber)
	{
		if (mMeshes.inputMesh == NULL)
			return 0;

		const size_t oldNumIndices = mMeshes.inputMesh->getNumIndices();
		const Samples::TriangleSubMesh* submesh = mMeshes.inputMesh->getSubMesh(subMeshNumber);
		if (submesh != NULL)
		{
			Meshes::SubdivideHistoryItem item;
			item.submesh = subMeshNumber;
			item.subdivision = ClothingAuthoring::getSubmeshSubdiv();
			mMeshes.subdivideHistory.push_back(item);

			mMeshes.inputMesh->subdivideSubMesh(item.submesh, item.subdivision, ClothingAuthoring::getEvenOutVertexDegrees());
			mMeshes.inputMesh->updatePaintingColors(Samples::PC_NUM_CHANNELS, ClothingAuthoring::getPaintingValueMin(), ClothingAuthoring::getPaintingValueMax(), _mApexRenderDebug);

			mDirty.workspace = true;
			updatePainter();
			setPainterIndexBufferRange();
		}
		else
		{
			// subdivide all selected
			for (int i = 0; i < (int)mMeshes.inputMesh->getNumSubmeshes(); i++)
			{
				submesh = mMeshes.inputMesh->getSubMesh(i);
				if (submesh->selected)
				{
					subdivideSubmesh(i);
				}
			}
		}

		return (int)(mMeshes.inputMesh->getNumIndices() - oldNumIndices) / 3;
	}



	void ClothingAuthoring::renameSubMeshMaterial(int submesh, const char* newName)
	{
		if (mMeshes.inputMesh == NULL)
			return;

		bool found = false;
		for (size_t i = 0; i < mMeshes.renameHistory.size(); i++)
		{
			if (mMeshes.renameHistory[i].submesh == submesh)
			{
				found = true;
				mMeshes.renameHistory[i].newName = newName;
			}
		}

		if (!found)
		{
			Meshes::SubmeshMaterialRename rename;
			rename.submesh = submesh;
			rename.newName = newName;

			mMeshes.renameHistory.push_back(rename);
		}

		mMeshes.inputMesh->setSubMeshMaterialName(submesh, newName, _mResourceCallback);

		mDirty.workspace = true;
	}



	void ClothingAuthoring::resetSubMeshMaterial(int submesh)
	{
		if (mMeshes.inputMesh == NULL)
			return;

		int found = -1;
		for (size_t i = 0; i < mMeshes.renameHistory.size(); i++)
		{
			if (mMeshes.renameHistory[i].submesh == submesh)
			{
				assert(found == -1);
				found = (int)i;
			}
		}
		if (found != -1)
		{
			mMeshes.renameHistory.erase(mMeshes.renameHistory.begin() + found);
		}

		const char* originalMaterial = mMeshes.inputMesh->getSubMesh(submesh)->originalMaterialName.c_str();
		mMeshes.inputMesh->setSubMeshMaterialName(submesh, originalMaterial, _mResourceCallback);

		mDirty.workspace |= found != -1;
	}



	void ClothingAuthoring::paint(const physx::PxVec3& rayOrigin, const physx::PxVec3& rayDirection, bool execute,
		bool leftButton, bool rightButton)
	{
		if (mMeshes.painter == NULL)
			return;

		mState.needsRedraw |= mMeshes.painter->raycastHit(); // hit last frame?

		if (mMeshes.inputMesh != NULL && execute)
		{
			physx::PxBounds3 bounds;
			mMeshes.inputMesh->getBounds(bounds);
			const float boundDiagonal = bounds.minimum.distance(bounds.maximum);

			float scale = 1.0f;
			if (mConfig.painting.channel == Samples::PC_MAX_DISTANCE)
				scale *= getAbsolutePaintingScalingMaxDistance();
			else if (mConfig.painting.channel == Samples::PC_COLLISION_DISTANCE)
				scale *= getAbsolutePaintingScalingCollisionFactor();

			const float paintingRadius = boundDiagonal * 0.002f * mConfig.painting.brushRadius;
			const float paintingColor = mConfig.painting.value / mConfig.painting.valueMax;
			mMeshes.painter->setRayAndRadius(rayOrigin, rayDirection, paintingRadius, mConfig.painting.brushMode,
				mConfig.painting.falloffExponent,
				mConfig.painting.value * scale, paintingColor);

			if ((leftButton || rightButton) && mMeshes.painter->raycastHit())
			{
				const float invalidValue = mConfig.painting.channel == Samples::PC_COLLISION_DISTANCE ? -1.1f : -0.01f;
				const float minimum = mConfig.painting.channel == Samples::PC_COLLISION_DISTANCE ? -1.1f : -0.01f;
				const float paintValue = rightButton ? invalidValue : mConfig.painting.value;

				mMeshes.painter->paintFloat(mConfig.painting.channel, minimum, mConfig.painting.valueMax, paintValue);

				ClothingAuthoring::updatePaintingColors();

				setAuthoringState(AuthoringState::PaintingChanged, false);
				mDirty.maxDistancePainting |= mConfig.painting.channel == Samples::PC_MAX_DISTANCE;
			}

			mState.needsRedraw |= mMeshes.painter->raycastHit(); // hit this frame?
		}
		else
		{
			// turn off brush
			mMeshes.painter->setRayAndRadius(rayOrigin, rayDirection, 0.0f, mConfig.painting.brushMode,
				mConfig.painting.falloffExponent, mState.simulationValuesScale,
				mConfig.painting.value / mConfig.painting.valueMax);
		}
	}



	void ClothingAuthoring::floodPainting(bool invalid)
	{
		if (mMeshes.painter != NULL)
		{
			const float paintingRadius = -1; //flood
			mMeshes.painter->setRayAndRadius(physx::PxVec3(0.0f), physx::PxVec3(0.0f), paintingRadius,
				BrushMode::PaintVolumetric, mConfig.painting.falloffExponent, 0.0f,
				mConfig.painting.value / mConfig.painting.valueMax);

			const float invalidValue = mConfig.painting.channel == Samples::PC_COLLISION_DISTANCE ? -1.1f : -0.01f;
			const float minimum = mConfig.painting.channel == Samples::PC_COLLISION_DISTANCE ? -1.1f : -0.01f;
			const float paintValue = invalid ? invalidValue : mConfig.painting.value;

			mMeshes.painter->paintFloat(mConfig.painting.channel, minimum, mConfig.painting.valueMax, paintValue);

			updatePaintingColors();

			setAuthoringState(AuthoringState::PaintingChanged, false);
			mDirty.maxDistancePainting |= mConfig.painting.channel == Samples::PC_MAX_DISTANCE;
			mDirty.workspace = true;
		}
	}



	void ClothingAuthoring::smoothPainting(int numIterations)
	{
		if (mMeshes.painter != NULL)
		{
			//mMeshes.painter->smoothFloat(mConfig.painting.channel, 0.5f, numIterations);
			mMeshes.painter->smoothFloatFast(mConfig.painting.channel, numIterations);

			updatePaintingColors();

			setAuthoringState(AuthoringState::PaintingChanged, false);
			mDirty.maxDistancePainting |= mConfig.painting.channel == Samples::PC_MAX_DISTANCE;
			mDirty.workspace = true;
		}
	}



	void ClothingAuthoring::updatePainter()
	{
		if (mMeshes.painter == NULL)
			return;

		mMeshes.painter->clear();

		Samples::TriangleMesh* inputMesh = getInputMesh();

		if (inputMesh != NULL)
		{
			const std::vector<physx::PxVec3> &verts = inputMesh->getVertices();
			const std::vector<physx::PxU32> &indices = inputMesh->getIndices();

			if ( !verts.empty() )
			{
				mMeshes.painter->initFrom(&verts[0], (int)verts.size(), sizeof(physx::PxVec3), &indices[0], (int)indices.size(), sizeof(physx::PxU32));
				mMeshes.painter->setFloatBuffer(Samples::PC_MAX_DISTANCE, &inputMesh->getPaintChannel(Samples::PC_MAX_DISTANCE)[0].paintValue, sizeof(Samples::PaintedVertex));
				mMeshes.painter->setFloatBuffer(Samples::PC_COLLISION_DISTANCE, &inputMesh->getPaintChannel(Samples::PC_COLLISION_DISTANCE)[0].paintValue, sizeof(Samples::PaintedVertex));
				mMeshes.painter->setFloatBuffer(Samples::PC_PHYSICAL, &inputMesh->getPaintChannel(Samples::PC_PHYSICAL)[0].paintValue, sizeof(Samples::PaintedVertex));
			}
		}
	}
	


	void ClothingAuthoring::setPainterIndexBufferRange()
	{
		if (mMeshes.painter == NULL)
			return;

		mMeshes.painter->clearIndexBufferRange();

		Samples::TriangleMesh* inputMesh = getInputMesh();

		if (inputMesh != NULL && inputMesh->getNumIndices() > 0)
		{
			for (size_t i = 0; i < inputMesh->getNumSubmeshes(); i++)
			{
				const Samples::TriangleSubMesh* submesh = inputMesh->getSubMesh(i);
				if (submesh->selected)
				{
					mMeshes.painter->addIndexBufferRange(submesh->firstIndex, submesh->firstIndex + submesh->numIndices);
				}
			}
		}
	}



	void ClothingAuthoring::initGroundMesh(const char* resourceDir)
	{
		if (mMeshes.groundMesh == NULL)
		{
			mMeshes.groundMesh = new Samples::TriangleMesh(0);

			mMeshes.groundMesh->initPlane(50.0f, 5.0f, "ClothingToolGround");

			mMeshes.groundMesh->loadMaterials(_mResourceCallback, false, resourceDir);
			mMeshes.groundMesh->setCullMode(physx::apex::NxRenderCullMode::CLOCKWISE, -1);
		}
	}



	void ClothingAuthoring::updatePaintingColors()
	{
		if (mMeshes.inputMesh != NULL)
		{
			if (mConfig.painting.channel == Samples::PC_MAX_DISTANCE)
			{
				mMeshes.inputMesh->updatePaintingColors(Samples::PC_MAX_DISTANCE, mConfig.painting.valueMin, mConfig.painting.valueMax, _mApexRenderDebug);
			}
			else
			{
				const float maxDistMax = mMeshes.inputMesh->getPaintChannelMaxValue(Samples::PC_MAX_DISTANCE);
				mMeshes.inputMesh->updatePaintingColors(Samples::PC_MAX_DISTANCE, 0, maxDistMax, _mApexRenderDebug);
			}

			mMeshes.inputMesh->updatePaintingColors(Samples::PC_COLLISION_DISTANCE, 0, 0, _mApexRenderDebug);
			mMeshes.inputMesh->updatePaintingColors(Samples::PC_PHYSICAL, 0, 0, _mApexRenderDebug);

		}
	}



	bool ClothingAuthoring::getMaxDistancePaintValues(const float*& values, int& numValues, int& byteStride)
	{
		if (mMeshes.inputMesh == NULL || mMeshes.inputMesh->getPaintChannel(Samples::PC_MAX_DISTANCE).empty())
		{
			values = NULL;
			numValues = 0;
		}
		else
		{
			values = &mMeshes.inputMesh->getPaintChannel(Samples::PC_MAX_DISTANCE)[0].paintValue;
			numValues = (int)mMeshes.inputMesh->getVertices().size();
		}
		byteStride = sizeof(Samples::PaintedVertex);

		return getAndClearMaxDistancePaintingDirty();
	}



	float ClothingAuthoring::getAbsolutePaintingScalingMaxDistance()
	{
		if (mMeshes.inputMesh != NULL)
		{
			physx::PxBounds3 bounds;
			mMeshes.inputMesh->getBounds(bounds);
			return bounds.minimum.distance(bounds.maximum) * mConfig.painting.scalingMaxdistance;
		}

		return 1.0f;
	}



	float ClothingAuthoring::getAbsolutePaintingScalingCollisionFactor()
	{
		if (mMeshes.inputMesh != NULL)
		{
			physx::PxBounds3 bounds;
			mMeshes.inputMesh->getBounds(bounds);
			return bounds.minimum.distance(bounds.maximum) * mConfig.painting.scalingCollisionFactor;
		}

		return 1.0f;
	}




	void ClothingAuthoring::setPhysicalLodValues(int numValues, const float* values)
	{
		mMeshes.physicalLodValues.clear();

		for (int i = 0; i < numValues; i++)
		{
			mMeshes.physicalLodValues.push_back(values[i] * getAbsolutePaintingScalingMaxDistance());
		}
		std::sort(mMeshes.physicalLodValues.begin(), mMeshes.physicalLodValues.end());

		mState.needsRedraw = true;
	}



	void ClothingAuthoring::setAnimationPose(int position)
	{
		if (mMeshes.inputMesh == NULL || mMeshes.skeleton == NULL)
			return;

		setAnimationTime((float)position);
		const int animation = mConfig.animation.selectedAnimation;
		if (animation != 0)
		{
			mMeshes.skeleton->setAnimPose(physx::PxAbs(animation)-1, mConfig.animation.times[0], mConfig.animation.lockRootbone);
			mMeshes.inputMesh->skin(*mMeshes.skeleton);
		}
		else
		{
			mMeshes.skeleton->setBindPose();
			mMeshes.inputMesh->unskin();
		}
		mConfig.animation.showSkinnedPose = animation != 0;
	}



	void ClothingAuthoring::setBindPose()
	{
		if (mConfig.animation.showSkinnedPose)
		{
			if (mMeshes.skeleton != NULL && mMeshes.inputMesh != NULL)
			{
				mMeshes.skeleton->setBindPose();
				mMeshes.inputMesh->unskin();
			}
			mConfig.animation.showSkinnedPose = false;
		}
	}



	void ClothingAuthoring::setAnimationTime(float time)
	{
		const int animation = mConfig.animation.selectedAnimation;

		if (animation == 0 || mMeshes.skeleton == NULL ||  physx::PxAbs(animation) > (int)mMeshes.skeleton->getAnimations().size())
		{
			setAnimationTimes(0.0f);
		}
		else
		{
			Samples::SkeletalAnimation* anim = mMeshes.skeleton->getAnimations()[physx::PxAbs(animation)-1];

			const float minTime = physx::PxMax(anim->minTime, mConfig.animation.cropMin);
			const float maxTime = physx::PxMin(anim->maxTime, mConfig.animation.cropMax);
			assert(maxTime > minTime);

			const float newAnimationTime = minTime + (maxTime - minTime) * time / 100.f;
			mState.manualAnimation = physx::PxAbs(mConfig.animation.times[0] - newAnimationTime) > 0.05f;

			setAnimationTimes(newAnimationTime);

			mConfig.animation.showSkinnedPose = true;
		}
	}



	float ClothingAuthoring::getAnimationTime() const
	{
		const int animation = mConfig.animation.selectedAnimation;

		if (animation == 0 || mMeshes.skeleton == NULL || physx::PxAbs(animation) > (int)mMeshes.skeleton->getAnimations().size())
			return 0.0f;

		Samples::SkeletalAnimation* anim = mMeshes.skeleton->getAnimations()[physx::PxAbs(animation)-1];

		const float minTime = physx::PxMax(anim->minTime, mConfig.animation.cropMin);
		const float maxTime = physx::PxMin(anim->maxTime, mConfig.animation.cropMax);

		return (mConfig.animation.times[0] - minTime) * 100.f / (maxTime - minTime);
	}



	bool ClothingAuthoring::updateAnimation(int delay)
	{
		if (mMeshes.skeleton == NULL || mMeshes.inputMesh == NULL)
			return false;

		assert(mMeshes.skeletonBehind != NULL);

		bool jumped = false;
		const std::vector<Samples::SkeletalAnimation*> &anims = mMeshes.skeleton->getAnimations();
		const int animation = mConfig.animation.selectedAnimation;
		if (animation == 0 || physx::PxAbs(animation) > (int)anims.size())
		{
			mMeshes.skeleton->setBindPose();
			mMeshes.skeletonBehind->setBindPose();

			mMeshes.inputMesh->unskin();
		}
		else
		{
			int anim = physx::PxAbs(animation) - 1;

			jumped |= clampAnimation(mConfig.animation.times[0], true, mConfig.animation.loop, anims[anim]->minTime, anims[anim]->maxTime);

			mMeshes.skeleton->setAnimPose(anim, mConfig.animation.times[0], mConfig.animation.lockRootbone);
			mMeshes.skeletonBehind->setAnimPose(anim, mConfig.animation.times[delay], mConfig.animation.lockRootbone);

			mMeshes.inputMesh->skin(*mMeshes.skeleton);

			mConfig.animation.showSkinnedPose = true;
		}

		jumped |= getAndClearManualAnimation();
		return jumped;
	}



	void ClothingAuthoring::clearCollisionVolumes()
	{
		mMeshes.collisionVolumes.clear();
	}

	unsigned int ClothingAuthoring::generateCollisionVolumes(bool useCapsule, bool commandMode, bool dirtyOnly)
	{
		if (mMeshes.skeleton == NULL)
			return 0;

		const std::vector<Samples::SkeletalBone> &bones = mMeshes.skeleton->getBones();
		int boneCount = (int)bones.size();


		for (int i = (int)mMeshes.collisionVolumes.size()-1; i >= 0; --i)
		{
			const int boneIndex = mMeshes.collisionVolumes[i].boneIndex;
			if ((bones[boneIndex].dirtyParams && dirtyOnly) || (!bones[boneIndex].manualShapes && !dirtyOnly))
			{
				mMeshes.collisionVolumes.erase(mMeshes.collisionVolumes.begin()+i);
				mMeshes.skeleton->clearShapeCount(boneIndex);
			}
		}

		unsigned int dirtyCount = 0;
		for (size_t i = 0; i < bones.size(); i++)
		{
			if ((bones[i].dirtyParams && dirtyOnly) || (!bones[i].manualShapes && !dirtyOnly))
				dirtyCount++;

		}
		if (dirtyCount == 0)
			return 0;

		if (!mMeshes.collisionVolumes.empty() && !commandMode)
			mDirty.workspace = true;

		physx::AutoGeometry* autoGeometry = createAutoGeometry();

		if (autoGeometry != NULL)
		{
			for (int i = 0; i < boneCount; i++)
			{
				const Samples::SkeletalBone& source = bones[i];
				physx::SimpleBone dest;
				dest.mOption = static_cast<physx::BoneOption>(source.boneOption);
				dest.mBoneName = source.name.c_str();
				dest.mParentIndex = source.parent;
				if ((dirtyOnly && !source.dirtyParams) || (!dirtyOnly && source.manualShapes))
				{
					// disable this bone
					dest.mBoneMinimalWeight = 100.0f;
				}
				else
				{
					dest.mBoneMinimalWeight = source.minimalBoneWeight;
					mMeshes.skeleton->setBoneDirty(i, false);
				}
				source.bindWorldPose.getColumnMajor44(dest.mTransform);
				source.invBindWorldPose.getColumnMajor44(dest.mInverseTransform);
				autoGeometry->addSimpleBone(dest);
			}


			unsigned int geomCount;
			float autoCollapsePercent = 5.0f;
			physx::SimpleHull **hulls = autoGeometry->createCollisionVolumes(autoCollapsePercent, geomCount);

			for (unsigned int i = 0; i < geomCount; i++)
			{
				addCollisionVolumeInternal(hulls[i], useCapsule && bones[hulls[i]->mBoneIndex].allowPrimitives);
			}

			physx::releaseAutoGeometry(autoGeometry);
		}

		if (useCapsule && mMeshes.skeleton != NULL && false)
		{
			for (size_t i = 0; i < mMeshes.collisionVolumes.size(); i++)
			{
				CollisionVolume& volume = mMeshes.collisionVolumes[i];

				if (mMeshes.skeleton->getBones()[volume.boneIndex].allowPrimitives)
					continue;

				physx::PxQuat quat;
				computeBestFitCapsule(volume.vertices.size(), (float*)&volume.vertices[0], sizeof(physx::PxVec3),
					volume.capsuleRadius, volume.capsuleHeight, &volume.shapeOffset.t.x, &quat.x, true);
				volume.shapeOffset.M.fromQuat(quat);

				// apply scale
				volume.capsuleRadius *= 100.0f / mState.simulationValuesScale;
				volume.capsuleHeight *= 100.0f / mState.simulationValuesScale;

				// PH: if the capsule radius is != 0, we use capsules, but we keep the convex just in case

				const float capsuleVolume = (volume.capsuleRadius * volume.capsuleRadius * NxPiF32 * volume.capsuleHeight) +
					(volume.capsuleRadius * volume.capsuleRadius * volume.capsuleRadius * 4.0f / 3.0f * NxPiF32);

				// PH: this does not seem like a good idea
				if (capsuleVolume > volume.meshVolume * 1.5f && false)
				{
					// turn off capsule if bad approximation
					volume.capsuleRadius = 0.0f;
				}
			}
		}

		return (unsigned int)mMeshes.collisionVolumes.size();
	}



	ClothingAuthoring::CollisionVolume* ClothingAuthoring::addCollisionVolume(bool useCapsule, unsigned int boneIndex)
	{
		if (mMeshes.skeleton == NULL)
			return NULL;

		const std::vector<Samples::SkeletalBone> &bones = mMeshes.skeleton->getBones();
		const size_t boneCount = bones.size();

		physx::AutoGeometry* autoGeometry = createAutoGeometry();

		if (autoGeometry != NULL)
		{
			for (size_t i = 0; i < boneCount; i++)
			{
				const Samples::SkeletalBone &source = bones[i];
				physx::SimpleBone dest;
				dest.mOption = static_cast<physx::BoneOption>(source.boneOption);
				dest.mBoneName = source.name.c_str();
				dest.mParentIndex = source.parent;
				dest.mBoneMinimalWeight = (i != boneIndex) ? 100.0f : source.minimalBoneWeight;

				source.bindWorldPose.getColumnMajor44(dest.mTransform);
				source.invBindWorldPose.getColumnMajor44(dest.mInverseTransform);

				autoGeometry->addSimpleBone(dest);
			}

			unsigned int geomCount;
			float autoCollapsePercent = 5.0f;
			physx::SimpleHull **hulls = autoGeometry->createCollisionVolumes(autoCollapsePercent, geomCount);

			assert(geomCount <= 1); // only one bone is turned on!
			for (unsigned int i=0; i<geomCount; i++) 
			{
				addCollisionVolumeInternal(hulls[i], useCapsule && bones[hulls[i]->mBoneIndex].allowPrimitives);
			}

			physx::releaseAutoGeometry(autoGeometry);

			return geomCount > 0 ? &mMeshes.collisionVolumes.back() : NULL;
		}

		return NULL;
	}



	ClothingAuthoring::CollisionVolume* ClothingAuthoring::getCollisionVolume(int index)
	{
		if (index >= 0 && (size_t)index < mMeshes.collisionVolumes.size())
			return &mMeshes.collisionVolumes[index];

		return NULL;
	}



	bool ClothingAuthoring::deleteCollisionVolume(int index)
	{
		if (index >= 0 && (size_t)index < mMeshes.collisionVolumes.size())
		{
			mMeshes.collisionVolumes.erase(mMeshes.collisionVolumes.begin() + index);
			return true;
		}

		return false;
	}



	void ClothingAuthoring::drawCollisionVolumes(bool wireframe) const
	{
		if (_mApexRenderDebug == NULL || mMeshes.collisionVolumes.empty())
			return;

		_mApexRenderDebug->pushRenderState();
		if (!wireframe)
			_mApexRenderDebug->addToCurrentState(physx::DebugRenderState::SolidShaded);
		_mApexRenderDebug->addToCurrentState(physx::DebugRenderState::CounterClockwise);


		for (size_t i = 0; i < mMeshes.collisionVolumes.size(); i++)
		{
			const CollisionVolume& volume = mMeshes.collisionVolumes[i];

			physx::PxMat34Legacy xform = volume.transform * volume.shapeOffset;

			unsigned int color = 0xdcd44eff;
			float inflation = 0.0f;
			bool selected = false;

			if ( mMeshes.skeleton != NULL )
			{
				const std::vector<Samples::SkeletalBone> &bones = mMeshes.skeleton->getBones();
				xform = bones[volume.boneIndex].currentWorldPose * volume.shapeOffset;

				if (!bones[volume.boneIndex].manualShapes)
					inflation = bones[volume.boneIndex].inflateConvex * mState.simulationValuesScale / 100.0f; // a bit magic, I know
				else if (volume.capsuleRadius <= 0.0f)
					inflation = volume.inflation * mState.simulationValuesScale / 100.0f;

				selected = bones[volume.boneIndex].selected;
			}
			// PH: render each bone with a different color
			if (selected)
			{
				color = 0xefffffff;
			}
			else
			{
				// get a color from the bone name
				const char* str = volume.boneName.c_str();
#if 0
				// djb2
				int c = *str;
				unsigned long hash = 5381 + volume.boneIndex;
				while (c = *str++)
					hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
#else
				// sdbm
				unsigned long hash = volume.boneIndex;
				while (*str)
				{
					int c = *str;
					hash = c + (hash << 6) + (hash << 16) - hash;
					str++;
				}
#endif
				color = hash | 0xff000000;
			}

			_mApexRenderDebug->setCurrentColor(color, color);

			if (volume.capsuleRadius > 0.0f)
			{
				const float height = volume.capsuleHeight * mState.simulationValuesScale / 100.0f;

				physx::PxVec3 p0(0.0f, -height * 0.5f, 0.0f);
				physx::PxVec3 p1(0.0f, height * 0.5f, 0.0f);
				xform.multiply(p0, p0);
				xform.multiply(p1, p1);

				const float radius = volume.capsuleRadius * mState.simulationValuesScale / 100.0f;
				_mApexRenderDebug->debugOrientedCapsule(radius + inflation, height, 2, xform);
			}
			else
			{
				size_t tcount = volume.indices.size()/3;
				if ( tcount > 0 )
				{

					physx::PxVec3 center(0.0f, 0.0f, 0.0f);

					if (inflation != 0.0f)
					{
						for (size_t i = 0; i < volume.vertices.size(); i++)
							center += volume.vertices[i];
						center /= (physx::PxF32)volume.vertices.size();
					}

					const unsigned int *indices = &volume.indices[0];
					for (size_t i=0; i<tcount; i++)
					{
						unsigned int i1 = indices[i*3+0];
						unsigned int i2 = indices[i*3+1];
						unsigned int i3 = indices[i*3+2];
						physx::PxVec3 p1 = volume.vertices[i1];
						physx::PxVec3 p2 = volume.vertices[i2];
						physx::PxVec3 p3 = volume.vertices[i3];
						if (inflation != 0.0f)
						{
							physx::PxVec3 out = p1 - center;
							float dist = out.normalize();
							if (dist + inflation > 0.0f)
								p1 += inflation * out;
							else
								p1 = center;

							out = p2 - center;
							dist = out.normalize();
							if (dist + inflation > 0.0f)
								p2 += inflation * out;
							else
								p2 = center;

							out = p3 - center;
							dist = out.normalize();
							if (dist + inflation > 0.0f)
								p3 += inflation * out;
							else
								p3 = center;
						}
						physx::PxVec3 t1,t2,t3;
						xform.multiply(p1,t1);
						xform.multiply(p2,t2);
						xform.multiply(p3,t3);

						_mApexRenderDebug->debugTri(t1, t2, t3);
					}
				}
			}
		}

		_mApexRenderDebug->popRenderState();
	}




	physx::apex::NxRenderMeshAssetAuthoring* ClothingAuthoring::getRenderMeshAsset(int index)
	{
		createRenderMeshAssets();

		if (index >= mConfig.cloth.numGraphicalLods)
			return NULL;

		return mAuthoringObjects.renderMeshAssets[index];
	}



	physx::apex::NxClothingPhysicalMesh* ClothingAuthoring::getClothMesh(int index, physx::apex::IProgressListener* progress)
	{
		createClothMeshes(progress);

		if (index >= (int)mAuthoringObjects.physicalMeshes.size())
			return NULL;

		return mAuthoringObjects.physicalMeshes[index];
	}



	void ClothingAuthoring::simplifyClothMesh(float factor)
	{
		for (size_t i = 0; i < mAuthoringObjects.physicalMeshes.size(); i++)
		{
			unsigned int maxSteps = (factor == 0.0f) ? SIMPLIFICATOR_MAX_STEPS : (unsigned int)(factor * mAuthoringObjects.physicalMeshes[i]->getNumVertices());
			unsigned int subdivSize = (factor == 0.0f) ? mConfig.cloth.simplify : 1;
			mAuthoringObjects.physicalMeshes[i]->simplify(subdivSize, maxSteps, -1, NULL);
		}
	}



	int ClothingAuthoring::getNumClothTriangles() const 
	{ 
		if (mAuthoringObjects.physicalMeshes.size() > 0 && !mAuthoringObjects.physicalMeshes[0]->isTetrahedralMesh())
			return mAuthoringObjects.physicalMeshes[0]->getNumIndices() / 3;

		return 0;
	}



	physx::apex::NxClothingIsoMesh* ClothingAuthoring::getIsoMesh(physx::apex::IProgressListener* progress)
	{
		createIsoMesh(progress);

		return mAuthoringObjects.isoMesh;
	}



	void ClothingAuthoring::simplifyIsoMesh(float factor, physx::apex::IProgressListener* progressParent)
	{
		ProgressCallback progress(100, progressParent);

		if (mAuthoringObjects.isoMesh != NULL || mState.authoringState < AuthoringState::PhysicalIsoMeshCreated)
		{
			progress.setSubtaskWork(40, "Create Iso Mesh");
			getIsoMesh(&progress);
			progress.completeSubtask();
		}

		assert(mAuthoringObjects.isoMesh != NULL);

		progress.setSubtaskWork(-1, "Simplify Iso Mesh");
		if (factor == 0.0f)
		{
			mAuthoringObjects.isoMesh->simplify(mConfig.softbody.isoSimplifySubdivision, SIMPLIFICATOR_MAX_STEPS, -1, &progress);
		}
		else
		{
			unsigned int numSteps = (physx::PxU32)(factor * mAuthoringObjects.isoMesh->getNumVertices());
			mAuthoringObjects.isoMesh->simplify(0, physx::PxMax(1u, numSteps), 5.0f, NULL);
		}
		progress.completeSubtask();
	}




	int ClothingAuthoring::getNumIsoSurfaceTriangles() const
	{
		if (mAuthoringObjects.isoMesh != NULL)
			return mAuthoringObjects.isoMesh->getNumIndices() / 3;

		return 0;
	}



	physx::apex::NxClothingPhysicalMesh* ClothingAuthoring::getTetraMesh(physx::apex::IProgressListener* progress)
	{
		createTetraMesh(progress);

		if (mAuthoringObjects.physicalMeshes.empty())
			return NULL;

		return mAuthoringObjects.physicalMeshes[0];
	}



	int ClothingAuthoring::getNumTetrahedra() const
	{
		assert(mAuthoringObjects.physicalMeshes.size() <= 1);
		if (mAuthoringObjects.physicalMeshes.size() == 1 && mAuthoringObjects.physicalMeshes[0]->isTetrahedralMesh())
			return mAuthoringObjects.physicalMeshes[0]->getNumIndices() / 4;

		return 0;
	}



	physx::apex::NxClothingPhysicalMesh* ClothingAuthoring::getPhysicalMesh()
	{
		return mAuthoringObjects.physicalMeshes[0];
	}



	physx::apex::NxClothingAssetAuthoring* ClothingAuthoring::getClothingAsset(physx::apex::IProgressListener* progress)
	{
		createClothingAsset(progress);

		assert(mAuthoringObjects.clothingAssetAuthoring != NULL);

		updateDeformableParameters();


		CurrentState::tMaterialLibraries::iterator found = mState.materialLibraries.find(mState.selectedMaterialLibrary);
		if (found != mState.materialLibraries.end())
		{
			// figure out material index
			NxParameterized::Interface* materialLibrary = found->second;
			NxParameterized::Handle arrayHandle(materialLibrary);

			NxParameterized::ErrorType error = NxParameterized::ERROR_NONE;
			error = arrayHandle.getParameter("materials");
			PX_ASSERT(error == NxParameterized::ERROR_NONE);

			int numMaterials = 0;
			error = arrayHandle.getArraySize(numMaterials);
			PX_ASSERT(error == NxParameterized::ERROR_NONE);

			int materialIndex = 0;
			for (int i = 0; i < numMaterials; i++)
			{
				error = arrayHandle.set(i);
				PX_ASSERT(error == NxParameterized::ERROR_NONE);

				NxParameterized::Handle nameHandle(materialLibrary);
				error = arrayHandle.getChildHandle(materialLibrary, "materialName", nameHandle);
				PX_ASSERT(error == NxParameterized::ERROR_NONE);

				const char* materialName = NULL;
				error = nameHandle.getParamString(materialName);
				PX_ASSERT(error == NxParameterized::ERROR_NONE);

				if (mState.selectedMaterial == materialName)
				{
					materialIndex = i;
					break;
				}

				arrayHandle.popIndex();
			}

			mAuthoringObjects.clothingAssetAuthoring->setMaterialLibrary(found->second, materialIndex, false);
		}


		if (!mMeshes.collisionVolumes.empty())
		{
			assert(mMeshes.skeleton != NULL);

			mAuthoringObjects.clothingAssetAuthoring->clearAllBoneActors();

			for (size_t i = 0; i < mMeshes.collisionVolumes.size(); i++)
			{
				const CollisionVolume& volume = mMeshes.collisionVolumes[i];
				const Samples::SkeletalBone& bone = mMeshes.skeleton->getBones()[volume.boneIndex];

				float inflation = (bone.manualShapes ? volume.inflation : bone.inflateConvex) * mState.simulationValuesScale / 100.0f;
				if (volume.capsuleRadius > 0.0f)
				{
					if (bone.manualShapes)
						inflation = 0.0f;

					const float radius = volume.capsuleRadius * mState.simulationValuesScale / 100.0f;
					const float height = volume.capsuleHeight * mState.simulationValuesScale / 100.0f;
					mAuthoringObjects.clothingAssetAuthoring->addBoneCapsule(volume.boneName.c_str(), radius + inflation, height, volume.shapeOffset);
				}
				else
				{
					if (inflation != 0.0f)
					{

						std::vector<physx::PxVec3> vertexCopy(volume.vertices);
						physx::PxVec3 center(0.0f, 0.0f, 0.0f);
						for (size_t i = 0; i < vertexCopy.size(); i++)
						{
							vertexCopy[i] = volume.shapeOffset * vertexCopy[i];
							center += vertexCopy[i];
						}
						center /= (float)vertexCopy.size();
						for (size_t i = 0; i < vertexCopy.size(); i++)
						{
							physx::PxVec3 out = vertexCopy[i] - center;
							const float dist = out.normalize();
							if (dist + inflation > 0.0f)
								vertexCopy[i] += inflation * out;
						}
						mAuthoringObjects.clothingAssetAuthoring->addBoneConvex(volume.boneName.c_str(), &vertexCopy[0], (unsigned int)volume.vertices.size());
					}
					else
					{
						mAuthoringObjects.clothingAssetAuthoring->addBoneConvex(volume.boneName.c_str(), &volume.vertices[0], (unsigned int)volume.vertices.size());
					}
				}
			}
		}

		return mAuthoringObjects.clothingAssetAuthoring;
	}




	void ClothingAuthoring::setZAxisUp(bool z)
	{
		mDirty.workspace |= mConfig.ui.zAxisUp != z;
		mConfig.ui.zAxisUp = z;

		setGroundplane(mConfig.simulation.groundplane);
		setGravity(mConfig.simulation.gravity);
	}



	void ClothingAuthoring::setCullMode(physx::apex::NxRenderCullMode::Enum cullMode)
	{
		mDirty.workspace |= mConfig.mesh.cullMode != cullMode;
		mConfig.mesh.cullMode = cullMode;

		if (mMeshes.inputMesh != NULL)
		{
			mMeshes.inputMesh->setCullMode(cullMode, -1);
		}
	}



	void ClothingAuthoring::setTextureUvOrigin(physx::apex::NxTextureUVOrigin::Enum origin)
	{
		mDirty.workspace |= mConfig.mesh.textureUvOrigin != origin;
		mConfig.mesh.textureUvOrigin = origin;

		if (mMeshes.inputMesh != NULL)
		{
			mMeshes.inputMesh->setTextureUVOrigin(origin);
		}
	}



	void ClothingAuthoring::setPaintingChannel(int channel)
	{
		if (mConfig.painting.channel != channel)
			mState.needsRedraw = true;

		mDirty.workspace |= mConfig.painting.channel != channel;
		mConfig.painting.channel = channel;

		switch(mConfig.painting.channel)
		{
		case Samples::PC_MAX_DISTANCE:
			mConfig.painting.value = physx::PxMax(-0.1f, mConfig.painting.value);
			break;
		case Samples::PC_COLLISION_DISTANCE:
			if (physx::PxAbs(mConfig.painting.value) > 1.0f)
			{
				mConfig.painting.value = physx::PxSign(mConfig.painting.value);
			}
			break;
		case Samples::PC_PHYSICAL:
			mConfig.painting.value = 1.0f;
			mConfig.painting.valueMax = 1.0f;
			break;
		}

		if (mConfig.painting.channel != Samples::PC_NUM_CHANNELS && mMeshes.painter == NULL)
		{
			mMeshes.painter = new SharedTools::MeshPainter();
			updatePainter();
			setPainterIndexBufferRange();
		}
	}



	void ClothingAuthoring::setPaintingValue(float val, float vmin, float vmax)
	{
		assert(val >= 0.0f);
		assert(val <= 1.0f);
		const float vval = val * vmax + (1-val) * vmin;
		mDirty.workspace |= mConfig.painting.value != vval;
		mConfig.painting.value = vval;

		if (mConfig.painting.valueMin != vmin || mConfig.painting.valueMax != vmax)
		{
			mConfig.painting.valueMin = vmin;
			mConfig.painting.valueMax = vmax;

			mState.needsRedraw = true;
			updatePaintingColors();
		}
	}



	void ClothingAuthoring::setAnimation(int animation)
	{
		if (mConfig.animation.selectedAnimation != animation)
		{
			mDirty.workspace = true;
			mState.manualAnimation = mConfig.animation.selectedAnimation != -animation;
			mConfig.animation.selectedAnimation = animation;

			if (mConfig.animation.selectedAnimation > 0)
			{
				if (!mConfig.animation.loop && mMeshes.skeleton != NULL)
				{
					// set the animation to beginning if at end if looping doesn't take care of it ?!?

					Samples::SkeletalAnimation* animation = mMeshes.skeleton->getAnimations()[mConfig.animation.selectedAnimation-1];
					if (mConfig.animation.times[0] >= animation->maxTime)
						mConfig.animation.times[0] = animation->minTime;
				}
				
				// reset frame delays
				mConfig.animation.times[1] = mConfig.animation.times[2] = mConfig.animation.times[0];
			}
		}
		else if (mConfig.animation.selectedAnimation == 0 && animation == 0 && !mSimulation.paused)
		{
			if (_mErrorCallback != NULL)
				_mErrorCallback->reportErrorPrintf("Animation Error", "Bind pose is selected, cannot play animation");
		}
		mSimulation.paused = false;
	}



	void ClothingAuthoring::stepAnimationTimes(float animStep)
	{
		mConfig.animation.times[2] = mConfig.animation.times[1];
		mConfig.animation.times[1] = mConfig.animation.times[0];

		if (mConfig.animation.selectedAnimation > 0)
		{
			mConfig.animation.times[0] += animStep;
		}
	}



	bool ClothingAuthoring::clampAnimation(float& time, bool stoppable, bool loop, float minTime, float maxTime)
	{
		bool jumped = false;

		const float animLength = maxTime - minTime;

		minTime = physx::PxMax(minTime, mConfig.animation.cropMin);
		maxTime = physx::PxMin(maxTime, mConfig.animation.cropMax);

		if (time < minTime)
		{
			if (loop)
			{
				jumped = true;
				time = maxTime;
			}
			else
			{
				if (stoppable)
					mConfig.animation.selectedAnimation = -mConfig.animation.selectedAnimation;

				time = minTime;
			}
		}
		else if (time > maxTime)
		{
			if (loop)
			{
				jumped = true;
				time = minTime;
			}
			else
			{
				if (stoppable)
					mConfig.animation.selectedAnimation = -mConfig.animation.selectedAnimation;

				time = maxTime;
			}
		}

		return jumped;
	}



	void  ClothingAuthoring::setAnimationCrop(float min, float max)
	{
		assert(min < max);
		mConfig.animation.cropMin = min;
		mConfig.animation.cropMax = max;
	}



	void ClothingAuthoring::ErrorCallback::reportErrorPrintf(const char* label, const char* fmt, ...)
	{
		const size_t stringLength = 512;
		char stringBuffer[stringLength];

		va_list va;
		va_start( va, fmt );
		vsnprintf( stringBuffer, stringLength, fmt, va );
		va_end(va);

		reportError(label, stringBuffer);
	}



	void ClothingAuthoring::resetTempConfiguration()
	{
		stopSimulation();

		mSimulation.clear();

		mMeshes.clear(_mRenderResourceManager, _mResourceCallback, false);
		mAuthoringObjects.clear();
		mState.init();

		selectMaterial("Default", "Default");
		//releasePhysX();

		mDirty.init();

		mRecordCommands.clear();
	}



	void ClothingAuthoring::initConfiguration()
	{
		mConfig.init();
	}


	void ClothingAuthoring::prepareConfiguration()
	{
		mFloatConfiguration.clear();
		mIntConfiguration.clear();
		mBoolConfiguration.clear();

#define ADD_PARAM_NEW(_CONFIG, _PARAM) _CONFIG[#_PARAM] = &_PARAM
#define ADD_PARAM_OLD(_CONFIG, _PARAMNAME, _PARAM) _CONFIG##Old[_PARAMNAME] = &_PARAM

		// configuration.UI
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.ui.zAxisUp);
		ADD_PARAM_OLD(mBoolConfiguration, "mZAxisUp", mConfig.ui.zAxisUp);

		// configuration.mesh
		ADD_PARAM_NEW(mIntConfiguration,  mConfig.mesh.originalMeshSubdivision);
		ADD_PARAM_OLD(mIntConfiguration, "mOriginalMeshSubdivision", mConfig.mesh.originalMeshSubdivision);
		ADD_PARAM_OLD(mIntConfiguration, "mSubmeshSubdiv", mConfig.mesh.originalMeshSubdivision);
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.mesh.evenOutVertexDegrees);
		ADD_PARAM_OLD(mBoolConfiguration, "mEvenOutVertexDegrees", mConfig.mesh.evenOutVertexDegrees);
		ADD_PARAM_NEW(mIntConfiguration,  mConfig.mesh.cullMode);
		ADD_PARAM_OLD(mIntConfiguration,  "mCullMode", mConfig.mesh.cullMode);
		ADD_PARAM_NEW(mIntConfiguration,  mConfig.mesh.textureUvOrigin);
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.mesh.useNewApi);
		ADD_PARAM_NEW(mIntConfiguration,  mConfig.mesh.physicalMeshType);

		// configuration.Apex
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.apex.parallelPhysXMeshSkinning);
		ADD_PARAM_OLD(mBoolConfiguration, "mParallelPhysXMeshSkinning", mConfig.apex.parallelPhysXMeshSkinning);
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.apex.parallelMeshMeshSkinning);
		ADD_PARAM_OLD(mBoolConfiguration, "mParallelMeshMeshSkinning", mConfig.apex.parallelMeshMeshSkinning);
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.apex.parallelCpuSkinning);
		ADD_PARAM_OLD(mBoolConfiguration, "mParallelCpuSkinning", mConfig.apex.parallelCpuSkinning);
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.apex.recomputeNormals);
		ADD_PARAM_OLD(mBoolConfiguration, "mRecomputeNormals", mConfig.apex.recomputeNormals);

		// configuration.tempMeshes.Cloth
		ADD_PARAM_NEW(mIntConfiguration,  mConfig.cloth.numGraphicalLods);
		ADD_PARAM_OLD(mIntConfiguration,  "mClothNumGraphicalLods", mConfig.cloth.numGraphicalLods);
		ADD_PARAM_NEW(mIntConfiguration,  mConfig.cloth.simplify);
		ADD_PARAM_OLD(mIntConfiguration,  "mClothSimplifySL", mConfig.cloth.simplify);
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.cloth.close);
		ADD_PARAM_OLD(mBoolConfiguration, "mCloseCloth", mConfig.cloth.close);
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.cloth.subdivide);
		ADD_PARAM_OLD(mBoolConfiguration, "mSlSubdivideCloth", mConfig.cloth.subdivide);
		ADD_PARAM_NEW(mIntConfiguration,  mConfig.cloth.subdivision);
		ADD_PARAM_OLD(mIntConfiguration,  "mClothMeshSubdiv", mConfig.cloth.subdivision);

		// configuration.tempMeshes.Softbody
		ADD_PARAM_NEW(mIntConfiguration,  mConfig.softbody.isoSurfaceSubdivision);
		ADD_PARAM_OLD(mIntConfiguration,  "mIsoSurfaceSubdivision", mConfig.softbody.isoSurfaceSubdivision);
		ADD_PARAM_OLD(mIntConfiguration, "mIsoGridSubdiv", mConfig.softbody.isoSurfaceSubdivision);
		ADD_PARAM_NEW(mIntConfiguration,  mConfig.softbody.isoSimplifySubdivision);
		ADD_PARAM_OLD(mIntConfiguration,  "mIsoSimplifySubdivision", mConfig.softbody.isoSimplifySubdivision);
		ADD_PARAM_OLD(mIntConfiguration, "mSimplificationSubdiv", mConfig.softbody.isoSimplifySubdivision);
		ADD_PARAM_NEW(mIntConfiguration,  mConfig.softbody.keepBiggestMeshes);
		ADD_PARAM_OLD(mIntConfiguration,  "mTetraKeepBiggestMeshes", mConfig.softbody.keepBiggestMeshes);

		// configuration.collisionVolumes
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.collisionVolumes.usePaintingChannel);
		ADD_PARAM_OLD(mBoolConfiguration, "mCollisionVolumeUsePaintingChannel", mConfig.collisionVolumes.usePaintingChannel);

		// configuration.painting
		ADD_PARAM_NEW(mIntConfiguration,   mConfig.painting.brushMode);
		ADD_PARAM_OLD(mIntConfiguration,   "mBrushMode", mConfig.painting.brushMode);
		ADD_PARAM_NEW(mFloatConfiguration, mConfig.painting.falloffExponent);
		ADD_PARAM_OLD(mFloatConfiguration, "mFalloffExponent", mConfig.painting.falloffExponent);
		ADD_PARAM_NEW(mBoolConfiguration,  mConfig.painting.ignoreUnusedVertices);
		ADD_PARAM_OLD(mBoolConfiguration,  "mPaintingIgnoreUnusedVertices", mConfig.painting.ignoreUnusedVertices);
		ADD_PARAM_NEW(mIntConfiguration,   mConfig.painting.channel);
		ADD_PARAM_OLD(mIntConfiguration,   "mPaintingChannel", mConfig.painting.channel);
		ADD_PARAM_NEW(mFloatConfiguration, mConfig.painting.value);
		ADD_PARAM_OLD(mFloatConfiguration, "mPaintingValue", mConfig.painting.value);
		//ADD_PARAM_NEW(mFloatConfiguration, mConfig.painting.valueMin); // not added on purpose
		//ADD_PARAM_NEW(mFloatConfiguration, mConfig.painting.valueMax); // not added on purpose
		ADD_PARAM_NEW(mIntConfiguration,   mConfig.painting.brushRadius);
		ADD_PARAM_OLD(mIntConfiguration,   "mBrushRadius", mConfig.painting.brushRadius);
		ADD_PARAM_NEW(mFloatConfiguration, mConfig.painting.scalingMaxdistance);
		ADD_PARAM_OLD(mFloatConfiguration, "mPaintingScalingMaxdistanceNew", mConfig.painting.scalingMaxdistance);
		ADD_PARAM_NEW(mFloatConfiguration, mConfig.painting.scalingCollisionFactor);
		ADD_PARAM_OLD(mFloatConfiguration, "mPaintingScalingCollisionFactorNew", mConfig.painting.scalingCollisionFactor);
		//ADD_PARAM_NEW(mFloatConfiguration, mConfig.painting.maxDistanceScale);
		//ADD_PARAM_NEW(mBoolConfiguration, mConfig.painting.maxDistanceScaleMultipliable);

		// configuration.setMeshes
		ADD_PARAM_NEW(mBoolConfiguration,  mConfig.setMeshes.fasterMeshMeshSkinning);
		ADD_PARAM_OLD(mBoolConfiguration,  "mFasterMeshMeshSkinning", mConfig.setMeshes.fasterMeshMeshSkinning);
		ADD_PARAM_NEW(mBoolConfiguration,  mConfig.setMeshes.deriveNormalsFromBones);
		ADD_PARAM_OLD(mBoolConfiguration,  "mDeriveNormalsFromBones", mConfig.setMeshes.deriveNormalsFromBones);

		// configuration.simulation
		ADD_PARAM_NEW(mFloatConfiguration, mConfig.simulation.frequency);
		ADD_PARAM_OLD(mFloatConfiguration, "mSimulationFrequency", mConfig.simulation.frequency);
		ADD_PARAM_NEW(mIntConfiguration,   mConfig.simulation.gravity);
		ADD_PARAM_OLD(mIntConfiguration,   "mGravity", mConfig.simulation.gravity);
		ADD_PARAM_NEW(mIntConfiguration,   mConfig.simulation.groundplane);
		ADD_PARAM_OLD(mIntConfiguration,   "mGroundplane", mConfig.simulation.groundplane);
		ADD_PARAM_NEW(mBoolConfiguration,  mConfig.simulation.groundplaneEnabled);
		ADD_PARAM_OLD(mBoolConfiguration,  "mGroundplaneEnabled", mConfig.simulation.groundplaneEnabled);
		ADD_PARAM_NEW(mIntConfiguration,   mConfig.simulation.budgetPercent);
		ADD_PARAM_OLD(mIntConfiguration,   "mBudgetPercent", mConfig.simulation.budgetPercent);
		ADD_PARAM_NEW(mFloatConfiguration, mConfig.simulation.blendTime);
		ADD_PARAM_OLD(mFloatConfiguration, "mBlendTime", mConfig.simulation.blendTime);
		//ADD_PARAM_NEW(mFloatConfiguration, mConfig.simulation.lodOverwrite); // not added on purpose
		ADD_PARAM_NEW(mIntConfiguration,   mConfig.simulation.windDirection);
		ADD_PARAM_OLD(mIntConfiguration,   "mWindDirection", mConfig.simulation.windDirection);
		ADD_PARAM_NEW(mIntConfiguration,   mConfig.simulation.windElevation);
		ADD_PARAM_OLD(mIntConfiguration,   "mWindElevation", mConfig.simulation.windElevation);
		ADD_PARAM_NEW(mIntConfiguration,   mConfig.simulation.windVelocity);
		ADD_PARAM_OLD(mIntConfiguration,   "mWindVelocity", mConfig.simulation.windVelocity);
		ADD_PARAM_NEW(mBoolConfiguration,  mConfig.simulation.gpuSimulation);
		ADD_PARAM_OLD(mBoolConfiguration,  "mGpuSimulation", mConfig.simulation.gpuSimulation);
		ADD_PARAM_NEW(mBoolConfiguration,  mConfig.simulation.fallbackSkinning);
		ADD_PARAM_OLD(mBoolConfiguration,  "mFallbackSkinning", mConfig.simulation.fallbackSkinning);
		//ADD_PARAM_NEW(mFloatConfiguration, mConfig.simulation.CCTSpeed); // Not added on purpose!
		ADD_PARAM_NEW(mIntConfiguration,   mConfig.simulation.graphicalLod);
		//ADD_PARAM_NEW(mBoolConfiguration,  mConfig.simulation.usePreview); // Not added on purpose
		//ADD_PARAM_NEW(mFloatConfiguration,  mConfig.simulation.timingNoise); // Not added on purpose

		// configuration.animation
		//ADD_PARAM_NEW(mBoolConfiguration, mConfig.animation.showSkinnedPose); // not done on purpose
		ADD_PARAM_NEW(mIntConfiguration,  mConfig.animation.selectedAnimation);
		ADD_PARAM_OLD(mIntConfiguration,  "mAnimation", mConfig.animation.selectedAnimation);
		ADD_PARAM_NEW(mIntConfiguration,  mConfig.animation.speed);
		ADD_PARAM_OLD(mIntConfiguration,  "mAnimationSpeed", mConfig.animation.speed);
		//ADD_PARAM_NEW(mIntConfiguration,  mConfig.animation.times[0]); // not done on purpose
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.animation.moveCameraWithAnimation);
		ADD_PARAM_OLD(mBoolConfiguration, "mMoveCameraWithAnimation", mConfig.animation.moveCameraWithAnimation);
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.animation.loop);
		ADD_PARAM_OLD(mBoolConfiguration, "mLoopAnimation", mConfig.animation.loop);
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.animation.lockRootbone);
		ADD_PARAM_OLD(mBoolConfiguration, "mLockRootbone", mConfig.animation.lockRootbone);
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.animation.continuous);
		ADD_PARAM_OLD(mBoolConfiguration, "mAnimationContinuous", mConfig.animation.continuous);
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.animation.useGlobalPoseMatrices);
		ADD_PARAM_OLD(mBoolConfiguration, "mUseGlobalPoseMatrices", mConfig.animation.useGlobalPoseMatrices);
		//ADD_PARAM_NEW(mFloatConfiguration,mConfig.animation.cropMin); // not added on purpose
		//ADD_PARAM_NEW(mFloatConfiguration,mConfig.animation.cropMax); // not added on purpose

		// configuration.deformable
		ADD_PARAM_NEW(mFloatConfiguration, mConfig.deformable.thickness);
		ADD_PARAM_OLD(mFloatConfiguration, "mDeformableThickness", mConfig.deformable.thickness);
		//ADD_PARAM(mBoolConfiguration, mConfig.deformable.drawThickness); // not added on purpose
		ADD_PARAM_NEW(mFloatConfiguration, mConfig.deformable.selfcollisionThickness);
		ADD_PARAM_OLD(mFloatConfiguration, "mDeformableSelfcollisionThickness", mConfig.deformable.selfcollisionThickness);
		//ADD_PARAM(mBoolConfiguration, mConfig.deformable.drawSelfcollisionThickness); // not added on purpose
		ADD_PARAM_NEW(mIntConfiguration,  mConfig.deformable.hierarchicalLevels);
		ADD_PARAM_OLD(mIntConfiguration,  "mDeformableHierarchicalLevels", mConfig.deformable.hierarchicalLevels);
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.deformable.disableCCD);
		ADD_PARAM_OLD(mBoolConfiguration, "mDeformableDisableCCD", mConfig.deformable.disableCCD);
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.deformable.selfcollision);
		ADD_PARAM_OLD(mBoolConfiguration, "mDeformableSelfcollision", mConfig.deformable.selfcollision);
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.deformable.twowayInteraction);
		ADD_PARAM_OLD(mBoolConfiguration, "mDeformableTwowayInteraction", mConfig.deformable.twowayInteraction);
		ADD_PARAM_NEW(mBoolConfiguration, mConfig.deformable.untangling);
		ADD_PARAM_OLD(mBoolConfiguration, "mDeformableUntangling", mConfig.deformable.untangling);

#undef ADD_PARAM_NEW
#undef ADD_PARAM_OLD
	}



	void ClothingAuthoring::addCommand(const char* commandString, int frameNumber)
	{
		if (frameNumber == -2)
			frameNumber = mState.currentFrameNumber;

		assert(mRecordCommands.empty() || frameNumber >= mRecordCommands.back().frameNumber);

		Command command;
		command.frameNumber = frameNumber;
		command.command = commandString;

		mRecordCommands.push_back(command);
	}



	void ClothingAuthoring::clearCommands()
	{
		mRecordCommands.clear();
	}



	bool ClothingAuthoring::loadParameterized(const char* filename, physx::PxFileBuf* filebuffer, NxParameterized::Serializer::DeserializedData& deserializedData, bool silent)
	{
		bool error = false;
		bool ownsFileBuffer = false;

		if (filebuffer == NULL)
		{
			filebuffer = _mApexSDK->createStream(filename, physx::apex::NxApexStreamFlags::OPEN_FOR_READ);
			ownsFileBuffer = true;
		}
		else if (filename == NULL)
		{
			filename = "unnamed buffer";
		}

		NxParameterized::Serializer::SerializeType inTypeExt = extensionToType(filename);

		if (filebuffer != NULL)
		{
			if (filebuffer->isOpen())
			{
				// sometimes we're reading from a ctw file here, so skip linefeeds/carriage returns until we get to valid data
				char skipByte;
				filebuffer->peek(&skipByte, 1);
				while(skipByte == '\r' || skipByte == '\n')
				{
					filebuffer->readByte();
					filebuffer->peek(&skipByte, 1);
				}
				
				NxParameterized::Serializer::SerializeType inTypeFile = _mApexSDK->getSerializeType(*filebuffer);
				PX_ASSERT(inTypeFile != NxParameterized::Serializer::NST_LAST);

				if (inTypeExt != NxParameterized::Serializer::NST_LAST && inTypeFile != inTypeExt && _mErrorCallback != NULL)
				{
					const char* realExtension = inTypeFile == NxParameterized::Serializer::NST_XML ? ".apx" : ".apb";
					_mErrorCallback->reportErrorPrintf("loadParameterized error", "File \'%s\' has wrong extension should be %s\n", filename, realExtension);
				}

				NxParameterized::Serializer* serializer = _mApexSDK->createSerializer(inTypeFile);

				NxParameterized::Serializer::ErrorType serError = NxParameterized::Serializer::ERROR_NONE;
				serError = serializer->deserialize(*filebuffer, deserializedData);

				error = parameterizedError(serError, silent ? NULL : filename);

				serializer->release();
				PX_ASSERT(error || deserializedData.size() > 0);
			}
			if (ownsFileBuffer)
			{
				filebuffer->release();
				filebuffer = NULL;
			}
		}

		if (!error && deserializedData.size() > 0)
		{
			return true;
		}

		return false;
	}



	bool ClothingAuthoring::saveParameterized(const char* filename, physx::PxFileBuf* filebuffer, const NxParameterized::Interface** pInterfaces, unsigned int numInterfaces)
	{
		NxParameterized::Serializer::SerializeType serType = extensionToType(filename);


		if (serType == NxParameterized::Serializer::NST_LAST)
		{
			if (_mErrorCallback != NULL)
			{
				_mErrorCallback->reportErrorPrintf("SaveParameterized Error", "Cannot find serialization for file \'%s\'", filename);
			}
			return false;
		}

		PX_ASSERT(pInterfaces != NULL);
		if (pInterfaces == NULL)
		{
			return false;
		}

		for (unsigned int i = 0; i  < numInterfaces; i++)
		{
			PX_ASSERT(pInterfaces[i] != NULL);
			if (pInterfaces[i] == NULL)
				return false;
		}

		bool error = false;
		bool ownsFileBuffer = false;
		if (filebuffer == NULL)
		{
			filebuffer = _mApexSDK->createStream(filename, physx::apex::NxApexStreamFlags::OPEN_FOR_WRITE);
			ownsFileBuffer = true;
		}

		if (filebuffer != NULL)
		{
			if (filebuffer->isOpen())
			{
				NxParameterized::Serializer* serializer = _mApexSDK->createSerializer(serType);
				NxParameterized::Serializer::ErrorType serError = NxParameterized::Serializer::ERROR_NONE;

				serError = serializer->serialize(*filebuffer, pInterfaces, numInterfaces);
				error = parameterizedError(serError, filename);

				serializer->release();
			}

			if (ownsFileBuffer)
			{
				filebuffer->release();
				filebuffer = NULL;
			}
		}


		// that's stupid, on error we still create the empty or half finished file!
		// maybe we should save to a memory stream
		return error;
	}



	NxParameterized::Serializer::SerializeType ClothingAuthoring::extensionToType(const char* filename) const
	{
		const char* lastSlash = std::max(strrchr(filename, '/'), strrchr(filename, '\\'));
		if (lastSlash == NULL)
			lastSlash = filename;

		const char* extension = strchr(lastSlash, '.');

		NxParameterized::Serializer::SerializeType serType = NxParameterized::Serializer::NST_LAST;

		while (serType == NxParameterized::Serializer::NST_LAST && extension != NULL)
		{
			extension++; // move beyond the '.'
			if (_stricmp(extension, "apx") == 0)
			{
				serType = NxParameterized::Serializer::NST_XML;
			}
			else if (_stricmp(extension, "apb") == 0)
			{
				serType = NxParameterized::Serializer::NST_BINARY;
			}

			extension = strchr(extension, '.');
		}

		return serType;
	}



	bool ClothingAuthoring::parameterizedError(NxParameterized::Serializer::ErrorType errorType, const char* filename)
	{
		if (errorType != NxParameterized::Serializer::ERROR_NONE)
		{
			char* errorString = NULL;
			switch (errorType)
			{
#define CASE(_A) case NxParameterized::Serializer::_A: errorString = #_A; break;
				CASE(ERROR_UNKNOWN)
				CASE(ERROR_STREAM_ERROR)
				CASE(ERROR_OBJECT_HAS_NO_ROOT_PARAMETER_DEFINITION)
				CASE(ERROR_INVALID_ARRAY)
				CASE(ERROR_VAL2STRING_FAILED)
				CASE(ERROR_STRING2VAL_FAILED)
				CASE(ERROR_INVALID_FILE_FORMAT)
				CASE(ERROR_INVALID_PARAMETERIZED_OBJECT)
				CASE(ERROR_ARRAY_INDEX_OUT_OF_RANGE)
				CASE(ERROR_UNKNOWN_STRUCT_MEMBER)
				CASE(ERROR_UNKNOWN_DATA_TYPE)
				CASE(ERROR_UNKNOWN_XML_TAG)
				CASE(ERROR_TYPE_MISMATCH)
				CASE(ERROR_NOT_IMPLEMENTED)
				CASE(ERROR_INVALID_PLATFORM)
				CASE(ERROR_INVALID_INTERNAL_PTR)
				CASE(ERROR_INVALID_VERSION)
				CASE(ERROR_INVALID_CHECKSUM)
				CASE(ERROR_MEMORY_ALLOCATION_FAILURE)
				CASE(ERROR_INVALID_ENUM)
				CASE(ERROR_INVALID_VALUE)
				CASE(ERROR_OBJECT_CREATION_FAILED)
				CASE(ERROR_INVALID_PARAM_HANDLE)
#undef CASE
			default:
				errorString = "un-implemented error";
			}

			if (errorString != NULL && _mErrorCallback != NULL && filename != NULL)
			{
				_mErrorCallback->reportErrorPrintf("Serialization Error", "%s in %s", filename, errorString);
			}

			return true;
		}

		return false;
	}



	void ClothingAuthoring::setAuthoringState(AuthoringState::Enum authoringState, bool allowAdvance)
	{
		if (mState.authoringState > authoringState || allowAdvance)
		{
			mState.authoringState = authoringState;
		}
	}



	physx::AutoGeometry* ClothingAuthoring::createAutoGeometry()
	{
		Samples::TriangleMesh* inputMesh = ClothingAuthoring::getInputMesh();
		Samples::SkeletalAnim* skeleton = ClothingAuthoring::getSkeleton();

		if (inputMesh == NULL || skeleton == NULL)
			return NULL;

		const std::vector<Samples::SkeletalBone> &bones = skeleton->getBones();
		unsigned int boneCount = (unsigned int)bones.size();

		const std::vector<physx::PxVec3>& meshVertices = inputMesh->getVertices();
		const std::vector<unsigned short>& boneIndices = inputMesh->getBoneIndices();
		const std::vector<physx::PxVec4>& boneWeights2 = inputMesh->getBoneWeights();

		if ( !meshVertices.empty() && (meshVertices.size()*4) == boneIndices.size() && boneCount > 0 )
		{
			physx::AutoGeometry *autoGeometry = physx::createAutoGeometry();

			const std::vector<unsigned int> &meshIndices = inputMesh->getIndices();
			const std::vector<Samples::PaintedVertex>& paintChannel = inputMesh->getPaintChannel(Samples::PC_PHYSICAL);

			const size_t numSubmeshes = inputMesh->getNumSubmeshes();
			for (size_t i=0; i<numSubmeshes; i++)
			{
				const Samples::TriangleSubMesh &sub = *inputMesh->getSubMesh(i);

				if ( sub.usedForCollision )
				{
					const unsigned int tcount = sub.numIndices/3;
					for (unsigned int j=0; j<tcount; j++)
					{
						const unsigned int triangleIndices[3] = {
							meshIndices[ (j*3+0)+sub.firstIndex ],
							meshIndices[ (j*3+1)+sub.firstIndex ],
							meshIndices[ (j*3+2)+sub.firstIndex ],
						};

						const bool used1 = paintChannel[triangleIndices[0]].paintValue > 0.5f;
						const bool used2 = paintChannel[triangleIndices[1]].paintValue > 0.5f;
						const bool used3 = paintChannel[triangleIndices[2]].paintValue > 0.5f;
						if (!ClothingAuthoring::getCollisionVolumeUsePaintChannel() || (used1 && used2 && used3))
						{
							physx::SimpleSkinnedVertex vertices[3];
							for (unsigned int k = 0; k < 3; k++)
							{
								(physx::PxVec3&)vertices[k].mPos[0] = meshVertices[triangleIndices[k]];
								for (unsigned int l = 0; l < 4; l++)
								{
									const unsigned int boneIndex = triangleIndices[k] * 4 + l;
									vertices[k].mBone[l] = boneIndices[boneIndex];
									vertices[k].mWeight[l] = boneWeights2[triangleIndices[k]][l];
								}
							}

							autoGeometry->addSimpleSkinnedTriangle(vertices[0], vertices[1], vertices[2]);
						}
					}
				}
			}

			return autoGeometry;
		}

		return NULL;
	}



	void ClothingAuthoring::addCollisionVolumeInternal(physx::SimpleHull* hull, bool useCapsule)
	{
		CollisionVolume volume;
		volume.boneIndex = hull->mBoneIndex;
		volume.parentIndex = hull->mParentIndex;
		volume.boneName    = hull->mBoneName;
		volume.meshVolume  = hull->mMeshVolume;
		volume.transform.setColumnMajor44(hull->mTransform);

		for (unsigned int j=0; j<hull->mVertexCount; j++)
		{
			const float *p = &hull->mVertices[j*3];
			physx::PxVec3 vp;
			physx::PxVec3FromArray( vp, p );
			volume.vertices.push_back(vp);
		}

		for (unsigned int j=0; j<hull->mTriCount*3; j++)
		{
			volume.indices.push_back( hull->mIndices[j] );
		}


		if (useCapsule)
		{
			physx::PxQuat quat;
			computeBestFitCapsule(volume.vertices.size(), (float*)&volume.vertices[0], sizeof(physx::PxVec3),
				volume.capsuleRadius, volume.capsuleHeight, &volume.shapeOffset.t.x, &quat.x, true);
			volume.shapeOffset.M.fromQuat(quat);

			// apply scale
			volume.capsuleRadius *= 100.0f / mState.simulationValuesScale;
			volume.capsuleHeight *= 100.0f / mState.simulationValuesScale;
		}

		mMeshes.collisionVolumes.push_back(volume);
		mMeshes.skeleton->incShapeCount(volume.boneIndex);
	}



	class TriMeshBuilderDeprecated : public physx::apex::NxRenderMeshBuilder
	{
	public:
		TriMeshBuilderDeprecated(const Samples::TriangleMesh& mesh, physx::PxU32 maxBonesPerVertex, physx::PxF32 paintScalingMaxDistance, physx::PxF32 paintScalingCollisionFactor, physx::PxU32 useTexCoords) :
		  mMesh(mesh), mMaxBonesPerVertex(maxBonesPerVertex), mPaintScalingMaxDistance(paintScalingMaxDistance),
			  mPaintScalingCollisionFactor(paintScalingCollisionFactor), mUseTexCoords(useTexCoords)
		  {
		  }

		  unsigned int getNumIndices()
		  {
			  int numIndices = 0;
			  for (physx::PxU32 i = 0; i < mMesh.getNumSubmeshes(); i++)
			  {
				  const Samples::TriangleSubMesh* submesh = mMesh.getSubMesh(i);
				  if (submesh != NULL && submesh->selected)
				  {
					  numIndices += submesh->numIndices;
				  }
			  }
			  assert(numIndices % 3 == 0);
			  return numIndices;
		  }

		  virtual void createTriangles(physx::apex::NxExplicitRenderTriangle* triangleBuffer,
			  const physx::apex::NxApexCustomBufferIterator& customBufferData,
			  physx::PxU32 triangleCount, void* /*userData*/ )
		  {
			  PX_FORCE_PARAMETER_REFERENCE(triangleCount);
			  const physx::PxU32 maxDistanceAttribute = customBufferData.getAttributeIndex("MAX_DISTANCE");
			  const physx::PxU32 collisionSphereDistanceAttribute = customBufferData.getAttributeIndex("COLLISION_SPHERE_DISTANCE");
			  const physx::PxU32 collisionSphereRadiusAttribute = customBufferData.getAttributeIndex("COLLISION_SPHERE_RADIUS");
			  const physx::PxU32 usedForPhysicsAttribute = customBufferData.getAttributeIndex("USED_FOR_PHYSICS");

			  physx::PxU32 trianglesBuilt = 0;
			  int incSubMeshIndex = 0;
			  for (physx::PxU32 submeshNr = 0; submeshNr < mMesh.getNumSubmeshes(); submeshNr++)
			  {
				  const Samples::TriangleSubMesh* submesh = mMesh.getSubMesh(submeshNr);
				  if (submesh != NULL && submesh->selected)
				  {
					  const physx::PxU32 start = submesh->firstIndex;
					  const physx::PxU32 end = start + submesh->numIndices;
					  for (physx::PxU32 index = start; index < end; index += 3)
					  {
						  triangleBuffer[trianglesBuilt].submeshIndex = incSubMeshIndex;

						  const physx::PxU32 indices[3] =
						  {
							  mMesh.getIndices()[index+0],
							  mMesh.getIndices()[index+1],
							  mMesh.getIndices()[index+2]
						  };

						  for (physx::PxU32 v = 0; v < 3; v++)
						  {
							  physx::NxVertex& vertex = triangleBuffer[trianglesBuilt].vertices[v];

							  vertex.position = mMesh.getVertices()[indices[v]];
							  vertex.normal = mMesh.getNormals()[indices[v]];
							  if (!mMesh.getTangents().empty())
							  {
								  vertex.tangent = mMesh.getTangents()[indices[v]];
								  vertex.binormal = mMesh.getBitangents()[indices[v]];
							  }
							  if (mUseTexCoords > 0)
							  {
								  physx::PxU32 numWritten = 0;
								  for (int t = 0; t < Samples::TriangleMesh::NUM_TEXCOORDS; t++)
								  {
									  if (mMesh.getTexCoords(t).size() == mMesh.getVertices().size())
									  {
										  vertex.uv[numWritten++] = mMesh.getTexCoords(t)[indices[v]];
									  }
								  }
								  assert(numWritten == mUseTexCoords);
							  }

							  if (mMesh.getBoneIndices().size() == mMesh.getVertices().size() * mMaxBonesPerVertex)
							  {
								  assert(mMaxBonesPerVertex == 4);
								  for (physx::PxU32 k = 0; k < mMaxBonesPerVertex; k++)
								  {
									  vertex.boneIndices[k] = mMesh.getBoneIndices()[indices[v] * mMaxBonesPerVertex + k];
									  vertex.boneWeights[k] = mMesh.getBoneWeights()[indices[v]][k];
								  }
							  }

							  const char* dummyName = NULL;
							  if (mMesh.getPaintChannel(Samples::PC_MAX_DISTANCE).size() == mMesh.getNumVertices() && maxDistanceAttribute != -1)
							  {
								  physx::NxRenderDataFormat::Enum outFormat;
								  physx::PxF32* maxDistance = (physx::PxF32*)customBufferData.getVertexAttribute(trianglesBuilt, v, maxDistanceAttribute, outFormat, dummyName);
								  assert(outFormat == physx::NxRenderDataFormat::FLOAT1);
								  *maxDistance = physx::PxMax(0.0f, mMesh.getPaintChannel(Samples::PC_MAX_DISTANCE)[indices[v]].paintValue * mPaintScalingMaxDistance);
							  }
							  if (mMesh.getPaintChannel(Samples::PC_COLLISION_DISTANCE).size() == mMesh.getNumVertices() && collisionSphereDistanceAttribute != -1 && collisionSphereRadiusAttribute != -1)
							  {
								  physx::NxRenderDataFormat::Enum outFormat;
								  physx::PxF32* collisionSphereDistance = (physx::PxF32*)customBufferData.getVertexAttribute(trianglesBuilt, v, collisionSphereDistanceAttribute, outFormat, dummyName);
								  assert(outFormat == physx::NxRenderDataFormat::FLOAT1);
								  physx::PxF32* collisionSphereRadius = (physx::PxF32*)customBufferData.getVertexAttribute(trianglesBuilt, v, collisionSphereRadiusAttribute, outFormat, dummyName);
								  assert(outFormat == physx::NxRenderDataFormat::FLOAT1);

								  const physx::PxF32 factor = mMesh.getPaintChannel(Samples::PC_COLLISION_DISTANCE)[indices[v]].paintValue;
								  const physx::PxF32 maxDistance = mMesh.getPaintChannel(Samples::PC_MAX_DISTANCE)[indices[v]].paintValue;
								  if (physx::PxAbs(factor) > 1.0f || maxDistance <= 0)
								  {
									  *collisionSphereRadius = 0.0f;
									  *collisionSphereDistance = 0.0f;
								  }
								  else
								  {
									  *collisionSphereRadius = 10.0f * maxDistance * mPaintScalingMaxDistance;
									  *collisionSphereDistance = factor * mPaintScalingCollisionFactor;
								  }
							  }
							  if (mMesh.getPaintChannel(Samples::PC_PHYSICAL).size() == mMesh.getNumVertices() && usedForPhysicsAttribute != -1)
							  {
								  physx::NxRenderDataFormat::Enum outFormat;
								  physx::PxU8* collisionPhysical = (physx::PxU8*)customBufferData.getVertexAttribute(trianglesBuilt, v, usedForPhysicsAttribute, outFormat, dummyName);
								  assert(outFormat == physx::NxRenderDataFormat::UBYTE1);
								  *collisionPhysical = mMesh.getPaintChannel(Samples::PC_PHYSICAL)[indices[v]].paintValue > 0.5f ? 1u : 0u;
							  }
						  }

						  trianglesBuilt++;
					  }
					  incSubMeshIndex++;
				  }
			  }
			  assert(trianglesBuilt == triangleCount);
		  }

	private:
		const Samples::TriangleMesh& mMesh;
		physx::PxU32 mMaxBonesPerVertex;
		physx::PxF32 mPaintScalingMaxDistance;
		physx::PxF32 mPaintScalingCollisionFactor;
		physx::PxU32 mUseTexCoords;
	};



	void ClothingAuthoring::createRenderMeshAssets()
	{
		if (mState.renderMeshAssetLocked || mMeshes.inputMesh == NULL)
			return;

		if (mState.authoringState >= AuthoringState::RenderMeshAssetCreated && mAuthoringObjects.renderMeshAssets.size() == mConfig.cloth.numGraphicalLods)
			return;

		for (size_t i = 0; i < mAuthoringObjects.renderMeshAssets.size(); i++)
		{
			mAuthoringObjects.renderMeshAssets[i]->release();
		}
		mAuthoringObjects.renderMeshAssets.clear();

		// find maximum #bones per vertex (only the vertices that are submitted!)
		unsigned int maxBonesPerVertex = 0;
		if (mMeshes.inputMesh->getBoneWeights().size() == mMeshes.inputMesh->getNumVertices())
		{
			const unsigned int* indices = &mMeshes.inputMesh->getIndices()[0];
			const physx::PxVec4* boneWeights2 = &mMeshes.inputMesh->getBoneWeights()[0];
			for (size_t i = 0; i < mMeshes.inputMesh->getNumSubmeshes(); i++)
			{
				const Samples::TriangleSubMesh* submesh = mMeshes.inputMesh->getSubMesh(i);
				if (submesh != NULL && submesh->selected)
				{
					const unsigned int start = submesh->firstIndex;
					const unsigned int end = start + submesh->numIndices;

					for (unsigned int i = start; i < end; i++)
					{
						const unsigned int index = indices[i];
						for (unsigned int j = 0; j < 4; j++)
						{
							if (boneWeights2[index][j] != 0.0f)
							{
								maxBonesPerVertex = physx::PxMax(maxBonesPerVertex, j+1);
							}
						}
					}
				}
			}
		}


		std::vector<float> distances2;
		if (mConfig.cloth.numGraphicalLods > 1)
		{
			// sort the edge distances for later subdivision
			const size_t numIndices = mMeshes.inputMesh->getNumIndices();
			const unsigned int* indices = &mMeshes.inputMesh->getIndices()[0];
			const physx::PxVec3* positions = &mMeshes.inputMesh->getVertices()[0];
			distances2.reserve(numIndices);

			for (size_t i = 0; i < numIndices; i += 3)
			{
				distances2.push_back(positions[indices[i+0]].distanceSquared(positions[indices[i+1]]));
				distances2.push_back(positions[indices[i+1]].distanceSquared(positions[indices[i+2]]));
				distances2.push_back(positions[indices[i+2]].distanceSquared(positions[indices[i+0]]));
			}

			class Compare
			{
			public:
				bool operator()( const float a, const float b ) const
				{
					return a > b;
				}

			};
			std::sort(distances2.begin(), distances2.end(), Compare());
		}


		Samples::TriangleMesh subdividedMesh(0);

		for (int graphicalLod = 0; graphicalLod < mConfig.cloth.numGraphicalLods; graphicalLod++)
		{
			const Samples::TriangleMesh* useThisMesh = mMeshes.inputMesh;

			if (graphicalLod > 0)
			{
				subdividedMesh.clear(NULL, NULL);
				subdividedMesh.copyFrom(*mMeshes.inputMesh);
				physx::PxBounds3 bounds;
				subdividedMesh.getBounds(bounds);
				const float dist2 = distances2[graphicalLod * distances2.size() / mConfig.cloth.numGraphicalLods];
				int subdivision = (int)(bounds.minimum.distance(bounds.maximum) / physx::PxSqrt(dist2));
				for (int j = 0; j < (int)subdividedMesh.getNumSubmeshes(); j++)
				{
					const Samples::TriangleSubMesh* submesh = subdividedMesh.getSubMesh(j);
					if (submesh != NULL && submesh->selected)
					{
						subdividedMesh.subdivideSubMesh(j, subdivision, mConfig.mesh.evenOutVertexDegrees);
					}
				}
				useThisMesh = &subdividedMesh;
			}

			physx::apex::NxRenderMeshAssetAuthoring* renderMeshAsset = NULL;

			if (mConfig.mesh.useNewApi)
			{

				physx::shdfnd2::Array<physx::apex::NxRenderMeshAssetAuthoring::SubmeshDesc> submeshDescs;
				physx::shdfnd2::Array<physx::apex::NxRenderMeshAssetAuthoring::VertexBuffer> vertexBufferDescs;

				struct PaintingValues
				{
					float maxDistance;
					float collisionSphereDistance;
					float collisionSphereRadius;
					unsigned char latchToNearest;
				};

				physx::shdfnd2::Array<PaintingValues> paintingValues((unsigned int)useThisMesh->getNumVertices());
				{
					const std::vector<Samples::PaintedVertex> maxDistances = useThisMesh->getPaintChannel(Samples::PC_MAX_DISTANCE);
					const std::vector<Samples::PaintedVertex> collisionDistances = useThisMesh->getPaintChannel(Samples::PC_COLLISION_DISTANCE);
					const std::vector<Samples::PaintedVertex> latchToNearest = useThisMesh->getPaintChannel(Samples::PC_PHYSICAL);

					const float paintScalingMaxDistance = getAbsolutePaintingScalingMaxDistance();
					const float paintScalingCollisionFactor = getAbsolutePaintingScalingCollisionFactor();

					for (unsigned int vertex = 0, numVertices = (unsigned int)useThisMesh->getNumVertices(); vertex < numVertices; ++vertex)
					{
						paintingValues[vertex].maxDistance = physx::PxMax(0.0f, maxDistances[vertex].paintValue * paintScalingMaxDistance);

						const float factor = collisionDistances[vertex].paintValue;
						const float maxDistance = maxDistances[vertex].paintValue;

						if (physx::PxAbs(factor) > 1.0f || maxDistance <= 0)
						{
							paintingValues[vertex].collisionSphereDistance = 0.0f;
							paintingValues[vertex].collisionSphereRadius = 0.0f;
						}
						else
						{
							paintingValues[vertex].collisionSphereDistance = factor * paintScalingCollisionFactor;
							paintingValues[vertex].collisionSphereRadius = 10.0f * maxDistance * paintScalingMaxDistance;
						}

						paintingValues[vertex].latchToNearest = latchToNearest[vertex].paintValue > 0.5f ? 1u : 0u;
					}
				}

				unsigned int numSelected = 0;
				for (size_t submeshIndex = 0, numSubmeshes = useThisMesh->getNumSubmeshes(); submeshIndex < numSubmeshes; submeshIndex++)
				{
					const Samples::TriangleSubMesh* submesh = useThisMesh->getSubMesh(submeshIndex);
					if (submesh != NULL && submesh->selected)
					{
						numSelected++;
					}
				}
				// PH: This array must be big enough from the start!
				vertexBufferDescs.reserve(numSelected);
				physx::shdfnd2::Array<physx::apex::NxApexRenderSemanticData> customSemanticData;
				customSemanticData.reserve(numSelected * 4);

				for (size_t submeshIndex = 0, numSubmeshes = useThisMesh->getNumSubmeshes(); submeshIndex < numSubmeshes; submeshIndex++)
				{
					const Samples::TriangleSubMesh* submesh = useThisMesh->getSubMesh(submeshIndex);
					if (submesh != NULL && submesh->selected)
					{
						physx::apex::NxRenderMeshAssetAuthoring::SubmeshDesc submeshDesc;

						submeshDesc.m_numVertices = (physx::PxU32)useThisMesh->getVertices().size();

						physx::apex::NxRenderMeshAssetAuthoring::VertexBuffer vertexBufferDesc;

						vertexBufferDesc.setSemanticData(physx::apex::NxRenderVertexSemantic::POSITION,
							&useThisMesh->getVertices()[0],
							sizeof(physx::PxVec3),
							physx::apex::NxRenderDataFormat::FLOAT3);

						if (useThisMesh->getNormals().size() == submeshDesc.m_numVertices)
						{
							vertexBufferDesc.setSemanticData(physx::apex::NxRenderVertexSemantic::NORMAL,
								&useThisMesh->getNormals()[0],
								sizeof(physx::PxVec3),
								physx::apex::NxRenderDataFormat::FLOAT3);
						}

						if (useThisMesh->getTangents().size() == submeshDesc.m_numVertices && useThisMesh->getBitangents().size() == submeshDesc.m_numVertices)
						{
							vertexBufferDesc.setSemanticData(physx::apex::NxRenderVertexSemantic::TANGENT,
								&useThisMesh->getTangents()[0],
								sizeof(physx::PxVec3),
								physx::apex::NxRenderDataFormat::FLOAT3);

							vertexBufferDesc.setSemanticData(physx::apex::NxRenderVertexSemantic::BINORMAL,
								&useThisMesh->getBitangents()[0],
								sizeof(physx::PxVec3),
								physx::apex::NxRenderDataFormat::FLOAT3);
						}

						for (physx::PxU32 texCoord = 0; texCoord < Samples::TriangleMesh::NUM_TEXCOORDS ; texCoord++)
						{
							if (texCoord < 4 && useThisMesh->getTexCoords(texCoord).size() == submeshDesc.m_numVertices)
							{
								physx::apex::NxRenderVertexSemantic::Enum semantic = 
									(physx::apex::NxRenderVertexSemantic::Enum)(physx::apex::NxRenderVertexSemantic::TEXCOORD0 + texCoord);

								vertexBufferDesc.setSemanticData(semantic,
									&useThisMesh->getTexCoords(texCoord)[0],
									sizeof(physx::apex::NxVertexUV),
									physx::apex::NxRenderDataFormat::FLOAT2);
							}
						}

						if (useThisMesh->getBoneWeights().size() == submeshDesc.m_numVertices && useThisMesh->getBoneIndices().size() == submeshDesc.m_numVertices * 4)
						{
							// check how many are actually used
							physx::PxU32 maxBonesPerSubmeshVertex = 0;
							for (physx::PxU32 index = submesh->firstIndex, end = submesh->firstIndex + submesh->numIndices; index < end; index++)
							{
								const physx::PxU32 vertexIndex = useThisMesh->getIndices()[index];
								for (physx::PxU32 j = 0; j < 4; j++)
								{
									if (useThisMesh->getBoneWeights()[vertexIndex][j] != 0.0f)
									{
										maxBonesPerSubmeshVertex = physx::PxMax(maxBonesPerSubmeshVertex, j+1);
									}
								}
							}

							physx::apex::NxRenderDataFormat::Enum format = 
								(physx::apex::NxRenderDataFormat::Enum)(physx::apex::NxRenderDataFormat::USHORT1 + maxBonesPerSubmeshVertex - 1);

							vertexBufferDesc.setSemanticData(physx::apex::NxRenderVertexSemantic::BONE_INDEX,
								&useThisMesh->getBoneIndices()[0],
								sizeof(physx::PxU16) * 4,
								format);

							format = (physx::apex::NxRenderDataFormat::Enum)(physx::apex::NxRenderDataFormat::FLOAT1 + maxBonesPerSubmeshVertex - 1);

							vertexBufferDesc.setSemanticData(physx::apex::NxRenderVertexSemantic::BONE_WEIGHT,
								&useThisMesh->getBoneWeights()[0],
								sizeof(physx::PxVec4),
								format);
						}

						const physx::PxU32 startCustom = customSemanticData.size();

						if (useThisMesh->getPaintChannel(Samples::PC_MAX_DISTANCE).size() == useThisMesh->getNumVertices())
						{
							physx::apex::NxApexRenderSemanticData customData;
							customData.data = &paintingValues[0].maxDistance;
							customData.stride = sizeof(PaintingValues);
							customData.format = physx::apex::NxRenderDataFormat::FLOAT1;
							customData.ident = "MAX_DISTANCE";
							customSemanticData.pushBack(customData);
						}

						if (useThisMesh->getPaintChannel(Samples::PC_COLLISION_DISTANCE).size() == useThisMesh->getNumVertices())
						{
							physx::apex::NxApexRenderSemanticData customData;
							customData.data = &paintingValues[0].collisionSphereDistance;
							customData.stride = sizeof(PaintingValues);
							customData.format = physx::apex::NxRenderDataFormat::FLOAT1;
							customData.ident = "COLLISION_SPHERE_DISTANCE";
							customSemanticData.pushBack(customData);

							customData.data = &paintingValues[0].collisionSphereRadius;
							customData.stride = sizeof(PaintingValues);
							customData.format = physx::apex::NxRenderDataFormat::FLOAT1;
							customData.ident = "COLLISION_SPHERE_RADIUS";
							customSemanticData.pushBack(customData);
						}
						if (useThisMesh->getPaintChannel(Samples::PC_PHYSICAL).size() == useThisMesh->getNumVertices())
						{
							physx::apex::NxApexRenderSemanticData customData;
							customData.data = &paintingValues[0].latchToNearest;
							customData.stride = sizeof(PaintingValues);
							customData.format = physx::apex::NxRenderDataFormat::UBYTE1;
							customData.ident = "USED_FOR_PHYSICS";
							customSemanticData.pushBack(customData);
						}

						if (startCustom < customSemanticData.size())
						{
							vertexBufferDesc.setCustomSemanticData(&customSemanticData[startCustom], customSemanticData.size() - startCustom);
						}

						vertexBufferDescs.pushBack(vertexBufferDesc);

						submeshDesc.m_materialName = submesh->materialName.c_str();
						submeshDesc.m_vertexBuffers = &vertexBufferDescs.back();
						submeshDesc.m_numVertexBuffers = 1;

						submeshDesc.m_primitive = physx::apex::NxRenderMeshAssetAuthoring::Primitive::TRIANGLE_LIST;
						submeshDesc.m_indexType = physx::apex::NxRenderMeshAssetAuthoring::IndexType::UINT;

						submeshDesc.m_vertexIndices = &useThisMesh->getIndices()[0] + submesh->firstIndex;
						submeshDesc.m_numIndices = submesh->numIndices;
						submeshDesc.m_firstVertex = 0;
						submeshDesc.m_partIndices = NULL;
						submeshDesc.m_numParts = 0;

						submeshDesc.m_cullMode = ClothingAuthoring::getCullMode();

						submeshDescs.pushBack(submeshDesc);
					}
				}

				PX_ASSERT(customSemanticData.capacity() == numSelected * 4);

				physx::apex::NxRenderMeshAssetAuthoring::MeshDesc meshDesc;
				meshDesc.m_numSubmeshes = submeshDescs.size();
				meshDesc.m_submeshes = submeshDescs.begin();

				renderMeshAsset = static_cast<physx::apex::NxRenderMeshAssetAuthoring*>(_mApexSDK->createAssetAuthoring(NX_RENDER_MESH_AUTHORING_TYPE_NAME, "rmAuthor"));

				renderMeshAsset->createRenderMesh(meshDesc, true);
			}
			else
			{
				renderMeshAsset = static_cast<physx::apex::NxRenderMeshAssetAuthoring*>(_mApexSDK->createAssetAuthoring(NX_RENDER_MESH_AUTHORING_TYPE_NAME, "rmAuthor"));

				physx::PxU32 numTexCoords = 0;
				for (int j = 0; j < Samples::TriangleMesh::NUM_TEXCOORDS; j++)
					numTexCoords += useThisMesh->getTexCoords(j).empty() ? 0 : 1;


				const bool serializeThem = false;
				for (physx::PxU32 j = 0; j < useThisMesh->getNumSubmeshes(); j++)
				{
					const Samples::TriangleSubMesh* submesh = useThisMesh->getSubMesh(j);
					if (submesh != NULL && submesh->selected)
					{
						physx::apex::NxVertexFormat* vertexFormat = renderMeshAsset->addSubmesh( submesh->materialName.c_str() );

						const physx::PxI32 positionIndex = vertexFormat->addBuffer(vertexFormat->getSemanticName(physx::apex::NxRenderVertexSemantic::POSITION));
						vertexFormat->setBufferFormat(positionIndex, physx::apex::NxRenderDataFormat::FLOAT3);

						const physx::PxI32 normalIndex = vertexFormat->addBuffer(vertexFormat->getSemanticName(physx::apex::NxRenderVertexSemantic::NORMAL));
						vertexFormat->setBufferFormat(normalIndex, physx::apex::NxRenderDataFormat::FLOAT3);

						if (!useThisMesh->getTangents().empty())
						{
							const physx::PxI32 tangentIndex = vertexFormat->addBuffer(vertexFormat->getSemanticName(physx::apex::NxRenderVertexSemantic::TANGENT));
							vertexFormat->setBufferFormat(tangentIndex, physx::apex::NxRenderDataFormat::FLOAT3);

							const physx::PxI32 bitangentIndex = vertexFormat->addBuffer(vertexFormat->getSemanticName(physx::apex::NxRenderVertexSemantic::BINORMAL));
							vertexFormat->setBufferFormat(bitangentIndex, physx::apex::NxRenderDataFormat::FLOAT3);
						}

						if (numTexCoords > 0)
						{
							const physx::PxI32 texCoordIndex = vertexFormat->addBuffer(vertexFormat->getSemanticName(physx::apex::NxRenderVertexSemantic::TEXCOORD0));
							vertexFormat->setBufferFormat(texCoordIndex, physx::apex::NxRenderDataFormat::FLOAT2);
						}
						if (numTexCoords > 1)
						{
							const physx::PxI32 texCoordIndex = vertexFormat->addBuffer(vertexFormat->getSemanticName(physx::apex::NxRenderVertexSemantic::TEXCOORD1));
							vertexFormat->setBufferFormat(texCoordIndex, physx::apex::NxRenderDataFormat::FLOAT2);
						}
						if (numTexCoords > 2)
						{
							const physx::PxI32 texCoordIndex = vertexFormat->addBuffer(vertexFormat->getSemanticName(physx::apex::NxRenderVertexSemantic::TEXCOORD2));
							vertexFormat->setBufferFormat(texCoordIndex, physx::apex::NxRenderDataFormat::FLOAT2);
						}
						if (numTexCoords > 3)
						{
							const physx::PxI32 texCoordIndex = vertexFormat->addBuffer(vertexFormat->getSemanticName(physx::apex::NxRenderVertexSemantic::TEXCOORD3));
							vertexFormat->setBufferFormat(texCoordIndex, physx::apex::NxRenderDataFormat::FLOAT2);
						}

						switch(maxBonesPerVertex)
						{
						case 0:
							break;
						case 1:
							{
								const physx::PxI32 boneIndex = vertexFormat->addBuffer(vertexFormat->getSemanticName(physx::apex::NxRenderVertexSemantic::BONE_INDEX));
								vertexFormat->setBufferFormat(boneIndex, physx::apex::NxRenderDataFormat::USHORT1);
							}
							break;
						case 2:
							{
								const physx::PxI32 boneIndex = vertexFormat->addBuffer(vertexFormat->getSemanticName(physx::apex::NxRenderVertexSemantic::BONE_INDEX));
								vertexFormat->setBufferFormat(boneIndex, physx::apex::NxRenderDataFormat::USHORT2);

								const physx::PxI32 boneWeight = vertexFormat->addBuffer(vertexFormat->getSemanticName(physx::apex::NxRenderVertexSemantic::BONE_WEIGHT));
								vertexFormat->setBufferFormat(boneWeight, physx::apex::NxRenderDataFormat::FLOAT2);
							}
							break;
						case 3:
							{
								const physx::PxI32 boneIndex = vertexFormat->addBuffer(vertexFormat->getSemanticName(physx::apex::NxRenderVertexSemantic::BONE_INDEX));
								vertexFormat->setBufferFormat(boneIndex, physx::apex::NxRenderDataFormat::USHORT3);

								const physx::PxI32 boneWeight = vertexFormat->addBuffer(vertexFormat->getSemanticName(physx::apex::NxRenderVertexSemantic::BONE_WEIGHT));
								vertexFormat->setBufferFormat(boneWeight, physx::apex::NxRenderDataFormat::FLOAT3);
							}
							break;
						case 4:
							{
								const physx::PxI32 boneIndex = vertexFormat->addBuffer(vertexFormat->getSemanticName(physx::apex::NxRenderVertexSemantic::BONE_INDEX));
								vertexFormat->setBufferFormat(boneIndex, physx::apex::NxRenderDataFormat::USHORT4);

								const physx::PxI32 boneWeight = vertexFormat->addBuffer(vertexFormat->getSemanticName(physx::apex::NxRenderVertexSemantic::BONE_WEIGHT));
								vertexFormat->setBufferFormat(boneWeight, physx::apex::NxRenderDataFormat::FLOAT4);
							}
							break;
						default:
							if (_mErrorCallback != NULL)
							{
								_mErrorCallback->reportErrorPrintf("NxVertexFormat setup", "MaxBonesPerVertex has invalid size: %d", maxBonesPerVertex);
							}
							break;
						}

						vertexFormat->setWinding(ClothingAuthoring::getCullMode());

						if (useThisMesh->getPaintChannel(Samples::PC_MAX_DISTANCE).size() == useThisMesh->getNumVertices())
						{
							const physx::PxI32 maxDistanceIndex = vertexFormat->addBuffer("MAX_DISTANCE");
							vertexFormat->setBufferFormat(maxDistanceIndex, physx::apex::NxRenderDataFormat::FLOAT1);
							vertexFormat->setBufferSerialize(maxDistanceIndex, serializeThem);
						}

						if (useThisMesh->getPaintChannel(Samples::PC_COLLISION_DISTANCE).size() == useThisMesh->getNumVertices())
						{
							const physx::PxI32 collisionSphereDistanceIndex = vertexFormat->addBuffer("COLLISION_SPHERE_DISTANCE");
							vertexFormat->setBufferFormat(collisionSphereDistanceIndex, physx::NxRenderDataFormat::FLOAT1);
							vertexFormat->setBufferSerialize(collisionSphereDistanceIndex, serializeThem);

							const physx::PxI32 collisionSphereRadiusIndex = vertexFormat->addBuffer("COLLISION_SPHERE_RADIUS");
							vertexFormat->setBufferFormat(collisionSphereRadiusIndex, physx::NxRenderDataFormat::FLOAT1);
							vertexFormat->setBufferSerialize(collisionSphereRadiusIndex, serializeThem);
						}
						if (useThisMesh->getPaintChannel(Samples::PC_PHYSICAL).size() == useThisMesh->getNumVertices())
						{
							const physx::PxI32 usedForPhysicsIndex = vertexFormat->addBuffer("USED_FOR_PHYSICS");
							vertexFormat->setBufferFormat(usedForPhysicsIndex, physx::NxRenderDataFormat::UBYTE1);
							vertexFormat->setBufferSerialize(usedForPhysicsIndex, serializeThem);
						}
					}
				}

				maxBonesPerVertex = 4; // TriangleMesh always uses 4...
				TriMeshBuilderDeprecated triMeshBuilder(*useThisMesh, maxBonesPerVertex, getAbsolutePaintingScalingMaxDistance(),
					getAbsolutePaintingScalingCollisionFactor(), numTexCoords);

				assert(triMeshBuilder.getNumIndices() % 3 == 0);
				renderMeshAsset->addPart(physx::apex::NxRenderMeshPartData(triMeshBuilder.getNumIndices() / 3, NULL) );
				renderMeshAsset->setMeshBuilder(&triMeshBuilder);

				renderMeshAsset->buildRenderMesh(false);
			}


			renderMeshAsset->setTextureUVOrigin(mMeshes.inputMesh->getTextureUVOrigin());

			if (renderMeshAsset != NULL)
				mAuthoringObjects.renderMeshAssets.push_back(renderMeshAsset);
		}

		setAuthoringState(AuthoringState::RenderMeshAssetCreated, true);
	}



	void ClothingAuthoring::createCustomMesh()
	{
		createRenderMeshAssets();

		if (mMeshes.inputMesh != NULL && mMeshes.customPhysicsMesh != NULL)
		{
			if (mAuthoringObjects.physicalMeshes.size() == 0)
			{
				physx::apex::NxClothingPhysicalMesh* physicalMesh = _mModuleClothing->createEmptyPhysicalMesh();
				physicalMesh->setGeometry(false,
					(physx::PxU32)mMeshes.customPhysicsMesh->getVertices().size(),
					sizeof(physx::PxVec3),
					&mMeshes.customPhysicsMesh->getVertices()[0],
					(physx::PxU32)mMeshes.customPhysicsMesh->getIndices().size(),
					sizeof(physx::PxU32),
					&mMeshes.customPhysicsMesh->getIndices()[0]);

				mAuthoringObjects.physicalMeshes.push_back(physicalMesh);

				setAuthoringState(AuthoringState::PhysicalCustomMeshCreated, true);
			}
			assert(mAuthoringObjects.physicalMeshes.size() == 1);
		}
	}



	void ClothingAuthoring::createClothMeshes(physx::apex::IProgressListener* progressParent)
	{
		if (mState.authoringState >= AuthoringState::PhysicalClothMeshCreated)
			return;

		createRenderMeshAssets();

		for (size_t i = 0; i < mAuthoringObjects.physicalMeshes.size(); i++)
		{
			if (mAuthoringObjects.physicalMeshes[i] != NULL)
				mAuthoringObjects.physicalMeshes[i]->release();

			mAuthoringObjects.physicalMeshes[i] = NULL;
		}
		mAuthoringObjects.physicalMeshes.clear();

		ProgressCallback progress(mConfig.cloth.numGraphicalLods, progressParent);

		//mApexPhysicalMeshes.resize(ClothingAuthoring::getNumRenderMeshAssets());
		for (int i = 0; i < mConfig.cloth.numGraphicalLods; i++)
		{
			progress.setSubtaskWork(1, "Create Physical Mesh");
			physx::apex::NxClothingPhysicalMesh* physicalMesh = _mModuleClothing->createSingleLayeredMesh(
				mAuthoringObjects.renderMeshAssets[i],
				mConfig.cloth.subdivide ? mConfig.cloth.subdivision : 0,
				true, // mergeVertices
				mConfig.cloth.close,
				&progress);

			mAuthoringObjects.physicalMeshes.push_back(physicalMesh);

			progress.completeSubtask();
		}

		setAuthoringState(AuthoringState::PhysicalClothMeshCreated, true);
		mConfig.mesh.physicalMeshType = 0;
	}



	void ClothingAuthoring::createIsoMesh(physx::apex::IProgressListener* progress)
	{
		if (mState.authoringState >= AuthoringState::PhysicalIsoMeshCreated)
			return;

		physx::apex::NxRenderMeshAssetAuthoring* renderMeshAsset = getRenderMeshAsset(0);

		if (mAuthoringObjects.isoMesh != NULL)
		{
			mAuthoringObjects.isoMesh->release();
			mAuthoringObjects.isoMesh = NULL;
		}

		mAuthoringObjects.isoMesh = _mModuleClothing->createMultiLayeredMesh(renderMeshAsset, mConfig.softbody.isoSurfaceSubdivision,
			mConfig.softbody.keepBiggestMeshes, progress);

		setAuthoringState(AuthoringState::PhysicalIsoMeshCreated, true);
		mConfig.mesh.physicalMeshType = 1;
	}



	void ClothingAuthoring::createTetraMesh(physx::apex::IProgressListener* progressParent)
	{
		if (mState.authoringState >= AuthoringState::PhysicalTetraMeshcreated)
			return;

		ProgressCallback progress(100, progressParent);

		if (mState.authoringState < AuthoringState::PhysicalIsoMeshCreated)
		{
			progress.setSubtaskWork(50, "Create Iso Mesh");
			createIsoMesh(&progress);
			progress.completeSubtask();
		}

		progress.setSubtaskWork(-1, "Create Tetra Mesh");

		for (size_t i = 0; i < mAuthoringObjects.physicalMeshes.size(); i++)
		{
			if (mAuthoringObjects.physicalMeshes[i] != NULL)
				mAuthoringObjects.physicalMeshes[i]->release();

			mAuthoringObjects.physicalMeshes[i] = NULL;
		}
		mAuthoringObjects.physicalMeshes.clear();

		mAuthoringObjects.physicalMeshes.push_back(mAuthoringObjects.isoMesh->generateSoftbodyMesh(0, &progress));

		progress.completeSubtask();

		setAuthoringState(AuthoringState::PhysicalTetraMeshcreated, true);
		mConfig.mesh.physicalMeshType = 1;
	}



	void ClothingAuthoring::createClothingAsset(physx::apex::IProgressListener* progressParent)
	{
		if (mState.authoringState >= AuthoringState::ClothingAssetCreated)
			return;

		if (mAuthoringObjects.clothingAssetAuthoring != NULL)
		{
			mAuthoringObjects.clothingAssetAuthoring->release();
			mAuthoringObjects.clothingAssetAuthoring = NULL;
		}

		unsigned int totalWork = mConfig.cloth.numGraphicalLods*2;
		ProgressCallback progress(totalWork, progressParent);

		if (mState.authoringState <= AuthoringState::RenderMeshAssetCreated)
		{
			createRenderMeshAssets();

			progress.setSubtaskWork(mConfig.cloth.numGraphicalLods, "Create Physics Mesh");

			if (mMeshes.customPhysicsMesh != NULL)
			{
				// only 1 lod
				createCustomMesh();
			}
			else if (mConfig.mesh.physicalMeshType == 0) //cloth
			{
				// all lods
				createClothMeshes(&progress);
			}
			else
			{
				// only 1 lod
				createTetraMesh(&progress);
			}

			progress.completeSubtask();
		}

		{
			physx::apex::NxApexAssetAuthoring* assetAuthoring = _mApexSDK->createAssetAuthoring(NX_CLOTHING_AUTHORING_TYPE_NAME, "The Authoring Asset");
			mAuthoringObjects.clothingAssetAuthoring = static_cast<physx::apex::NxClothingAssetAuthoring*>(assetAuthoring);

			updateDeformableParameters();

			mAuthoringObjects.clothingAssetAuthoring->setDeriveNormalsFromBones(ClothingAuthoring::getDeriveNormalsFromBones());

			if (mMeshes.skeleton != NULL)
			{
				const std::vector<Samples::SkeletalBone>& bones = mMeshes.skeleton->getBones();
				for (unsigned int i = 0; i < (unsigned int)bones.size(); i++)
				{
					mAuthoringObjects.clothingAssetAuthoring->setBoneInfo(i, bones[i].name.c_str(), bones[i].bindWorldPose, bones[i].parent);
				}
			}
		}

		assert(!mAuthoringObjects.physicalMeshes.empty());

		for (int i = 0; i < (int)mAuthoringObjects.physicalMeshes.size(); i++)
		{
			progress.setSubtaskWork(1, "combine graphical and physical mesh");

			const int lod = mState.authoringState == AuthoringState::PhysicalClothMeshCreated ? mConfig.cloth.numGraphicalLods-1 - i : 0;

			int numPhysicalLods = (int)mMeshes.physicalLodValues.size();
			float* physicalLodValues = (mMeshes.physicalLodValues.size() > 0) ? &mMeshes.physicalLodValues[0] : NULL;

			mAuthoringObjects.clothingAssetAuthoring->setMeshes(lod,
				mAuthoringObjects.renderMeshAssets[i],
				mAuthoringObjects.physicalMeshes[i],
				numPhysicalLods, physicalLodValues,
				25.0f, mConfig.painting.ignoreUnusedVertices,
				mConfig.setMeshes.fasterMeshMeshSkinning, &progress);

			progress.completeSubtask();
		}

		setAuthoringState(AuthoringState::ClothingAssetCreated, true);
	}



	void ClothingAuthoring::updateDeformableParameters()
	{
		float thicknessScaling = 1.0f; 
		if (mMeshes.inputMesh != NULL)
		{
			physx::PxBounds3 bounds;
			mMeshes.inputMesh->getBounds(bounds);
			thicknessScaling = 0.1f * bounds.minimum.distance(bounds.maximum);
		}

		if (mAuthoringObjects.clothingAssetAuthoring != NULL)
		{
			mAuthoringObjects.clothingAssetAuthoring->setSimulationThickness(mConfig.deformable.thickness * thicknessScaling);
			mAuthoringObjects.clothingAssetAuthoring->setSimulationSelfcollisionThickness(mConfig.deformable.selfcollisionThickness * thicknessScaling);
			mAuthoringObjects.clothingAssetAuthoring->setSimulationHierarchicalLevels(mConfig.deformable.hierarchicalLevels);

			mAuthoringObjects.clothingAssetAuthoring->setSimulationDisableCCD(mConfig.deformable.disableCCD);
			mAuthoringObjects.clothingAssetAuthoring->setSimulationSelfcollision(mConfig.deformable.selfcollision);
			mAuthoringObjects.clothingAssetAuthoring->setSimulationTwowayInteraction(mConfig.deformable.twowayInteraction);
			mAuthoringObjects.clothingAssetAuthoring->setSimulationUntangling(mConfig.deformable.untangling);

			mAuthoringObjects.clothingAssetAuthoring->setSimulationTargetFrequency(mConfig.simulation.frequency);
		}
	}



	void ClothingAuthoring::Simulation::clear()
	{
		for (size_t i = 0; i < actors.size(); i++)
		{
			if (actors[i].actor != NULL)
			{
				actors[i].actor->release();
				actors[i].actor = NULL;
			}
			if (actors[i].preview != NULL)
			{
				actors[i].preview->release();
				actors[i].preview = NULL;
			}
		}
		actors.clear();

		if (clearAssets)
		{
			for (size_t i = 0; i < assets.size(); i++)
			{
				assets[i]->release();
				assets[i] = NULL;
			}
		}
		assets.clear();
		clearAssets = true;

		actorCount = 1;
		stepsUntilPause = 0;
	}



	void ClothingAuthoring::Meshes::clear(physx::apex::NxUserRenderResourceManager* rrm, physx::apex::NxResourceCallback* rcb, bool groundAsWell)
	{
		if (inputMesh != NULL)
		{
			inputMesh->clear(rrm, rcb);
			delete inputMesh;
			inputMesh = NULL;
		}
		//inputMeshFilename.clear(); // PH: MUST not be reset

		if (painter != NULL)
		{
			delete painter;
			painter = NULL;
		}
		physicalLodValues.clear();

		if (customPhysicsMesh != NULL)
		{
			customPhysicsMesh->clear(rrm, rcb);
			delete customPhysicsMesh;
			customPhysicsMesh = NULL;
		}
		customPhysicsMeshFilename.clear();

		if (groundMesh != NULL && groundAsWell)
		{
			groundMesh->clear(rrm, rcb);
			delete groundMesh;
			groundMesh = NULL;
		}

		if (skeleton != NULL)
		{
			delete skeleton;
			skeleton = NULL;
		}

		if (skeletonBehind != NULL)
		{
			delete skeletonBehind;
			skeletonBehind = NULL;
		}

		collisionVolumes.clear();
		subdivideHistory.clear();
		renameHistory.clear();
	}



	void ClothingAuthoring::AuthoringObjects::clear()
	{
		for (size_t i = 0; i < renderMeshAssets.size(); i++)
		{
			if (renderMeshAssets[i] != NULL)
				renderMeshAssets[i]->release();

			renderMeshAssets[i] = NULL;
		}
		renderMeshAssets.clear();

		for (size_t i = 0; i < physicalMeshes.size(); i++)
		{
			if (physicalMeshes[i] != NULL)
				physicalMeshes[i]->release();
			physicalMeshes[i] = NULL;
		}
		physicalMeshes.clear();

		if (isoMesh != NULL)
		{
			isoMesh->release();
			isoMesh = NULL;
		}

		if (clothingAssetAuthoring != NULL)
		{
			clothingAssetAuthoring->release();
			clothingAssetAuthoring = NULL;
		}
	}

} // namespace SharedTools

#endif // defined(PX_WINDOWS)
