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


#pragma warning(disable:4702) // disabling a warning that only shows up when building VC7

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include "PsUserAllocated.h"
#include "MeshImport.h"
#include "ImportEZM.h"
#include "StringDict.h"
#include "sutil.h"
#include "stable.h"
#include "asc2bin.h"
#include "inparser.h"
#include "FastXml.h"
#include "PxMemoryBuffer.h"

#pragma warning(disable:4100)
#pragma warning(disable:4996)

#define IMP_NEW(x) new x

namespace physx
{
	namespace shdfnd2
	{


#define DEBUG_LOG 0

    enum NodeType
    {
        NT_NONE,
        NT_SCENE_GRAPH,
        NT_MESH,
        NT_ANIMATION,
        NT_SKELETON,
        NT_BONE,
        NT_MESH_SECTION,
        NT_VERTEX_BUFFER,
        NT_INDEX_BUFFER,
        NT_NODE_TRIANGLE,
        NT_NODE_INSTANCE,
        NT_ANIM_TRACK,
        NT_MESH_SYSTEM,
        NT_MESH_AABB,
        NT_SKELETONS,
        NT_ANIMATIONS,
        NT_MATERIALS,
        NT_MATERIAL,
        NT_MESHES,
        NT_MESH_COLLISION_REPRESENTATIONS,
        NT_MESH_COLLISION_REPRESENTATION,
        NT_MESH_COLLISION,
        NT_MESH_COLLISION_CONVEX,
        NT_LAST
    };

    enum AttributeType
    {
        AT_NONE,
        AT_ASSET_NAME,
        AT_ASSET_INFO,
        AT_NAME,
        AT_COUNT,
        AT_TRIANGLE_COUNT,
        AT_PARENT,
        AT_MATERIAL,
        AT_CTYPE,
        AT_SEMANTIC,
        AT_POSITION,
        AT_ORIENTATION,
        AT_DURATION,
        AT_DTIME,
        AT_TRACK_COUNT,
        AT_FRAME_COUNT,
        AT_HAS_SCALE,
        AT_MESH_SYSTEM_VERSION,
        AT_MESH_SYSTEM_ASSET_VERSION,
        AT_MIN,
        AT_MAX,
        AT_SCALE,
        AT_META_DATA,
        AT_SKELETON,
        AT_SUBMESH_COUNT,
        AT_INFO,
        AT_TYPE,
        AT_TRANSFORM,
        AT_LAST
    };

	class MeshImportEZM : public MeshImporter, public FAST_XML::FastXml::Callback, public UserAllocated
    {
        public:
            MeshImportEZM(void)
            {
                mType     = NT_NONE;
                mBone     = 0;
                mFrameCount = 0;
                mDuration   = 0;
                mTrackCount = 0;
                mDtime      = 0;
                mTrackIndex = 0;
                mVertexFlags = 0;

                mToElement.SetCaseSensitive(false);
                mToAttribute.SetCaseSensitive(false);

                mToElement.Add("SceneGraph",                       NT_SCENE_GRAPH);
                mToElement.Add("Mesh",                             NT_MESH);
                mToElement.Add("Animation",                        NT_ANIMATION);
                mToElement.Add("Skeleton",                         NT_SKELETON);
                mToElement.Add("Bone",                             NT_BONE);
                mToElement.Add("MeshSection",                      NT_MESH_SECTION);
                mToElement.Add("VertexBuffer",                     NT_VERTEX_BUFFER);
                mToElement.Add("IndexBuffer",                      NT_INDEX_BUFFER);
                mToElement.Add("NodeTriangle",                     NT_NODE_TRIANGLE);
                mToElement.Add("NodeInstance",                     NT_NODE_INSTANCE);
                mToElement.Add("AnimTrack",                        NT_ANIM_TRACK);
                mToElement.Add("MeshSystem",                       NT_MESH_SYSTEM);
                mToElement.Add("MeshAABB",                         NT_MESH_AABB);
                mToElement.Add("Skeletons",                        NT_SKELETONS);
                mToElement.Add("Animations",                       NT_ANIMATIONS);
                mToElement.Add("Materials",                        NT_MATERIALS);
                mToElement.Add("Material",                         NT_MATERIAL);
                mToElement.Add("Meshes",                           NT_MESHES);
                mToElement.Add("MeshCollisionRepresentations",     NT_MESH_COLLISION_REPRESENTATIONS);
                mToElement.Add("MeshCollisionRepresentation",      NT_MESH_COLLISION_REPRESENTATION);
                mToElement.Add("MeshCollision",                    NT_MESH_COLLISION);
                mToElement.Add("MeshCollisionConvex",              NT_MESH_COLLISION_CONVEX);

                mToAttribute.Add("name",                           AT_NAME);
                mToAttribute.Add("count",                          AT_COUNT);
                mToAttribute.Add("triangle_count",                 AT_TRIANGLE_COUNT);
                mToAttribute.Add("parent",                         AT_PARENT);
                mToAttribute.Add("material",                       AT_MATERIAL);
                mToAttribute.Add("ctype",                          AT_CTYPE);
                mToAttribute.Add("semantic",                       AT_SEMANTIC);
                mToAttribute.Add("position",                       AT_POSITION);
                mToAttribute.Add("orientation",                    AT_ORIENTATION);
                mToAttribute.Add("duration",                       AT_DURATION);
                mToAttribute.Add("dtime",                          AT_DTIME);
                mToAttribute.Add("trackcount",                     AT_TRACK_COUNT);
                mToAttribute.Add("framecount",                     AT_FRAME_COUNT);
                mToAttribute.Add("has_scale",                      AT_HAS_SCALE);
                mToAttribute.Add("asset_name",                     AT_ASSET_NAME);
                mToAttribute.Add("asset_info",                     AT_ASSET_INFO);
                mToAttribute.Add("mesh_system_version",            AT_MESH_SYSTEM_VERSION);
                mToAttribute.Add("mesh_system_asset_version",                  AT_MESH_SYSTEM_ASSET_VERSION);
                mToAttribute.Add("min", AT_MIN);
                mToAttribute.Add("max", AT_MAX);
                mToAttribute.Add("scale", AT_SCALE);
                mToAttribute.Add("meta_data", AT_META_DATA);
                mToAttribute.Add("skeleton", AT_SKELETON);
                mToAttribute.Add("submesh_count", AT_SUBMESH_COUNT);
                mToAttribute.Add("info", AT_INFO);
                mToAttribute.Add("type", AT_TYPE);
                mToAttribute.Add("transform", AT_TRANSFORM);

                mHasScale      = false;
                mName          = 0;
                mCount         = 0;
                mParent        = 0;
                mCtype         = 0;
                mSemantic      = 0;
                mSkeleton      = 0;
                mBoneIndex     = 0;
                mIndexBuffer   = 0;
                mVertexBuffer  = 0;
                mVertices      = 0;
                mVertexCount   = 0;
                mIndexCount    = 0;
                mAnimTrack     = 0;
                mAnimation     = 0;
                mVertices      = 0;
                mMeshSystemVersion = 0;
                mMeshSystemAssetVersion = 0;
                mMeshCollisionRepresentation = 0;
                mMeshCollision = 0;
                mMeshCollisionConvex = 0;

            }

