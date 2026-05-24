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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "MeshImport.h"

#ifdef WIN32
#include <windows.h>
#include <windowsx.h>
#endif

#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

namespace physx
{
	namespace shdfnd2
	{


#ifdef WIN32

static void *getMeshBindingInterface(const char *dll,PxI32 version_number,physx::Foundation *foundation) // loads the tetra maker DLL and returns the interface pointer.
{
  void *ret = 0;

  UINT errorMode = 0;
  errorMode = SEM_FAILCRITICALERRORS;
  UINT oldErrorMode = SetErrorMode(errorMode);
  HMODULE module = LoadLibraryA(dll);
  SetErrorMode(oldErrorMode);
  if ( module )
  {
    void *proc = GetProcAddress(module,"getInterface");
    if ( proc )
    {
      typedef void * (__cdecl * NX_GetToolkit)(PxI32 version,physx::Foundation *foundation);
      ret = ((NX_GetToolkit)proc)(version_number,foundation);
    }
  }
  return ret;
}



#endif

}; // end of namespace

#ifdef LINUX_GENERIC
#include <sys/types.h>
#include <sys/dir.h>
#endif

#define MAXNAME 512

namespace MESHIMPORT_PXSHARE
{

class FileFind
{
public:
  FileFind(const char *dirname,const char *spec)
  {
    if ( dirname && strlen(dirname) )
      sprintf(mSearchName,"%s\\%s",dirname,spec);
    else
      sprintf(mSearchName,"%s",spec);
   }

  ~FileFind(void)
  {
  }


  bool FindFirst(char *name)
  {
    bool ret=false;

    #ifdef WIN32
    hFindNext = FindFirstFileA(mSearchName, &finddata);
    if ( hFindNext == INVALID_HANDLE_VALUE )
      ret =  false;
     else
     {
       bFound = 1; // have an initial file to check.
       ret =  FindNext(name);
     }
     #endif
     #ifdef LINUX_GENERIC
     mDir = opendir(".");
     ret = FindNext(name);
    #endif
    return ret;
  }

  bool FindNext(char *name)
  {
    bool ret = false;

    #ifdef WIN32
    while ( bFound )
    {
      bFound = FindNextFileA(hFindNext, &finddata);
      if ( bFound && (finddata.cFileName[0] != '.') && !(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
      {
        strncpy(name,finddata.cFileName,MAXNAME);
        ret = true;
        break;
      }
    }
    if ( !ret )
    {
      bFound = 0;
      FindClose(hFindNext);
    }
    #endif

    #ifdef LINUX_GENERIC

    if ( mDir )
    {
      while ( 1 )
      {

        struct direct *di = readdir( mDir );

        if ( !di )
        {
          closedir( mDir );
          mDir = 0;
          ret = false;
          break;
        }

        if ( strcmp(di->d_name,".") == 0 || strcmp(di->d_name,"..") == 0 )
        {
          // skip it!
        }
        else
        {
          strncpy(name,di->d_name,MAXNAME);
          ret = true;
          break;
        }
      }
    }
    #endif
    return ret;
  }

private:
  char mSearchName[MAXNAME];
#ifdef WIN32
  WIN32_FIND_DATAA finddata;
  HANDLE hFindNext;
  PxI32 bFound;
#endif
#ifdef LINUX_GENERIC
  DIR      *mDir;
#endif
};

}; // end of namespace
};

namespace physx
{
	namespace shdfnd2
	{


	using namespace MESHIMPORT_PXSHARE;

static const char *lastSlash(const char *foo)
{
  const char *ret = foo;
  const char *last_slash = 0;

  while ( *foo )
  {
    if ( *foo == '\\' ) last_slash = foo;
    if ( *foo == '/' ) last_slash = foo;
    if ( *foo == ':' ) last_slash = foo;
    foo++;
  }
  if ( last_slash ) ret = last_slash+1;
  return ret;
}

bool loadLibs(const char* directory, physx::FileFind& ff, physx::MeshImport *ret, physx::Foundation *foundation )
{
	bool success = false;
    char name[MAXNAME];
	char scratch[512];
    if ( ff.FindFirst(name) )
    {
      do
      {
        const char *scan = lastSlash(name);
		if ( physx::string::stricmp(scan,"MeshImport.dll") == 0 )
        {
          printf("Skipping 'MeshImport.dll'\r\n");
        }
#ifndef _DEBUG
		else if (strstr(scan, "DEBUG") != NULL)
		{
			// PH: Trying to load a DEBUG version of all dlls found, but not directly loading the dll versions
			//printf("Skipping DEBUG dll\r\n");
		}
#endif
        else
        {
          printf("Loading '%s'\r\n", scan );
		  const char *fname;

		  if ( directory && strlen(directory) )
		  {
			  sprintf(scratch,"%s\\%s", directory, scan);
			  fname = scratch;
		  }
		  else
		  {
			  fname = name;
		  }

#ifdef WIN32
		  physx::MeshImporter *imp = (physx::MeshImporter *)getMeshBindingInterface(fname,MESHIMPORT_VERSION,foundation);
#else
		  physx::MeshImporter *imp = 0;
#endif
          if ( imp )
          {
            ret->addImporter(imp);
            printf("Added importer '%s'\r\n", name );
			success = true;
          }
        }
      } while ( ff.FindNext(name) );
    }
	return success;
}


physx::MeshImport * loadMeshImporters(const char * directory,physx::Foundation *foundation) // loads the mesh import library (dll) and all available importers from the same directory.
{
	physx::MeshImport *ret = 0;

	char scratch[512];
	if ( directory && strlen(directory) )
	{
		const char *slash = strchr(directory,'/');
		if ( slash )
			sprintf(scratch,"%s/MeshImport.dll", directory);
		else
			sprintf(scratch,"%s\\MeshImport.dll", directory);
	}
	else
	{
		strcpy(scratch,"MeshImport.dll");
	}

#ifdef WIN32
	ret = (physx::MeshImport *)getMeshBindingInterface(scratch,MESHIMPORT_VERSION,foundation);
#else
  ret = 0;
#endif

  if ( ret )
  {
    physx::FileFind ff(directory,"MeshImport*.dll");
	loadLibs(directory, ff, ret, foundation);
  }
  return ret;
}

}; // end of namespace
};