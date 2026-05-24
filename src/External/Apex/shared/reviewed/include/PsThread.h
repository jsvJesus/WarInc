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

#ifndef PX_FOUNDATION_PSTHREAD_H
#define PX_FOUNDATION_PSTHREAD_H

#include "PxThreadPriority.h"
#include "PsUserAllocated.h"

// dsequeira: according to existing comment here (David Black would be my guess)
// "This is useful to reduce bus contention on tight spin locks. And it needs
// to be a macro as the xenon compiler often ignores even __forceinline." What's not
// clear is why a pause function needs inlining...? (TODO: check with XBox team)

// todo: these need to go somewhere else 

#if defined(PX_WINDOWS) 
#	define PxSpinLockPause() __asm pause
#elif defined(PX_X360)
#	define PxSpinLockPause() __asm nop
#elif defined(PX_LINUX)
#   define PxSpinLockPause() asm ("nop")
#elif defined(PX_PS3)
#    ifdef __SNC__
#        warning Sauce -- define this!
#        define PxSpinLockPause() 
#    else // __SNC__
#	    define PxSpinLockPause() asm ("nop") // don't know if it's correct yet...
#    endif // __SNC__
#define PX_TLS_MAX_SLOTS 64
#elif defined(PX_WII)
#	define PxSpinLockPause() asm { nop } // don't know if it's correct yet...
#endif


namespace physx
{
namespace shdfnd2
{
	/**
	Thread abstraction API
	*/

	class Thread : public UserAllocated
	{
	public:
		static const PxU32 DEFAULT_STACK_SIZE;
		typedef		size_t	Id;								// space for a pointer or an integer
		typedef		void*	(*ExecuteFn)(void *);

		static Id getId();
			
		/**  
		Construct (but do not start) the thread object. Executes in the context
		of the spawning thread
		*/

		Thread();

		/**  
		Construct and start the the thread, passing the given arg to the given fn. (pthread style)
		*/

		Thread(ExecuteFn fn, void *arg);


		/**
		Deallocate all resources associated with the thread. Should be called in the
		context of the spawning thread.
		*/

		virtual ~Thread();


		/**
		start the thread running. Called in the context of the spawning thread.
		*/

		void start(PxU32 stackSize);

		/**
		Violently kill the current thread. Blunt instrument, not recommended since
		it can leave all kinds of things unreleased (stack, memory, mutexes...) Should
		be called in the context of the spawning thread.
		*/

		void kill();

		/**
		The virtual execute() method is the user defined function that will
		run in the new thread. Called in the context of the spawned thread.
		*/

		virtual void execute(void);

		/**
		stop the thread. Signals the spawned thread that it should stop, so the 
		thread should check regularly
		*/

		void signalQuit();

		/**
		Wait for a thread to stop. Should be called in the context of the spawning
		thread. Returns false if the thread has not been started.
		*/

		bool waitForQuit();

		/**
		check whether the thread is signalled to quit. Called in the context of the
		spawned thread.
		*/

		bool quitIsSignalled();

		/**
		Cleanly shut down this thread. Called in the context of the spawned thread.
		*/
		void quit();

		/**
		Change the affinity mask for this thread.
		On Xbox360, sets the hardware thread to the first non-zero bit.

		Returns previous mask if successful, or zero on failure
		*/	
		virtual PxU32 setAffinityMask(PxU32 mask);

		/** Set thread priority. */
		void setPriority(PxThreadPriority::Enum prio);

		/** set the thread's name */
		void setName(const char *name);

		/** Put the current thread to sleep for the given number of milliseconds */
		static void sleep(PxU32 ms);

		/** Yield the current thread's slot on the CPU */
		static void yield();

	private:
		class ThreadImpl *mImpl;
	};


	PxU32			TlsAlloc();
	void			TlsFree(PxU32 index);
	void *			TlsGet(PxU32 index);
	PxU32			TlsSet(PxU32 index,void *value);

} // namespace shdfnd2
} // namespace physx

#endif
