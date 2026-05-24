#ifndef BEST_FIT_H

#define BEST_FIT_H

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


// A code snippet to compute the best fit AAB, OBB, plane, capsule and sphere
// Quaternions are assumed a float X,Y,Z,W
// Matrices are assumed 4x4 D3DX style format passed as a float pointer
// The orientation of a capsule is assumed that height is along the Y axis, the same format as the PhysX SDK uses
// The best fit plane routine is derived from code previously published by David Eberly on his Magic Software site.
// The best fit OBB is computed by first approximating the best fit plane, and then brute force rotating the points
// around a single axis to derive the closest fit.  If you set 'bruteforce' to false, it will just use the orientation
// derived from the best fit plane, which is close enough in most cases, but not all.
// Each routine allows you to pass the point stride between position elements in your input vertex stream.
// These routines should all be thread safe as they make no use of any global variables.



namespace SharedTools
{

	bool  computeBestFitPlane(size_t vcount,	// number of input data points
		const float *points,					// starting address of points array.
		size_t vstride,							// stride between input points.
		const float *weights,					// *optional point weighting values.
		size_t wstride,							// weight stride for each vertex.
		float plane[4]);

	float  computeBestFitAABB(size_t vcount,const float *points,size_t pstride,float bmin[3],float bmax[3]); // returns the diagonal distance
	float  computeBestFitSphere(size_t vcount,const float *points,size_t pstride,float center[3]);
	void   computeBestFitOBB(size_t vcount,const float *points,size_t pstride,float *sides,float matrix[16],bool bruteForce);
	void   computeBestFitOBB(size_t vcount,const float *points,size_t pstride,float *sides,float pos[3],float quat[4],bool bruteForce);
	void   computeBestFitCapsule(size_t vcount,const float *points,size_t pstride,float &radius,float &height,float matrix[16],bool bruteForce);
	void   computeBestFitCapsule(size_t vcount,const float *points,size_t pstride,float &radius,float &height,float pos[3],float quat[4],bool bruteForce);

};

#endif
