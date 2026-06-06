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

#include "PsFPU.h"
#include "float.h"

#pragma warning(disable:4996) // intentionally suppressing this warning message

namespace physx
{
namespace shdfnd2
{

#ifndef PX_X64
void fpuSetExceptions(bool exOn) 
{
	if (exOn)
	{
		_controlfp(
			_EM_INVALID|
			_EM_DENORMAL|
			_EM_ZERODIVIDE|
			_EM_OVERFLOW|
			_EM_UNDERFLOW|
			_EM_INEXACT
			,_MCW_EM);
	}
	else
	{
		_controlfp(0,_MCW_EM);
	}
}

void fpuReset()
{
	// PT: setup FPU rounding mode & accuracy each time we run the simulation. This is now mandatory 
	// since we compile with /QIfist. However we use a small fake cast to check whether reseting the FPU 
	// is useful or not. Using a fake cast looks faster than actually checking the FPU control word (and 
	// it's also portable!). "volatile" ensures the compiler doesn't optimize thecode out. "1.9" catches 
	// both "up" and "near" FPU rounding modes. The FPU reset is only executed once, provided external
	// programs don't change the FPU mode as well.

	volatile float DummyF = 1.9f;
	volatile int DummyI = (int)DummyF;
	if(DummyI!=1)
		_controlfp(_RC_CHOP,	_MCW_RC);		// set rounding mode to chop

	_controlfp(_PC_64,	_MCW_PC);				// set precision to 64 bit just in case
}

#else
	void fpuSetExceptions(bool) {} 
	void fpuReset() {}
#endif

FPUGuard::FPUGuard()
: mControlWord(0)
{
#ifndef PX_X64
	PxU16 CW;
	_asm	fstcw	CW
	mControlWord = CW;
#endif
}

FPUGuard::~FPUGuard()
{
#ifndef PX_X64
	PxU16 CW = (PxU16)mControlWord;
	_asm	fldcw	CW
#endif
}

} // namespace shdfnd2
} // namespace physx
