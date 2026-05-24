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
#ifndef CLOTHING_AUTHORING_H
#define CLOTHING_AUTHORING_H

#include <PxPreprocessor.h>
#if defined(PX_WINDOWS)
#include <PsShare.h>

#include <PxMath.h>
#include <PxMat33.h>
#include <PxMat44.h>

#include <NxApexUserProgress.h>
#include "AutoGeometry.h"

#include <vector>
#include <map>


class NxPhysicsSDK;

namespace physx
{
	namespace apex
	{
		class NxApexSDK;
		class NxApexRenderDebug;
		class NxApexScene;
		class NxResourceCallback;
		class NxUserRenderResourceManager;

		class NxRenderMeshAssetAuthoring;

		class NxModuleClothing;
		class NxClothingActor;
		class NxClothingAsset;
		class NxClothingAssetAuthoring;
		class NxClothingIsoMesh;
		class NxClothingPhysicalMesh;
		class NxClothingPreview;
		class NxClothingUserRecompute;
	}

	namespace pxtask
	{
		class TaskManager;
	}
}


namespace Samples
{
	class TriangleMesh;
	class SkeletalAnim;
}

class UserAllocator;
class UserPxAllocator;



namespace SharedTools
{

	class MeshPainter;

	class ClothingAuthoring
	{
	public:

		class ErrorCallback
		{
		public:
			void reportErrorPrintf(const char* label, const char* fmt, ...);
			virtual void reportError(const char* label, const char* message) = 0;
		};



		struct BrushMode
		{
			enum Enum
			{
				PaintFlat,
				PaintVolumetric,
				Smooth,
			};
		};



		struct CollisionVolume
		{
			CollisionVolume() : boneIndex(-1), parentIndex(-1), transform(true), meshVolume(0.0f),
				capsuleHeight(0.0f), capsuleRadius(0.0f), shapeOffset(true), inflation(0.0f) {}

			void draw(physx::apex::NxApexRenderDebug *batcher, Samples::SkeletalAnim *anim, bool wireframe, float simulationScale);

			int							boneIndex;
			int							parentIndex;
			std::string					boneName;
			physx::PxMat34Legacy		transform;
			std::vector<physx::PxVec3>	vertices;
			std::vector<unsigned int>	indices;
			float						meshVolume;
			float						capsuleHeight;
			float						capsuleRadius;
			physx::PxMat34Legacy		shapeOffset;
			float						inflation;
		};



		ClothingAuthoring(
			NxPhysicsSDK* physicsSDK,
			physx::apex::NxApexSDK* apexSDK,
			physx::apex::NxModuleClothing* moduleClothing, 
			physx::apex::NxResourceCallback* resourceCallback,
			physx::apex::NxUserRenderResourceManager* renderResourceManager,
			ClothingAuthoring::ErrorCallback* errorCallback
			);

		virtual ~ClothingAuthoring();

		void releasePhysX();

		int  getNumParameters() const;
		bool getParameter(unsigned int i, std::string& name, std::string& type, std::string& val) const;
		bool setParameter(std::string& name, std::string& type, std::string& val);


		// state information

		bool  getAndClearNeedsRedraw()								{ bool temp = mState.needsRedraw; mState.needsRedraw = false; return temp; }

		bool  getAndClearNeedsRestart()								{ bool temp = mState.needsRestart; mState.needsRestart = false; return temp; }

		bool  getAndClearManualAnimation()							{ bool temp = mState.manualAnimation; mState.manualAnimation = false; return temp; }
		void  setManualAnimation(bool on)							{ mState.manualAnimation = on; }

		struct AuthoringState
		{
			enum Enum {
				None = 0,
				MeshLoaded,
				SubmeshSelectionChanged,
				PaintingChanged,
				RenderMeshAssetCreated,
				PhysicalCustomMeshCreated,
				PhysicalClothMeshCreated,
				PhysicalIsoMeshCreated,
				PhysicalTetraMeshcreated,
				ClothingAssetCreated,
			};
		};

		AuthoringState::Enum getAuthoringState() const				{ return mState.authoringState; }

		bool createDefaultMaterialLibrary();
		bool addMaterialLibrary(const char* name, NxParameterized::Interface* newInterface);
		void removeMaterialLibrary(const char* name);

		void addMaterialToLibrary(const char* libName, const char* matName);
		void removeMaterialFromLibrary(const char* libName, const char* matName);

		void selectMaterial(const char* libName, const char* matName);
		void clearMaterialLibraries();

		const char* getSelectedMaterialLibrary() const				{ return mState.selectedMaterialLibrary.c_str(); }
		const char* getSelectedMaterial() const						{ return mState.selectedMaterial.c_str(); }

		physx::PxBounds3 getSimulationBounds()						{ return mState.apexBounds; }

		size_t getNumMaterialLibraries() const						{ return mState.materialLibraries.size(); }
		NxParameterized::Interface* getMaterialLibrary(unsigned int libraryIndex) const;
		bool setMaterialLibrary(unsigned int libraryIndex, NxParameterized::Interface* data);
		const char* getMaterialLibraryName(unsigned int libraryIndex) const;
		unsigned int getNumMaterials(unsigned int libraryIndex) const;
		const char* getMaterialName(unsigned int libraryIndex, unsigned int materialIndex) const;

		float getDrawWindTime() const								{ return mState.drawWindTime; }
		const physx::PxVec3& getWindOrigin() const					{ return mState.windOrigin; }
		const physx::PxVec3& getWindTarget() const					{ return mState.windTarget; }

		// dirty information

		void  setMaxDistancePaintingDirty(bool dirty)				{ mDirty.maxDistancePainting |= dirty; }
		bool  getAndClearMaxDistancePaintingDirty()					{ bool temp = mDirty.maxDistancePainting; mDirty.maxDistancePainting = false; return temp;}