            virtual ~MeshImportEZM(void)
            {

            }

            const PxU8 * getVertex(const PxU8 *src,MeshVertex &v,const char **types,PxI32 tcount)
            {
                bool firstTexel =true;

                for (PxI32 i=0; i<tcount; i++)
                {
                    const char *type = types[i];
					if ( physx::string::stricmp(type,"position") == 0 )
                    {
                        const PxF32 * scan = (const PxF32 *)src;
                        v.mPos[0] = scan[0];
                        v.mPos[1] = scan[1];
                        v.mPos[2] = scan[2];
                        src+=sizeof(PxF32)*3;
                    }
					else if ( physx::string::stricmp(type,"normal") == 0 )
                    {
                        const PxF32 * scan = (const PxF32 *)src;
                        v.mNormal[0] = scan[0];
                        v.mNormal[1] = scan[1];
                        v.mNormal[2] = scan[2];
                        src+=sizeof(PxF32)*3;
                    }
					else if ( physx::string::stricmp(type,"color") == 0 )
                    {
                        const PxU32 *scan = (const PxU32 *)src;
                        v.mColor = scan[0];
                        src+=sizeof(PxU32);
                    }
					else if ( physx::string::stricmp(type,"texcoord") == 0 || physx::string::stricmp(type,"texcoord1") == 0 || physx::string::stricmp(type,"texel1") == 0 )
                    {
                        const PxF32 * scan = (const PxF32 *)src;
                        if ( firstTexel )
                        {
                            v.mTexel1[0] = scan[0];
                            v.mTexel1[1] = scan[1];
                            firstTexel =false;
                        }
                        else
                        {
                            v.mTexel2[0] = scan[0];
                            v.mTexel2[1] = scan[1];
                        }
                        src+=sizeof(PxF32)*2;
                    }
                    else if ( physx::string::stricmp(type,"texcoord2") == 0 || physx::string::stricmp(type,"texel2") == 0 )
                    {
                        const PxF32 * scan = (const PxF32 *)src;
                        v.mTexel2[0] = scan[0];
                        v.mTexel2[1] = scan[1];
                        src+=sizeof(PxF32)*2;
                    }
                    else if ( physx::string::stricmp(type,"texcoord3") == 0 || physx::string::stricmp(type,"texel3") == 0 )
                    {
                        const PxF32 * scan = (const PxF32 *)src;
                        v.mTexel3[0] = scan[0];
                        v.mTexel3[1] = scan[1];
                        src+=sizeof(PxF32)*2;
                    }
                    else if ( physx::string::stricmp(type,"texcoord4") == 0 || physx::string::stricmp(type,"texel4") == 0 )
                    {
                        const PxF32 * scan = (const PxF32 *)src;
                        v.mTexel4[0] = scan[0];
                        v.mTexel4[1] = scan[1];
                        src+=sizeof(PxF32)*2;
                    }
                    else if ( physx::string::stricmp(type,"interp1") == 0 )
                    {
                        const PxF32 * scan = (const PxF32 *)src;
                        v.mInterp1[0] = scan[0];
                        v.mInterp1[1] = scan[1];
                        v.mInterp1[2] = scan[2];
                        v.mInterp1[3] = scan[3];
                        src+=sizeof(PxF32)*3;
                    }
                    else if ( physx::string::stricmp(type,"interp2") == 0 )
                    {
                        const PxF32 * scan = (const PxF32 *)src;
                        v.mInterp2[0] = scan[0];
                        v.mInterp2[1] = scan[1];
                        v.mInterp2[2] = scan[2];
                        v.mInterp2[3] = scan[3];
                        src+=sizeof(PxF32)*3;
                    }
                    else if ( physx::string::stricmp(type,"interp3") == 0 )
                    {
                        const PxF32 * scan = (const PxF32 *)src;
                        v.mInterp3[0] = scan[0];
                        v.mInterp3[1] = scan[1];
                        v.mInterp3[2] = scan[2];
                        v.mInterp3[3] = scan[3];
                        src+=sizeof(PxF32)*3;
                    }
                    else if ( physx::string::stricmp(type,"interp4") == 0 )
                    {
                        const PxF32 * scan = (const PxF32 *)src;
                        v.mInterp4[0] = scan[0];
                        v.mInterp4[1] = scan[1];
                        v.mInterp4[2] = scan[2];
                        v.mInterp4[3] = scan[3];
                        src+=sizeof(PxF32)*3;
                    }
                    else if ( physx::string::stricmp(type,"interp5") == 0 )
                    {
                        const PxF32 * scan = (const PxF32 *)src;
                        v.mInterp5[0] = scan[0];
                        v.mInterp5[1] = scan[1];
                        v.mInterp5[2] = scan[2];
                        v.mInterp5[3] = scan[3];
                        src+=sizeof(PxF32)*3;
                    }
                    else if ( physx::string::stricmp(type,"interp6") == 0 )
                    {
                        const PxF32 * scan = (const PxF32 *)src;
                        v.mInterp6[0] = scan[0];
                        v.mInterp6[1] = scan[1];
                        v.mInterp6[2] = scan[2];
                        v.mInterp6[3] = scan[3];
                        src+=sizeof(PxF32)*3;
                    }
                    else if ( physx::string::stricmp(type,"interp7") == 0 )
                    {
                        const PxF32 * scan = (const PxF32 *)src;
                        v.mInterp7[0] = scan[0];
                        v.mInterp7[1] = scan[1];
                        v.mInterp7[2] = scan[2];
                        v.mInterp7[3] = scan[3];
                        src+=sizeof(PxF32)*3;
                    }
                    else if ( physx::string::stricmp(type,"interp8") == 0 )
                    {
                        const PxF32 * scan = (const PxF32 *)src;
                        v.mInterp8[0] = scan[0];
                        v.mInterp8[1] = scan[1];
                        v.mInterp8[2] = scan[2];
                        v.mInterp8[3] = scan[3];
                        src+=sizeof(PxF32)*3;
                    }
                    else if ( physx::string::stricmp(type,"tangent") == 0 )
                    {
                        const PxF32 * scan = (const PxF32 *)src;
                        v.mTangent[0] = scan[0];
                        v.mTangent[1] = scan[1];
                        v.mTangent[2] = scan[2];
                        src+=sizeof(PxF32)*3;
                    }
                    else if ( physx::string::stricmp(type,"binormal") == 0 )
                    {
                        const PxF32 * scan = (const PxF32 *)src;
                        v.mBiNormal[0] = scan[0];
                        v.mBiNormal[1] = scan[1];
                        v.mBiNormal[2] = scan[2];
                        src+=sizeof(PxF32)*3;
                    }
                    else if ( physx::string::stricmp(type,"blendweights") == 0 || physx::string::stricmp(type,"blendweighting") == 0 || physx::string::stricmp(type,"boneweighting") == 0 )
                    {
                        const PxF32 * scan = (const PxF32 *)src;
                        v.mWeight[0] = scan[0];
                        v.mWeight[1] = scan[1];
                        v.mWeight[2] = scan[2];
                        v.mWeight[3] = scan[3];
                        src+=sizeof(PxF32)*4;
                    }
                    else if ( physx::string::stricmp(type,"blendindices") == 0 || physx::string::stricmp(type,"blendindices") == 0 )
                    {
                        const unsigned short * scan = (const unsigned short *)src;
                        v.mBone[0] = scan[0];
                        v.mBone[1] = scan[1];
                        v.mBone[2] = scan[2];
                        v.mBone[3] = scan[3];
                        if ( v.mWeight[0] == 0 )
                        {
                            v.mBone[0] = 0;
                        }
                        if ( v.mWeight[1] == 0 )
                        {
                            v.mBone[1] = 0;
                        }
                        if ( v.mWeight[2] == 0 )
                        {
                            v.mBone[2] = 0;
                        }
                        if ( v.mWeight[3] == 0 )
                        {
                            v.mBone[3] = 0;
                        }

                        assert( v.mBone[0] < 256 );
                        assert( v.mBone[1] < 256 );
                        assert( v.mBone[2] < 256 );
                        assert( v.mBone[3] < 256 );
                        src+=sizeof(unsigned short)*4;
                    }
                    else if ( physx::string::stricmp(type,"radius") == 0 )
                    {
                        const PxF32 *scan = (const PxF32 *)src;
                        v.mRadius = scan[0];
                        src+=sizeof(PxF32);
                    }
                    else
                    {
                        assert(0);
                    }

                }
                return src;
            }

