#pragma once

#include "WarIncFormats.h"

#include <max.h>
#include <iskin.h>
#include <stdmat.h>
#include <iparamb2.h>

struct WI_MaxState
{
    WI_ProjectIndex index;
    WI_Mesh lastMesh;
    WI_Skeleton skeleton;
    WI_Weights weights;

    INode* meshNode;
    std::vector<INode*> boneNodes;

    bool hasMesh;
    bool hasSkeleton;
    bool hasWeights;

    WI_MaxState()
    {
        meshNode = 0;
        hasMesh = false;
        hasSkeleton = false;
        hasWeights = false;
    }
};

bool WI_MaxImportSCO(Interface* ip, WI_MaxState& st, const std::string& fileName, std::string& error);
bool WI_MaxImportSKL(Interface* ip, WI_MaxState& st, const std::string& fileName, std::string& error);
bool WI_MaxApplyWGT(Interface* ip, WI_MaxState& st, const std::string& fileName, std::string& error);
bool WI_MaxImportANM(Interface* ip, WI_MaxState& st, const std::string& fileName, std::string& error);