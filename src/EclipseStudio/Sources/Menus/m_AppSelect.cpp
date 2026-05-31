#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "m_AppSelect.h"
#include "APINoesisGUI.h"

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
	r3dRenderer->pd3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	r3dRenderer->SetViewport(
		0.f,
		0.f,
		(float)r3dRenderer->d3dpp.BackBufferWidth,
		(float)r3dRenderer->d3dpp.BackBufferHeight
	);
	D3D_V(r3dRenderer->pd3ddev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.f, 0));
	r3dRenderer->pd3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
}

static int AppSelectCommandToResult(const char* command)
{
	if(!command || !command[0])
		return -1;

	if(strcmp(command, "BtnUpdateDB") == 0)
		return Menu_AppSelect::bUpdateDB;

	if(strcmp(command, "BtnGamePublic") == 0)
		return Menu_AppSelect::bStartGamePublic;

	if(strcmp(command, "BtnGameDev") == 0)
		return Menu_AppSelect::bStartGameSVN;

	if(strcmp(command, "BtnLevelEditor") == 0)
		return Menu_AppSelect::bStartLevelEditor;

	if(strcmp(command, "BtnParticleEditor") == 0)
		return Menu_AppSelect::bStartParticleEditor;

	if(strcmp(command, "BtnPhysicsEditor") == 0)
		return Menu_AppSelect::bStartPhysicsEditor;

	if(strcmp(command, "BtnCharacterEditor") == 0)
		return Menu_AppSelect::bStartCharacterEditor;

	return -1;
}

static int ProcessAppSelectNoesisCommands()
{
	char command[128];
	char value[256];

	while(r3dNoesisPopEditorCommand(command, sizeof(command), value, sizeof(value)))
	{
		r3dOutToLog("Noesis AppSelect command: %s value=%s\n", command, value);

		int result = AppSelectCommandToResult(command);

		if(result != -1)
			return result;
	}

	return -1;
}

int Menu_AppSelect::DoModal()
{
	AppSelectMode = 100;
	released_id = -1;

	Desktop().SetViewSize(r3dRenderer->ScreenW, r3dRenderer->ScreenH);

	r3dMouse::Show();

	if(gNoesisGUI)
	{
		gNoesisGUI->SetSize((int)r3dRenderer->ScreenW, (int)r3dRenderer->ScreenH);
		gNoesisGUI->LoadXaml("Studio/AppSelect.xaml");
	}

	while(1)
	{
		if(g_bExit)
			return 0;

		mUpdate();

		int noesisResult = ProcessAppSelectNoesisCommands();
		if(noesisResult != -1)
			return noesisResult;

		r3dStartFrame();

		mDrawStart();

		ClearFullScreen_Menu();

		r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);
		r3dSetFiltering(R3D_POINT);
		r3dRenderer->SetMipMapBias(-6.0f, -1);

		if(gNoesisGUI && gNoesisGUI->IsLoaded())
		{
			gNoesisGUI->SetSize((int)r3dRenderer->ScreenW, (int)r3dRenderer->ScreenH);
			gNoesisGUI->Update(r3dGetTime());
			gNoesisGUI->Render();

			Font_Label->PrintF(
				10,
				10,
				r3dColor(0, 255, 0),
				"NOESIS APPSELECT: loaded=1 last_cmd=%s",
				r3dNoesisGetLastEditorCommand()
			);
		}
		else
		{
			Font_Label->PrintF(
				10,
				10,
				r3dColor(255, 0, 0),
				"NOESIS APPSELECT: not loaded"
			);
		}

		r3dRenderer->pd3ddev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		r3dRenderer->SetRenderingMode(R3D_BLEND_NOALPHA | R3D_BLEND_NZ);

		mDrawEnd();

		r3dEndFrame();
	}

	return 0;
}