            PxU32 validateSemantics(const char **a1,const char **a2,PxI32 count)
            {
                bool ret = true;

                for (PxI32 i=0; i<count; i++)
                {
                    const char *p = a1[i];
                    const char *t = a2[i];

                    if ( physx::string::stricmp(t,"position") == 0 ||
                            physx::string::stricmp(t,"normal") == 0 ||
                            physx::string::stricmp(t,"tangent") == 0 ||
                            physx::string::stricmp(t,"binormal") == 0 ||
                            physx::string::stricmp(t,"interp1") == 0 ||
                            physx::string::stricmp(t,"interp2") == 0 ||
                            physx::string::stricmp(t,"interp3") == 0 ||
                            physx::string::stricmp(t,"interp4") == 0 ||
                            physx::string::stricmp(t,"interp5") == 0 ||
                            physx::string::stricmp(t,"interp6") == 0 ||
                            physx::string::stricmp(t,"interp7") == 0 ||
                            physx::string::stricmp(t,"interp8") == 0 )
                    {
                        if ( physx::string::stricmp(p,"fff") != 0 )
                        {
                            ret = false;
                            break;
                        }
                    }
                    else if ( physx::string::stricmp(t,"color") == 0 )
                    {
                        if ( physx::string::stricmp(p,"x4") != 0 )
                        {
                            ret = false;
                            break;
                        }
                    }
                    else if ( physx::string::stricmp(t,"texel1") == 0 ||
                            physx::string::stricmp(t,"texel2") == 0 ||
                            physx::string::stricmp(t,"texel3") == 0 ||
                            physx::string::stricmp(t,"texel4") == 0 ||
                            physx::string::stricmp(t,"tecoord") == 0 ||
                            physx::string::stricmp(t,"texcoord1") == 0 ||
                            physx::string::stricmp(t,"texcoord2") == 0 ||
                            physx::string::stricmp(t,"texcoord3") == 0 ||
                            physx::string::stricmp(t,"texcoord4") == 0 )
                    {
                        if ( physx::string::stricmp(p,"ff") != 0 )
                        {
                            ret =false;
                            break;
                        }
                    }
                    else if ( physx::string::stricmp(t,"blendweights") == 0 )
                    {
                        if ( physx::string::stricmp(p,"ffff") != 0 )
                        {
                            ret = false;
                            break;
                        }
                    }
                    else if ( physx::string::stricmp(t,"blendindices") == 0 )
                    {
                        if ( physx::string::stricmp(p,"hhhh") != 0 )
                        {
                            ret = false;
                            break;
                        }
                    }
                    else if ( physx::string::stricmp(t,"radius") == 0 )
                    {
                        if ( physx::string::stricmp(p,"f") != 0 )
                        {
                            ret = false;
                            break;
                        }
                    }
                }

                PxU32 flags = 0;

                if ( ret )
                {
                    for (PxI32 i=0; i<count; i++)
                    {
                        const char *t = a2[i];

                        if ( physx::string::stricmp(t,"position") == 0 ) flags|=MIVF_POSITION;
                        else if ( physx::string::stricmp(t,"normal") == 0 )   flags|=MIVF_NORMAL;
                        else if ( physx::string::stricmp(t,"color") == 0 )    flags|=MIVF_COLOR;
                        else if ( physx::string::stricmp(t,"texel1") == 0 )   flags|=MIVF_TEXEL1;
                        else if ( physx::string::stricmp(t,"texel2") == 0 )   flags|=MIVF_TEXEL2;
                        else if ( physx::string::stricmp(t,"texel3") == 0 )   flags|=MIVF_TEXEL3;
                        else if ( physx::string::stricmp(t,"texel4") == 0 )   flags|=MIVF_TEXEL4;
                        else if ( physx::string::stricmp(t,"texcoord1") == 0 )   flags|=MIVF_TEXEL1;
                        else if ( physx::string::stricmp(t,"texcoord2") == 0 )   flags|=MIVF_TEXEL2;
                        else if ( physx::string::stricmp(t,"texcoord3") == 0 )   flags|=MIVF_TEXEL3;
                        else if ( physx::string::stricmp(t,"texcoord4") == 0 )   flags|=MIVF_TEXEL4;
                        else if ( physx::string::stricmp(t,"texcoord") == 0 )
                        {
                            if ( flags & MIVF_TEXEL1 )
                                flags|=MIVF_TEXEL2;
                            else
                                flags|=MIVF_TEXEL1;
                        }
                        else if ( physx::string::stricmp(t,"tangent") == 0) flags|=MIVF_TANGENT;
                        else if ( physx::string::stricmp(t,"interp1") == 0) flags|=MIVF_INTERP1;
                        else if ( physx::string::stricmp(t,"interp2") == 0) flags|=MIVF_INTERP2;
                        else if ( physx::string::stricmp(t,"interp3") == 0) flags|=MIVF_INTERP3;
                        else if ( physx::string::stricmp(t,"interp4") == 0) flags|=MIVF_INTERP4;
                        else if ( physx::string::stricmp(t,"interp5") == 0) flags|=MIVF_INTERP5;
                        else if ( physx::string::stricmp(t,"interp6") == 0) flags|=MIVF_INTERP6;
                        else if ( physx::string::stricmp(t,"interp7") == 0) flags|=MIVF_INTERP7;
                        else if ( physx::string::stricmp(t,"interp8") == 0) flags|=MIVF_INTERP8;
                        else if ( physx::string::stricmp(t,"binormal") == 0 ) flags|=MIVF_BINORMAL;
                        else if ( physx::string::stricmp(t,"blendweights") == 0 ) flags|=MIVF_BONE_WEIGHTING;
                        else if ( physx::string::stricmp(t,"blendindices") == 0 ) flags|=MIVF_BONE_WEIGHTING;
                        else if ( physx::string::stricmp(t,"boneweights") == 0 ) flags|=MIVF_BONE_WEIGHTING;
                        else if ( physx::string::stricmp(t,"boneindices") == 0 ) flags|=MIVF_BONE_WEIGHTING;
                        else if ( physx::string::stricmp(t,"radius") == 0 ) flags|=MIVF_RADIUS;
                    }
                }

                return flags;
            }


