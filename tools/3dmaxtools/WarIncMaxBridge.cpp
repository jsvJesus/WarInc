#include "WarIncMaxBridge.h"

static Point3 WI_P3(const WI_Vec3& v)
{
    return Point3(v.x, v.y, v.z);
}

static Matrix3 WI_Matrix3(const WI_Mat4& m)
{
    Matrix3 tm(TRUE);

    tm.SetRow(0, Point3(m.m[0][0], m.m[0][1], m.m[0][2]));
    tm.SetRow(1, Point3(m.m[1][0], m.m[1][1], m.m[1][2]));
    tm.SetRow(2, Point3(m.m[2][0], m.m[2][1], m.m[2][2]));
    tm.SetTrans(Point3(m.m[3][0], m.m[3][1], m.m[3][2]));

    return tm;
}

static Mtl* WI_CreateStdMaterial(WI_ProjectIndex& index, const WI_Material& src, const std::string& materialFile)
{
    StdMat2* m = NewDefaultStdMat();
    if(!m)
        return 0;

    m->SetName(src.name.c_str());
    m->SetDiffuse(Color(0.75f, 0.75f, 0.75f), 0);

    std::string diffuse = index.FindTextureFile(src.diffuse, src.imagesDir, materialFile);
    if(!diffuse.empty())
    {
        BitmapTex* bt = NewDefaultBitmapTex();
        if(bt)
        {
            bt->SetMapName(diffuse.c_str());
            m->SetSubTexmap(ID_DI, bt);
            m->EnableMap(ID_DI, TRUE);
        }
    }

    std::string normal = index.FindTextureFile(src.normal, src.imagesDir, materialFile);
    if(!normal.empty())
    {
        BitmapTex* bt = NewDefaultBitmapTex();
        if(bt)
        {
            bt->SetMapName(normal.c_str());
            m->SetSubTexmap(ID_BU, bt);
            m->EnableMap(ID_BU, TRUE);
        }
    }

    m->SetTwoSided(src.doubleSided ? TRUE : FALSE);

    if(src.transparent)
    {
        m->SetOpacity(0.75f, 0);
    }

    return m;
}

static Mtl* WI_CreateMultiMaterial(WI_ProjectIndex& index, const WI_Mesh& mesh)
{
    int n = (int)mesh.materialOrder.size();

    if(n <= 0)
        return NewDefaultStdMat();

    if(n == 1)
    {
        const std::string& name = mesh.materialOrder[0];
        std::string mf = index.FindMaterialFile(name, mesh.sourceFile);
        std::map<std::string, WI_Material>::const_iterator it = mesh.materials.find(name);
        if(it != mesh.materials.end())
            return WI_CreateStdMaterial(index, it->second, mf);

        StdMat2* fallback = NewDefaultStdMat();
        fallback->SetName(name.c_str());
        return fallback;
    }

    MultiMtl* mm = NewDefaultMultiMtl();
    mm->SetNumSubMtls(n);
    mm->SetName(mesh.name.c_str());

    for(int i = 0; i < n; ++i)
    {
        const std::string& name = mesh.materialOrder[i];
        std::string mf = index.FindMaterialFile(name, mesh.sourceFile);

        Mtl* sub = 0;

        std::map<std::string, WI_Material>::const_iterator it = mesh.materials.find(name);
        if(it != mesh.materials.end())
            sub = WI_CreateStdMaterial(index, it->second, mf);

        if(!sub)
        {
            StdMat2* fallback = NewDefaultStdMat();
            fallback->SetName(name.c_str());
            sub = fallback;
        }

        mm->SetSubMtl(i, sub);
        mm->SetSubMtlAndName(i, sub, name.c_str());
    }

    return mm;
}

static int WI_MaterialId(const WI_Mesh& mesh, const std::string& name)
{
    for(size_t i = 0; i < mesh.materialOrder.size(); ++i)
    {
        if(_stricmp(mesh.materialOrder[i].c_str(), name.c_str()) == 0)
            return (int)i + 1;
    }

    return 1;
}