		bool  getAndClearGroundPlaneDirty()							{ bool temp = mDirty.groundPlane; mDirty.groundPlane = false; return temp; }

		bool  getAndClearGravityDirty()								{ bool temp = mDirty.gravity; mDirty.gravity = false; return temp; }

		bool  getAndClearWorkspaceDirty()							{ bool temp = mDirty.workspace; mDirty.workspace = false; return temp; }

		void  setSimulationValueScale(float v)						{ mState.simulationValuesScale = v; }
		float getSimulationValueScale() const						{ return mState.simulationValuesScale; }

		void  setGravityValueScale(float v)							{ mState.gravityValueScale = v; }
		float getGravityValueScale() const							{ return mState.gravityValueScale; }



		// simulation objects
		size_t getNumSimulationActors() const						{ return mSimulation.actors.size(); }
		physx::apex::NxClothingActor* getSimulationActor(size_t index);
		physx::apex::NxApexRenderable* getSimulationRenderable(size_t index);
		const physx::PxMat44& getSimulationActorPose(size_t index) const;

		void  setActorCount(int count, bool addToCommandQueue);
		int   getActorCount() const									{ return mSimulation.actorCount; }

		void  setMatrices(const physx::PxMat44& viewMatrix, const physx::PxMat44& projectionMatrix);

		void  startCreateApexScene();
		bool  addAssetToScene(physx::apex::NxClothingAsset* asset, bool remapBones, physx::apex::IProgressListener* progress);
		void  finishCreateApexScene(bool recomputeScale);

		void  startSimulation();
		void  stopSimulation();
		void  restartSimulation();
		void  updateCCT(float deltaT);
		virtual void restartSimulationCallback() = 0;

		void  stepsUntilPause(int steps);
		int   getMaxLodValue() const;
		physx::apex::NxApexScene* getApexScene() const				{ return mSimulation.apexScene; }
		NxScene* getPhysXScene()									{ return mSimulation.physxScene; }



		// Meshes
		virtual bool  loadInputMesh(const char* filename, bool allowConversion, bool silentOnError);
		bool saveInputMeshToXml(const char *filename);
		void setInputMeshFilename(const char* filename)				{ mDirty.workspace |= mMeshes.inputMeshFilename.compare(filename) != 0;	mMeshes.inputMeshFilename = filename; }
		const char* getInputMeshFilename() const					{ return mMeshes.inputMeshFilename.c_str(); }
		void selectSubMesh(int subMeshNr, bool on);

		bool loadCustomPhysicsMesh(const char* filename);
		const char* getCustomPhysicsMeshFilename() const			{ return mMeshes.customPhysicsMeshFilename.empty() ? NULL : mMeshes.customPhysicsMeshFilename.c_str(); }
		void clearCustomPhysicsMesh();

		Samples::TriangleMesh* getInputMesh()						{ return mMeshes.inputMesh; }
		Samples::SkeletalAnim* getSkeleton()						{ return mMeshes.skeleton; }
		Samples::TriangleMesh* getCustomPhysicsMesh()				{ return mMeshes.customPhysicsMesh; }
		Samples::TriangleMesh* getGroundMesh()						{ assert(mMeshes.groundMesh); return mMeshes.groundMesh; }

		// modify mesh
		int subdivideSubmesh(int subMeshNumber);
		size_t getNumSubdivisions()									{ return mMeshes.subdivideHistory.size(); }
		int getSubdivisionSubmesh(int index)						{ return mMeshes.subdivideHistory[index].submesh; }
		int getSubdivisionSize(int index)							{ return mMeshes.subdivideHistory[index].subdivision; }

		void renameSubMeshMaterial(int submesh, const char* newName);
		void resetSubMeshMaterial(int submesh);
		size_t getNumRenames()										{ return mMeshes.renameHistory.size(); }
		int getRenameSubmesh(int index)								{ return mMeshes.renameHistory[index].submesh; }
		const char* getRenameName(int index)						{ return mMeshes.renameHistory[index].newName.c_str(); }

		// painting
		MeshPainter* getPainter()									{ return mMeshes.painter;}
		void paint(const physx::PxVec3& rayOrigin, const physx::PxVec3& rayDirection, bool execute, bool leftButton, bool rightButton);
		void floodPainting(bool invalid);
		void smoothPainting(int numIterations);

		void updatePainter();
		void setPainterIndexBufferRange();
		void updatePaintingColors();
		bool getMaxDistancePaintValues(const float*& values, int& numValues, int& byteStride);
		float getAbsolutePaintingScalingMaxDistance();
		float getAbsolutePaintingScalingCollisionFactor();

		void setPhysicalLodValues(int numValues, const float* values);
		const float* getPhysicalLodValues() const					{ return mMeshes.physicalLodValues.empty() ? NULL : &mMeshes.physicalLodValues[0]; }
		size_t getNumPhysicalLodValues() const						{ return mMeshes.physicalLodValues.size(); }


		void initGroundMesh(const char* resourceDir);

		// modify animation
		void setAnimationPose(int position);
		void setBindPose();
		void setAnimationTime(float t);
		float getAnimationTime() const;
		bool updateAnimation(int delay);

		// collision volumes
		void clearCollisionVolumes();
		unsigned int generateCollisionVolumes(bool useCapsule, bool commandMode, bool dirtyOnly);
		CollisionVolume* addCollisionVolume(bool useCapsule, unsigned int boneIndex);
		size_t getNumCollisionVolumes() const { return mMeshes.collisionVolumes.size(); }
		CollisionVolume* getCollisionVolume(int index);
		bool deleteCollisionVolume(int index);
		void drawCollisionVolumes(bool wireframe) const;



