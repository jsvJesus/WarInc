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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "VtxWeld.h"
#include "PxSimpleTypes.h"

namespace physx
{
	namespace shdfnd2
	{

template<> MeshVertex VertexLess<MeshVertex>::mFind = MeshVertex();
template<> std::vector<MeshVertex > *VertexLess<MeshVertex>::mList=0;


template<>
bool VertexLess<MeshVertex>::operator()(PxI32 v1,PxI32 v2) const
{

	const MeshVertex& a = Get(v1);
	const MeshVertex& b = Get(v2);

	if ( a.mPos[0] < b.mPos[0] ) return true;
	if ( a.mPos[0] > b.mPos[0] ) return false;

	if ( a.mPos[1] < b.mPos[1] ) return true;
	if ( a.mPos[1] > b.mPos[1] ) return false;

	if ( a.mPos[2] < b.mPos[2] ) return true;
	if ( a.mPos[2] > b.mPos[2] ) return false;


	if ( a.mNormal[0] < b.mNormal[0] ) return true;
	if ( a.mNormal[0] > b.mNormal[0] ) return false;

	if ( a.mNormal[1] < b.mNormal[1] ) return true;
	if ( a.mNormal[1] > b.mNormal[1] ) return false;

	if ( a.mNormal[2] < b.mNormal[2] ) return true;
	if ( a.mNormal[2] > b.mNormal[2] ) return false;

  if ( a.mColor < b.mColor ) return true;
  if ( a.mColor > b.mColor ) return false;


	if ( a.mTexel1[0] < b.mTexel1[0] ) return true;
	if ( a.mTexel1[0] > b.mTexel1[0] ) return false;

	if ( a.mTexel1[1] < b.mTexel1[1] ) return true;
	if ( a.mTexel1[1] > b.mTexel1[1] ) return false;

	if ( a.mTexel2[0] < b.mTexel2[0] ) return true;
	if ( a.mTexel2[0] > b.mTexel2[0] ) return false;

	if ( a.mTexel2[1] < b.mTexel2[1] ) return true;
	if ( a.mTexel2[1] > b.mTexel2[1] ) return false;

	if ( a.mTexel3[0] < b.mTexel3[0] ) return true;
	if ( a.mTexel3[0] > b.mTexel3[0] ) return false;

	if ( a.mTexel3[1] < b.mTexel3[1] ) return true;
	if ( a.mTexel3[1] > b.mTexel3[1] ) return false;


	if ( a.mTexel4[0] < b.mTexel4[0] ) return true;
	if ( a.mTexel4[0] > b.mTexel4[0] ) return false;

	if ( a.mTexel4[1] < b.mTexel4[1] ) return true;
	if ( a.mTexel4[1] > b.mTexel4[1] ) return false;

	if ( a.mTangent[0] < b.mTangent[0] ) return true;
	if ( a.mTangent[0] > b.mTangent[0] ) return false;

	if ( a.mTangent[1] < b.mTangent[1] ) return true;
	if ( a.mTangent[1] > b.mTangent[1] ) return false;

	if ( a.mTangent[2] < b.mTangent[2] ) return true;
	if ( a.mTangent[2] > b.mTangent[2] ) return false;

	if ( a.mBiNormal[0] < b.mBiNormal[0] ) return true;
	if ( a.mBiNormal[0] > b.mBiNormal[0] ) return false;

	if ( a.mBiNormal[1] < b.mBiNormal[1] ) return true;
	if ( a.mBiNormal[1] > b.mBiNormal[1] ) return false;

	if ( a.mBiNormal[2] < b.mBiNormal[2] ) return true;
	if ( a.mBiNormal[2] > b.mBiNormal[2] ) return false;

	if ( a.mWeight[0] < b.mWeight[0] ) return true;
	if ( a.mWeight[0] > b.mWeight[0] ) return false;

	if ( a.mWeight[1] < b.mWeight[1] ) return true;
	if ( a.mWeight[1] > b.mWeight[1] ) return false;

	if ( a.mWeight[2] < b.mWeight[2] ) return true;
	if ( a.mWeight[2] > b.mWeight[2] ) return false;

	if ( a.mWeight[3] < b.mWeight[3] ) return true;
	if ( a.mWeight[3] > b.mWeight[3] ) return false;

	if ( a.mBone[0] < b.mBone[0] ) return true;
	if ( a.mBone[0] > b.mBone[0] ) return false;

	if ( a.mBone[1] < b.mBone[1] ) return true;
	if ( a.mBone[1] > b.mBone[1] ) return false;

	if ( a.mBone[2] < b.mBone[2] ) return true;
	if ( a.mBone[2] > b.mBone[2] ) return false;

	if ( a.mBone[3] < b.mBone[3] ) return true;
	if ( a.mBone[3] > b.mBone[3] ) return false;

  if ( a.mRadius < b.mRadius ) return true;
  if ( a.mRadius > b.mRadius ) return false;

	return false;
};



}; // end of name space
};
