#pragma once

#ifndef __API_SCALEFORM_GFX_DX11_H__
#define __API_SCALEFORM_GFX_DX11_H__

#include <windows.h>

enum r3dScaleformDX11AppSelectCommand
{
    R3D_SF_DX11_APPSELECT_NONE = 0,
    R3D_SF_DX11_APPSELECT_PLAY_GAME,
    R3D_SF_DX11_APPSELECT_LEVEL_EDITOR,
    R3D_SF_DX11_APPSELECT_PARTICLE_EDITOR,
    R3D_SF_DX11_APPSELECT_PHYSICS_EDITOR,
    R3D_SF_DX11_APPSELECT_CHARACTER_EDITOR,
    R3D_SF_DX11_APPSELECT_EXIT
};

bool r3dScaleformGfxDX11Create();
void r3dScaleformGfxDX11Destroy();
void r3dScaleformGfxDX11Reset();

bool r3dScaleformGfxDX11IsReady();

bool r3dScaleformGfxDX11LoadAppSelect(const char* movieFile);
void r3dScaleformGfxDX11UnloadAppSelect();

int r3dScaleformGfxDX11RenderAppSelect();
bool r3dScaleformGfxDX11ProcessWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif