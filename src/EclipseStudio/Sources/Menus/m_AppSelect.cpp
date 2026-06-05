#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "m_AppSelect.h"
#include "..\UI\UIMenu.h"

#include "r3dDX11.h"

int AppSelectMode = 100;

extern bool g_bExit;

namespace
{
	const int APPSELECT_WAITING_FOR_COMMAND = 100;

	void RestoreDX11MenuBackBuffer()
	{
#ifndef WO_SERVER
		if(r3dRenderer && !r3dRenderer->GetUseD3D9Present() && g_r3dDX11.IsInitialized())
		{
			g_r3dDX11.ResetBackBufferTarget();

			const float width  = (float)R3D_MAX(g_r3dDX11.GetWidth(), 1);
			const float height = (float)R3D_MAX(g_r3dDX11.GetHeight(), 1);

			r3dRenderer->ScreenW  = width;
			r3dRenderer->ScreenH  = height;
			r3dRenderer->ScreenW2 = width * 0.5f;
			r3dRenderer->ScreenH2 = height * 0.5f;

			r3dRenderer->AllowNullViewport = 0;

			r3dRenderer->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
			r3dRenderer->DoSetViewport(0.0f, 0.0f, width, height);
		}
#endif
	}
}

Menu_AppSelect::Menu_AppSelect()
{
}

Menu_AppSelect::~Menu_AppSelect()
{
}

void Menu_AppSelect::Draw()
{
	return;
}

void ClearFullScreen_Menu()
{
	RestoreDX11MenuBackBuffer();

	float width  = (float)r3dRenderer->d3dpp.BackBufferWidth;
	float height = (float)r3dRenderer->d3dpp.BackBufferHeight;

#ifndef WO_SERVER
	if(!r3dRenderer->GetUseD3D9Present() && g_r3dDX11.IsInitialized())
	{
		width  = (float)R3D_MAX(g_r3dDX11.GetWidth(), 1);
		height = (float)R3D_MAX(g_r3dDX11.GetHeight(), 1);
	}
#endif

	r3dRenderer->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	r3dRenderer->DoSetViewport(0.0f, 0.0f, width, height);
	r3dRenderer->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
}

int Menu_AppSelect::DoModal()
{
	AppSelectMode = APPSELECT_WAITING_FOR_COMMAND;
	released_id = -1;

	Desktop().SetViewSize(r3dRenderer->ScreenW, r3dRenderer->ScreenH);
	r3dMouse::Show(true);

	while(1)
	{
		if(g_bExit)
			return bQuit;

		r3dProcessWindowMessages();

		r3dMouse::Show(true);

		r3dStartFrame();

		mUpdate();

		imgui_Update();

		mDrawStart();

		ClearFullScreen_Menu();

		r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);
		r3dSetFiltering(R3D_POINT);
		r3dRenderer->SetMipMapBias(-6.0f, -1);

		switch(AppSelectMode)
		{
		case APPSELECT_WAITING_FOR_COMMAND:
			{
				const static char* BNames1[] =
				{
					"Update DB",
					"Game (Public Server)",
					"Game (DEV Server)"
				};

				for(int i = 0; i < R3D_ARRAYSIZE(BNames1); i++)
				{
					if(imgui_Button(
						r3dRenderer->ScreenW / 2 - (210 * R3D_ARRAYSIZE(BNames1)) / 2 + 210 * i,
						r3dRenderer->ScreenH / 2 - 30,
						200,
						30,
						BNames1[i],
						0))
					{
						released_id = bUpdateDB + i;
					}
				}

				const static char* BNames2[] =
				{
					"Level Editor",
					"Particle Editor",
					"Physics Editor",
					"Character Editor"
				};

				for(int i = 0; i < R3D_ARRAYSIZE(BNames2); i++)
				{
					if(imgui_Button(
						r3dRenderer->ScreenW / 2 - (210 * R3D_ARRAYSIZE(BNames2)) / 2 + 210 * i,
						r3dRenderer->ScreenH / 2 + 30,
						200,
						30,
						BNames2[i],
						0))
					{
						released_id = bStartLevelEditor + i;
					}
				}
			}
			break;
		}

		r3dRenderer->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		r3dRenderer->SetRenderingMode(R3D_BLEND_NOALPHA | R3D_BLEND_NZ);

		mDrawEnd();

		r3dEndFrame();

		switch(released_id)
		{
		case -1:
			break;

		default:
			return released_id;
		}
	}

	return 0;
}