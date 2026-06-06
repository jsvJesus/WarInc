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

#ifndef PX_FOUNDATION_PS_H
#define PX_FOUNDATION_PS_H

/*! \file top level include file for shared foundation */

#include "Px.h"

/**
Platform specific defines
*/
#ifdef PX_WINDOWS
	#pragma intrinsic(memcmp)
	#pragma intrinsic(memcpy)
	#pragma intrinsic(memset)
	#pragma intrinsic(abs)
	#pragma intrinsic(labs)
#endif

#ifdef PX_VC // get rid of browser info warnings 
	#pragma warning( disable : 4786 ) // identifier was truncated to '255' characters in the debug information
	#pragma warning( disable : 4251 ) // class needs to have dll-interface to be used by clients of class
	#pragma warning( default : 4239 ) // report rvalue to non-const reference conversion as error
	#pragma warning( default : 4265 ) // 'class' : class has virtual functions, but destructor is not virtual.
	#pragma warning( default : 4287 ) // 'operator' : unsigned/negative constant mismatch.
	#pragma warning( default : 4296 ) // 'operator' : expression is always false.
	#pragma warning( default : 4302 ) // 'conversion' : truncation from 'type 1' to 'type 2'.
	#pragma warning( default : 4529 ) // 'member_name' : forming a pointer-to-member requires explicit use of the address-of operator ('&') and a qualified name.
	#pragma warning( default : 4555 ) // expression has no effect; expected expression with side-effect.
	#pragma warning( disable : 4324 ) // structure was padded due to __declspec(align())
#endif

#ifdef __PPCGEKKO__
#pragma warn_unusedarg off
#pragma warn_hidevirtual off
#pragma warn_implicitconv off
#endif

/*! restrict macro */
#if defined(PX_PS3) || defined(PX_VC)
#	define PX_RESTRICT __restrict
#elif defined(PX_CW) && __STDC_VERSION__ >= 199901L
#	define PX_RESTRICT restrict
#else
#	define PX_RESTRICT
#endif

// An expression that should expand to nothing in _DEBUG builds.  
// We currently use this only for tagging the purpose of containers for memory use tracking.
#if defined(_DEBUG)
#define PX_DEBUG_EXP(x) (x)
#define PX_DEBUG_EXP_C(x) x,
#else
#define PX_DEBUG_EXP(x)
#define PX_DEBUG_EXP_C(x)
#endif

// PT: typical "invalid" value in various CD algorithms
#define	PX_INVALID_U32		0xffffffff

#define PX_SIGN_BITMASK		0x80000000

// PT: this used to be replicated everywhere in the code, causing bugs to sometimes reappear (e.g. TTP 3587).
// It is better to define it in a header and use the same constant everywhere. The original value (1e-05f)
// caused troubles (e.g. TTP 1705, TTP 306).
#define PX_PARALLEL_TOLERANCE	1e-02f

namespace physx
{
	namespace shdfnd2 
	{
		// Int-as-bool type - has some uses for efficiency and with SIMD
		typedef int IntBool;
		static const IntBool IntFalse = 0;
		static const IntBool IntTrue = 1;

		template<class T, class Alloc> class Array;

		class ProfilerManager;
		class ProfileZone;
		class Sync;
		class RenderOutput;
		class RenderBuffer;

		class Matrix34;

		struct Triangle32;

		// PX_SERIALIZATION
		class RefCountable;
		struct FieldDescriptor;
		class FieldDescriptors;
		//~PX_SERIALIZATION

		// todo: forward declare all classes
	}

	namespace pubfnd2
	{
		class PxMat33Legacy;
		class PxMat34Legacy;

	}

        namespace shdfnd = shdfnd2;

	// pull public foundation into physx namespace
	using namespace pubfnd;
} // namespace physx


#endif
