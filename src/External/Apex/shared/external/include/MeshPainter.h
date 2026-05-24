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
#ifndef MESH_PAINTER_H
#define MESH_PAINTER_H

#include "PsShare.h"
#include "PxVec3.h"

#ifdef PX_WINDOWS

#include <vector>

namespace physx
{
	namespace apex
	{
		class NxClothingPhysicalMesh;
		class NxApexRenderDebug;
	}
}

namespace SharedTools
{
	struct DistTriPair;
	struct PaintFloatBuffer;

	class MeshPainter {
	public:
		MeshPainter();
		~MeshPainter();
		void clear();

		void initFrom(const physx::NxClothingPhysicalMesh* mesh);
		void initFrom(const physx::PxVec3* vertices, int numVertices, int vertexStride, const physx::PxU32* indices, int numIndices, int indexStride);

		void clearIndexBufferRange();
		void addIndexBufferRange(physx::PxU32 start, physx::PxU32 end);

		void allocateFloatBuffer(physx::PxU32 id);

		void setFloatBuffer(physx::PxU32 id, float *buffer, int stride);

		float *getFloatBuffer(physx::PxU32 id);

		const std::vector<physx::PxVec3> getVertices() const { return mVertices; }
		const std::vector<physx::PxU32>  getIndices() const { return mIndices; }

		void changeRadius(float paintRadius);
		void setRayAndRadius(const physx::PxVec3& rayOrig, const physx::PxVec3& rayDir, float paintRadius, int brushMode, float falloffExponent, float scaledTargetValue, float targetColor);
		bool raycastHit() { return !mLastRaycastNormal.isZero(); }

		void paintFloat(physx::PxU32 id, float min, float max, float target) const;

		void smoothFloat(physx::PxU32 id, float smoothingFactor, physx::PxU32 numIterations) const;
		void smoothFloatFast(physx::PxU32 id, physx::PxU32 numIterations) const;

		void drawBrush(physx::apex::NxApexRenderDebug* batcher) const;

	private:
		PaintFloatBuffer& MeshPainter::getInternalFloatBuffer(physx::PxU32 id);

		void complete();
		void computeNormals();
		void createNeighborInfo();
		bool rayCast(int &triNr, float &t) const;
		bool rayTriangleIntersection(const physx::PxVec3 &orig, const physx::PxVec3 &dir, const physx::PxVec3 &a,
			const physx::PxVec3 &b, const physx::PxVec3 &c,float &t, float &u, float &v) const;

		void computeSiblingInfo(float distanceThreshold);

		physx::PxVec3 getTriangleCenter(int triNr) const;
		physx::PxVec3 getTriangleNormal(int triNr) const;
		void collectTriangles() const;
		bool isValidRange(int vertexNumber) const;

		std::vector<physx::PxVec3> mVertices;
		std::vector<bool> mVerticesDisabled;
		std::vector<physx::PxU32> mIndices;
		struct IndexBufferRange
		{
			bool isOverlapping(const IndexBufferRange& other) const;
			physx::PxU32 start;
			physx::PxU32 end;
		};
		std::vector<IndexBufferRange> mIndexRanges;
		std::vector<int> mNeighbors;
		mutable std::vector<int> mTriMarks;
		mutable std::vector<DistTriPair> mCollectedTriangles;
		mutable std::vector<physx::PxU32> mCollectedVertices;
		mutable std::vector<float> mCollectedVerticesFloats;
		mutable std::vector<physx::PxU32> mSmoothingCollectedIndices;

		std::vector<physx::PxVec3> mNormals;
		std::vector<physx::PxVec3> mTetraNormals;

		std::vector<PaintFloatBuffer> mFloatBuffers;

		mutable int mCurrentMark;

		physx::PxVec3 mRayOrig, mRayDir;
		float mPaintRadius;
		mutable float mTargetValue;
		float mScaledTargetValue;
		int mBrushMode;
		float mFalloffExponent;
		float mBrushColor;

		mutable physx::PxI32 mLastTriangle;
		mutable physx::PxVec3 mLastRaycastPos;
		mutable physx::PxVec3 mLastRaycastNormal;

		std::vector<physx::PxI32> mFirstSibling;
		std::vector<physx::PxI32> mSiblings;
	};



	struct DistTriPair
	{
		void set(int triNr, float dist) {
			this->triNr = triNr;
			this->dist = dist;
		}
		bool operator < (const DistTriPair &f) const {
			return dist < f.dist;
		}
		int triNr;
		float dist;
	};



	struct PaintFloatBuffer
	{
		float& operator[] (int i)  const
		{ 
			return *(float*)((physx::PxU8*)buffer + i*stride);
		}
		physx::PxU32 id;
		float *buffer;
		int stride;
		bool allocated;
	};

} // namespace SharedTools


#endif // PX_WINDOWS

#endif
