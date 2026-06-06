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
// Copyright  2009-2011 NVIDIA Corporation. All rights reserved.
// Copyright  2002-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright  2001-2006 NovodeX. All rights reserved.   

#include "PsMathUtils.h"
#include "PxMat33Legacy.h"
#include "PsUtilities.h"
#include "PxMat33.h"

namespace physx
{
namespace shdfnd2
{

	void transformInertiaTensor(const PxVec3& invD, const PxMat33Legacy& M, PxMat33Legacy& mIInv)
	{
		const float	axx = invD.x*M(0,0), axy = invD.x*M(1,0), axz = invD.x*M(2,0);
		const float	byx = invD.y*M(0,1), byy = invD.y*M(1,1), byz = invD.y*M(2,1);
		const float	czx = invD.z*M(0,2), czy = invD.z*M(1,2), czz = invD.z*M(2,2);

		mIInv(0,0) = axx*M(0,0) + byx*M(0,1) + czx*M(0,2);
		mIInv(1,1) = axy*M(1,0) + byy*M(1,1) + czy*M(1,2);
		mIInv(2,2) = axz*M(2,0) + byz*M(2,1) + czz*M(2,2);

		mIInv(0,1) = mIInv(1,0)	= axx*M(1,0) + byx*M(1,1) + czx*M(1,2);
		mIInv(0,2) = mIInv(2,0)	= axx*M(2,0) + byx*M(2,1) + czx*M(2,2);
		mIInv(1,2) = mIInv(2,1)	= axy*M(2,0) + byy*M(2,1) + czy*M(2,2);
	}


	PxQuat computeQuatFromNormal(const PxVec3& n)
	{
		//parallel or anti-parallel
		if(n.x > 0.9999f)
		{
			//parallel
			return PxQuat::identity();
		}
		else if (n.x < -0.9999f)
		{
			//anti-parallel
			//contactQuaternion.fromAngleAxisFast(PXD_PI, Vector3(0.0f, 1.0f, 0.0f));
			return PxQuat(0.0f, 1.0f, 0.0f, 0.0f);
		}
		else
		{
			PxVec3 rotVec(0.0f, -n.z, n.y);

			//Convert to quat
			PxReal angle = rotVec.magnitude();
			rotVec *= 1.0f/angle;
			if(angle > 1.0f) angle = 1.0f;

			// djs: injudiciously imbecilic use of trig functions, good thing Adam is going to trample this path like a
			// frustrated rhinoceros in mating season

			angle = PxAsin(angle);

			if(n.x < 0)
				angle = PxPi - angle;

			return PxQuat(angle, rotVec);
		}
	}

	/**
	\brief computes a oriented bounding box around the scaled basis.
	\param basis Input = skewed basis, Output = (normalized) orthogonal basis.
	\return Bounding box extent.
	*/
	PxVec3 optimizeBoundingBox(PxMat33& basis)
	{
		PxVec3 vec[3] = { basis.column0, basis.column1, basis.column2 };
		PxVec3 magnitude( vec[0].magnitudeSquared(), vec[1].magnitudeSquared(), vec[2].magnitudeSquared() );

		// find indices sorted by magnitude
		int i = magnitude[1] > magnitude[ 0 ] ? 1 :  0;
		int j = magnitude[2] > magnitude[1-i] ? 2 : 1-i;
		int k = 3 - i - j;
		if(magnitude[i] < magnitude[j])
			swap(i, j);

		// ortho-normalize basis

		// corresponds to: magnitude[i] = vec[i].normalize();
		PxReal invSqrt = PxRecipSqrt(magnitude[i]); 
		magnitude[i] *= invSqrt; vec[i] *= invSqrt; 
		vec[j] -= vec[i] * vec[i].dot(vec[j]);
		vec[k] -= vec[i] * vec[i].dot(vec[k]);

		magnitude[j] = vec[j].normalize();
		vec[k] -= vec[j] * vec[j].dot(vec[k]);

		magnitude[k] = vec[k].normalize();

		basis.column0 = vec[0];
		basis.column1 = vec[1];
		basis.column2 = vec[2];

		return magnitude;
	}

