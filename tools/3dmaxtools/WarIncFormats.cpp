#define _CRT_SECURE_NO_WARNINGS

#include "WarIncFormats.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ctype.h>

static unsigned int WI_Tag(const char* s)
{
    return ((unsigned int)(unsigned char)s[0]) |
           ((unsigned int)(unsigned char)s[1] << 8) |
           ((unsigned int)(unsigned char)s[2] << 16) |
           ((unsigned int)(unsigned char)s[3] << 24);
}

static std::string WI_Trim(const std::string& s)
{
    size_t a = 0;
    while(a < s.size() && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r' || s[a] == '\n'))
        ++a;

    size_t b = s.size();
    while(b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r' || s[b - 1] == '\n'))
        --b;

    return s.substr(a, b - a);
}

static bool WI_StartsWithNoCase(const std::string& s, const char* p)
{
    std::string a = WI_ToLower(s);
    std::string b = WI_ToLower(p);
    return a.find(b) == 0;
}

static std::string WI_NormalizePath(std::string s)
{
    for(size_t i = 0; i < s.size(); ++i)
    {
        if(s[i] == '/')
            s[i] = '\\';
    }
    return s;
}

std::string WI_ToLower(std::string s)
{
    for(size_t i = 0; i < s.size(); ++i)
        s[i] = (char)tolower((unsigned char)s[i]);
    return s;
}

std::string WI_DirName(const std::string& p)
{
    size_t a = p.find_last_of("\\/");
    if(a == std::string::npos)
        return "";
    return p.substr(0, a);
}

std::string WI_BaseName(const std::string& p)
{
    size_t a = p.find_last_of("\\/");
    if(a == std::string::npos)
        return p;
    return p.substr(a + 1);
}

std::string WI_BaseNameNoExt(const std::string& p)
{
    std::string b = WI_BaseName(p);
    size_t d = b.find_last_of('.');
    if(d == std::string::npos)
        return b;
    return b.substr(0, d);
}

std::string WI_ReplaceExt(const std::string& p, const std::string& ext)
{
    size_t d = p.find_last_of('.');
    if(d == std::string::npos)
        return p + ext;
    return p.substr(0, d) + ext;
}

bool WI_FileExists(const std::string& p)
{
    DWORD a = GetFileAttributesA(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

bool WI_ProjectIndex::Build(const std::string& rootDir)
{
    m_root = WI_NormalizePath(rootDir);
    m_byFullLower.clear();
    m_byNameLower.clear();

    if(m_root.empty())
        return false;

    ScanDir(m_root);
    return true;
}

std::string WI_ProjectIndex::Root() const
{
    return m_root;
}

void WI_ProjectIndex::ScanDir(const std::string& dir)
{
    std::string mask = dir + "\\*.*";

    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(mask.c_str(), &fd);
    if(h == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if(strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
            continue;

        std::string full = dir + "\\" + fd.cFileName;

        if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            ScanDir(full);
            continue;
        }

        std::string lowFull = WI_ToLower(WI_NormalizePath(full));
        std::string lowName = WI_ToLower(fd.cFileName);

        m_byFullLower[lowFull] = full;
        m_byNameLower[lowName] = full;
    }
    while(FindNextFileA(h, &fd));

    FindClose(h);
}

std::string WI_ProjectIndex::FindFile(const std::string& nameOrPath, const std::string& forcedExt, const std::string& nearFile) const
{
    if(nameOrPath.empty())
        return "";

    std::string n = WI_NormalizePath(nameOrPath);

    if(WI_FileExists(n))
        return n;

    if(!forcedExt.empty())
        n = WI_ReplaceExt(n, forcedExt);

    if(WI_FileExists(n))
        return n;

    std::string nearDir = WI_DirName(nearFile);
    if(!nearDir.empty())
    {
        std::string p = nearDir + "\\" + WI_BaseName(n);
        if(WI_FileExists(p))
            return p;
    }

    if(!m_root.empty())
    {
        std::string p = m_root + "\\" + n;
        if(WI_FileExists(p))
            return p;
    }

    std::string key = WI_ToLower(WI_BaseName(n));
    std::map<std::string, std::string>::const_iterator it = m_byNameLower.find(key);
    if(it != m_byNameLower.end())
        return it->second;

    return "";
}

std::string WI_ProjectIndex::FindMaterialFile(const std::string& materialName, const std::string& meshFile) const
{
    std::string matName = materialName;
    if(matName.find('.') == std::string::npos)
        matName += ".mat";

    std::string nearDir = WI_DirName(meshFile);
    if(!nearDir.empty())
    {
        std::string p = nearDir + "\\" + matName;
        if(WI_FileExists(p))
            return p;
    }

    return FindFile(matName, ".mat", meshFile);
}

std::string WI_ProjectIndex::FindTextureFile(const std::string& textureName, const std::string& imagesDir, const std::string& materialFile) const
{
    if(textureName.empty())
        return "";

    std::string tex = textureName;
    if(tex.find('.') == std::string::npos)
        tex += ".dds";

    if(WI_FileExists(tex))
        return tex;

    if(!imagesDir.empty())
    {
        std::string p = WI_NormalizePath(imagesDir) + "\\" + tex;
        if(WI_FileExists(p))
            return p;

        if(!m_root.empty())
        {
            p = m_root + "\\" + WI_NormalizePath(imagesDir) + "\\" + tex;
            if(WI_FileExists(p))
                return p;
        }
    }

    return FindFile(tex, ".dds", materialFile);
}

static bool WI_ReadLine(FILE* f, std::string& out)
{
    char buf[2048];
    if(!fgets(buf, sizeof(buf), f))
        return false;
    out = WI_Trim(buf);
    return true;
}

bool WI_LoadSCO(const std::string& fileName, WI_ProjectIndex& index, WI_Mesh& outMesh, std::string& error)
{
    FILE* f = fopen(fileName.c_str(), "rt");
    if(!f)
    {
        error = "can't open sco";
        return false;
    }

    std::string line;
    bool found = false;

    while(WI_ReadLine(f, line))
    {
        if(WI_StartsWithNoCase(line, "[ObjectBegin]"))
        {
            found = true;
            break;
        }
    }

    if(!found)
    {
        fclose(f);
        error = "missing [ObjectBegin]";
        return false;
    }

    outMesh = WI_Mesh();
    outMesh.sourceFile = fileName;

    if(!WI_ReadLine(f, line))
    {
        fclose(f);
        error = "missing Name";
        return false;
    }

    {
        char key[128] = {0};
        char name[256] = {0};
        sscanf(line.c_str(), "%127s %255s", key, name);
        if(_stricmp(key, "Name=") != 0)
        {
            fclose(f);
            error = "invalid Name";
            return false;
        }
        outMesh.name = name;
    }

    if(!WI_ReadLine(f, line))
    {
        fclose(f);
        error = "missing pivot";
        return false;
    }

    {
        char key[128] = {0};
        sscanf(line.c_str(), "%127s %f %f %f", key, &outMesh.pivot.x, &outMesh.pivot.y, &outMesh.pivot.z);
    }

    int numVerts = 0;

    if(!WI_ReadLine(f, line))
    {
        fclose(f);
        error = "missing Verts";
        return false;
    }

    {
        char key[128] = {0};
        sscanf(line.c_str(), "%127s %d", key, &numVerts);
        if(_stricmp(key, "Verts=") != 0 || numVerts <= 0)
        {
            fclose(f);
            error = "invalid Verts";
            return false;
        }
    }

    outMesh.positions.resize(numVerts);
    outMesh.normals.resize(numVerts);
    outMesh.tangents.resize(numVerts);
    outMesh.tangentSigns.resize(numVerts);

    for(int i = 0; i < numVerts; ++i)
    {
        if(!WI_ReadLine(f, line))
        {
            fclose(f);
            error = "missing vertex line";
            return false;
        }

        float sign = 1.0f;
        int cr = 255;
        int cg = 255;
        int cb = 255;

        int scanned = sscanf(
            line.c_str(),
            "%f %f %f %f %f %f %f %f %f %f %d %d %d",
            &outMesh.positions[i].x,
            &outMesh.positions[i].y,
            &outMesh.positions[i].z,
            &outMesh.normals[i].x,
            &outMesh.normals[i].y,
            &outMesh.normals[i].z,
            &outMesh.tangents[i].x,
            &outMesh.tangents[i].y,
            &outMesh.tangents[i].z,
            &sign,
            &cr,
            &cg,
            &cb
        );

        if(scanned < 3)
        {
            fclose(f);
            error = "bad vertex";
            return false;
        }

        if(scanned < 6)
        {
            outMesh.normals[i].x = 0.0f;
            outMesh.normals[i].y = 0.0f;
            outMesh.normals[i].z = 1.0f;
        }

        if(scanned < 10)
        {
            outMesh.tangents[i].x = 1.0f;
            outMesh.tangents[i].y = 0.0f;
            outMesh.tangents[i].z = 0.0f;
            sign = 1.0f;
        }

        outMesh.tangentSigns[i] = sign > 0.0f ? 255 : 0;
    }

    int numFaces = 0;

    if(!WI_ReadLine(f, line))
    {
        fclose(f);
        error = "missing Faces";
        return false;
    }

    {
        char key[128] = {0};
        sscanf(line.c_str(), "%127s %d", key, &numFaces);
        if(_stricmp(key, "Faces=") != 0 || numFaces <= 0)
        {
            fclose(f);
            error = "invalid Faces";
            return false;
        }
    }

    outMesh.faces.resize(numFaces);
    std::map<std::string, bool> materialSeen;

    for(int i = 0; i < numFaces; ++i)
    {
        if(!WI_ReadLine(f, line))
        {
            fclose(f);
            error = "missing face line";
            return false;
        }

        int tmp = 0;
        char mat[256] = {0};

        WI_Face face;
        int scanned = sscanf(
            line.c_str(),
            "%d %d %d %d %255s %f %f %f %f %f %f",
            &tmp,
            &face.v[0],
            &face.v[1],
            &face.v[2],
            mat,
            &face.uv[0].x,
            &face.uv[0].y,
            &face.uv[1].x,
            &face.uv[1].y,
            &face.uv[2].x,
            &face.uv[2].y
        );

        if(scanned < 11)
        {
            fclose(f);
            error = "bad face";
            return false;
        }

        face.material = mat;
        outMesh.faces[i] = face;

        if(materialSeen.find(face.material) == materialSeen.end())
        {
            materialSeen[face.material] = true;
            outMesh.materialOrder.push_back(face.material);
        }
    }

    fclose(f);

    for(size_t i = 0; i < outMesh.materialOrder.size(); ++i)
    {
        const std::string& matName = outMesh.materialOrder[i];
        std::string matFile = index.FindMaterialFile(matName, fileName);

        WI_Material mat;
        mat.name = matName;

        if(!matFile.empty())
        {
            std::string matError;
            WI_LoadMAT(matFile, mat, matError);
        }

        outMesh.materials[matName] = mat;
    }

    return true;
}

bool WI_LoadMAT(const std::string& fileName, WI_Material& outMat, std::string& error)
{
    FILE* f = fopen(fileName.c_str(), "rt");
    if(!f)
    {
        error = "can't open mat";
        return false;
    }

    outMat = WI_Material();

    std::string line;
    bool found = false;

    while(WI_ReadLine(f, line))
    {
        if(line.find("[MaterialBegin]") != std::string::npos)
        {
            found = true;
            break;
        }
    }

    if(!found)
    {
        fclose(f);
        error = "missing [MaterialBegin]";
        return false;
    }

    outMat.imagesDir = WI_DirName(fileName);

    while(WI_ReadLine(f, line))
    {
        if(line.find("[MaterialEnd]") != std::string::npos)
            break;

        size_t eq = line.find('=');
        if(eq == std::string::npos)
            continue;

        std::string key = WI_ToLower(WI_Trim(line.substr(0, eq)));
        std::string val = WI_Trim(line.substr(eq + 1));

        if(_stricmp(val.c_str(), "NONE") == 0)
            val.clear();

        if(key == "name") outMat.name = val;
        else if(key == "imagesdir") outMat.imagesDir = WI_NormalizePath(val);
        else if(key == "texture") outMat.diffuse = val;
        else if(key == "normalmap") outMat.normal = val;
        else if(key == "specularmap") outMat.specular = val;
        else if(key == "glowmap") outMat.glow = val;
        else if(key == "envmap") outMat.env = val;
        else if(key == "detailnmap") outMat.detailNormal = val;
        else if(key == "densitymap") outMat.density = val;
        else if(key == "camomask") outMat.camoMask = val;
        else if(key == "distortionmap") outMat.distortion = val;
        else if(key == "specpowmap") outMat.specPow = val;
        else if(key == "doublesided") outMat.doubleSided = atoi(val.c_str()) != 0;
        else if(key == "alphatransparent") outMat.transparent = atoi(val.c_str()) != 0;
        else if(key == "specularpower") outMat.specularPower = (float)atof(val.c_str());
        else if(key == "reflectionpower") outMat.reflectionPower = (float)atof(val.c_str());
    }

    fclose(f);
    return true;
}

class WI_BinReader
{
public:
    WI_BinReader() : f(0) {}

    bool Open(const std::string& p)
    {
        f = fopen(p.c_str(), "rb");
        return f != 0;
    }

    void Close()
    {
        if(f)
            fclose(f);
        f = 0;
    }

    bool U32(unsigned int& v)
    {
        return fread(&v, 4, 1, f) == 1;
    }

    bool I32(int& v)
    {
        return fread(&v, 4, 1, f) == 1;
    }

    bool F32(float& v)
    {
        return fread(&v, 4, 1, f) == 1;
    }

    bool Bytes(void* p, size_t n)
    {
        return fread(p, n, 1, f) == 1;
    }

private:
    FILE* f;
};

bool WI_LoadWGT(const std::string& fileName, WI_Weights& outWeights, std::string& error)
{
    WI_BinReader r;
    if(!r.Open(fileName))
    {
        error = "can't open wgt";
        return false;
    }

    unsigned int r3dID = 0;
    unsigned int id = 0;
    unsigned int ver = 0;

    if(!r.U32(r3dID) || !r.U32(id) || !r.U32(ver))
    {
        r.Close();
        error = "bad wgt header";
        return false;
    }

    if(r3dID != WI_Tag("2d3r") || id != WI_Tag("thgw") || ver != 1)
    {
        r.Close();
        error = "invalid wgt header";
        return false;
    }

    outWeights = WI_Weights();

    if(!r.U32(outWeights.skeletonId) || !r.U32(outWeights.vertexCount))
    {
        r.Close();
        error = "bad wgt counts";
        return false;
    }

    outWeights.vertices.resize(outWeights.vertexCount);

    for(unsigned int i = 0; i < outWeights.vertexCount; ++i)
    {
        if(!r.Bytes(outWeights.vertices[i].boneId, 4))
        {
            r.Close();
            error = "bad wgt bone ids";
            return false;
        }

        if(!r.Bytes(outWeights.vertices[i].weight, sizeof(float) * 4))
        {
            r.Close();
            error = "bad wgt weights";
            return false;
        }

        float sum = 0.0f;
        for(int k = 0; k < 4; ++k)
            sum += outWeights.vertices[i].weight[k];

        if(sum > 0.0f)
        {
            for(int k = 0; k < 4; ++k)
                outWeights.vertices[i].weight[k] /= sum;
        }
    }

    r.Close();
    return true;
}

bool WI_LoadSKL(const std::string& fileName, WI_Skeleton& outSkeleton, std::string& error)
{
    WI_BinReader r;
    if(!r.Open(fileName))
    {
        error = "can't open skl";
        return false;
    }

    unsigned int r3dID = 0;
    unsigned int id = 0;
    unsigned int ver = 0;

    if(!r.U32(r3dID) || !r.U32(id) || !r.U32(ver))
    {
        r.Close();
        error = "bad skl header";
        return false;
    }

    if(r3dID != WI_Tag("2d3r") || id != WI_Tag("tlks") || ver != 1)
    {
        r.Close();
        error = "invalid skl header";
        return false;
    }

    outSkeleton = WI_Skeleton();
    outSkeleton.sourceFile = fileName;

    unsigned int numBones = 0;

    if(!r.U32(outSkeleton.skeletonId) || !r.U32(numBones))
    {
        r.Close();
        error = "bad skl counts";
        return false;
    }

    outSkeleton.bones.resize(numBones);

    for(unsigned int i = 0; i < numBones; ++i)
    {
        char name[33];
        memset(name, 0, sizeof(name));

        if(!r.Bytes(name, 32))
        {
            r.Close();
            error = "bad bone name";
            return false;
        }

        outSkeleton.bones[i].name = name;

        int parent = -1;
        if(!r.I32(parent))
        {
            r.Close();
            error = "bad bone parent";
            return false;
        }

        outSkeleton.bones[i].parentId = parent;

        if(!r.F32(outSkeleton.bones[i].length))
        {
            r.Close();
            error = "bad bone length";
            return false;
        }

        WI_Mat4& m = outSkeleton.bones[i].absPlacement;
        memset(&m, 0, sizeof(m));

        r.F32(m.m[0][0]); r.F32(m.m[1][0]); r.F32(m.m[2][0]); r.F32(m.m[3][0]);
        r.F32(m.m[0][1]); r.F32(m.m[1][1]); r.F32(m.m[2][1]); r.F32(m.m[3][1]);
        r.F32(m.m[0][2]); r.F32(m.m[1][2]); r.F32(m.m[2][2]); r.F32(m.m[3][2]);

        m.m[0][3] = 0.0f;
        m.m[1][3] = 0.0f;
        m.m[2][3] = 0.0f;
        m.m[3][3] = 1.0f;
    }

    r.Close();
    return true;
}

bool WI_LoadANM(const std::string& fileName, WI_Animation& outAnim, std::string& error)
{
    WI_BinReader r;
    if(!r.Open(fileName))
    {
        error = "can't open anm";
        return false;
    }

    unsigned int r3dID = 0;
    unsigned int id = 0;
    unsigned int ver = 0;

    if(!r.U32(r3dID) || !r.U32(id) || !r.U32(ver))
    {
        r.Close();
        error = "bad anm header";
        return false;
    }

    if(r3dID != WI_Tag("2d3r") || id != WI_Tag("dmna") || ver != 3)
    {
        r.Close();
        error = "invalid anm header";
        return false;
    }

    outAnim = WI_Animation();
    outAnim.sourceFile = fileName;

    unsigned int numTracks = 0;

    if(!r.U32(outAnim.skeletonId) || !r.U32(numTracks) || !r.U32(outAnim.numFrames) || !r.U32(outAnim.frameRate))
    {
        r.Close();
        error = "bad anm counts";
        return false;
    }

    outAnim.tracks.resize(numTracks);

    for(unsigned int t = 0; t < numTracks; ++t)
    {
        char name[33];
        memset(name, 0, sizeof(name));

        if(!r.Bytes(name, 32))
        {
            r.Close();
            error = "bad track name";
            return false;
        }

        outAnim.tracks[t].boneName = name;

        if(!r.U32(outAnim.tracks[t].flags))
        {
            r.Close();
            error = "bad track flags";
            return false;
        }

        outAnim.tracks[t].frames.resize(outAnim.numFrames);

        for(unsigned int f = 0; f < outAnim.numFrames; ++f)
        {
            WI_AnimFrame& fr = outAnim.tracks[t].frames[f];

            if(!r.F32(fr.q.x) || !r.F32(fr.q.y) || !r.F32(fr.q.z) || !r.F32(fr.q.w) ||
               !r.F32(fr.v.x) || !r.F32(fr.v.y) || !r.F32(fr.v.z))
            {
                r.Close();
                error = "bad frame data";
                return false;
            }
        }
    }

    r.Close();
    return true;
}