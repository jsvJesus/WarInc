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
#include <assert.h>
#include "PxSimpleTypes.h"
#include "MeshImport.h"
#include "ImportEzm.h"

#pragma warning(disable:4100)

bool doShutdown(void);

namespace physx
{
	namespace shdfnd2
	{


class MyMeshImportEzm : public MeshImporter, public UserAllocated
{
public:
  MyMeshImportEzm(void)
  {
  }

  virtual ~MyMeshImportEzm(void)
  {
  }

  bool shutdown(void)
  {
    return doShutdown();
  }

  virtual const char * getExtension(PxI32 index)  // report the default file name extension for this mesh type.
  {
    return ".ezm";
  }

  virtual const char * getDescription(PxI32 index)
  {
    return "EZ-Mesh format";
  }


  virtual bool importMesh(const char *meshName,const void *data,PxU32 dlen,physx::MeshImportInterface *callback,const char *options,MeshImportApplicationResource *appResource)
  {
    bool ret = false;

    MeshImporter *mi = physx::createMeshImportEZM();
    if ( mi )
    {
      ret = mi->importMesh(meshName,data,dlen,callback,options,appResource);
      physx::releaseMeshImportEZM(mi);
    }

    return ret;
  }
};


static MyMeshImportEzm *gInterface=0;

MeshImporter * getInterfaceMeshImportEzm(PxI32 version_number,physx::Foundation *foundation)
{
	if (!verifyFoundation(foundation))
		return NULL;

	assert( gInterface == 0 );
	if ( gInterface == 0 && version_number == MESHIMPORT_VERSION )
	{
		gInterface = PX_NEW(MyMeshImportEzm);
	}
	return static_cast<MeshImporter *>(gInterface);
};

};  // End of namespace PATHPLANNING
};