            virtual const char * getExtension(PxI32 index)  // report the default file name extension for this mesh type.
            {
                return ".ezm";
            }

            virtual const char * getDescription(PxI32 index)
            {
                return "PhysX Rocket EZ-Mesh format";
            }

            virtual bool  importMesh(const char *meshName,const void *data,PxU32 dlen,MeshImportInterface *callback,const char *options,MeshImportApplicationResource *appResource)
            {
                bool ret = false;

                mCallback = callback;

                if ( data && mCallback )
                {
					FAST_XML::FastXml *f = FAST_XML::createFastXml(this);
					PxMemoryBuffer mb(data,dlen);
                    bool ok = f->processXml(mb);
                    if ( ok )
                    {
                        mCallback->importAssetName(mAssetName.Get(), mAssetInfo.Get());
                        ret = true;
                    }
                    if ( mAnimation )
                    {
                        mCallback->importAnimation(*mAnimation);
                        for (PxI32 i=0; i<mAnimation->mTrackCount; i++)
                        {
                            MeshAnimTrack *t = mAnimation->mTracks[i];
                            delete []t->mPose;
                            delete t;
                        }
                        PX_FREE(mAnimation->mTracks);
                        delete mAnimation;
                        mAnimation = 0;
                    }

                    delete mMeshCollisionRepresentation;
                    delete mMeshCollision;
                    delete mMeshCollisionConvex;
                    mMeshCollisionRepresentation = 0;
                    mMeshCollision = 0;
                    mMeshCollisionConvex = 0;

                    f->release();

                }

                return ret;
            }