		// Authoring objecst
		size_t getNumRenderMeshAssets()								{ return mConfig.cloth.numGraphicalLods; }
		physx::apex::NxRenderMeshAssetAuthoring* getRenderMeshAsset(int index);

		physx::apex::NxClothingPhysicalMesh* getClothMesh(int index, physx::apex::IProgressListener* progress);
		void simplifyClothMesh(float factor);
		int getNumClothTriangles() const;

		physx::apex::NxClothingIsoMesh* getIsoMesh(physx::apex::IProgressListener* progress);
		void simplifyIsoMesh(float factor, physx::apex::IProgressListener* progress);
		int getNumIsoSurfaceTriangles() const;

		physx::apex::NxClothingPhysicalMesh* getTetraMesh(physx::apex::IProgressListener* progress);
		int getNumTetrahedra() const;

		physx::apex::NxClothingPhysicalMesh* getPhysicalMesh();
		physx::apex::NxClothingAssetAuthoring* getClothingAsset(physx::apex::IProgressListener* progress);




		// configuration.UI
		void  setZAxisUp(bool z);
		bool  getZAxisUp() const									{ return mConfig.ui.zAxisUp; }



		// configuration.mesh
		void  setSubmeshSubdiv(int value)							{ mDirty.workspace |= mConfig.mesh.originalMeshSubdivision != value;	mConfig.mesh.originalMeshSubdivision = value; }
		int   getSubmeshSubdiv() const								{ return mConfig.mesh.originalMeshSubdivision; }

		void  setEvenOutVertexDegrees(bool on)						{ mDirty.workspace |= mConfig.mesh.evenOutVertexDegrees != on;			mConfig.mesh.evenOutVertexDegrees = on; }
		bool  getEvenOutVertexDegrees() const						{ return mConfig.mesh.evenOutVertexDegrees; }

		void  setCullMode(physx::apex::NxRenderCullMode::Enum mode);
		physx::apex::NxRenderCullMode::Enum getCullMode() const		{ return (physx::apex::NxRenderCullMode::Enum)mConfig.mesh.cullMode; }

		void  setTextureUvOrigin(physx::apex::NxTextureUVOrigin::Enum origin);
		physx::apex::NxTextureUVOrigin::Enum getTextureUvOrigin() const { return (physx::apex::NxTextureUVOrigin::Enum)mConfig.mesh.textureUvOrigin; }



		// configuration.Apex

		void  setParallelPhysXMeshSkinning(bool s)					{ mDirty.workspace |= mConfig.apex.parallelPhysXMeshSkinning != s;		mConfig.apex.parallelPhysXMeshSkinning = s; mDirty.clothingActorFlags = true; }
		bool  getParallelPhysXMeshSkinning() const					{ return mConfig.apex.parallelPhysXMeshSkinning; }

		void  setParallelMeshMeshSkinning(bool s)					{ mDirty.workspace |= mConfig.apex.parallelMeshMeshSkinning != s;		mConfig.apex.parallelMeshMeshSkinning = s; mDirty.clothingActorFlags = true; }
		bool  getParallelMeshMeshSkinning() const					{ return mConfig.apex.parallelMeshMeshSkinning; }

		void  setParallelCpuSkinning(bool s)						{ mDirty.workspace |= mConfig.apex.parallelCpuSkinning != s;			mConfig.apex.parallelCpuSkinning = s; mDirty.clothingActorFlags = true; }
		bool  getParallelCpuSkinning() const						{ return mConfig.apex.parallelCpuSkinning; }

		void  setRecomputeNormals(bool r)							{ mDirty.workspace = mConfig.apex.recomputeNormals != r;				mConfig.apex.recomputeNormals = r; mDirty.clothingActorFlags = true; }
		bool  getRecomputeNormals() const							{ return mConfig.apex.recomputeNormals; }



		// configuration.tempMeshes.Cloth
		void  setClothNumGraphicalLods(int numLods)					{ mDirty.workspace |= mConfig.cloth.numGraphicalLods != numLods;	mConfig.cloth.numGraphicalLods = numLods; setAuthoringState(AuthoringState::RenderMeshAssetCreated, false); }
		int   getClothNumGraphicalLods() const						{ return mConfig.cloth.numGraphicalLods; }

		void  setClothSimplifySL(int simplification)				{ mDirty.workspace |= mConfig.cloth.simplify != simplification;		mConfig.cloth.simplify = simplification; setAuthoringState(AuthoringState::RenderMeshAssetCreated, false); }
		int   getClothSimplifySL() const							{ return mConfig.cloth.simplify; }

		void  setCloseCloth(bool close)								{ mDirty.workspace |= mConfig.cloth.close != close;					mConfig.cloth.close = close; setAuthoringState(AuthoringState::RenderMeshAssetCreated, false); }
		bool  getCloseCloth() const									{ return mConfig.cloth.close; }

		void  setSlSubdivideCloth(bool subdivide)					{ mDirty.workspace |= mConfig.cloth.subdivide != subdivide;			mConfig.cloth.subdivide = subdivide; setAuthoringState(AuthoringState::RenderMeshAssetCreated, false); }
		bool  getSlSubdivideCloth() const							{ return mConfig.cloth.subdivide; }

		void  setClothMeshSubdiv(int subdivision)					{ mDirty.workspace |= mConfig.cloth.subdivision != subdivision;		mConfig.cloth.subdivision = subdivision; setAuthoringState(AuthoringState::RenderMeshAssetCreated, false); }
		int   getClothMeshSubdiv() const							{ return mConfig.cloth.subdivision; }



