#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "m_AppSelect.h"
#include "APIScaleformGfxDX11.h"

#include "r3dDX11.h"
#include "r3dDX11State.h"
#include "r3dDX11Geometry.h"

#include <windows.h>

int AppSelectMode = 100;

Menu_AppSelect::Menu_AppSelect()
{
}

Menu_AppSelect::~Menu_AppSelect()
{
}

void Menu_AppSelect::Draw()
{
}

extern bool g_bExit;

void ClearFullScreen_Menu()
{
	if(g_r3dDX11.IsInitialized())
		return;

	r3dRenderer->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	r3dRenderer->SetViewport(
		0.0f,
		0.0f,
		(float)r3dRenderer->d3dpp.BackBufferWidth,
		(float)r3dRenderer->d3dpp.BackBufferHeight
	);

	r3dRenderer->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

	r3dRenderer->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
}

static int AppSelectCommandToResult(int command)
{
	switch(command)
	{
	case R3D_SF_DX11_APPSELECT_PLAY_GAME:
		return Menu_AppSelect::bStartGamePublic;

	case R3D_SF_DX11_APPSELECT_LEVEL_EDITOR:
		return Menu_AppSelect::bStartLevelEditor;

	case R3D_SF_DX11_APPSELECT_PARTICLE_EDITOR:
		return Menu_AppSelect::bStartParticleEditor;

	case R3D_SF_DX11_APPSELECT_PHYSICS_EDITOR:
		return Menu_AppSelect::bStartPhysicsEditor;

	case R3D_SF_DX11_APPSELECT_CHARACTER_EDITOR:
		return Menu_AppSelect::bStartCharacterEditor;

	case R3D_SF_DX11_APPSELECT_EXIT:
		return Menu_AppSelect::bQuit;
	}

	return -1;
}

static int RenderAppSelectDX11()
{
	if(!g_r3dDX11.IsInitialized())
		return R3D_SF_DX11_APPSELECT_NONE;

	g_r3dDX11.BeginFrame(0.0f, 0.0f, 0.0f, 1.0f);

	int command = r3dScaleformGfxDX11RenderAppSelect();

	g_r3dDX11State.InvalidateCache();
	g_r3dDX11Geometry.InvalidateCache();

	g_r3dDX11.EndFrame(true);

	return command;
}

static int RenderAppSelectDX9Message()
{
	r3dStartFrame();

	ClearFullScreen_Menu();

	r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);
	r3dSetFiltering(R3D_BILINEAR);
	r3dRenderer->SetMipMapBias(-6.0f, -1);

	CD3DFont* font = NULL;

	{
		r3dIntegrityGuardian ig;
		font = new CD3DFont(ig, "Verdana", 18, D3DFONT_BOLD);
	}

	if(font && SUCCEEDED(font->CreateSystemFont()))
	{
		font->PrintF(
			r3dRenderer->ScreenW * 0.5f - 315.0f,
			r3dRenderer->ScreenH * 0.5f - 35.0f,
			r3dColor(255, 180, 80),
			"Scaleform DX11 AppSelect requires -dx11"
		);

		font->PrintF(
			r3dRenderer->ScreenW * 0.5f - 145.0f,
			r3dRenderer->ScreenH * 0.5f + 10.0f,
			r3dColor(180, 220, 255),
			"Press ESC to exit"
		);
	}

	SAFE_DELETE(font);

	r3dRenderer->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	r3dRenderer->SetRenderingMode(R3D_BLEND_NOALPHA | R3D_BLEND_NZ);

	r3dEndFrame();

	if((GetAsyncKeyState(VK_ESCAPE) & 1) != 0)
		return R3D_SF_DX11_APPSELECT_EXIT;

	return R3D_SF_DX11_APPSELECT_NONE;
}

int Menu_AppSelect::DoModal()
{
	AppSelectMode = 100;
	released_id = -1;

	Desktop().SetViewSize(r3dRenderer->ScreenW, r3dRenderer->ScreenH);

	r3dMouse::Show();

	if(!g_r3dDX11.IsInitialized())
	{
		r3dOutToLog("AppSelect: DX9 mode detected, Scaleform DX11 frontend disabled\n");

		while(1)
		{
			if(g_bExit)
				return 0;

			mUpdate();

			int command = RenderAppSelectDX9Message();
			int result = AppSelectCommandToResult(command);

			if(result != -1)
				return result;
		}
	}

	if(!r3dScaleformGfxDX11Create())
	{
		r3dOutToLog("AppSelect: Scaleform DX11 create failed\n");
		return Menu_AppSelect::bQuit;
	}

	if(!r3dScaleformGfxDX11LoadAppSelect("data/menu/AppSelect.gfx"))
	{
		r3dOutToLog("AppSelect: data/menu/AppSelect.gfx failed\n");
		return Menu_AppSelect::bQuit;
	}

	r3dOutToLog("AppSelect: Scaleform DX11 UI enabled\n");

	while(1)
	{
		if(g_bExit)
		{
			r3dScaleformGfxDX11UnloadAppSelect();
			return 0;
		}

		mUpdate();

		int command = RenderAppSelectDX11();
		int result = AppSelectCommandToResult(command);

		if(result != -1)
		{
			r3dOutToLog("AppSelect: Scaleform DX11 command=%d result=%d\n", command, result);

			r3dScaleformGfxDX11UnloadAppSelect();
			return result;
		}
	}

	r3dScaleformGfxDX11UnloadAppSelect();
	return 0;
}