            void ProcessNode(const char *svalue)
            {
                mType = (NodeType)mToElement.Get(svalue);
                switch ( mType )
                {
                    case NT_NONE:
                        assert(0);
                        break;
                    case NT_MESH_COLLISION_REPRESENTATION:
                        delete mMeshCollisionRepresentation;
                        mMeshCollisionRepresentation = IMP_NEW(MeshCollisionRepresentation);
                        break;
                    case NT_MESH_COLLISION:
                        if ( mMeshCollisionRepresentation )
                        {
                            mCallback->importCollisionRepresentation( mMeshCollisionRepresentation->mName, mMeshCollisionRepresentation->mInfo );
                            mCollisionRepName = SGET(mMeshCollisionRepresentation->mName);
                            delete mMeshCollisionRepresentation;
                            mMeshCollisionRepresentation = 0;
                        }
                        delete mMeshCollision;
                        mMeshCollision = IMP_NEW(MeshCollision);
                        break;
                    case NT_MESH_COLLISION_CONVEX:
                        assert(mMeshCollision);
                        if ( mMeshCollision )
                        {
                            delete mMeshCollisionConvex;
                            mMeshCollisionConvex = IMP_NEW(MeshCollisionConvex);
                            MeshCollision *d = static_cast< MeshCollision *>(mMeshCollisionConvex);
                            *d = *mMeshCollision;
                            delete mMeshCollision;
                            mMeshCollision = 0;
                        }
                        break;
                    case NT_ANIMATION:
                        if ( mAnimation )
                        {
                            mCallback->importAnimation(*mAnimation);
                            delete mAnimation;
                            mAnimation = 0;
                        }
                        mName       = 0;
                        mFrameCount = 0;
                        mDuration   = 0;
                        mTrackCount = 0;
                        mDtime      = 0;
                        mTrackIndex = 0;
                        break;
                    case NT_ANIM_TRACK:
                        if ( mAnimation == 0 )
                        {
                            if ( mName && mFrameCount && mDuration && mTrackCount && mDtime )
                            {
                                PxI32 framecount = atoi( mFrameCount );
                                PxF32 duration = (PxF32) atof( mDuration );
                                PxI32 trackcount = atoi(mTrackCount);
                                PxF32 dtime = (PxF32) atof(mDtime);
                                if ( trackcount >= 1 && framecount >= 1 )
                                {
                                    mAnimation = IMP_NEW(MeshAnimation);
                                    mAnimation->mName = mName;
                                    mAnimation->mTrackCount = trackcount;
                                    mAnimation->mFrameCount = framecount;
                                    mAnimation->mDuration = duration;
                                    mAnimation->mDtime = dtime;
                                    mAnimation->mTracks = (MeshAnimTrack **)PX_ALLOC(sizeof(MeshAnimTrack *)*mAnimation->mTrackCount);
                                    for (PxI32 i=0; i<mAnimation->mTrackCount; i++)
                                    {
                                        MeshAnimTrack *track = IMP_NEW(MeshAnimTrack);
                                        track->mDtime = mAnimation->mDuration;
                                        track->mFrameCount = mAnimation->mFrameCount;
                                        track->mDuration = mAnimation->mDuration;
                                        track->mPose = IMP_NEW(MeshAnimPose)[track->mFrameCount];
                                        mAnimation->mTracks[i] = track;
                                    }
                                }
                            }
                        }
                        if ( mAnimation )
                        {
                            mAnimTrack = mAnimation->GetTrack(mTrackIndex);
                            mTrackIndex++;
                        }
                        break;
                    case NT_SKELETON:
                        {
                            delete mSkeleton;
                            mSkeleton = IMP_NEW(MeshSkeleton);
                        }
                    case NT_BONE:
                        if ( mSkeleton )
                        {
                            mBone = mSkeleton->GetBonePtr(mBoneIndex);
                        }
                        break;
                }
            }
            void ProcessData(const char *svalue)
            {
                if ( svalue )
                {
                    switch ( mType )
                    {
                        case NT_ANIM_TRACK:
                            if ( mAnimTrack )
                            {
                                mAnimTrack->SetName(mStrings.Get(mName).Get());
                                PxI32 count = atoi( mCount );
                                if ( count == mAnimTrack->GetFrameCount() )
                                {
                                    if ( mHasScale )
                                    {
                                        PxF32 *buff = (PxF32 *) PX_ALLOC(sizeof(PxF32)*10*count);
                                        Asc2Bin(svalue, count, "fff ffff fff", buff );
                                        for (PxI32 i=0; i<count; i++)
                                        {
                                            MeshAnimPose *p = mAnimTrack->GetPose(i);
                                            const PxF32 *src = &buff[i*10];

                                            p->mPos[0]  = src[0];
                                            p->mPos[1]  = src[1];
                                            p->mPos[2]  = src[2];

                                            p->mQuat[0] = src[3];
                                            p->mQuat[1] = src[4];
                                            p->mQuat[2] = src[5];
                                            p->mQuat[3] = src[6];

                                            p->mScale[0] = src[7];
                                            p->mScale[1] = src[8];
                                            p->mScale[2] = src[9];
                                        }
                                    }
                                    else
                                    {
                                        PxF32 *buff = (PxF32 *) PX_ALLOC(sizeof(PxF32)*7*count);
                                        Asc2Bin(svalue, count, "fff ffff", buff );
                                        for (PxI32 i=0; i<count; i++)
                                        {
                                            MeshAnimPose *p = mAnimTrack->GetPose(i);
                                            const PxF32 *src = &buff[i*7];

                                            p->mPos[0]  = src[0];
                                            p->mPos[1]  = src[1];
                                            p->mPos[2]  = src[2];

                                            p->mQuat[0] = src[3];
                                            p->mQuat[1] = src[4];
                                            p->mQuat[2] = src[5];
                                            p->mQuat[3] = src[6];

                                            p->mScale[0] = 1;
                                            p->mScale[1] = 1;
                                            p->mScale[2] = 1;
                                        }
                                    }
                                }
                            }
                            break;
                        case NT_NODE_INSTANCE:
#if 0 // TODO TODO
                            if ( mName )
                            {
                                PxF32 transform[4*4];
                                Asc2Bin(svalue, 4, "ffff", transform );
                                MeshBone b;
                                b.SetTransform(transform);
                                PxF32 pos[3];
                                PxF32 quat[3];
                                PxF32 scale[3] = { 1, 1, 1 };
                                b.ExtractOrientation(quat);
                                b.GetPos(pos);
                                mCallback->importMeshInstance(mName,pos,quat,scale);
                                mName = 0;
                            }
#endif
                            break;
                        case NT_NODE_TRIANGLE:
                            if ( mCtype && mSemantic )
                            {
                                PxI32 c1,c2;
                                char scratch1[2048];
                                char scratch2[2048];
                                strcpy(scratch1,mCtype);
                                strcpy(scratch2,mSemantic);
                                const char **a1 = mParser1.GetArglist(scratch1,c1);
                                const char **a2 = mParser2.GetArglist(scratch2,c2);
                                if ( c1 > 0 && c2 > 0 && c1 == c2 )
                                {
                                    mVertexFlags = validateSemantics(a1,a2,c1);
                                    if ( mVertexFlags )
                                    {
                                        MeshVertex vtx[3];
                                        const PxU8 *temp = (const PxU8 *)Asc2Bin(svalue, 3, mCtype, 0 );
                                        temp = getVertex(temp,vtx[0],a2,c2);
                                        temp = getVertex(temp,vtx[1],a2,c2);
                                        temp = getVertex(temp,vtx[2],a2,c2);
                                        mCallback->importTriangle(mCurrentMesh.Get(),mCurrentMaterial.Get(),mVertexFlags,vtx[0],vtx[1],vtx[2]);
                                        PX_FREE((void *)temp);
                                    }
                                }
                                mCtype = 0;
                                mSemantic = 0;
                            }
                            break;
                        case NT_VERTEX_BUFFER:
                            PX_FREE( mVertexBuffer);
                            delete []mVertices;
                            mVertices = 0;
                            mVertexCount = 0;
                            mVertexBuffer = 0;

                            if ( mCtype && mCount )
                            {
                                mVertexCount  = atoi(mCount);
                                if ( mVertexCount > 0 )
                                {
                                    mVertexBuffer = Asc2Bin(svalue, mVertexCount, mCtype, 0 );

                                    if ( mVertexBuffer )
                                    {

                                        PxI32 c1,c2;
                                        char scratch1[2048];
                                        char scratch2[2048];
                                        strcpy(scratch1,mCtype);
                                        strcpy(scratch2,mSemantic);

                                        const char **a1 = mParser1.GetArglist(scratch1,c1);
                                        const char **a2 = mParser2.GetArglist(scratch2,c2);

                                        if ( c1 > 0 && c2 > 0 && c1 == c2 )
                                        {
                                            mVertexFlags = validateSemantics(a1,a2,c1);
                                            if ( mVertexFlags )
                                            {
                                                mVertices = IMP_NEW(MeshVertex)[mVertexCount];
                                                const PxU8 *scan = (const PxU8 *)mVertexBuffer;
                                                for (PxI32 i=0; i<mVertexCount; i++)
                                                {
                                                    scan = getVertex(scan,mVertices[i],a2,c2);
                                                }
                                            }
                                        }
                                        PX_FREE( mVertexBuffer);
                                        mVertexBuffer = 0;
                                    }

                                }
                                mCtype = 0;
                                mCount = 0;
                            }
                            break;
                        case NT_INDEX_BUFFER:
                            if ( mCount )
                            {
                                mIndexCount = atoi(mCount);
                                if ( mIndexCount > 0 )
                                {
                                    mIndexBuffer = Asc2Bin(svalue, mIndexCount, "ddd", 0 );
                                }
                            }

                            if ( mIndexBuffer && mVertices )
                            {
                                if ( mMeshCollisionConvex )
                                {
                                    PxF32 *vertices = (PxF32 *)PX_ALLOC(sizeof(PxF32)*mVertexCount*3);
                                    PxF32 *dest = vertices;
                                    for (PxI32 i=0; i<mVertexCount; i++)
                                    {
                                        dest[0] = mVertices[i].mPos[0];
                                        dest[1] = mVertices[i].mPos[1];
                                        dest[2] = mVertices[i].mPos[2];
                                        dest+=3;
                                    }

                                    mCallback->importConvexHull(mCollisionRepName.Get(),
                                            mMeshCollisionConvex->mName,
                                            mMeshCollisionConvex->mTransform,
                                            mVertexCount,
                                            vertices,
                                            mIndexCount,
                                            (const PxU32 *)mIndexBuffer);

                                    delete []vertices;
                                    delete mMeshCollisionConvex;
                                    mMeshCollisionConvex = 0;
                                }
                                else
                                {
                                    mCallback->importIndexedTriangleList(mCurrentMesh.Get(),mCurrentMaterial.Get(),mVertexFlags,mVertexCount,mVertices,mIndexCount,(const PxU32 *)mIndexBuffer );
                                }
                            }


                            PX_FREE( mIndexBuffer);
                            mIndexBuffer = 0;
                            mIndexCount = 0;
                            break;
                    }
                }
            }