		// configuration.tempMeshes.Softbody
		void  setIsoSurfaceSubdivision(int subdivision)				{ mDirty.workspace |= mConfig.softbody.isoSurfaceSubdivision != subdivision; mConfig.softbody.isoSurfaceSubdivision = subdivision; setAuthoringState(AuthoringState::RenderMeshAssetCreated, false); }
		int   getIsoSurfaceSubdivision() const						{ return mConfig.softbody.isoSurfaceSubdivision; }

		void  setIsoSimplifySubdivision(int subdivision)			{ mDirty.workspace |= mConfig.softbody.isoSimplifySubdivision != subdivision; mConfig.softbody.isoSimplifySubdivision = subdivision; setAuthoringState(AuthoringState::RenderMeshAssetCreated, false); }
		int   getIsoSimplifySubdivision() const						{ return mConfig.softbody.isoSimplifySubdivision; }

		void  setTetraKeepBiggestMeshes(int count)					{ mDirty.workspace |= mConfig.softbody.keepBiggestMeshes != count; mConfig.softbody.keepBiggestMeshes = count; setAuthoringState(AuthoringState::RenderMeshAssetCreated, false); }
		int   getTetraKeepBiggestMeshes() const						{ return mConfig.softbody.keepBiggestMeshes; }



		// configuration.collisionVolumes
		void  setCollisionVolumeUsePaintChannel(bool on)			{ mDirty.workspace |= mConfig.collisionVolumes.usePaintingChannel != on; mConfig.collisionVolumes.usePaintingChannel = on; }
		bool  getCollisionVolumeUsePaintChannel() const				{ return mConfig.collisionVolumes.usePaintingChannel; }



		// configuration.painting
		void  setBrushMode(BrushMode::Enum mode)					{ mDirty.workspace |= mConfig.painting.brushMode != mode;				mConfig.painting.brushMode = mode; }
		BrushMode::Enum getBrushMode() const						{ return (BrushMode::Enum)mConfig.painting.brushMode; }

		void  setFalloffExponent(float exp)							{ mDirty.workspace |= mConfig.painting.falloffExponent != exp;			mConfig.painting.falloffExponent = exp; mState.needsRedraw = true; }
		float getFalloffExponent() const							{ return mConfig.painting.falloffExponent; }

		void  setPaintingIgnoreUnusedVertices(bool on)				{ mDirty.workspace |= mConfig.painting.ignoreUnusedVertices != on;		mConfig.painting.ignoreUnusedVertices = on; }
		bool  getPaintingIgnoreUnusedVertices() const				{ return mConfig.painting.ignoreUnusedVertices; }

		void  setPaintingChannel(int channel);
		int getPaintingChannel() const								{ return mConfig.painting.channel; }

		virtual void  setPaintingValue(float val, float vmin, float vmax);
		float getPaintingValue() const								{ return mConfig.painting.value; }
		float getPaintingValueMin() const							{ return mConfig.painting.valueMin; }
		float getPaintingValueMax() const							{ return mConfig.painting.valueMax; }

		void  setBrushRadius(int radius)							{ mDirty.workspace |= mConfig.painting.brushRadius != radius;			mConfig.painting.brushRadius = radius; }
		int   getBrushRadius() const								{ return mConfig.painting.brushRadius; }

		void  setPaintingScalingMaxDistance(float scaling)			{ mDirty.workspace |= mConfig.painting.scalingMaxdistance != scaling;	mConfig.painting.scalingMaxdistance = scaling;	mDirty.maxDistancePainting = true; mDirty.clothingActorFlags = true; setAuthoringState(AuthoringState::PaintingChanged, false); }
		float getPaintingScalingMaxDistance() const					{ return mConfig.painting.scalingMaxdistance; }

		void  setPaintingScalingCollisionFactor(float scaling)		{ mDirty.workspace |= mConfig.painting.scalingCollisionFactor != scaling; mConfig.painting.scalingCollisionFactor = scaling; mDirty.clothingActorFlags = true; setAuthoringState(AuthoringState::PaintingChanged, false); }
		float getPaintingScalingCollisionFactor() const				{ return mConfig.painting.scalingCollisionFactor; }

		void  setMaxDistanceScale(float scale)						{ mDirty.maxDistanceScale |= mConfig.painting.maxDistanceScale != scale; mConfig.painting.maxDistanceScale = scale; }
		float getMaxDistanceScale() const							{ return mConfig.painting.maxDistanceScale; }

		void  setMaxDistanceScaleMultipliable(bool multipliable)	{ mDirty.maxDistanceScale |= mConfig.painting.maxDistanceScaleMultipliable != multipliable; mConfig.painting.maxDistanceScaleMultipliable = multipliable; }
		bool  getMaxDistanceScaleMultipliable() const				{ return mConfig.painting.maxDistanceScaleMultipliable; }



		// configuration.setMeshes
		void  setDeriveNormalsFromBones(bool on)					{ mDirty.workspace |= mConfig.setMeshes.deriveNormalsFromBones != on;	mConfig.setMeshes.deriveNormalsFromBones = on; } 
		bool  getDeriveNormalsFromBones() const						{ return mConfig.setMeshes.deriveNormalsFromBones; }

		void  setFasterMeshMeshSkinning(bool on)					{ mDirty.workspace |= mConfig.setMeshes.fasterMeshMeshSkinning != on;	mConfig.setMeshes.fasterMeshMeshSkinning = on; }
		bool  getFasterMeshMeshSkinning() const						{ return mConfig.setMeshes.fasterMeshMeshSkinning; }



		// configuration.simulation
		void  setSimulationFrequency(int freq)						{ mDirty.workspace |= mConfig.simulation.frequency != (float)freq;		mConfig.simulation.frequency = (float)freq; }
		int   getSimulationFrequency() const						{ return (int)mConfig.simulation.frequency; }

		void  setGravity(int gravity)								{ mDirty.workspace |= mConfig.simulation.gravity != gravity;			mConfig.simulation.gravity = gravity; mDirty.gravity = true; }
		int   getGravity() const									{ return mConfig.simulation.gravity; }

