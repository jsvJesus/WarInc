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
#ifndef TRIANGLE_MESH_H
#define TRIANGLE_MESH_H


#include <NxUserRenderResourceManager.h>

#include <vector>
#include <string>

#include "FastXml.h"
#include "MeshImport.h"

namespace MESHIMPORT
{
  class MeshSystemContainer;
};



namespace physx
{
	namespace apex
	{
		class NxClothingIsoMesh;
		class NxClothingPhysicalMesh;
		class NxRenderMeshAssetAuthoring;

		class NxResourceCallback;

		class NxUserRenderer;
		class NxUserRenderResourceManager;
		class NxUserRenderResource;
		class NxUserRenderVertexBuffer;
		class NxUserRenderIndexBuffer;
		class NxUserRenderBoneBuffer;
	}
}





namespace Samples
{

class SkeletalAnim;

enum PaintChannelType
{
	PC_MAX_DISTANCE,
	PC_COLLISION_DISTANCE,
	PC_PHYSICAL,
	PC_NUM_CHANNELS,
};

struct MaterialResource
{
	physx::PxVec3 color;
	physx::PxU32 handle;
	bool hasAlpha;
	std::string name;
};

//------------------------------------------------------------------------------------
struct TriangleSubMesh
{
	void init() {
		name = ""; textureFile = ""; materialName = ""; originalMaterialName = ""; firstIndex = (physx::PxU32)-1; numIndices = 0; color = 0xfefefeff;
		materialResource = NULL; mRenderResource = NULL; cullMode = physx::NxRenderCullMode::NONE;
		show = true; selected = false; selectionActivated = false; usedForCollision = false; hasApexAsset = false; invisible = false;
		resourceNeedsUpdate = false;
	}
	std::string name;
	std::string textureFile;
	std::string materialName;
	std::string originalMaterialName;
	unsigned int firstIndex;
	unsigned int numIndices;
	unsigned int color;
	MaterialResource* materialResource;
	physx::apex::NxUserRenderResource* mRenderResource;
	physx::apex::NxRenderCullMode::Enum cullMode;
	bool show;
	bool selected;
	bool selectionActivated;
	bool usedForCollision;
	bool hasApexAsset;
	bool invisible;
	bool resourceNeedsUpdate;
	bool operator<(const TriangleSubMesh& other) const
	{
		if (name == other.name)
			return materialName < other.materialName;
		return name < other.name;
	}
};

// ----------------------------------------------------------------------
struct TriangleMeshMaterial
{
	void init();
	std::string name;
	std::string textureFile;

	float ambient[3];
	float diffuse[3];
	float specular[3];
	float shininess;
	float alpha;
};

// ----------------------------------------------------------------------
struct float4
{
	float4() {}
	float4(float R, float G, float B, float A) : r(R), g(G), b(B), a(A) {}
	float r;
	float g;
	float b;
	float a;
};

//------------------------------------------------------------------------------------
struct TriangleEdgeSplit		// for mesh subdivision
{
	int adjVertNr;
	int newVertNr;
	int next;
};

//------------------------------------------------------------------------------------
struct PaintedVertex
{
	explicit PaintedVertex() : paintValue(0.0f), color(0) {}
	explicit PaintedVertex(float v) : paintValue(v), color(0) {}

	void setColor(float r, float g, float b)
	{
		union
		{
			unsigned int ucolor;
			unsigned char ccolor[4];
		};
		ccolor[0] = (unsigned char)(r * 255);
		ccolor[1] = (unsigned char)(g * 255);
		ccolor[2] = (unsigned char)(b * 255);
		ccolor[3] = 0xff;
		color = ucolor;
	}
	float paintValue;
	unsigned int color;
};

//------------------------------------------------------------------------------------
class TriangleMesh : public FAST_XML::FastXml::Callback {
public:
	TriangleMesh(physx::PxU32 moduleIdentifier);
	virtual ~TriangleMesh();
	void clear(physx::NxUserRenderResourceManager* rrm, physx::NxResourceCallback* rcb);
	void loadMaterials(physx::NxResourceCallback* resourceCallback, bool dummyMaterial = false, const char* materialPrefix = NULL, const char* materialSuffix = NULL, bool onlyVisibleMaterials = false);
	void moveAboveGround(physx::PxF32 level);
	void initSingleMesh();

	void copyFrom(const TriangleMesh &mesh);
	void copyFromSubMesh(const TriangleMesh &mesh, int subMeshNr = -1, bool filterVertices = false);