            void ProcessAttribute(const char *aname,  // the name of the attribute
                    const char *savalue) // the value of the attribute
            {
                AttributeType attrib = (AttributeType) mToAttribute.Get(aname);
                switch ( attrib )
                {
                    case AT_NONE:
                        assert(0);
                        break;
                    case AT_MESH_SYSTEM_VERSION:
                        mMeshSystemVersion = atoi(savalue);
                        break;
                    case AT_MESH_SYSTEM_ASSET_VERSION:
                        mMeshSystemAssetVersion = atoi(savalue);
                        break;
                    case AT_ASSET_NAME:
                        mAssetName = mStrings.Get(savalue);
                        break;
                    case AT_ASSET_INFO:
                        mAssetInfo = mStrings.Get(savalue);
                        break;
                    case AT_SCALE:
                        if ( mType == NT_BONE && mBone )
                        {
                            Asc2Bin(savalue,1,"fff", mBone->mScale );
                        }
                        break;
                    case AT_POSITION:
                        if ( mType == NT_BONE && mBone )
                        {
                            Asc2Bin(savalue,1,"fff", mBone->mPosition );
                            mBoneIndex++;
                            if ( mBoneIndex == mSkeleton->GetBoneCount() )
                            {
                                mCallback->importSkeleton(*mSkeleton);
                                delete mSkeleton;
                                mSkeleton = 0;
                                mBoneIndex = 0;
                            }
                        }
                        break;
                    case AT_ORIENTATION:
                        if ( mType == NT_BONE && mBone )
                        {
                            Asc2Bin(savalue,1,"ffff", mBone->mOrientation );
                        }
                        break;
                    case AT_HAS_SCALE:
                        mHasScale = getBool(savalue);
                        break;
                    case AT_DURATION:
                        mDuration = savalue;
                        break;

                    case AT_DTIME:
                        mDtime = savalue;
                        break;

                    case AT_TRACK_COUNT:
                        mTrackCount = savalue;
                        break;

                    case AT_FRAME_COUNT:
                        mFrameCount = savalue;
                        break;
                    case AT_INFO:
                        switch ( mType )
                        {
                            case NT_MESH_COLLISION_REPRESENTATION:
                                assert(mMeshCollisionRepresentation);
                                if ( mMeshCollisionRepresentation )
                                {
                                    mMeshCollisionRepresentation->mInfo = mStrings.Get(savalue).Get();
                                }
                                break;
                        }
                        break;
                    case AT_TRANSFORM:
                        if (mType == NT_MESH_COLLISION )
                        {
                            assert( mMeshCollision );
                            if ( mMeshCollision )
                            {
                                Asc2Bin(savalue,4,"ffff", mMeshCollision->mTransform );
                            }
                        }
                        break;

                    case AT_NAME:
                        mName = savalue;

                        switch ( mType )
                        {
                            case NT_MESH_COLLISION:
                                assert( mMeshCollision );
                                if ( mMeshCollision )
                                {
                                    mMeshCollision->mName = mStrings.Get(savalue).Get();
                                }
                                break;
                            case NT_MESH_COLLISION_REPRESENTATION:
                                assert(mMeshCollisionRepresentation);
                                if ( mMeshCollisionRepresentation )
                                {
                                    mMeshCollisionRepresentation->mName = mStrings.Get(savalue).Get();
                                }
                                break;
                            case NT_MESH:
                                mCurrentMesh = mStrings.Get(savalue);
                                mCallback->importMesh(savalue,0);
                                break;
                            case NT_SKELETON:
                                if ( mSkeleton )
                                {
                                    mSkeleton->SetName(mStrings.Get(savalue).Get());
                                }
                                break;
                            case NT_BONE:
                                if ( mBone )
                                {
                                    mBone->SetName(mStrings.Get(savalue).Get());
                                }
                                break;
                        }
                        break;
                    case AT_TRIANGLE_COUNT:
                        mCount = savalue;
                        break;
                    case AT_COUNT:
                        mCount = savalue;
                        if ( mType == NT_SKELETON )
                        {
                            if ( mSkeleton )
                            {
                                PxI32 count = atoi( savalue );
                                if ( count > 0 )
                                {
                                    MeshBone *bones;
                                    bones = IMP_NEW(MeshBone)[count];
                                    mSkeleton->SetBones(count,bones);
                                    mBoneIndex = 0;
                                }
                            }
                        }
                        break;
                    case AT_PARENT:
                        mParent = savalue;
                        if ( mBone )
                        {
                            for (PxI32 i=0; i<mBoneIndex; i++)
                            {
                                const MeshBone &b = mSkeleton->GetBone(i);
                                if ( strcmp(mParent,b.mName) == 0 )
                                {
                                    mBone->mParentIndex = i;
                                    break;
                                }
                            }
                        }
                        break;
                    case AT_MATERIAL:
                        if ( mType == NT_MESH_SECTION )
                        {
                            mCallback->importMaterial(savalue,0);
                            mCurrentMaterial = mStrings.Get(savalue);
                        }
                        break;
                    case AT_CTYPE:
                        mCtype = savalue;
                        break;
                    case AT_SEMANTIC:
                        mSemantic = savalue;
                        break;
                }

            }

