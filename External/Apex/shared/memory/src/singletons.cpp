#ifdef WIN32
#include <windows.h>
#endif

#include "MemoryTracker.h"

namespace physx
{
	namespace shdfnd2
	{


class FileSystem;
class MeshImport;
class CommLayer;

#if USE_MEMORY_TRACKER
MemoryTracker *gMemoryTracker=0;
#endif

FileSystem *gFileSystem=0;
//SendTextMessage *gSendTextMessage=0;
MeshImport *gMeshImport=0;
CommLayer *gCommLayer=0;

#if USE_MEMORY_TRACKER
MemoryTracker * createMemoryTracker(const char *dllLocation) // loads the DLL to track memory allocations.
{
    MemoryTracker *ret = 0;
#ifdef WIN32
    if ( gMemoryTracker == 0 )
    {
      UINT errorMode = SEM_FAILCRITICALERRORS;
      UINT oldErrorMode = SetErrorMode(errorMode);
      HMODULE module = LoadLibraryA(dllLocation);
      SetErrorMode(oldErrorMode);
      void *proc = GetProcAddress(module,"getInterface");
      if ( proc )
      {
        typedef void * (__cdecl * NX_GetToolkit)(int version,void *systemServices);
      	ret = gMemoryTracker = (MemoryTracker *)((NX_GetToolkit)proc)(MEMORY_TRACKER_VERSION,0);
       }
    }
#endif
    return ret;
}
#endif

}; // end of namespace
};