		void  setGroundplane(int v)									{ mDirty.workspace |= mConfig.simulation.groundplane != v;				mConfig.simulation.groundplane = v; mDirty.groundPlane = true; }
		int   getGroundplane() const								{ return mConfig.simulation.groundplane; }

		void  setGroundplaneEnabled(bool on)						{ mDirty.workspace |= mConfig.simulation.groundplaneEnabled != on;		mConfig.simulation.groundplaneEnabled = on; mDirty.groundPlane = true; }
		bool  getGroundplaneEnabled() const							{ return mConfig.simulation.groundplaneEnabled; }

		void  setBudgetPercent(int p)								{ p = physx::PxClamp(p, 0, 100);										mConfig.simulation.budgetPercent = p; }
		int   getBudgetPercent() const								{ return mConfig.simulation.budgetPercent; }

		void  setBlendTime(float time)								{ mDirty.workspace |= mConfig.simulation.blendTime != time;				mConfig.simulation.blendTime = time; mDirty.blendTime = true; }
		float getBlendTime() const									{ return mConfig.simulation.blendTime; }

		void  setLodOverwrite(int lod)								{ mDirty.workspace |= mConfig.simulation.lodOverwrite != lod;			mConfig.simulation.lodOverwrite = lod; }
		int   getLodOverwrite() const								{ return mConfig.simulation.lodOverwrite; }

		void  setWindDirection(int w)								{ mDirty.workspace |= mConfig.simulation.windDirection != w;			mConfig.simulation.windDirection = w; mState.drawWindTime = 3.0f; }
		int   getWindDirection() const								{ return mConfig.simulation.windDirection; }

		void  setWindElevation(int w)								{ mDirty.workspace |= mConfig.simulation.windElevation != w;			mConfig.simulation.windElevation = w; mState.drawWindTime = 3.0f; }
		int   getWindElevation() const								{ return mConfig.simulation.windElevation; }

		void  setWindVelocity(int w)								{ mDirty.workspace |= mConfig.simulation.windVelocity != w;				mConfig.simulation.windVelocity = w; mState.drawWindTime = 3.0f; }
		int   getWindVelocity() const								{ return mConfig.simulation.windVelocity; }

		void  setGpuSimulation(bool gpuSimulation)					{ mDirty.workspace |= mConfig.simulation.gpuSimulation != gpuSimulation; mConfig.simulation.gpuSimulation = gpuSimulation; mState.needsRestart = true; }
		bool  getGpuSimulation() const								{ return mConfig.simulation.gpuSimulation; }

		virtual void  setFallbackSkinning(bool fallbackSkinning)	{ mDirty.workspace |= mConfig.simulation.fallbackSkinning != fallbackSkinning; mConfig.simulation.fallbackSkinning = fallbackSkinning; }
		bool  getFallbackSkinning() const							{ return mConfig.simulation.fallbackSkinning; }

		void  setCCTSpeed(float speed)								{																		mConfig.simulation.CCTSpeed = speed; }
		float getCCTSpeed() const									{ return mConfig.simulation.CCTSpeed; }

		void  setTimingNoise(float noise)							{																		mConfig.simulation.timingNoise = noise; }
		float getTimingNoise() const								{ return mConfig.simulation.timingNoise; }

		void  setCCTDirection(const physx::PxVec3& direction)		{ mSimulation.CCTDirection = direction; }

		void  setGraphicalLod(int lod)								{ mDirty.workspace |= mConfig.simulation.graphicalLod != lod;			mConfig.simulation.graphicalLod = lod; }
		int   getGraphicalLod() const								{ return mConfig.simulation.graphicalLod; }

		void  setUsePreview(bool on)								{ mState.needsRestart |= mSimulation.actors.size() > 0;					mConfig.simulation.usePreview = on; }
		bool  getUsePreview() const									{ return mConfig.simulation.usePreview; }



		// configuration.animation
		void  setShowSkinnedPose(bool on)							{																		mConfig.animation.showSkinnedPose = on; }
		bool  getShowSkinnedPose()									{ return mConfig.animation.showSkinnedPose; }

		void  setAnimation(int animation);
		int   getAnimation() const									{ return mConfig.animation.selectedAnimation; }

		void  setAnimationSpeed(int s)								{ mDirty.workspace |= mConfig.animation.speed != s;						mConfig.animation.speed = s; }
		int   getAnimationSpeed() const								{ return mConfig.animation.speed; }

		void  setAnimationTimes(float val)							{ mConfig.animation.times[0] = mConfig.animation.times[1] = mConfig.animation.times[2] = val; }
		float getAnimationTimes(int delay) const					{ assert(delay < 3); return mConfig.animation.times[delay]; }
		void  stepAnimationTimes(float animStep);
		bool  clampAnimation(float& time, bool stoppable, bool loop, float minTime, float maxTime);

		void  setMoveCameraWithAnimation(bool on)					{ mDirty.workspace |= mConfig.animation.moveCameraWithAnimation != on;	mConfig.animation.moveCameraWithAnimation = on; }
		bool  getMoveCameraWithAnimation() const					{ return mConfig.animation.moveCameraWithAnimation; }

		void  setLoopAnimation(bool on)								{ mDirty.workspace |= mConfig.animation.loop != on;						mConfig.animation.loop = on; }
		bool  getLoopAnimation() const								{ return mConfig.animation.loop; }

		void  setLockRootbone(bool on)								{ mDirty.workspace |= mConfig.animation.lockRootbone != on;				mConfig.animation.lockRootbone = on; mState.manualAnimation = true; }
		bool  getLockRootbone() const								{ return mConfig.animation.lockRootbone; }

