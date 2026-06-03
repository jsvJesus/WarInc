#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <map>

struct WI_Vec2
{
    float x;
    float y;
};

struct WI_Vec3
{
    float x;
    float y;
    float z;
};

struct WI_Quat
{
    float x;
    float y;
    float z;
    float w;
};

struct WI_Mat4
{
    float m[4][4];
};

struct WI_Face
{
    int v[3];
    WI_Vec2 uv[3];
    std::string material;
};

struct WI_Material
{
    std::string name;
    std::string imagesDir;
    std::string diffuse;
    std::string normal;
    std::string specular;
    std::string glow;
    std::string env;
    std::string detailNormal;
    std::string density;
    std::string camoMask;
    std::string distortion;
    std::string specPow;
    bool doubleSided;
    bool transparent;
    float specularPower;
    float reflectionPower;

    WI_Material()
    {
        doubleSided = false;
        transparent = false;
        specularPower = 0.0f;
        reflectionPower = 0.0f;
    }
};

struct WI_Mesh
{
    std::string sourceFile;
    std::string name;
    WI_Vec3 pivot;
    std::vector<WI_Vec3> positions;
    std::vector<WI_Vec3> normals;
    std::vector<WI_Vec3> tangents;
    std::vector<unsigned char> tangentSigns;
    std::vector<WI_Face> faces;
    std::vector<std::string> materialOrder;
    std::map<std::string, WI_Material> materials;
};

struct WI_VertexWeight
{
    unsigned char boneId[4];
    float weight[4];
};

struct WI_Weights
{
    unsigned int skeletonId;
    unsigned int vertexCount;
    std::vector<WI_VertexWeight> vertices;
};

struct WI_Bone
{
    std::string name;
    int parentId;
    float length;
    WI_Mat4 absPlacement;
};

struct WI_Skeleton
{
    std::string sourceFile;
    unsigned int skeletonId;
    std::vector<WI_Bone> bones;
};

struct WI_AnimFrame
{
    WI_Quat q;
    WI_Vec3 v;
};

struct WI_AnimTrack
{
    std::string boneName;
    unsigned int flags;
    std::vector<WI_AnimFrame> frames;
};

struct WI_Animation
{
    std::string sourceFile;
    unsigned int skeletonId;
    unsigned int frameRate;
    unsigned int numFrames;
    std::vector<WI_AnimTrack> tracks;
};

class WI_ProjectIndex
{
public:
    bool Build(const std::string& rootDir);
    std::string Root() const;
    std::string FindFile(const std::string& nameOrPath, const std::string& forcedExt, const std::string& nearFile) const;
    std::string FindMaterialFile(const std::string& materialName, const std::string& meshFile) const;
    std::string FindTextureFile(const std::string& textureName, const std::string& imagesDir, const std::string& materialFile) const;

private:
    std::string m_root;
    std::map<std::string, std::string> m_byFullLower;
    std::map<std::string, std::string> m_byNameLower;

    void ScanDir(const std::string& dir);
};

bool WI_LoadSCO(const std::string& fileName, WI_ProjectIndex& index, WI_Mesh& outMesh, std::string& error);
bool WI_LoadMAT(const std::string& fileName, WI_Material& outMat, std::string& error);
bool WI_LoadWGT(const std::string& fileName, WI_Weights& outWeights, std::string& error);
bool WI_LoadSKL(const std::string& fileName, WI_Skeleton& outSkeleton, std::string& error);
bool WI_LoadANM(const std::string& fileName, WI_Animation& outAnim, std::string& error);

std::string WI_ToLower(std::string s);
std::string WI_DirName(const std::string& p);
std::string WI_BaseName(const std::string& p);
std::string WI_BaseNameNoExt(const std::string& p);
std::string WI_ReplaceExt(const std::string& p, const std::string& ext);
bool WI_FileExists(const std::string& p);