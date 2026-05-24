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
#ifndef SKELETAL_ANIM
#define SKELETAL_ANIM

#include "MeshImport.h"
#include "FastXml.h"
#include <vector>
#include "TriangleMesh.h"

class NxActor;
class NxScene;

namespace MESHIMPORT
{
  class MeshSystemContainer;
};

namespace physx
{
	namespace apex
	{
		class NxApexRenderDebug;
	}
}

namespace Samples
{

	class TriangleMesh;

// ---------------------------------------------------------------------------
struct SkeletalBone
{
	void clear();

	std::string name;
	int id;
	physx::PxMat34Legacy pose;
	physx::PxVec3 scale;
	int parent;
	int firstChild;
	int numChildren;
	int firstVertex;
	
	physx::PxMat34Legacy bindWorldPose;
	physx::PxMat34Legacy invBindWorldPose;
	physx::PxMat34Legacy currentWorldPose;
	int boneOption;
	float inflateConvex;
	float minimalBoneWeight;
	int numShapes;
	bool selected;
	bool allowPrimitives;
	bool dirtyParams;
	bool manualShapes;
};

struct BoneKeyFrame {
	void clear();
	physx::PxMat34Legacy relPose;
	float time;
	physx::PxVec3 scale;
};

struct BoneTrack {
	void clear();
	int firstFrame;
	int numFrames;
};

struct SkeletalAnimation {
	void clear();
	std::string name;
	std::vector<BoneTrack> mBoneTracks;
	float minTime;
	float maxTime;
};

// ---------------------------------------------------------------------------
class SkeletalAnim : public FAST_XML::FastXml::Callback
{
public:
	SkeletalAnim();
	virtual ~SkeletalAnim();

	void clear();
	void copyFrom(const SkeletalAnim &anim);

	bool loadFromXML(const std::string& xmlFile, std::string& error);
	bool saveToXML(const std::string& xmlFile) const;
	bool loadFromParent(const SkeletalAnim* parent);
	bool loadFromMeshImport(physx::MeshSystemContainer *msc, std::string& error);

	bool initFrom(physx::apex::NxRenderMeshAssetAuthoring& mesh);

	void setBindPose();
	void setAnimPose(int animNr, float time, bool lockRootbone = false);
	const std::vector<SkeletalBone> &getBones() const { return mBones; }
	void setBoneCollision(physx::PxU32 boneNr, int option);
	void setBoneSelected(physx::PxU32 boneNr, bool selected) { mBones[boneNr].selected = selected; }
	void setBoneAllowPrimitives(physx::PxU32 boneNr, bool on) { mBones[boneNr].dirtyParams |= mBones[boneNr].allowPrimitives != on; mBones[boneNr].allowPrimitives = on; }
	void setBoneInflation(physx::PxU32 boneNr, float value) { mBones[boneNr].inflateConvex = value; }
	void setBoneMinimalWeight(physx::PxU32 boneNr, float value) { mBones[boneNr].dirtyParams |= mBones[boneNr].minimalBoneWeight != value; mBones[boneNr].minimalBoneWeight = value; }
	void setBoneDirty(physx::PxU32 boneNr, bool on) { mBones[boneNr].dirtyParams = on; }
	void setBoneManualShapes(physx::PxU32 boneNr, bool on) { mBones[boneNr].manualShapes = on; }
	const std::vector<int> &getChildren() const { return mChildren; }
	const std::vector<physx::PxMat44>& getSkinningMatrices() const { return mSkinningMatrices; }
	const std::vector<physx::PxMat44>& getSkinningMatricesWorld() const { return mSkinningMatricesWorld; }
	const std::vector<SkeletalAnimation*> &getAnimations() const { if (mParent != NULL) return mParent->getAnimations(); return mAnimations; }

	void draw(physx::NxApexRenderDebug* batcher);

	void clearShapeCount(int boneIndex = -1);
	void incShapeCount(int boneIndex);
	void decShapeCount(int boneIndex);

	void setRagdoll(bool on);

	virtual bool processElement(const char *elementName, int argc, const char **argv, const char *elementData, int lineno);
	virtual bool processComment(const char *comment) // encountered a comment in the XML
	{
		PX_FORCE_PARAMETER_REFERENCE(comment);
		return true;
	}

	virtual bool processClose(const char *element,physx::PxU32 depth,bool &isError)	  // process the 'close' indicator for a previously encountered element
	{
		PX_FORCE_PARAMETER_REFERENCE(element);
		PX_FORCE_PARAMETER_REFERENCE(depth);
		isError = false;
		return true;
	}

	virtual void *  fastxml_malloc(physx::PxU32 size)
	{
		return PX_ALLOC(size);
	}
	virtual void	fastxml_free(void *mem) 
	{
		PX_FREE(mem);
	}


private:
	void init();
	void initBindPoses(int boneNr, const physx::PxVec3& scale);
	void setAnimPoseRec(int animNr, int boneNr, float time, bool lockBoneTranslation);

	void interpolateBonePose(int animNr, int boneNr, float time, physx::PxMat34Legacy &pose, physx::PxVec3 &scale);
	int  findBone(const std::string& name);
	void setupConnectivity();

	// skeleton
	std::vector<SkeletalBone> mBones;
	std::vector<physx::PxMat44> mSkinningMatrices;
	std::vector<physx::PxMat44> mSkinningMatricesWorld;
	std::vector<int> mChildren;

	// animation
	std::vector<SkeletalAnimation*> mAnimations;
	std::vector<BoneKeyFrame> mKeyFrames;

	const SkeletalAnim* mParent;

	bool ragdollMode;
};

} // namespace Samples

#endif