	bool loadFromObjFile(const std::string& filename, bool useCustomChannels);
	bool saveToObjFile(const std::string& filename) const;
	bool loadFromXML(const std::string& filename, bool loadCustomChannels);
	bool saveToXML(const std::string& filename);
	bool loadFromParent(TriangleMesh* parent);
	bool loadFromMeshImport(physx::MeshSystemContainer *msc, bool useCustomChannels);

	void initPlane(float length, float uvDist, const char* materialName);
	void initFrom(physx::apex::NxClothingIsoMesh& mesh);
	void initFrom(physx::apex::NxClothingPhysicalMesh& mesh, bool initCustomChannels);
	void initFrom(physx::apex::NxRenderMeshAssetAuthoring& mesh, bool initCustomChannels);

	void drawPainting(PaintChannelType channelType, bool skinned, physx::apex::NxApexRenderDebug* batcher);
	void drawVertices(PaintChannelType channelType, float maxDistanceScaling, float collisionDistanceScaling, float pointScaling,
		float vmin, float vmax, physx::apex::NxApexRenderDebug* batcher);
	void drawVertices(size_t boneNr, float minWeight, float pointScaling, physx::apex::NxApexRenderDebug* batcher) const;
	void drawVertices(float normalScale, bool activeVerticesOnly, physx::apex::NxApexRenderDebug* batcher) const;
	void drawMaxDistancePartitions(float paintingScale, const float* partitions, size_t numPartitions, physx::apex::NxApexRenderDebug* batcher);
	void drawTetrahedrons(bool wireframe, float scale, physx::apex::NxApexRenderDebug* batcher);

	void updateRenderResources(bool rewriteBuffers, physx::apex::NxUserRenderResourceManager& rrm, void * userRenderData = 0);
	void dispatchRenderResources(physx::apex::NxUserRenderer& r, const physx::PxMat34Legacy& currentPose);

	void skin(const SkeletalAnim &anim);
	void unskin();

	enum
	{
		NUM_TEXCOORDS = 4,
	};

	// accessors
	size_t getNumSubmeshes() const { return mSubMeshes.size(); }
	const TriangleSubMesh* getSubMesh(size_t i) const { if (i < mSubMeshes.size()) return &mSubMeshes[i]; return NULL; }

	void showSubmesh(int index, bool on);
	void selectSubMesh(int index, bool selected);
	void selectSubMesh(const char* submeshName, const char* materialName, bool select);
	size_t getNumTriangles(int subMeshNr) const;
	size_t getNumIndices() const { return mIndices.size(); }
	size_t getNumVertices() const { return mVertices.size(); }
	const std::vector<physx::PxVec3> &getVertices() const { return mVertices; }
	const std::vector<physx::PxVec3> &getNormals() const { return mNormals; }
	const std::vector<physx::PxVec3> &getTangents() const { return mTangents; }
	const std::vector<physx::PxVec3> &getBitangents() const { return mBitangents; }
	std::vector<PaintedVertex> &getPaintChannel(PaintChannelType channelType);
	const std::vector<PaintedVertex> &getPaintChannel(PaintChannelType channelType) const;
	float getPaintChannelMaxValue(PaintChannelType channelType) const;

	const std::vector<physx::NxVertexUV> &getTexCoords(int index) const { assert(index >= 0); assert(index < NUM_TEXCOORDS); return mTexCoords[index]; }
	const std::vector<unsigned short>  &getBoneIndices() const { return mBoneIndices; }
	const std::vector<physx::PxVec4> &getBoneWeights() const { return mBoneWeights; }

	const std::vector<unsigned int> &getIndices() const { return mIndices; }

	const std::vector<bool>& getActiveSubmeshVertices() const { return mActiveSubmeshVertices; }

	void getBounds(physx::PxBounds3 &bounds) const { bounds = mBounds; }

	int getBoneAssignments(physx::PxU32 vertNr, const physx::PxU16* &bones, const physx::PxF32* &weights) const;
	bool hasBoneAssignments() const { return mBoneIndices.size() == mVertices.size() * 4; }

	int getMaxBoneIndex() { return mMaxBoneIndex; }
	// manipulators

	void displaceAlongNormal(float displacement);

	void updateBounds();

	void setSubMeshColor(int subMeshNr, physx::PxU32 color);
	void setSubMeshMaterialName(int subMeshNr, const char* materialName, physx::NxResourceCallback* resourceCallback);
	void setSubMeshUsedForCollision(int subMeshNr, bool enable);
	void setSubMeshHasPhysics(int subMeshNr, bool enable);
	void setAllColors(physx::PxU32 color);