bool WI_MaxImportSCO(Interface* ip, WI_MaxState& st, const std::string& fileName, std::string& error)
{
    WI_Mesh mesh;
    if(!WI_LoadSCO(fileName, st.index, mesh, error))
        return false;

    Mesh* maxMesh = new Mesh();

    int numVerts = (int)mesh.positions.size();
    int numFaces = (int)mesh.faces.size();

    maxMesh->setNumVerts(numVerts);
    maxMesh->setNumFaces(numFaces);

    for(int i = 0; i < numVerts; ++i)
    {
        WI_Vec3 p = mesh.positions[i];
        p.x -= mesh.pivot.x;
        p.y -= mesh.pivot.y;
        p.z -= mesh.pivot.z;
        maxMesh->setVert(i, Point3(p.x, p.y, p.z));
    }

    maxMesh->setNumTVerts(numFaces * 3);
    maxMesh->setNumTVFaces(numFaces);

    for(int i = 0; i < numFaces; ++i)
    {
        const WI_Face& f = mesh.faces[i];

        Face& mf = maxMesh->faces[i];
        mf.setVerts(f.v[0], f.v[1], f.v[2]);
        mf.setEdgeVisFlags(1, 1, 1);
        mf.setSmGroup(1);
        mf.setMatID((MtlID)WI_MaterialId(mesh, f.material));

        int tv0 = i * 3 + 0;
        int tv1 = i * 3 + 1;
        int tv2 = i * 3 + 2;

        maxMesh->setTVert(tv0, Point3(f.uv[0].x, 1.0f - f.uv[0].y, 0.0f));
        maxMesh->setTVert(tv1, Point3(f.uv[1].x, 1.0f - f.uv[1].y, 0.0f));
        maxMesh->setTVert(tv2, Point3(f.uv[2].x, 1.0f - f.uv[2].y, 0.0f));

        TVFace& tf = maxMesh->tvFace[i];
        tf.setTVerts(tv0, tv1, tv2);
    }

    maxMesh->InvalidateGeomCache();
    maxMesh->InvalidateTopologyCache();
    maxMesh->buildNormals();

    TriObject* tri = CreateNewTriObject();
    tri->GetMesh() = *maxMesh;
    delete maxMesh;

    INode* node = ip->CreateObjectNode(tri);
    if(!node)
    {
        error = "can't create mesh node";
        return false;
    }

    node->SetName(mesh.name.empty() ? WI_BaseNameNoExt(fileName).c_str() : mesh.name.c_str());

    Matrix3 tm(TRUE);
    tm.SetTrans(Point3(mesh.pivot.x, mesh.pivot.y, mesh.pivot.z));
    node->SetNodeTM(0, tm);

    Mtl* mtl = WI_CreateMultiMaterial(st.index, mesh);
    if(mtl)
        node->SetMtl(mtl);

    ip->SelectNode(node);
    ip->RedrawViews(ip->GetTime());

    st.lastMesh = mesh;
    st.meshNode = node;
    st.hasMesh = true;
    st.hasWeights = false;

    return true;
}

bool WI_MaxImportSKL(Interface* ip, WI_MaxState& st, const std::string& fileName, std::string& error)
{
    WI_Skeleton skel;
    if(!WI_LoadSKL(fileName, skel, error))
        return false;

    st.boneNodes.clear();
    st.boneNodes.resize(skel.bones.size());

    for(size_t i = 0; i < skel.bones.size(); ++i)
    {
        DummyObject* dummy = (DummyObject*)CreateInstance(HELPER_CLASS_ID, Class_ID(DUMMY_CLASS_ID, 0));
        if(!dummy)
        {
            error = "can't create dummy bone";
            return false;
        }

        dummy->SetBox(Box3(Point3(-1.5f, -1.5f, -1.5f), Point3(1.5f, 1.5f, 1.5f)));

        INode* node = ip->CreateObjectNode(dummy);
        if(!node)
        {
            error = "can't create bone node";
            return false;
        }

        node->SetName(skel.bones[i].name.c_str());

        Matrix3 tm = WI_Matrix3(skel.bones[i].absPlacement);
        node->SetNodeTM(0, tm);

        st.boneNodes[i] = node;
    }

    for(size_t i = 0; i < skel.bones.size(); ++i)
    {
        int parent = skel.bones[i].parentId;
        if(parent >= 0 && parent < (int)st.boneNodes.size())
            st.boneNodes[parent]->AttachChild(st.boneNodes[i], 1);
    }

    st.skeleton = skel;
    st.hasSkeleton = true;
    st.hasWeights = false;

    ip->RedrawViews(ip->GetTime());
    return true;
}

