#include "r3dPCH.h"
#include "r3d.h"

#include "PhysXRepXHelpers.h"

UserStream::UserStream(const char* filename, bool load)
: fpw(NULL)
, fpr(NULL)
{
	if (load)
		fpr = r3d_open(filename, "rb");
	else
		fpw = fopen_for_write(filename, "wb");
}

UserStream::~UserStream()
{
	if (fpr)
		fclose(fpr);

	if (fpw)
		fclose(fpw);
}

PxU32 UserStream::read(void* dest, PxU32 count)
{
	if (!fpr || !dest || count == 0)
		return 0;

	return (PxU32)fread(dest, 1, count, fpr);
}

PxU32 UserStream::write(const void* src, PxU32 count)
{
	if (!fpw || !src || count == 0)
		return 0;

	return (PxU32)fwrite(src, 1, count, fpw);
}

PxU32 UserStream::getLength() const
{
	if (!fpr)
		return 0;

	return (PxU32)fpr->size;
}

void UserStream::seek(PxU32 offset)
{
	if (fpr)
		fseek(fpr, offset, SEEK_SET);
}

PxU32 UserStream::tell() const
{
	if (!fpr)
		return 0;

	return (PxU32)ftell(fpr);
}

MemoryWriteBuffer::MemoryWriteBuffer()
: currentSize(0)
, maxSize(0)
, data(NULL)
{
}

MemoryWriteBuffer::~MemoryWriteBuffer()
{
	delete[] data;
}

void MemoryWriteBuffer::clear()
{
	currentSize = 0;
}

PxU32 MemoryWriteBuffer::write(const void* src, PxU32 count)
{
	if (!src || count == 0)
		return 0;

	const PxU32 expectedSize = currentSize + count;

	if (expectedSize > maxSize)
	{
		maxSize = expectedSize + 4096;

		PxU8* newData = new PxU8[maxSize];
		PX_ASSERT(newData);

		if (data)
		{
			memcpy(newData, data, currentSize);
			delete[] data;
		}

		data = newData;
	}

	memcpy(data + currentSize, src, count);
	currentSize += count;

	return count;
}

MemoryReadBuffer::MemoryReadBuffer(const PxU8* sourceData, PxU32 sourceLength)
: buffer(sourceData)
, size(sourceLength)
, pos(0)
{
}

MemoryReadBuffer::~MemoryReadBuffer()
{
}

PxU32 MemoryReadBuffer::read(void* dest, PxU32 count)
{
	if (!dest || !buffer || count == 0)
		return 0;

	if (pos + count > size)
		count = size - pos;

	memcpy(dest, buffer + pos, count);
	pos += count;

	return count;
}

PxU32 MemoryReadBuffer::getLength() const
{
	return size;
}

void MemoryReadBuffer::seek(PxU32 offset)
{
	pos = R3D_MIN(offset, size);
}

PxU32 MemoryReadBuffer::tell() const
{
	return pos;
}

FileSerialStream::FileSerialStream(const char* fileName)
: writtenBytes(0)
, f(fopen_for_write(fileName, "wb"))
{
}

FileSerialStream::~FileSerialStream()
{
	if (f)
		fclose(f);
}

PxU32 FileSerialStream::write(const void* src, PxU32 count)
{
	if (!f || !src || count == 0)
		return 0;

	PxU32 written = (PxU32)fwrite(src, 1, count, f);
	writtenBytes += written;

	return written;
}

PxU32 FileSerialStream::getTotalStoredSize() const
{
	return writtenBytes;
}