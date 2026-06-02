#pragma once

#ifndef __API_IMGUI_H__
#define __API_IMGUI_H__

#include <windows.h>

enum r3dImGuiAppSelectCommand
{
    R3D_IMGUI_APPSELECT_NONE = 0,
    R3D_IMGUI_APPSELECT_PLAY_GAME,
    R3D_IMGUI_APPSELECT_LEVEL_EDITOR,
    R3D_IMGUI_APPSELECT_PARTICLE_EDITOR,
    R3D_IMGUI_APPSELECT_PHYSICS_EDITOR,
    R3D_IMGUI_APPSELECT_CHARACTER_EDITOR,
    R3D_IMGUI_APPSELECT_EXIT
};

class APIImGui
{
public:
    APIImGui();
    ~APIImGui();

    bool Init();
    void Shutdown();

    bool SetD3D11Device(void* device, void* context);

    bool BeginFrame();
    void EndFrame();

    bool IsInitialized() const;
    bool IsRendererReady() const;

    bool ProcessWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    void ShutdownRenderer();

    bool Initialized;
    bool MsgProcRegistered;
    bool FrameStarted;
    bool DX11Ready;

    void* D3D11Device;
    void* D3D11Context;
};

extern APIImGui* gImGuiGUI;

void r3dImGuiCreate();
void r3dImGuiDestroy();
void r3dImGuiResetDX11();

bool r3dImGuiBeginFrame();
void r3dImGuiEndFrame();

int r3dImGuiRenderAppSelectMenu();

#endif