			virtual bool processComment(const char *comment) // encountered a comment in the XML
			{
				return true;
			}

			virtual void *  fastxml_malloc(physx::PxU32 size)
			{
				return PX_ALLOC(size);
			}

			virtual void	fastxml_free(void *mem) 
			{
				PX_FREE(mem);
			}

			virtual bool processClose(const char *element,physx::PxU32 depth,bool &isError)	  // process the 'close' indicator for a previously encountered element
			{
				return true;
			}

            virtual bool processElement(const char *elementName,         // name of the element
                    PxI32         argc,                // number of attributes
                    const char **argv,               // list of attributes.
                    const char  *elementData,        // element data, null if none
                    PxI32         lineno)         // line number in the source XML file
            {
                ProcessNode(elementName);
                PxI32 acount = argc/2;
                for (PxI32 i=0; i<acount; i++)
                {
                    const char *key = argv[i*2];
                    const char *value = argv[i*2+1];
                    ProcessAttribute(key,value);
                }
                ProcessData(elementData);
                return true;
            }




        private:
            MeshImportInterface     *mCallback;

            bool                   mHasScale;

            StringTableInt         mToElement;         // convert string to element enumeration.
            StringTableInt         mToAttribute;       // convert string to attribute enumeration
            NodeType               mType;

