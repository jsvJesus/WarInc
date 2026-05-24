// This code contains NVIDIA Confidential Information and is disclosed 
// under the Mutual Non-Disclosure Agreement.
//
// Notice
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES 
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO 
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT, 
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable. 
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such 
// information or for any infringement of patents or other rights of third parties that may 
// result from its use. No license is granted by implication or otherwise under any patent 
// or patent rights of NVIDIA Corporation. Details are subject to change without notice. 
// This code supersedes and replaces all information previously supplied. 
// NVIDIA Corporation products are not authorized for use as critical 
// components in life support devices or systems without express written approval of 
// NVIDIA Corporation.
//
// Copyright (c) 2009-2011 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2002-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2006 NovodeX. All rights reserved.   

#ifndef PX_FOUNDATION_PSRENDEROUTPUT_H
#define PX_FOUNDATION_PSRENDEROUTPUT_H

#include "PsRenderBuffer.h"
#include "PsArray.h"
#include "PsNoCopy.h"
#include "PxMat44.h"
#include "PsUserAllocated.h"

namespace physx
{
namespace shdfnd2
{
	struct DebugText;

	/**
	Output stream to fill RenderBuffer
	*/
	class RenderOutput 
	{
	public:

		RenderOutput(RenderBuffer& buffer) 
			: mTransform(PxMat44::getIdentity()), mBuffer(buffer) 
		{}

		enum Primitive {
			POINTS,
			LINES,
			LINESTRIP,
			TRIANGLES,
			TRIANGLESTRIP,
			TEXT
		};

		RenderOutput& operator<<(Primitive prim);
		RenderOutput& operator<<(PxU32 color);
		RenderOutput& operator<<(const PxMat44& transform);
		RenderOutput& operator<<(const PxTransform&);

		RenderOutput& operator<<(PxVec3 vertex);
		RenderOutput& operator<<(const DebugText& text);

	private:

		RenderOutput& operator=(const RenderOutput&);

		Primitive mPrim;
		PxU32 mColor;
		PxVec3 mVertex0, mVertex1;
		PxU32 mVertexCount;
		PxMat44 mTransform;
		RenderBuffer& mBuffer;
	};

	/** debug render helper types */
	struct DebugText 
	{
		DebugText(const PxVec3& position, PxReal size, const char* string, ...); 
		static const int sBufferSize = 1008; // sizeof(DebugText)==1kB
		char buffer[sBufferSize]; 
		PxVec3 position;
		PxReal size;
	};

	struct DebugBox 
	{
		explicit DebugBox(const PxVec3& extents, bool wireframe = true) 
		: minimum(-extents), maximum(extents), wireframe(wireframe) {}

		explicit DebugBox(const PxVec3& pos, const PxVec3& extents, bool wireframe = true) 
		: minimum(pos-extents), maximum(pos+extents), wireframe(wireframe) {}

		explicit DebugBox(const PxBounds3& bounds, bool wireframe = true)
		: minimum(bounds.minimum), maximum(bounds.maximum), wireframe(wireframe) {}

		PxVec3 minimum, maximum;
		bool wireframe;
	};
	RenderOutput& operator<<(RenderOutput& out, const DebugBox& box);

	struct DebugArrow 
	{
		DebugArrow(const PxVec3& pos, const PxVec3& vec) 
		: base(pos), tip(pos+vec), headLength(vec.magnitude()*0.15f) {}

		DebugArrow(const PxVec3& pos, const PxVec3& vec, PxReal headLength) 
		: base(pos), tip(pos+vec), headLength(headLength) {}

		PxVec3 base, tip;
		PxReal headLength;
	};
	RenderOutput& operator<<(RenderOutput& out, const DebugArrow& arrow);

	struct DebugBasis
	{
		DebugBasis(const PxVec3& extends, PxU32 colorX = PxDebugColor::eARGB_RED, 
			PxU32 colorY = PxDebugColor::eARGB_GREEN, PxU32 colorZ = PxDebugColor::eARGB_BLUE) 
		: extends(extends), colorX(colorX), colorY(colorY), colorZ(colorZ) {}
		PxVec3 extends;
		PxU32 colorX, colorY, colorZ;
	};
	RenderOutput& operator<<(RenderOutput& out, const DebugBasis& basis);

	struct DebugCircle
	{
		DebugCircle(PxU32 nSegments, PxReal radius) 
		: nSegments(nSegments), radius(radius) {}
		PxU32 nSegments;
		PxReal radius;
	};
	RenderOutput& operator<<(RenderOutput& out, const DebugCircle& circle);

	struct DebugArc
	{
		DebugArc(PxU32 nSegments, PxReal radius, PxReal minAngle, PxReal maxAngle) 
		: nSegments(nSegments), radius(radius), minAngle(minAngle), maxAngle(maxAngle) {}
		PxU32 nSegments;
		PxReal radius;
		PxReal minAngle, maxAngle;
	};
	RenderOutput& operator<<(RenderOutput& out, const DebugArc& arc);

} // namespace shdfnd2
} // namespace physx

#endif
