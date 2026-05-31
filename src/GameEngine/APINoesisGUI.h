#pragma once

#ifndef __API_NOESIS_GUI_H__
#define __API_NOESIS_GUI_H__

#include "r3d.h"

class APINoesisGUI
{
public:
    APINoesisGUI();
    ~APINoesisGUI();

    bool Init();
    void Shutdown();

    bool LoadXaml(const char* xamlFile);
    void UnloadXaml();

    void SetSize(int width, int height);
    void Update(float timeSeconds);
    void Render();

    bool IsInitialized() const;
    bool IsLoaded() const;

private:
    void* DllHandle;

    typedef int  (__cdecl* FN_WarNoesis_Init)(const char* rootPath);
    typedef void (__cdecl* FN_WarNoesis_Shutdown)();
    typedef int  (__cdecl* FN_WarNoesis_LoadXamlFile)(const char* filename);
    typedef void (__cdecl* FN_WarNoesis_UnloadXaml)();
    typedef void (__cdecl* FN_WarNoesis_SetSize)(int width, int height);
    typedef void (__cdecl* FN_WarNoesis_Update)(double timeSeconds);
    typedef void (__cdecl* FN_WarNoesis_Render)();
    typedef int  (__cdecl* FN_WarNoesis_IsLoaded)();

    FN_WarNoesis_Init FnInit;
    FN_WarNoesis_Shutdown FnShutdown;
    FN_WarNoesis_LoadXamlFile FnLoadXamlFile;
    FN_WarNoesis_UnloadXaml FnUnloadXaml;
    FN_WarNoesis_SetSize FnSetSize;
    FN_WarNoesis_Update FnUpdate;
    FN_WarNoesis_Render FnRender;
    FN_WarNoesis_IsLoaded FnIsLoaded;

    bool Initialized;
    bool Loaded;
};

extern APINoesisGUI* gNoesisGUI;

void r3dNoesisGUICreate();
void r3dNoesisGUIDestroy();
void r3dNoesisGUIReset();

#endif