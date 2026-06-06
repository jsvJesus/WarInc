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
#include "AutoGeometry.h"
#include "MeshImport.h"
#include "SkeletalAnim.h"
#include "TriangleMesh.h"
#include "PsFile.h"
#include "NxFromPx.h"
#include "PsMathUtils.h"
#include "PxFileBuffer.h"
#include <NxApexRenderDebug.h>

namespace Samples
{

// -------------------------------------------------------------------
void SkeletalBone::clear()
{
	name = "";
	id = -1;
	pose.id();
	bindWorldPose.id();
	invBindWorldPose.id();
	currentWorldPose.id();

	scale.set(1.0f, 1.0f, 1.0f);
	parent = -1;
	firstChild = -1;
	numChildren = 0;
	firstVertex = -1;
	boneOption = 0;
	inflateConvex = 0.0f;
	minimalBoneWeight = 0.4f;
	numShapes = 0;
	selected = false;
	allowPrimitives = true;
	dirtyParams = false;
	manualShapes = false;
}

// -------------------------------------------------------------------
void BoneKeyFrame::clear() 
{
	relPose.id();
	time = 0.0f;
	scale.set(1.0f, 1.0f, 1.0f);
}

// -------------------------------------------------------------------
void BoneTrack::clear()
{
	firstFrame = -1;
	numFrames = 0;
}

// -------------------------------------------------------------------
void SkeletalAnimation::clear()
{
	name = "";
	mBoneTracks.clear();
	minTime = 0.0f;
	maxTime = 0.0f;
};

// -------------------------------------------------------------------
SkeletalAnim::SkeletalAnim()
{
	clear();
}

// -------------------------------------------------------------------
SkeletalAnim::~SkeletalAnim()
{
	clear();
}

// -------------------------------------------------------------------
void SkeletalAnim::clear()
{
	ragdollMode = false;

	mBones.clear();
	mBones.resize(0);

	for (int i = 0; i < (int)mAnimations.size(); i++)
		delete mAnimations[i];

	mAnimations.clear();
	mAnimations.resize(0);

	mKeyFrames.clear();
	mKeyFrames.resize(0);

	mSkinningMatrices.clear();
	mSkinningMatrices.resize(0);
	mSkinningMatricesWorld.clear();
	mSkinningMatricesWorld.resize(0);

	mParent = NULL;
}

// -------------------------------------------------------------------
int SkeletalAnim::findBone(const std::string& name)
{
	for (int i = 0; i < (int)mBones.size(); i++) {
		if (mBones[i].name == name)
			return i;
	}
	return -1;
}

// -------------------------------------------------------------------
void SkeletalAnim::interpolateBonePose(int animNr, int boneNr, float time, physx::PxMat34Legacy &pose, physx::PxVec3 &scale)
{
	// the default
	pose.id();	
	scale.set(1.0f, 1.0f, 1.0f);

	const std::vector<SkeletalAnimation*>& animations = mParent == NULL ? mAnimations : mParent->mAnimations;
	const std::vector<BoneKeyFrame>& keyFrames = mParent == NULL ? mKeyFrames : mParent->mKeyFrames;

	if (animNr < 0 || animNr >= (int)animations.size())
		return;
	if (boneNr < 0 || boneNr >= (int)animations[animNr]->mBoneTracks.size())
		return;

	BoneTrack &t = animations[animNr]->mBoneTracks[boneNr];
	if (t.numFrames == 0) return;

	// special cases
	int frameNr = -1;
	if (t.numFrames == 1) 
		frameNr = t.firstFrame;
	else if (time <= keyFrames[t.firstFrame].time)
		frameNr = t.firstFrame;
	else if (time >= keyFrames[t.firstFrame + t.numFrames-1].time)
		frameNr = t.firstFrame + t.numFrames-1;

	if (frameNr >= 0) {
		pose = keyFrames[frameNr].relPose;
		scale = keyFrames[frameNr].scale;
		return;
	}
	// binary search
	int l = t.firstFrame;
	int r = t.firstFrame + t.numFrames - 1;
	while (r > l+1) {
		int m = (l+r) / 2;
		if (keyFrames[m].time == time) {
			pose = keyFrames[m].relPose;
			scale = keyFrames[m].scale;
			return;
		}
		else if (keyFrames[m].time > time)
			r = m;
		else 
			l = m;
	}
	float dt = keyFrames[r].time - keyFrames[l].time;
	// avoid singular case
	if (dt == 0.0f) {
		pose = keyFrames[l].relPose;
		scale = keyFrames[l].scale;
	}

	// interpolation
	float sr = (time - keyFrames[l].time) / dt;
	float sl = 1.0f - sr;

	scale = keyFrames[l].scale * sl + keyFrames[r].scale * sr;
	pose.t = keyFrames[l].relPose.t * sl + keyFrames[r].relPose.t * sr;
	physx::PxQuat ql = physx::PxQuat(keyFrames[l].relPose.M);
	physx::PxQuat qr = physx::PxQuat(keyFrames[r].relPose.M);
	physx::PxQuat q = physx::slerp(sr, ql,qr);
	pose.M.fromQuat(q);
}

// -------------------------------------------------------------------
void SkeletalAnim::setBindPose()
{
	assert(mBones.size() == mSkinningMatrices.size());
	assert(mBones.size() == mSkinningMatricesWorld.size());
	for (physx::PxU32 i = 0; i < mBones.size(); i++)
	{
		mSkinningMatrices[i] = physx::PxMat44::identity();
		mBones[i].currentWorldPose = mBones[i].bindWorldPose;
		mSkinningMatricesWorld[i] = mBones[i].currentWorldPose;
	}
}

// -------------------------------------------------------------------
void SkeletalAnim::setAnimPose(int animNr, float time, bool lockRootbone /* = false */)
{
	if (animNr >= 0)
	{
		for (physx::PxU32 i = 0; i < mBones.size(); i++)
		{
			if (mBones[i].parent < 0)
			{
				setAnimPoseRec(animNr, i, time, lockRootbone);
			}
		}

		assert(mBones.size() == mSkinningMatrices.size());
		assert(mBones.size() == mSkinningMatricesWorld.size());
		for (physx::PxU32 i = 0; i < mBones.size(); i++)
		{
			SkeletalBone &b = mBones[i];
			mSkinningMatrices[i] = b.currentWorldPose * b.invBindWorldPose;
			mSkinningMatricesWorld[i] = b.currentWorldPose;
		}
	}
	else
	{
		for (physx::PxU32 i = 0; i < mBones.size(); i++)
		{
			mSkinningMatrices[i] = physx::PxMat44::identity();
			mSkinningMatricesWorld[i] = mBones[i].bindWorldPose;
		}
	}
}

// -------------------------------------------------------------------
void SkeletalAnim::setBoneCollision(physx::PxU32 boneNr, int option)
{
	if (mBones[boneNr].boneOption != option)
	{
		if (mBones[boneNr].boneOption == physx::BO_COLLAPSE || option == physx::BO_COLLAPSE)
		{
			// PH: Follow up the hierarchy until something is not set to collapse and mark all dirty
			int current = mBones[boneNr].parent;
			while (current != -1)
			{
				mBones[current].dirtyParams = true;
				if (mBones[current].boneOption != physx::BO_COLLAPSE)
					break;

				if (mBones[current].parent == current)
					break;

				current = mBones[current].parent;
			}
		}
		mBones[boneNr].dirtyParams = true;

		// Find all children that collapse into this bone and mark them dirty
		for (physx::PxU32 i = 0; i < mBones.size(); i++)
		{
			// See whether boneNr is one of it's parents
			bool found = false;
			int current = i;
			while (current != -1 && !found)
			{
				if (current == boneNr)
					found = true;

				if (mBones[current].boneOption != physx::BO_COLLAPSE)
					break;

				if (current == mBones[current].parent)
					break;

				current = mBones[current].parent;
			}
			if (found)
				mBones[i].dirtyParams = true;
		}
	}

	mBones[boneNr].boneOption = option;
}

// -------------------------------------------------------------------
void SkeletalAnim::setAnimPoseRec(int animNr, int boneNr, float time, bool lockBoneTranslation)
{
	SkeletalBone &b = mBones[boneNr];

	{
		physx::PxMat34Legacy keyPose;
		physx::PxVec3 keyScale;
		interpolateBonePose(animNr, boneNr, time, keyPose, keyScale);

		// todo: consider scale
		physx::PxMat34Legacy combinedPose;
		combinedPose.t = b.pose.t;
		if (!lockBoneTranslation)
			combinedPose.t += keyPose.t;

		combinedPose.M = b.pose.M * keyPose.M;
		if (b.parent < 0)
		{
			b.currentWorldPose = combinedPose;
		}
		else
		{
			b.currentWorldPose = mBones[b.parent].currentWorldPose * combinedPose;
		}
	}

	const int* children = mParent == NULL ? &mChildren[0] : &mParent->mChildren[0];
	for (int i = b.firstChild; i < b.firstChild + b.numChildren; i++)
	{
		setAnimPoseRec(animNr, children[i], time, false);
	}
}

// -------------------------------------------------------------------
bool SkeletalAnim::loadFromMeshImport(physx::MeshSystemContainer *msc, std::string& error)
{
	bool ret = false;

	mBones.clear();
	mSkinningMatrices.clear();
	mSkinningMatricesWorld.clear();
	for (unsigned int i=0; i<mAnimations.size(); i++)
	{
		SkeletalAnimation *a = mAnimations[i];
		delete a;
	}
	mAnimations.clear();

	if ( msc )
	{
		physx::MeshSystem *ms = physx::gMeshImport->getMeshSystem(msc);
		if ( ms->mSkeletonCount )
		{
			physx::MeshSkeleton *sk = ms->mSkeletons[0];

			for (int i=0; i<sk->mBoneCount; i++)
			{
				physx::MeshBone &b = sk->mBones[i];
				SkeletalBone sb;
				sb.clear();
				sb.name = b.mName;
				sb.id   = i;
				sb.parent = b.mParentIndex;
				sb.firstChild = 0;
				sb.numChildren = 0;

				physx::PxQuat q;
				physx::PxQuatFromArray( q, b.mOrientation );
				sb.pose.M.fromQuat(q);
				physx::PxVec3FromArray( sb.pose.t, b.mPosition );
				physx::PxVec3FromArray( sb.scale, b.mScale );

				for (physx::PxU32 bi = 0; bi < mBones.size(); bi++)
				{
					if (mBones[bi].name == b.mName)
					{
						if (error.empty())
							error = "Duplicated Bone Names, rename one:\n";

						error.append(b.mName);
						error.append("\n");
					}
				}

				mBones.push_back(sb);
			}
		}

		if ( ms->mAnimationCount )
		{
			for (unsigned int i=0; i<ms->mAnimationCount; i++)
			{
				physx::MeshAnimation *a = ms->mAnimations[i];
				SkeletalAnimation *anim = new SkeletalAnimation;
				anim->clear();
				anim->name = a->mName;

				int numBones = (physx::PxU32)mBones.size();
				anim->mBoneTracks.resize(numBones);
				for (int j=0; j<numBones; j++)
					anim->mBoneTracks[j].clear();

				for (int j=0; j<a->mTrackCount; j++)
				{
					physx::MeshAnimTrack *track = a->mTracks[j];
					std::string boneName = track->mName;
					int boneNr = findBone(boneName);
					if ( boneNr >= 0 && boneNr < numBones )
					{
						anim->mBoneTracks[boneNr].firstFrame = (int)(mKeyFrames.size());
						anim->mBoneTracks[boneNr].numFrames  = track->mFrameCount;

						physx::PxMat34Legacy parent = mBones[boneNr].pose;
						physx::PxMat33Legacy pinverse;
						parent.M.getInverse(pinverse);

						float ftime = 0;

						for (int k=0; k<track->mFrameCount; k++)
						{
							physx::MeshAnimPose &p = track->mPose[k];
							BoneKeyFrame frame;
							frame.clear();

							physx::PxMat34Legacy mat;

							physx::PxQuat q;
							physx::PxQuatFromArray( q, p.mQuat );

							mat.M.fromQuat(q);
							physx::PxVec3FromArray( mat.t,p.mPos );

							frame.time = ftime;
							physx::PxVec3FromArray( frame.scale, p.mScale );

							frame.relPose.t = mat.t - parent.t;
							frame.relPose.M.multiply(pinverse,mat.M);

							mKeyFrames.push_back(frame);

							// eazymesh samples at 60 Hz, not 1s
							ftime+=track->mDtime / 200.f;
						}
					}
					else
					{
						assert(0);
					}

				}

				mAnimations.push_back(anim);
			}
		}

		ret = true;
		physx::PxMat34Legacy matId; matId.id();
		mSkinningMatrices.resize((physx::PxU32)mBones.size(), matId);
		mSkinningMatricesWorld.resize((physx::PxU32)mBones.size(), matId);
		init();
	}

	return ret;
}

// -------------------------------------------------------------------
bool SkeletalAnim::initFrom(physx::apex::NxRenderMeshAssetAuthoring& mesh)
{
	assert(mesh.getPartCount() == 1);

	physx::PxU32 numBones = 0;
	for (physx::PxU32 submeshIndex = 0; submeshIndex < mesh.getSubmeshCount(); submeshIndex++)
	{
		const physx::NxVertexBuffer& vb = mesh.getSubmesh(submeshIndex).getVertexBuffer();
		const physx::NxVertexFormat& vf = vb.getFormat();
		physx::PxU32 bufferIndex = vf.getBufferIndexFromID( vf.getSemanticID( physx::apex::NxRenderVertexSemantic::BONE_INDEX ) );

		physx::apex::NxRenderDataFormat::Enum format;
		const physx::PxU16* boneIndices = (const physx::PxU16*)vb.getBufferAndFormat( format, bufferIndex );

		unsigned int numBonesPerVertex = 0;
		switch (format)
		{
		case physx::apex::NxRenderDataFormat::USHORT1: numBonesPerVertex = 1; break;
		case physx::apex::NxRenderDataFormat::USHORT2: numBonesPerVertex = 2; break;
		case physx::apex::NxRenderDataFormat::USHORT3: numBonesPerVertex = 3; break;
		case physx::apex::NxRenderDataFormat::USHORT4: numBonesPerVertex = 4; break;
		}

		if (boneIndices == NULL || numBonesPerVertex == 0)
		{
			return false;
		}

		const unsigned int numElements = numBonesPerVertex * vb.getVertexCount();

		for (unsigned int i = 0; i < numElements; i++)
		{
			numBones = std::max(numBones, boneIndices[i]+1u);
		}
	}

	SkeletalBone initBone;
	initBone.clear();
	mBones.resize(numBones, initBone);

	for (unsigned int i = 0; i < numBones; i++)
	{
		mBones[i].id = i;
	}

	mSkinningMatrices.resize(numBones);
	mSkinningMatricesWorld.resize(numBones);

	init();

	return numBones > 0;
}


// -------------------------------------------------------------------
bool SkeletalAnim::loadFromXML(const std::string& xmlFile, std::string& error)
{
	clear();

	physx::PxFileBuffer fb(xmlFile.c_str(),physx::PxFileBuf::OPEN_READ_ONLY);
	if (!fb.isOpen())
	{
		return false;
	}

	FAST_XML::FastXml* fastXml = FAST_XML::createFastXml(this);
	fastXml->processXml(fb);

	int errorLineNumber = -1;
	const char* xmlError = fastXml->getError(errorLineNumber);
	if (xmlError != NULL)
	{
		char temp[1024];
		physx::string::sprintf_s(temp, 1024, "Xml parse error in %s on line %d:\n\n%s", xmlFile.c_str(), errorLineNumber, xmlError);
		error = temp;
		return false;
	}

	fastXml->release();

	physx::PxMat34Legacy matId; matId.id();
	mSkinningMatrices.resize((physx::PxU32)mBones.size(), matId);
	mSkinningMatricesWorld.resize((physx::PxU32)mBones.size(), matId);

	init();
	return true;
}

// -------------------------------------------------------------------
bool SkeletalAnim::loadFromParent(const SkeletalAnim* parent)
{
	if (parent == NULL)
		return false;

	mParent = parent;

	mBones.resize(mParent->mBones.size());
	physx::PxMat34Legacy matId; matId.id();
	mSkinningMatrices.resize((physx::PxU32)mBones.size(), matId);
	mSkinningMatricesWorld.resize((physx::PxU32)mBones.size(), matId);
	for (physx::PxU32 i = 0; i < mBones.size(); i++)
	{
		mBones[i] = mParent->mBones[i];
	}

	return true;
}

// -------------------------------------------------------------------
bool SkeletalAnim::saveToXML(const std::string& xmlFile) const
{
	FILE *f;
	if (physx::fopen_s(&f, xmlFile.c_str(), "w") != 0)
		return false;

	fprintf(f, "<skeleton>\n\n");

	fprintf(f, "  <bones>\n");
	for (int i = 0; i < (int)mBones.size(); i++) {
		const SkeletalBone &bone = mBones[i];

		physx::PxQuat q(bone.pose.M);
		float angle;
		physx::PxVec3 axis;
		q.toRadiansAndUnitAxis(angle, axis);
		angle = q.getAngle();	

		fprintf(f, "    <bone id = \"%i\" name = \"%s\">\n", bone.id, bone.name.c_str());
		fprintf(f, "      <position x=\"%f\" y=\"%f\" z=\"%f\" />\n", bone.pose.t.x, bone.pose.t.y, bone.pose.t.z);
		fprintf(f, "      <rotation angle=\"%f\">\n", angle);
		fprintf(f, "        <axis x=\"%f\" y=\"%f\" z=\"%f\" />\n", axis.x, axis.y, axis.z);
		fprintf(f, "      </rotation>\n");
		fprintf(f, "      <scale x=\"%f\" y=\"%f\" z=\"%f\" />\n", 1.0f, 1.0f, 1.0f);		
//		dont' use bone.scale.x, bone.scale.y, bone.scale.z because the length is baked into the bones
		fprintf(f, "    </bone>\n");
	}
	fprintf(f, "  </bones>\n\n");

	fprintf(f, "  <bonehierarchy>\n");
	for (int i = 0; i < (int)mBones.size(); i++) {
		const SkeletalBone &bone = mBones[i];
		if (bone.parent < 0)
			continue;
		fprintf(f, "    <boneparent bone=\"%s\" parent=\"%s\" />\n", bone.name.c_str(), mBones[bone.parent].name.c_str());
	}
	fprintf(f, "  </bonehierarchy>\n\n");

	fprintf(f, "  <animations>\n");
	for (int i = 0; i < (int)mAnimations.size(); i++) {
		const SkeletalAnimation *anim = mAnimations[i];

		fprintf(f, "    <animation name = \"%s\" length=\"%f\">\n", anim->name.c_str(), anim->maxTime);
		fprintf(f, "      <tracks>\n");

		for (int j = 0; j < (int)anim->mBoneTracks.size(); j++) {
			const BoneTrack &track = anim->mBoneTracks[j];
			if (track.numFrames == 0)
				continue;

			fprintf(f, "        <track bone = \"%s\">\n", mBones[j].name.c_str());
			fprintf(f, "          <keyframes>\n");

			for (int k = track.firstFrame; k < track.firstFrame + track.numFrames; k++) {
				const BoneKeyFrame &frame = mKeyFrames[k];
				physx::PxQuat q(frame.relPose.M);
				float angle;
				physx::PxVec3 axis;
				q.toRadiansAndUnitAxis(angle, axis);
				angle = q.getAngle();

				fprintf(f, "            <keyframe time = \"%f\">\n", frame.time);
				fprintf(f, "              <translate x=\"%f\" y=\"%f\" z=\"%f\" />\n", frame.relPose.t.x, frame.relPose.t.y, frame.relPose.t.z);
				fprintf(f, "              <rotate angle=\"%f\">\n", angle);
				fprintf(f, "                <axis x=\"%f\" y=\"%f\" z=\"%f\" />\n", axis.x, axis.y, axis.z);
				fprintf(f, "              </rotate>\n");
				fprintf(f, "              <scale x=\"%f\" y=\"%f\" z=\"%f\" />\n", frame.scale.x, frame.scale.y, frame.scale.z);
				fprintf(f, "            </keyframe>\n");
			}
			fprintf(f, "          </keyframes>\n");
			fprintf(f, "        </track>\n");
		}
		fprintf(f, "      </tracks>\n");
		fprintf(f, "    </animation>\n");
	}
	fprintf(f, "  </animations>\n");
	fprintf(f, "</skeleton>\n\n");
	  
	fclose(f);


	return true;
}

// -------------------------------------------------------------------
void SkeletalAnim::init()
{
	setupConnectivity();

	// init bind poses
	physx::PxVec3 oneOneOne(1.0f, 1.0f, 1.0f);
	for (int i = 0; i < (int)mBones.size(); i++)
	{
		if (mBones[i].parent < 0)
			initBindPoses(i, oneOneOne);

		// collapse finger and toes
		if (
			mBones[i].name.find("finger") != std::string::npos ||
			mBones[i].name.find("Finger") != std::string::npos ||
			mBones[i].name.find("FINGER") != std::string::npos ||
			mBones[i].name.find("toe") != std::string::npos ||
			mBones[i].name.find("Toe") != std::string::npos ||
			mBones[i].name.find("TOE") != std::string::npos)
		{
			mBones[i].boneOption = 2; // this is collapse
		}


	}

	assert(mBones.size() == mSkinningMatrices.size());
	assert(mBones.size() == mSkinningMatricesWorld.size());
	for (physx::PxU32 i = 0; i < mBones.size(); i++)
	{
		SkeletalBone &b = mBones[i];
		b.bindWorldPose.getInverseRT(b.invBindWorldPose);
		b.currentWorldPose = mBones[i].bindWorldPose;
		mSkinningMatrices[i] = physx::PxMat44::identity();
		mSkinningMatricesWorld[i] = b.currentWorldPose;
	}

	// init time interval of animations
	for (int i = 0; i < (int)mAnimations.size(); i++) {
		SkeletalAnimation *a = mAnimations[i];
		bool first = true;
		for (int j = 0; j < (int)a->mBoneTracks.size(); j++) {
			BoneTrack &b = a->mBoneTracks[j];
			for (int k = b.firstFrame; k < b.firstFrame + b.numFrames; k++) {
				float time = mKeyFrames[k].time;
				if (first) {
					a->minTime = time;
					a->maxTime = time;
					first = false;
				}
				else {
					if (time < a->minTime) a->minTime = time;
					if (time > a->maxTime) a->maxTime = time;
				}
			}
		}
	}
}

// -------------------------------------------------------------------
void SkeletalAnim::initBindPoses(int boneNr, const physx::PxVec3& scale)
{
	SkeletalBone &b = mBones[boneNr];
	b.pose.t = b.pose.t.arrayMultiply(scale);

	physx::PxVec3 newScale = scale.arrayMultiply(b.scale);

	if (b.parent < 0)
	{
		b.bindWorldPose = b.pose;
	}
	else
	{
		b.bindWorldPose = mBones[b.parent].bindWorldPose * b.pose;
	}

	for (int i = b.firstChild; i < b.firstChild + b.numChildren; i++)
	{
		initBindPoses(mChildren[i], newScale);
	}
}

// -------------------------------------------------------------------
void SkeletalAnim::setupConnectivity()
{
	int i;
	int numBones = (physx::PxU32)mBones.size();
	for (i = 0; i < numBones; i++) {
		SkeletalBone &b = mBones[i];
		if (b.parent >= 0) mBones[b.parent].numChildren++;
	}
	int first = 0;
	for (i = 0; i < numBones; i++) {
		mBones[i].firstChild = first;
		first += mBones[i].numChildren;
	}
	mChildren.resize(first);
	for (i = 0; i < numBones; i++) {
		if (mBones[i].parent < 0) continue;
		SkeletalBone &p = mBones[mBones[i].parent];
		mChildren[p.firstChild++] = i;
	}
	for (i = 0; i < numBones; i++) {
		mBones[i].firstChild -= mBones[i].numChildren;
	}
}

// -------------------------------------------------------------------
void SkeletalAnim::draw(physx::NxApexRenderDebug* batcher)
{
	assert(batcher != NULL);
	if (batcher == NULL)
		return;

	const physx::PxU32 colorWhite = batcher->getDebugColor(physx::DebugColors::White);
	const physx::PxU32 colorBlack = batcher->getDebugColor(physx::DebugColors::Black);
	for (physx::PxU32 i = 0; i < mBones.size(); i++)
	{
		SkeletalBone& bone = mBones[i];

		physx::PxU32 color = bone.selected ? colorWhite : colorBlack;
		batcher->setCurrentColor(color);

		if (bone.parent >= 0 /*&& mBones[bone.parent].parent >= 0*/)
		{
			SkeletalBone& parent = mBones[bone.parent];

			batcher->debugLine(bone.currentWorldPose.t, parent.currentWorldPose.t);
		}
	}
}

// -------------------------------------------------------------------
void SkeletalAnim::copyFrom(const SkeletalAnim &anim)
{
	clear();

	mBones.resize(anim.mBones.size());
	for (int i = 0; i < (int)anim.mBones.size(); i++)
		mBones[i] = anim.mBones[i];

	mSkinningMatrices.resize(anim.mSkinningMatrices.size());
	for (int i = 0; i < (int)anim.mSkinningMatrices.size(); i++)
		mSkinningMatrices[i] = anim.mSkinningMatrices[i];

	mSkinningMatricesWorld.resize(anim.mSkinningMatricesWorld.size());
	for (int i = 0; i < (int)anim.mSkinningMatricesWorld.size(); i++)
		mSkinningMatricesWorld[i] = anim.mSkinningMatricesWorld[i];

	mChildren.resize(anim.mChildren.size());
	for (int i = 0; i < (int)anim.mChildren.size(); i++)
		mChildren[i] = anim.mChildren[i];

	for (int i = 0; i < (int)anim.mAnimations.size(); i++) {
		SkeletalAnimation *a = anim.mAnimations[i];
		SkeletalAnimation *na = new SkeletalAnimation();
		na->minTime = a->minTime;
		na->maxTime = a->maxTime;
		na->name = a->name;
		na->mBoneTracks.resize(a->mBoneTracks.size());
		for (int j = 0; j < (int)a->mBoneTracks.size(); j++)
			na->mBoneTracks[j] = a->mBoneTracks[j];
		mAnimations.push_back(na);
	}

	mKeyFrames.resize(anim.mKeyFrames.size());
	for (int i = 0; i < (int)anim.mKeyFrames.size(); i++)
		mKeyFrames[i] = anim.mKeyFrames[i];
}


// -------------------------------------------------------------------
void SkeletalAnim::clearShapeCount(int boneIndex)
{
	if (boneIndex < 0)
	{
		for (physx::PxU32 i = 0; i < mBones.size(); i++)
		{
			mBones[i].numShapes = 0;
		}
	}
	else
	{
		assert((physx::PxU32)boneIndex < mBones.size());
		mBones[boneIndex].numShapes = 0;
	}
}

// -------------------------------------------------------------------
void SkeletalAnim::incShapeCount(int boneIndex)
{
	if (boneIndex >= 0 && (physx::PxU32)boneIndex < mBones.size())
		mBones[boneIndex].numShapes++;
}

// -------------------------------------------------------------------
void SkeletalAnim::decShapeCount(int boneIndex)
{
	if (boneIndex >= 0 && (physx::PxU32)boneIndex < mBones.size())
	{
		assert(mBones[boneIndex].numShapes > 0);
		mBones[boneIndex].numShapes--;
	}
}
// -------------------------------------------------------------------
bool SkeletalAnim::processElement(const char *elementName, int argc, const char **argv, const char *elementData, int lineno)
{
	static int activeBoneTrack = -1;
	static BoneKeyFrame* activeKeyFrame;
	static bool isAnimation = false;

	if (strcmp(elementName, "skeleton") == 0)
	{
		// ok, a start
	}
	else if (strcmp(elementName, "bones") == 0)
	{
		// the list of bones
	}
	else if (strcmp(elementName, "bone") == 0)
	{
		assert(argc == 4);
		assert(strcmp(argv[0], "id") == 0);
		assert(strcmp(argv[2], "name") == 0);
		SkeletalBone bone;
		bone.clear();
		bone.id = atoi(argv[1]);
		bone.name = argv[3];
		mBones.push_back(bone);
	}
	else if (strcmp(elementName, "position") == 0)
	{
		assert(argc == 6);
		assert(strcmp(argv[0], "x") == 0);
		assert(strcmp(argv[2], "y") == 0);
		assert(strcmp(argv[4], "z") == 0);
		physx::PxVec3 pos;
		pos.x = (physx::PxF32)atof(argv[1]);
		pos.y = (physx::PxF32)atof(argv[3]);
		pos.z = (physx::PxF32)atof(argv[5]);
		mBones.back().pose.t = pos;
	}
	else if (strcmp(elementName, "rotation") == 0)
	{
		assert(argc == 2);
		assert(strcmp(argv[0], "angle") == 0);
		mBones.back().pose.M(0, 0) = (physx::PxF32)atof(argv[1]);
		isAnimation = false;
	}
	else if (strcmp(elementName, "axis") == 0 && !isAnimation)
	{
		assert(argc == 6);
		assert(strcmp(argv[0], "x") == 0);
		assert(strcmp(argv[2], "y") == 0);
		assert(strcmp(argv[4], "z") == 0);
		physx::PxVec3 axis;
		axis.x = (physx::PxF32)atof(argv[1]);
		axis.y = (physx::PxF32)atof(argv[3]);
		axis.z = (physx::PxF32)atof(argv[5]);
		physx::PxF32 angle = mBones.back().pose.M(0, 0);
		physx::PxQuat quat(angle, axis);
		mBones.back().pose.M.fromQuat(quat);
	}
	else if (strcmp(elementName, "scale") == 0)
	{
		assert(argc == 6);
		assert(strcmp(argv[0], "x") == 0);
		assert(strcmp(argv[2], "y") == 0);
		assert(strcmp(argv[4], "z") == 0);
		physx::PxVec3 scale;
		scale.x = (physx::PxF32)atof(argv[1]);
		scale.y = (physx::PxF32)atof(argv[3]);
		scale.z = (physx::PxF32)atof(argv[5]);
		mBones.back().scale = scale;
	}
	else if (strcmp(elementName, "bonehierarchy") == 0)
	{
		// ok
	}
	else if (strcmp(elementName, "boneparent") == 0)
	{
		assert(argc == 4);
		assert(strcmp(argv[0], "bone") == 0);
		assert(strcmp(argv[2], "parent") == 0);
		int child = findBone(argv[1]);
		int parent = findBone(argv[3]);
		if (child >= 0 && child < (int)mBones.size() && parent >= 0 && parent < (int)mBones.size())
			mBones[child].parent = parent;
	}
	else if (strcmp(elementName, "animations") == 0)
	{
		// ok
	}
	else if (strcmp(elementName, "animation") == 0)
	{
		assert(argc == 4);
		assert(strcmp(argv[0], "name") == 0);

		SkeletalAnimation* anim = new SkeletalAnimation;
		anim->clear();
		anim->name = argv[1];
		anim->mBoneTracks.resize((physx::PxU32)mBones.size());

		mAnimations.push_back(anim);
	}
	else if (strcmp(elementName, "tracks") == 0)
	{
		// ok
	}
	else if (strcmp(elementName, "track") == 0)
	{
		assert(argc == 2);
		assert(strcmp(argv[0], "bone") == 0);
		activeBoneTrack = findBone(argv[1]);
		if (activeBoneTrack >= 0 && activeBoneTrack < (int)mBones.size())
		{
			mAnimations.back()->mBoneTracks[activeBoneTrack].firstFrame = (int)(mKeyFrames.size());
			mAnimations.back()->mBoneTracks[activeBoneTrack].numFrames = 0;
		}
	}
	else if (strcmp(elementName, "keyframes") == 0)
	{
		// ok
	}
	else if (strcmp(elementName, "keyframe") == 0)
	{
		assert(argc == 2);
		assert(strcmp(argv[0], "time") == 0);

		mAnimations.back()->mBoneTracks[activeBoneTrack].numFrames++;
		
		mKeyFrames.push_back(BoneKeyFrame());
		activeKeyFrame = &mKeyFrames.back();
		activeKeyFrame->clear();
		activeKeyFrame->time = (float)atof(argv[1]);
	}
	else if (strcmp(elementName, "translate") == 0)
	{
		assert(argc == 6);
		assert(strcmp(argv[0], "x") == 0);
		assert(strcmp(argv[2], "y") == 0);
		assert(strcmp(argv[4], "z") == 0);
		activeKeyFrame->relPose.t.x = (physx::PxF32)atof(argv[1]);
		activeKeyFrame->relPose.t.y = (physx::PxF32)atof(argv[3]);
		activeKeyFrame->relPose.t.z = (physx::PxF32)atof(argv[5]);
	}
	else if (strcmp(elementName, "rotate") == 0)
	{
		assert(argc == 2);
		assert(strcmp(argv[0], "angle") == 0);
		activeKeyFrame->relPose.M(0, 0) = (physx::PxF32)atof(argv[1]);
		isAnimation = true;
	}
	else if (strcmp(elementName, "axis") == 0 && isAnimation)
	{
		assert(argc == 6);
		assert(strcmp(argv[0], "x") == 0);
		assert(strcmp(argv[2], "y") == 0);
		assert(strcmp(argv[4], "z") == 0);
		physx::PxVec3 axis;
		axis.x = (physx::PxF32)atof(argv[1]);
		axis.y = (physx::PxF32)atof(argv[3]);
		axis.z = (physx::PxF32)atof(argv[5]);
		axis.normalize();
		physx::PxF32 angle = activeKeyFrame->relPose.M(0, 0);
		physx::PxQuat quat(angle, axis);
		activeKeyFrame->relPose.M.fromQuat(quat);
	}
	else if (strcmp(elementName, "scale") == 0)
	{
		assert(argc == 6);
		assert(strcmp(argv[0], "x") == 0);
		assert(strcmp(argv[2], "y") == 0);
		assert(strcmp(argv[4], "z") == 0);
		activeKeyFrame->scale.x = (physx::PxF32)atof(argv[1]);
		activeKeyFrame->scale.y = (physx::PxF32)atof(argv[3]);
		activeKeyFrame->scale.z = (physx::PxF32)atof(argv[5]);
	}
	else
	{
		// always break here, at least in debug mode
		assert(0);
	}

	return true;
}

} // namespace Samples
