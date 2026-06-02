#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "m_AppSelect.h"
#include "APIImGui.h"

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
	case R3D_IMGUI_APPSELECT_PLAY_GAME:
		return Menu_AppSelect::bStartGamePublic;

	case R3D_IMGUI_APPSELECT_LEVEL_EDITOR:
		return Menu_AppSelect::bStartLevelEditor;

	case R3D_IMGUI_APPSELECT_PARTICLE_EDITOR:
		return Menu_AppSelect::bStartParticleEditor;

	case R3D_IMGUI_APPSELECT_PHYSICS_EDITOR:
		return Menu_AppSelect::bStartPhysicsEditor;

	case R3D_IMGUI_APPSELECT_CHARACTER_EDITOR:
		return Menu_AppSelect::bStartCharacterEditor;

	case R3D_IMGUI_APPSELECT_EXIT:
		return Menu_AppSelect::bQuit;
	}

	return -1;
}

static void AppSelectEnsureImGuiDX11()
{
	if(!g_r3dDX11.IsInitialized())
		return;

	r3dImGuiCreate();
	r3dImGuiResetDX11();
}

static int RenderAppSelectDX11()
{
	if(!g_r3dDX11.IsInitialized())
		return R3D_IMGUI_APPSELECT_NONE;

	g_r3dDX11.BeginFrame(0.0f, 0.0f, 0.0f, 1.0f);

	int command = r3dImGuiRenderAppSelectMenu();

	g_r3dDX11State.InvalidateCache();
	g_r3dDX11Geometry.InvalidateCache();

	g_r3dDX11.EndFrame(true);

	return command;
}

static int RenderAppSelectDX9()
{
	r3dStartFrame();

	//mDrawStart();

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
			r3dRenderer->ScreenW * 0.5f - 260.0f,
			r3dRenderer->ScreenH * 0.5f - 20.0f,
			r3dColor(255, 180, 80),
			"AppSelect ImGui frontend requires -dx11"
		);

		font->PrintF(
			r3dRenderer->ScreenW * 0.5f - 220.0f,
			r3dRenderer->ScreenH * 0.5f + 20.0f,
			r3dColor(180, 220, 255),
			"Press ESC to exit"
		);
	}

	SAFE_DELETE(font);

	r3dRenderer->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	r3dRenderer->SetRenderingMode(R3D_BLEND_NOALPHA | R3D_BLEND_NZ);

	//mDrawEnd();

	r3dEndFrame();

	if((GetAsyncKeyState(VK_ESCAPE) & 1) != 0)
		return R3D_IMGUI_APPSELECT_EXIT;

	return R3D_IMGUI_APPSELECT_NONE;
}

int Menu_AppSelect::DoModal()
{
	AppSelectMode = 100;
	released_id = -1;

	Desktop().SetViewSize(r3dRenderer->ScreenW, r3dRenderer->ScreenH);

	r3dMouse::Show();

	AppSelectEnsureImGuiDX11();

	r3dOutToLog("AppSelect: Dear ImGui UI enabled\n");

	while(1)
	{
		if(g_bExit)
		{
			return 0;
		}

		mUpdate();

		AppSelectEnsureImGuiDX11();

		int command = R3D_IMGUI_APPSELECT_NONE;

		if(g_r3dDX11.IsInitialized())
		{
			command = RenderAppSelectDX11();
		}
		else
		{
			command = RenderAppSelectDX9();
		}

		int result = AppSelectCommandToResult(command);

		if(result != -1)
		{
			r3dOutToLog("AppSelect: ImGui command=%d result=%d\n", command, result);
			return result;
		}
	}

	return 0;
}