	PxQuat slerp(const PxReal t, const PxQuat& left, const PxQuat& right) // this = slerp(t, a, b)
	{
		const PxReal quatEpsilon = (PxReal(1.0e-8f));

		PxReal cosine = left.dot(right);
		PxReal sign = PxReal(1);
		if (cosine < 0)
		{
			cosine = -cosine;
			sign = PxReal(-1);
		}

		PxReal sine = PxReal(1) - cosine*cosine;

		if(sine>=quatEpsilon*quatEpsilon)	
		{
			sine = PxSqrt(sine);
			const PxReal angle = PxAtan2(sine, cosine);
			const PxReal i_sin_angle = PxReal(1) / sine;

			const PxReal leftw = PxSin(angle*(PxReal(1)-t)) * i_sin_angle;
			const PxReal rightw = PxSin(angle * t) * i_sin_angle * sign;

			return left * leftw + right * rightw;
		}

		return left;
	}


	PxVec3 ellipseClamp(const PxVec3& p,
		const PxVec3& e)
	{
		// finds the closest point on the ellipse to a given point

		// (p.y, p.z) is the input point
		// (e.y, e.z) are the radii of the ellipse

		// lagrange multiplier method with Newton/Halley hybrid root-finder.
		// see http://www.geometrictools.com/Documentation/DistancePointToEllipse2.pdf
		// for proof of Newton step robustness and initial estimate. 
		// Halley converges much faster but sometimes overshoots - when that happens we take
		// a newton step instead

		// converges in 1-2 iterations where D&C works well, and it's good with 4 iterations 
		// with any ellipse that isn't completely crazy

		const int MAX_ITERATIONS = 6;
		const PxReal convergenceThreshold = 1e-4f; 

		// iteration requires first quadrant but we recover generality later

		PxVec3 q(0,PxAbs(p.y),PxAbs(p.z));
		const PxReal tinyEps = (PxReal)(1e-6);		// very close to minor axis is numerically problematic but trivial
		if(e.y>=e.z)
		{
			if(q.z<tinyEps)
				return PxVec3(0, p.y>0 ? e.y : -e.y, 0);
		}
		else
		{
			if(q.y<tinyEps)
				return PxVec3(0, 0, p.z>0 ? e.z : -e.z);	
		}

		PxVec3 denom, e2 = e.arrayMultiply(e), q2 = q.arrayMultiply(q), e2q2 = e2.arrayMultiply(q2);

		// we can use any initial guess which is > maximum(-e.y^2,-e.z^2) and for which f(t) is > 0. 
		// this guess works well near the axes, but is weak along the diagonals. 

		PxReal t = PxMax(e.y*(q.y - e.y), e.z*(q.z - e.z));

		PxReal df = 0, st = 0, sf = 0; // initialize to prevent compiler warnings

		for (PxU32 i = 0; i < MAX_ITERATIONS; i++)
		{
			denom = PxVec3(0,1/(t + e2.y), 1/(t + e2.z));
			PxVec3 denom2 = denom.arrayMultiply(denom);

			PxVec3 fv = e2q2.arrayMultiply(denom2);
			PxReal f = fv.y + fv.z - 1;

			if (fabs(f) < convergenceThreshold)
				return e2.arrayMultiply(p).arrayMultiply(denom);

			if(f>0)	//	Halley or Newton iteration, save t and f in case we overshoot with Halley
			{
				st = t;
				sf = f;
				df = fv.dot(denom) * -2.0f;
				PxReal d2f = fv.dot(denom2) * 6.0f;
				PxReal divisor = (2 * df * df - f * d2f);
				t-= PxAbs(divisor)>tinyEps ? (2 * f * df) / divisor : f/df;
			}
			else
				t = st - sf/df;
		}

		// we didn't converge, so clamp what we have
		PxVec3 r = e2.arrayMultiply(p).arrayMultiply(denom);
		return r * PxRecipSqrt(sqr(r.y/e.y) + sqr(r.z/e.z));
	}

} // namespace shdfnd2
} // namespace physx
