#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "m_AppSelect.h"
#include "APINoesisGUI.h"

#include <windows.h>
#include <windowsx.h>

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

struct StudioMenuButton
{
	float x;
	float y;
	float w;
	float h;
	const char* text;
	const char* command;
	int result;
};

static bool gStudioPrevMouseDown = false;

static CD3DFont* gStudioMenuFont = NULL;

static CD3DFont* StudioGetFont()
{
	if(Font_Label)
		return Font_Label;

	if(gStudioMenuFont)
		return gStudioMenuFont;

	if(!r3dRenderer || !r3dRenderer->pd3ddev)
		return NULL;

	r3dIntegrityGuardian ig;

	gStudioMenuFont = new CD3DFont(
		ig,
		"Verdana",
		10,
		D3DFONT_BOLD | D3DFONT_FILTERED | D3DFONT_SKIPGLYPH
	);

	if(FAILED(gStudioMenuFont->CreateSystemFont()))
	{
		SAFE_DELETE(gStudioMenuFont);
		return NULL;
	}

	return gStudioMenuFont;
}

static void StudioPrintText(float x, float y, const r3dColor24& color, const char* text)
{
	CD3DFont* font = StudioGetFont();

	if(!font)
		return;

	if(!text)
		text = "";

	font->PrintF(x, y, color, "%s", text);
}

static float StudioGetTextWidth(const char* text)
{
	CD3DFont* font = StudioGetFont();

	if(!font)
		return 0.0f;

	if(!text)
		text = "";

	SIZE textSize;
	textSize.cx = 0;
	textSize.cy = 0;

	font->GetTextExtent(text, &textSize);

	return (float)textSize.cx;
}

static bool StudioPointInRect(float px, float py, float x, float y, float w, float h)
{
	return px >= x && px <= x + w && py >= y && py <= y + h;
}

static void StudioGetMouse(float* outX, float* outY, bool* outDown)
{
	POINT pt;
	GetCursorPos(&pt);

	if(win::hWnd)
		ScreenToClient(win::hWnd, &pt);

	*outX = (float)pt.x;
	*outY = (float)pt.y;
	*outDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
}

static void StudioDrawButton(const StudioMenuButton& button, float mouseX, float mouseY, bool mouseDown)
{
	bool hovered = StudioPointInRect(mouseX, mouseY, button.x, button.y, button.w, button.h);
	bool pressed = hovered && mouseDown;

	r3dColor borderColor = hovered ? r3dColor(255, 30, 20, 255) : r3dColor(80, 88, 98, 255);
	r3dColor fillColor = pressed ? r3dColor(46, 54, 64, 230) : hovered ? r3dColor(32, 36, 43, 230) : r3dColor(5, 6, 7, 230);
	r3dColor glossColor = r3dColor(255, 255, 255, 22);

	r3dDrawBox2D(button.x, button.y, button.w, button.h, borderColor);
	r3dDrawBox2D(button.x + 1, button.y + 1, button.w - 2, button.h - 2, fillColor);
	r3dDrawBox2D(button.x + 2, button.y + 2, button.w - 4, 8, glossColor);

	const char* buttonText = button.text ? button.text : "";

	float textW = StudioGetTextWidth(buttonText);

	StudioPrintText(
		button.x + button.w * 0.5f - textW * 0.5f,
		button.y + 8,
		r3dColor(235, 240, 250, 255),
		buttonText
	);
}

static int StudioDrawAndProcessMenu()
{
	float sw = r3dRenderer->ScreenW;
	float sh = r3dRenderer->ScreenH;

	float mouseX = 0.0f;
	float mouseY = 0.0f;
	bool mouseDown = false;

	StudioGetMouse(&mouseX, &mouseY, &mouseDown);

	bool clicked = !mouseDown && gStudioPrevMouseDown;
	gStudioPrevMouseDown = mouseDown;

	const float btnW = 220.0f;
	const float btnH = 30.0f;
	const float gap = 16.0f;

	float row1W = btnW * 3.0f + gap * 2.0f;
	float row2W = btnW * 4.0f + gap * 3.0f;

	float row1X = sw * 0.5f - row1W * 0.5f;
	float row2X = sw * 0.5f - row2W * 0.5f;

	float row1Y = sh * 0.5f - 55.0f;
	float row2Y = sh * 0.5f + 28.0f;

	StudioMenuButton buttons[] =
	{
		{ row1X + (btnW + gap) * 0.0f, row1Y, btnW, btnH, "Update DB",             "BtnUpdateDB",        Menu_AppSelect::bUpdateDB },
		{ row1X + (btnW + gap) * 1.0f, row1Y, btnW, btnH, "Game (Public Server)",  "BtnGamePublic",      Menu_AppSelect::bStartGamePublic },
		{ row1X + (btnW + gap) * 2.0f, row1Y, btnW, btnH, "Game (DEV Server)",     "BtnGameDev",         Menu_AppSelect::bStartGameSVN },

		{ row2X + (btnW + gap) * 0.0f, row2Y, btnW, btnH, "Level Editor",          "BtnLevelEditor",     Menu_AppSelect::bStartLevelEditor },
		{ row2X + (btnW + gap) * 1.0f, row2Y, btnW, btnH, "Particle Editor",       "BtnParticleEditor",  Menu_AppSelect::bStartParticleEditor },
		{ row2X + (btnW + gap) * 2.0f, row2Y, btnW, btnH, "Physics Editor",        "BtnPhysicsEditor",   Menu_AppSelect::bStartPhysicsEditor },
		{ row2X + (btnW + gap) * 3.0f, row2Y, btnW, btnH, "Character Editor",      "BtnCharacterEditor", Menu_AppSelect::bStartCharacterEditor }
	};

	StudioPrintText(
		sw * 0.5f - 78.0f,
		sh * 0.5f - 125.0f,
		r3dColor(190, 200, 215, 255),
		"ECLIPSE STUDIO"
	);

	int result = -1;

	for(int i = 0; i < _countof(buttons); ++i)
	{
		StudioDrawButton(buttons[i], mouseX, mouseY, mouseDown);

		if(clicked && StudioPointInRect(mouseX, mouseY, buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h))
		{
			r3dOutToLog("Studio AppSelect command: %s\n", buttons[i].command);
			result = buttons[i].result;
		}
	}

	return result;
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

	StudioGetFont();

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
		}

		int nativeResult = StudioDrawAndProcessMenu();
		if(nativeResult != -1)
		{
			mDrawEnd();
			r3dEndFrame();
			return nativeResult;
		}

		{
			char debugText[512];

			sprintf(
				debugText,
				"NOESIS APPSELECT: loaded=%d last_cmd=%s",
				gNoesisGUI && gNoesisGUI->IsLoaded() ? 1 : 0,
				r3dNoesisGetLastEditorCommand()
			);

			StudioPrintText(
				8,
				8,
				r3dColor(0, 255, 0),
				debugText
			);
		}

		StudioPrintText(
			8,
			28,
			r3dColor(255, 160, 0),
			"Noesis RenderDevice not connected yet, drawing temporary native shell"
		);

		r3dRenderer->pd3ddev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		r3dRenderer->SetRenderingMode(R3D_BLEND_NOALPHA | R3D_BLEND_NZ);

		mDrawEnd();

		r3dEndFrame();
	}

	return 0;
}