            InPlaceParser mParser1;
            InPlaceParser mParser2;

            const char * mName;
            const char * mCount;
            const char * mParent;
            const char * mCtype;
            const char * mSemantic;

            const char * mFrameCount;
            const char * mDuration;
            const char * mTrackCount;
            const char * mDtime;

            PxI32          mTrackIndex;
            PxI32          mBoneIndex;
            MeshBone       * mBone;

            MeshAnimation  * mAnimation;
            MeshAnimTrack  * mAnimTrack;
            MeshSkeleton   * mSkeleton;
            PxI32          mVertexCount;
            PxI32          mIndexCount;
            void       * mVertexBuffer;
            void       * mIndexBuffer;

            PxI32          mMeshSystemVersion;
            PxI32          mMeshSystemAssetVersion;

            StringRef    mCurrentMesh;
            StringRef    mCurrentMaterial;
            StringDict   mStrings;
            StringRef    mAssetName;
            StringRef    mAssetInfo;

            PxU32 mVertexFlags;
            MeshVertex  *mVertices;

            StringRef                    mCollisionRepName;
            MeshCollisionRepresentation *mMeshCollisionRepresentation;
            MeshCollision               *mMeshCollision;
            MeshCollisionConvex         *mMeshCollisionConvex;


    };


    MeshImporter * createMeshImportEZM(void)
    {
        MeshImportEZM *m = PX_NEW(MeshImportEZM);
        return static_cast< MeshImporter *>(m);
    }

    void         releaseMeshImportEZM(MeshImporter *iface)
    {
        MeshImportEZM *m = static_cast< MeshImportEZM *>(iface);
        PX_DELETE(m);
    }

}; // end of namepsace
};