static Modifier* WI_GetOrCreateSkinModifier(INode* node)
{
    Object* obj = node->GetObjectRef();

    IDerivedObject* dobj = 0;

    if(obj && obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
    {
        dobj = (IDerivedObject*)obj;

        for(int i = 0; i < dobj->NumModifiers(); ++i)
        {
            Modifier* m = dobj->GetModifier(i);
            if(m && m->ClassID() == SKIN_CLASSID)
                return m;
        }
    }
    else
    {
        dobj = CreateDerivedObject(obj);
        node->SetObjectRef(dobj);
    }

    Modifier* skin = (Modifier*)CreateInstance(OSM_CLASS_ID, SKIN_CLASSID);
    if(!skin)
        return 0;

    dobj->AddModifier(skin);
    return skin;
}

bool WI_MaxApplyWGT(Interface* ip, WI_MaxState& st, const std::string& fileName, std::string& error)
{
    if(!st.hasSkeleton || st.boneNodes.empty())
    {
        error = "first import skeleton";
        return false;
    }

    INode* node = ip->GetSelNodeCount() > 0 ? ip->GetSelNode(0) : st.meshNode;
    if(!node)
    {
        error = "select mesh node";
        return false;
    }

    WI_Weights wgt;
    if(!WI_LoadWGT(fileName, wgt, error))
        return false;

    ObjectState os = node->EvalWorldState(ip->GetTime());
    if(!os.obj || !os.obj->IsSubClassOf(triObjectClassID))
    {
        error = "selected node is not editable mesh/triobject";
        return false;
    }

    TriObject* tri = (TriObject*)os.obj;
    int numVerts = tri->GetMesh().getNumVerts();

    if((int)wgt.vertexCount != numVerts)
    {
        error = "wgt vertex count mismatch";
        return false;
    }

    Modifier* skin = WI_GetOrCreateSkinModifier(node);
    if(!skin)
    {
        error = "can't create Skin modifier";
        return false;
    }

    ISkinImportData* skinData = (ISkinImportData*)skin->GetInterface(I_SKINIMPORTDATA);
    if(!skinData)
    {
        error = "can't get ISkinImportData";
        return false;
    }

    Matrix3 meshTM = node->GetNodeTM(0);
    skinData->SetSkinTm(node, meshTM, meshTM);

    for(size_t i = 0; i < st.boneNodes.size(); ++i)
    {
        INode* bone = st.boneNodes[i];
        Matrix3 boneTM = bone->GetNodeTM(0);

        skinData->AddBoneEx(bone, FALSE);
        skinData->SetBoneTm(bone, boneTM, boneTM);
    }

    for(int v = 0; v < numVerts; ++v)
    {
        Tab<INode*> bones;
        Tab<float> weights;

        for(int k = 0; k < 4; ++k)
        {
            int bid = (int)wgt.vertices[v].boneId[k];
            float bw = wgt.vertices[v].weight[k];

            if(bw > 0.0001f && bid >= 0 && bid < (int)st.boneNodes.size())
            {
                bones.Append(1, &st.boneNodes[bid]);
                weights.Append(1, &bw);
            }
        }

        if(bones.Count() > 0)
            skinData->AddWeights(node, v, bones, weights);
    }

    st.weights = wgt;
    st.meshNode = node;
    st.hasWeights = true;

    ip->RedrawViews(ip->GetTime());
    return true;
}

static INode* WI_FindBoneNode(WI_MaxState& st, const std::string& name)
{
    for(size_t i = 0; i < st.skeleton.bones.size() && i < st.boneNodes.size(); ++i)
    {
        if(_stricmp(st.skeleton.bones[i].name.c_str(), name.c_str()) == 0)
            return st.boneNodes[i];
    }

    return 0;
}

bool WI_MaxImportANM(Interface* ip, WI_MaxState& st, const std::string& fileName, std::string& error)
{
    if(!st.hasSkeleton || !st.hasWeights)
    {
        error = "animation requires imported skeleton and applied weights";
        return false;
    }

    WI_Animation anim;
    if(!WI_LoadANM(fileName, anim, error))
        return false;

    if(anim.frameRate == 0)
        anim.frameRate = 30;

    TimeValue start = 0;
    TimeValue end = SecToTicks((float)anim.numFrames / (float)anim.frameRate);
    ip->SetAnimRange(Interval(start, end));

    for(size_t t = 0; t < anim.tracks.size(); ++t)
    {
        INode* node = WI_FindBoneNode(st, anim.tracks[t].boneName);
        if(!node)
            continue;

        Control* tmCtrl = node->GetTMController();
        if(!tmCtrl)
            continue;

        Control* posCtrl = tmCtrl->GetPositionController();
        Control* rotCtrl = tmCtrl->GetRotationController();

        for(size_t f = 0; f < anim.tracks[t].frames.size(); ++f)
        {
            TimeValue tv = SecToTicks((float)f / (float)anim.frameRate);

            const WI_AnimFrame& fr = anim.tracks[t].frames[f];

            Point3 p(fr.v.x, fr.v.y, fr.v.z);
            Quat q(fr.q.x, fr.q.y, fr.q.z, fr.q.w);

            if(posCtrl)
                posCtrl->SetValue(tv, &p, TRUE, CTRL_ABSOLUTE);

            if(rotCtrl)
                rotCtrl->SetValue(tv, &q, TRUE, CTRL_ABSOLUTE);
        }
    }

    ip->RedrawViews(ip->GetTime());
    return true;
}