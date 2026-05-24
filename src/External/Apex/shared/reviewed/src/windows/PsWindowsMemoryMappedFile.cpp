#include "PsMemoryMappedFile.h"
#include "PsWindowsInclude.h"

namespace physx
{
	namespace shdfnd2
	{

class MemoryMappedFileImpl
{
public:
	HANDLE	mMapFile;
	void *  mHeader;
};

MemoryMappedFile::MemoryMappedFile(const char *mappingObject,unsigned int mapSize)
{
	mImpl = (MemoryMappedFileImpl *)PX_ALLOC(sizeof(MemoryMappedFileImpl));
	mImpl->mHeader = NULL;
   	mImpl->mMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS,FALSE,mappingObject);
	if ( mImpl->mMapFile == NULL )
	{
		mImpl->mMapFile = CreateFileMappingA(
      				INVALID_HANDLE_VALUE,    // use paging file
       				NULL,                    // default security
       				PAGE_READWRITE,          // read/write access
       				0,                       // maximum object size (high-order DWORD)
       				mapSize,                // maximum object size (low-order DWORD)
       				mappingObject);
	}
	if ( mImpl->mMapFile )
	{
		mImpl->mHeader = MapViewOfFile(mImpl->mMapFile,FILE_MAP_ALL_ACCESS,0,0,mapSize);
	}

}

MemoryMappedFile::~MemoryMappedFile(void)
{

	if ( mImpl->mHeader )
   	{
   		UnmapViewOfFile(mImpl->mHeader);
   		if ( mImpl->mMapFile )
   		{
   			CloseHandle(mImpl->mMapFile);
   		}
	}

	PX_FREE(mImpl);
}

void * MemoryMappedFile::getBaseAddress(void)
{
	return mImpl->mHeader;
}

}; // end of namespace
}; // end of namespace