		void  setAnimationContinuous(bool on)						{ mDirty.workspace |= mConfig.animation.continuous != on;				mConfig.animation.continuous = on; }
		bool  getAnimationContinuous() const						{ return mConfig.animation.continuous; }

		void  setUseGlobalPoseMatrices(bool on)						{ mDirty.workspace |= mConfig.animation.useGlobalPoseMatrices != on; mState.needsRestart = mConfig.animation.useGlobalPoseMatrices != on; mConfig.animation.useGlobalPoseMatrices = on; }
		bool  getUseGlobalPoseMatrices() const						{ return mConfig.animation.useGlobalPoseMatrices; }

		void  setAnimationCrop(float min, float max);




		// configuration.deformable
		void  setDeformableThickness(float value)					{ mDirty.workspace |= mConfig.deformable.thickness != value;			mConfig.deformable.thickness = value; mConfig.deformable.drawThickness = true; }
		float getDeformableThickness() const						{ return mConfig.deformable.thickness; }

		void  setDrawDeformableThickness(bool on)					{																		mConfig.deformable.drawThickness = on; }
		bool  getDrawDeformableThickness() const					{ return mConfig.deformable.drawThickness; }

		void  setDeformableSelfcollisionThickness(float value)		{ mDirty.workspace |= mConfig.deformable.selfcollisionThickness != value; mConfig.deformable.selfcollisionThickness = value; mConfig.deformable.drawSelfcollisionThickness = true; }
		float getDeformableSelfcollisionThickness() const			{ return mConfig.deformable.selfcollisionThickness; }

		void  setDrawDeformableSelfcollisionThickness(bool on)		{																		mConfig.deformable.drawSelfcollisionThickness = on; }
		bool  getDrawDeformableSelfcollisionThickness() const		{ return mConfig.deformable.drawSelfcollisionThickness; }

		void  setDeformableHierarchicalLevels(int value)			{ mDirty.workspace |= mConfig.deformable.hierarchicalLevels != value;	mConfig.deformable.hierarchicalLevels = value; }
		int   getDeformableHierarchicalLevels() const				{ return mConfig.deformable.hierarchicalLevels; }

		void  setDeformableDisableCCD(bool on)						{ mDirty.workspace |= mConfig.deformable.disableCCD != on;				mConfig.deformable.disableCCD = on; }
		bool  getDeformableDisableCCD() const						{ return mConfig.deformable.disableCCD; }

		void  setDeformableSelfcollision(bool on)					{ mDirty.workspace |= mConfig.deformable.selfcollision != on;			mConfig.deformable.selfcollision = on; }
		bool  getDeformableSelfcollision() const					{ return mConfig.deformable.selfcollision; }

		void  setDeformableTwowayInteraction(bool on)				{ mDirty.workspace |= mConfig.deformable.twowayInteraction != on;		mConfig.deformable.twowayInteraction = on; }
		bool  getDeformableTwowayInteraction() const				{ return mConfig.deformable.twowayInteraction; }

		void  setDeformableUntangling(bool on)						{ mDirty.workspace |= mConfig.deformable.untangling != on;				mConfig.deformable.untangling = on; }
		bool  getDeformableUntangling() const						{ return mConfig.deformable.untangling; }



		// init configuration
		void resetTempConfiguration();
		void initConfiguration();
		void prepareConfiguration();


		size_t getNumCommands() const								{ return mRecordCommands.size(); }
		int getCommandFrameNumber(size_t index) const				{ return mRecordCommands[index].frameNumber; }
		const char* getCommandString(size_t index) const			{ return mRecordCommands[index].command.c_str(); }
		void addCommand(const char* command, int frameNumber = -2);
		void clearCommands();

		// file IO
		bool loadParameterized(const char* filename, physx::PxFileBuf* filebuffer, NxParameterized::Serializer::DeserializedData& deserializedData, bool silent = false);
		bool saveParameterized(const char* filename, physx::PxFileBuf* filebuffer, const NxParameterized::Interface** pInterfaces, unsigned int numInterfaces);

	private:
		NxParameterized::Serializer::SerializeType extensionToType(const char* filename) const;
		bool parameterizedError(NxParameterized::Serializer::ErrorType errorType, const char* filename);
	
		void setAuthoringState(AuthoringState::Enum authoringState, bool allowAdvance);

		// internal methods
		physx::AutoGeometry* createAutoGeometry();
		void addCollisionVolumeInternal(physx::SimpleHull* hull, bool useCapsule);


		void createRenderMeshAssets();

		void createCustomMesh();

		void createClothMeshes(physx::apex::IProgressListener* progress);

		void createIsoMesh(physx::apex::IProgressListener* progress);

		void createTetraMesh(physx::apex::IProgressListener* progress);

		void createClothingAsset(physx::apex::IProgressListener* progress);

		void updateDeformableParameters();


		struct CurrentState
		{
			bool  needsRedraw;
			bool  needsRestart;
			bool  manualAnimation;
			bool  renderMeshAssetLocked;
			AuthoringState::Enum authoringState;
			float simulationValuesScale;
			float gravityValueScale;

			typedef std::map<std::string, NxParameterized::Interface*> tMaterialLibraries;
			tMaterialLibraries materialLibraries;

			std::string selectedMaterialLibrary;
			std::string selectedMaterial;

			physx::PxVec3 windOrigin;
			physx::PxVec3 windTarget;
			float drawWindTime;

			physx::PxBounds3 apexBounds;

			int currentFrameNumber;