	void subdivideSubMesh(int subMeshNr, int subdivision, bool evenOutVertexDegrees);
	void evenOutVertexDegree(int subMeshNr, int numIters);

	void setCullMode(physx::NxRenderCullMode::Enum cullMode, physx::PxI32 submeshIndex);

	void updatePaintingColors(PaintChannelType channelType, float maxDistMin, float maxDistMax, physx::apex::NxApexRenderDebug* batcher);

	virtual bool processElement(const char *elementName, int argc, const char **argv, const char *elementData, int lineno);
	virtual bool processComment(const char *comment) // encountered a comment in the XML
	{
		PX_FORCE_PARAMETER_REFERENCE(comment);
		return true;
	}

	virtual bool processClose(const char *element,physx::PxU32 depth,bool &isError)	  // process the 'close' indicator for a previously encountered element
	{
		PX_FORCE_PARAMETER_REFERENCE(element);
		PX_FORCE_PARAMETER_REFERENCE(depth);
		isError = false;
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


	void setTextureUVOrigin(physx::NxTextureUVOrigin::Enum origin) { textureUvOriginChanged |= mTextureUVOrigin != origin; mTextureUVOrigin = origin; }
	physx::NxTextureUVOrigin::Enum getTextureUVOrigin() const { return mTextureUVOrigin; }

	bool hasSkinningVertices();
private:
	void operator=(const TriangleMesh& other) { *this = other; /* empty */ }
	void updateNormals(int subMeshNr);
	void updateTangents(int subMeshNr);
	void updateBoneWeights();
	void complete(bool useCustomChannels);
	bool importMtlFile(std::string filename, std::string path, std::vector<TriangleMeshMaterial> &materials);
	//void drawSubMesh(int nr, const physx::PxMat34Legacy* skinningMatrices);
	void hasRandomColors(size_t howmany);

	void updateSubmeshInfo();

	enum ParserState
	{
		PS_Uninitialized,
		PS_Mesh,
		PS_Submeshes,
		PS_Skeleton,
	};
	ParserState mParserState;

	// vertices
	physx::NxUserRenderVertexBuffer* mDynamicVertexBuffer;
	physx::NxUserRenderVertexBuffer* mStaticVertexBuffer;
	physx::NxUserRenderIndexBuffer* mIndexBuffer;
	physx::NxUserRenderBoneBuffer* mBoneBuffer;

	std::vector<physx::PxVec3> mVertices;
	std::vector<physx::PxVec3> mNormals;
	std::vector<physx::PxVec3> mTangents;
	std::vector<physx::PxVec3> mBitangents;

	std::vector<PaintedVertex>	mPaintChannels[PC_NUM_CHANNELS];

	std::vector<physx::NxVertexUV> mTexCoords[NUM_TEXCOORDS];

	// triangles
	std::vector<TriangleSubMesh> mSubMeshes;
	std::vector<unsigned int> mIndices;

	// skeleton binding
	std::string mSkeletonFile;
	std::vector<unsigned short> mBoneIndices;
	std::vector<physx::PxVec4> mBoneWeights;
	std::vector<physx::PxU32> mNumBoneWeights;

	std::vector<physx::PxVec3> mSkinnedVertices;
	std::vector<physx::PxVec3> mSkinnedNormals;

	std::vector<physx::PxMat34Legacy> mSkinningMatrices; // PH: VERY CAREFUL WHEN CHANGING THIS!!! The bone buffer doesn't validate types in writeBuffer calls!

	// others
	std::string mName;
	physx::PxBounds3 mBounds;

	physx::PxI32 mMaxBoneIndex;

	// submesh info for per-vertex drawing
	std::vector<bool> mActiveSubmeshVertices;

	// temporary, used in painting
	std::vector<physx::PxI32> mTriangleMarks;
	std::vector<physx::PxI32> mVertexMarks;
	int mNextMark;

	TriangleMesh* mParent;

	// temporary, subdivision data structures
	int addSplitVert(int vertNr0, int vertNr1);
	std::vector<physx::PxI32> mVertexFirstSplit;
	std::vector<TriangleEdgeSplit> mVertexSplits;

	bool vertexValuesChangedDynamic;
	bool vertexValuesChangedStatic;
	bool vertexCountChanged;
	bool indicesChanged;
	bool skinningMatricesChanged;
	bool oneCullModeChanged;
	bool needSkinningVertices;
	bool textureUvOriginChanged;

	std::vector<physx::PxU32> mRandomColors;

	std::string mMaterialPrefix;
	std::string mMaterialSuffix;

	physx::NxTextureUVOrigin::Enum mTextureUVOrigin;
};

} // namespace Samples

#endif
