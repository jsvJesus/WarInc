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

#ifndef PX_FOUNDATION_PSMUTEX_H
#define PX_FOUNDATION_PSMUTEX_H

#include "PsAllocator.h"
#include "PsNoCopy.h"

namespace physx
{
namespace shdfnd2
{
    class MutexImpl
    {
    public:

        /**
        The constructor for Mutex creates a mutex. It is initially unlocked.
        */
        MutexImpl();

        /**
        The destructor for Mutex deletes the mutex.
        */
        ~MutexImpl();

        /**
        Acquire (lock) the mutex. If the mutex is already locked
        by another thread, this method blocks until the mutex is
        unlocked.
        */
        bool lock();

        /**
        Acquire (lock) the mutex. If the mutex is already locked
        by another thread, this method returns false without blocking.
        */
        bool trylock();

        /**
        Release (unlock) the mutex.
        */
        bool unlock();

        /**
        Size of this class.
        */
        static const PxU32 size;
    };

	template <typename Alloc = ReflectionAllocator<MutexImpl> >
	class MutexT : protected Alloc
	{
	public:

		class ScopedLock : private NoCopy
		{
			MutexT<Alloc>& mMutex;
		public:
			PX_INLINE	ScopedLock(MutexT<Alloc> &mutex): mMutex(mutex) { mMutex.lock(); }
			PX_INLINE	~ScopedLock() { mMutex.unlock(); }
		};

		/**
		The constructor for Mutex creates a mutex. It is initially unlocked.
		*/
		MutexT(const Alloc& alloc = Alloc())
			: Alloc(alloc)
		{
			mImpl = (MutexImpl*)Alloc::allocate(MutexImpl::size, __FILE__, __LINE__);
			PX_PLACEMENT_NEW(mImpl, MutexImpl)();
		}

		/**
		The destructor for Mutex deletes the mutex.
		*/
		~MutexT()
		{
			mImpl->~MutexImpl();
			Alloc::deallocate(mImpl);
		}

		/**
		Acquire (lock) the mutex. If the mutex is already locked
		by another thread, this method blocks until the mutex is
		unlocked.
		*/
		bool lock()		const	{ return mImpl->lock(); }

		/**
		Acquire (lock) the mutex. If the mutex is already locked
		by another thread, this method returns false without blocking.
		*/
		bool trylock()	const	{ return mImpl->trylock(); }

		/**
		Release (unlock) the mutex.
		*/
		bool unlock()	const	{ return mImpl->unlock(); }

	private:
		MutexImpl* mImpl;
	};

    class ReadWriteLock : private NoCopy
    {
    public:
        ReadWriteLock();
        ~ReadWriteLock();

        void lockReader();
        void lockWriter();

        void unlockReader();
        void unlockWriter();

    private:
        class ReadWriteLockImpl*    mImpl;
    };

	class ScopedReadLock : private NoCopy
	{
	public:
		PX_INLINE	ScopedReadLock(ReadWriteLock& lock): mLock(lock)	{			mLock.lockReader(); 	}
		PX_INLINE	~ScopedReadLock()									{			mLock.unlockReader();	}

	private:
		ReadWriteLock& mLock;
	};

	class ScopedWriteLock : private NoCopy
	{
	public:
		PX_INLINE	ScopedWriteLock(ReadWriteLock& lock): mLock(lock)	{		mLock.lockWriter(); 	}
		PX_INLINE	~ScopedWriteLock()									{		mLock.unlockWriter();	}

	private:
		ReadWriteLock& mLock;
	};

	typedef MutexT<> Mutex;

} // namespace shdfnd2
} // namespace physx

#endif