			void init()
			{
				needsRedraw = false;
				needsRestart = false;
				manualAnimation = false;
				renderMeshAssetLocked = false;
				authoringState = AuthoringState::None;
				simulationValuesScale = 1.0f;
				gravityValueScale = 0.0f;

				for (tMaterialLibraries::iterator it = materialLibraries.begin(); it != materialLibraries.end(); ++it)
					it->second->destroy();

				materialLibraries.clear();

				selectedMaterialLibrary.clear();
				selectedMaterial.clear();

				windOrigin.setZero();
				windTarget.setZero();
				drawWindTime = 0;

				apexBounds.setEmpty();

				currentFrameNumber = -1;
			}
		};
		CurrentState mState;



		struct DirtyFlags
		{
			bool maxDistancePainting;
			bool maxDistanceScale;
			bool clothingActorFlags;
			bool groundPlane;
			bool gravity;
			bool blendTime;
			bool workspace;

			void init()
			{
				maxDistancePainting = false;
				maxDistanceScale = false;
				clothingActorFlags = false;
				groundPlane = false;
				gravity = false;
				blendTime = false;
				workspace = false;
			}
		};
		DirtyFlags mDirty;



		struct Simulation
		{
			struct ClothingActor
			{
				ClothingActor() : actor(NULL), preview(NULL)
				{
					initPose = physx::PxMat44::identity();
					currentPose = physx::PxMat44::identity();
				}
				physx::apex::NxClothingActor* actor;
				physx::apex::NxClothingPreview* preview;
				physx::PxMat44 initPose;
				physx::PxMat44 currentPose;
			};

			physx::pxtask::TaskManager* taskManager;
			physx::apex::NxApexScene* apexScene;
			NxScene* physxScene;
			bool running;

			// Normal/Tangent re-computation
			physx::apex::NxClothingUserRecompute* userRecompute;

			std::vector<physx::apex::NxClothingAsset*> assets;
			bool clearAssets;
			int actorCount;
			std::vector<ClothingActor> actors;
			bool paused;

			NxActor* groundPlane;

			unsigned int stepsUntilPause;

			physx::PxVec3 CCTPosition;
			physx::PxVec3 CCTDirection;


			void init()
			{
				taskManager = NULL;
				apexScene = NULL;
				physxScene = NULL;
				running = false;
				userRecompute = NULL;
				clearAssets = true;
				actorCount = 1;
				paused = false;
				groundPlane = NULL;
				stepsUntilPause = 0;

				CCTPosition.setZero();
				CCTDirection.setZero();
			}

			void clear();
		};
		Simulation mSimulation;



		struct Meshes
		{
			Samples::TriangleMesh* inputMesh;
			std::string inputMeshFilename;
			MeshPainter* painter;
			std::vector<float> physicalLodValues;
			Samples::TriangleMesh* customPhysicsMesh;
			std::string customPhysicsMeshFilename;
			Samples::TriangleMesh* groundMesh;
			Samples::SkeletalAnim* skeleton;
			Samples::SkeletalAnim* skeletonBehind;

			std::vector<CollisionVolume> collisionVolumes;



			struct SubdivideHistoryItem
			{
				int submesh;
				int subdivision;
			};
			std::vector<SubdivideHistoryItem> subdivideHistory;


			struct SubmeshMaterialRename
			{
				int submesh;
				std::string newName;
			};
			std::vector<SubmeshMaterialRename> renameHistory;

			void init()
			{
				inputMesh = NULL;
				//inputMeshFilename.clear();
				painter = NULL;
				physicalLodValues.clear();
				customPhysicsMesh = NULL;
				customPhysicsMeshFilename.clear();
				groundMesh = NULL;
				skeleton = NULL;
				skeletonBehind = NULL;
				collisionVolumes.clear();
				subdivideHistory.clear();
				renameHistory.clear();
			}

			void clear(physx::apex::NxUserRenderResourceManager* rrm, physx::apex::NxResourceCallback* rcb, bool groundAsWell);
		};
		Meshes mMeshes;



		struct AuthoringObjects
		{
			std::vector<physx::apex::NxRenderMeshAssetAuthoring*> renderMeshAssets;
			std::vector<physx::apex::NxClothingPhysicalMesh*> physicalMeshes;
			physx::apex::NxClothingIsoMesh*	isoMesh;
			physx::apex::NxClothingAssetAuthoring* clothingAssetAuthoring;

			void init()
			{
				renderMeshAssets.clear();
				physicalMeshes.clear();
				isoMesh = NULL;
				clothingAssetAuthoring = NULL;
			}

			void clear();
		};
		AuthoringObjects mAuthoringObjects;



		// configuration
		std::map<std::string, float*> mFloatConfiguration;
		std::map<std::string, float*> mFloatConfigurationOld;
		std::map<std::string, int*> mIntConfiguration;
		std::map<std::string, int*> mIntConfigurationOld;
		std::map<std::string, bool*> mBoolConfiguration;
		std::map<std::string, bool*> mBoolConfigurationOld;

		struct configUI
		{
			bool zAxisUp;

			void init()
			{
				zAxisUp = false;
			}
		};

		struct configMesh
		{
			int		originalMeshSubdivision;
			bool	evenOutVertexDegrees;
			int		cullMode;
			int		textureUvOrigin;
			bool	useNewApi;
			int     physicalMeshType;

			void init()
			{
				originalMeshSubdivision = 10;
				evenOutVertexDegrees = false;
				cullMode = physx::apex::NxRenderCullMode::NONE;
				textureUvOrigin = physx::apex::NxTextureUVOrigin::ORIGIN_TOP_LEFT;
				useNewApi = true;
				physicalMeshType = 0;
			}
		};

		struct configApex
		{
			bool	parallelPhysXMeshSkinning;
			bool	parallelMeshMeshSkinning;
			bool	parallelCpuSkinning;
			bool	recomputeNormals;

			void init()
			{
				parallelPhysXMeshSkinning = false;
				parallelMeshMeshSkinning = false;
				parallelCpuSkinning = true;
				recomputeNormals = false;
			}
		};

