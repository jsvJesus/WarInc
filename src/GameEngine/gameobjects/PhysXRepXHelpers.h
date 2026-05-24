#pragma once

#include "PxPhysicsAPI.h"

using namespace physx;

class UserStream : public PxInputData, public PxOutputStream
{
public:
	UserStream(const char* filename, bool load);
	virtual ~UserStream();

	virtual PxU32 read(void* dest, PxU32 count);
	virtual PxU32 write(const void* src, PxU32 count);

	virtual PxU32 getLength() const;
	virtual void seek(PxU32 offset);
	virtual PxU32 tell() const;

	FILE* fpw;
	r3dFile* fpr;
};

class MemoryWriteBuffer : public PxOutputStream
{
public:
	MemoryWriteBuffer();
	virtual ~MemoryWriteBuffer();

	void clear();

	virtual PxU32 write(const void* src, PxU32 count);

	PxU32 currentSize;
	PxU32 maxSize;
	PxU8* data;
};

class MemoryReadBuffer : public PxInputData
{
public:
	MemoryReadBuffer(const PxU8* data, PxU32 length);
	virtual ~MemoryReadBuffer();

	virtual PxU32 read(void* dest, PxU32 count);
	virtual PxU32 getLength() const;
	virtual void seek(PxU32 offset);
	virtual PxU32 tell() const;

	const PxU8* buffer;
	PxU32 size;
	PxU32 pos;
};

class FileSerialStream : public PxOutputStream
{
	PxU32 writtenBytes;
	FILE* f;

public:
	explicit FileSerialStream(const char* fileName);
	virtual ~FileSerialStream();

	virtual PxU32 write(const void* src, PxU32 count);
	PxU32 getTotalStoredSize() const;
};