		struct configCloth
		{
			int		numGraphicalLods;
			int		simplify;
			bool	close;
			bool	subdivide;
			int		subdivision;

			void init()
			{
				numGraphicalLods = 1;
				simplify = 40;
				close = false;
				subdivide = false;
				subdivision = 40;
			}
		};

		struct configSoftbody
		{
			int		isoSurfaceSubdivision;
			int		isoSimplifySubdivision;
			int		keepBiggestMeshes;

			void init()
			{
				isoSurfaceSubdivision = 80;
				isoSimplifySubdivision = 30;
				keepBiggestMeshes = 0;
			}
		};

		struct configCollisionVolumes
		{
			bool	usePaintingChannel;

			void init()
			{
				usePaintingChannel = false;
			}
		};

		struct configPainting
		{
			int		brushMode;
			float	falloffExponent;
			bool	ignoreUnusedVertices;
			int		channel;
			float	value;
			float	valueMin;
			float	valueMax;
			int		brushRadius;
			float	scalingMaxdistance;
			float	scalingCollisionFactor;
			float	maxDistanceScale;
			bool	maxDistanceScaleMultipliable;

			void init()
			{
				brushMode = BrushMode::PaintFlat;
				falloffExponent = 1.0f;
				ignoreUnusedVertices = true;
				channel = 3; // NUM_CHANNELS
				value = 1.0f;
				valueMin = 0.0f;
				valueMax = 1.0f;
				brushRadius = 50;
				scalingMaxdistance = 1.0f;
				scalingCollisionFactor = 1.0f;
				maxDistanceScale = 1.0f;
				maxDistanceScaleMultipliable = true;
			}
		};

		struct configSetMeshes
		{
			bool	fasterMeshMeshSkinning;
			bool	deriveNormalsFromBones;

			void init()
			{
				fasterMeshMeshSkinning = true;
				deriveNormalsFromBones = false;
			}
		};

		struct configSimulation
		{
			float	frequency;
			int		gravity;
			int		groundplane;
			bool	groundplaneEnabled;
			int		budgetPercent;
			float	blendTime;
			int		lodOverwrite;
			int		windDirection;
			int		windElevation;
			int		windVelocity;
			bool	gpuSimulation;
			bool	fallbackSkinning;
			float	CCTSpeed;
			int		graphicalLod;
			bool	usePreview;
			float	timingNoise;

			void init()
			{
				frequency = 50.0f;
				gravity = 10;
				groundplane = 0;
				groundplaneEnabled = true;
				budgetPercent = 100;
				blendTime = 1.0f;
				lodOverwrite = -1;
				windDirection = 0;
				windElevation = 0;
				windVelocity = -1;
				gpuSimulation = true;
				fallbackSkinning = false;
				CCTSpeed = 0.0f;
				graphicalLod = 0;
				usePreview = false;
				timingNoise = 0.0f;
			}
		};

		struct configAnimation
		{
			bool	showSkinnedPose; // can overwrite mAnimatio>0 for the non-simulating part
			int		selectedAnimation; // 0 and negative mean no animation (use negative to switch off and remember which one)
			int		speed;
			float	times[3];
			bool	moveCameraWithAnimation;
			bool	loop;
			bool	lockRootbone;
			bool	continuous;
			bool	useGlobalPoseMatrices;
			
			float   cropMin;
			float   cropMax;

			void init()
			{
				showSkinnedPose = false;
				selectedAnimation = -1;
				speed = 100;
				times[0] = times[1] = times[2] = 0.0f;
				moveCameraWithAnimation = true;
				loop = true;
				lockRootbone = false;
				continuous = false;
				useGlobalPoseMatrices = true;

				cropMin = 0.0f;
				cropMax = 1000000.0f;
			}
		};

		struct configDeformable
		{
			float	thickness;
			bool	drawThickness;
			float	selfcollisionThickness;
			bool	drawSelfcollisionThickness;
			int		hierarchicalLevels;
			bool	disableCCD;
			bool	selfcollision;
			bool	twowayInteraction;
			bool	untangling;

			void init()
			{
				thickness = 0.01f;
				drawThickness = false;
				selfcollisionThickness = 0.01f;
				drawSelfcollisionThickness = false;
				hierarchicalLevels = 0;
				disableCCD = false;
				selfcollision = false;
				twowayInteraction = false;
				untangling = false;
			}
		};

		struct Configuration
		{
			configUI ui;
			configMesh mesh;
			configApex apex;
			configCloth cloth;
			configSoftbody softbody;
			configCollisionVolumes collisionVolumes;
			configPainting painting;
			configSetMeshes setMeshes;
			configSimulation simulation;
			configAnimation animation;
			configDeformable deformable;

			void init()
			{
				ui.init();
				mesh.init();
				apex.init();
				cloth.init();
				softbody.init();
				collisionVolumes.init();
				painting.init();
				setMeshes.init();
				simulation.init();
				animation.init();
				deformable.init();
			}
		};

		Configuration mConfig;

		struct Command
		{
			int frameNumber;
			std::string command;
		};

		std::vector<Command> mRecordCommands;


	protected:

		// PhysX
		NxPhysicsSDK* _mPhysicsSDK;

		// APEX
		physx::apex::NxApexSDK* _mApexSDK;
		physx::apex::NxModuleClothing* _mModuleClothing;

		physx::apex::NxApexRenderDebug* _mApexRenderDebug;

		physx::apex::NxResourceCallback* _mResourceCallback;
		physx::apex::NxUserRenderResourceManager* _mRenderResourceManager;

		// Callback
		ErrorCallback* _mErrorCallback;

	};

} // namespace SharedTools

#endif //defined(PX_WINDOWS)

#endif //CLOTHING_AUTHORING_H

