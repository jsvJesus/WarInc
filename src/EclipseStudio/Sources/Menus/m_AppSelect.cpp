#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"

#include "m_AppSelect.h"
#include "APINoesisGUI.h"

#include <windows.h>
#include <windowsx.h>
#include <string.h>

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
	int primary;
};

static bool gStudioPrevMouseDown = false;
static CD3DFont* gStudioMenuFont = NULL;

static float StudioMinFloat(float a, float b)
{
	return a < b ? a : b;
}

static CD3DFont* StudioGetFont()
{
	if(gStudioMenuFont)
		return gStudioMenuFont;

	if(!r3dRenderer || !r3dRenderer->pd3ddev)
		return NULL;

	r3dIntegrityGuardian ig;

	gStudioMenuFont = new CD3DFont(
		ig,
		"Verdana",
		14,
		D3DFONT_BOLD
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

static void StudioDrawTextCentered(float x, float y, float w, const r3dColor24& color, const char* text)
{
	float textW = StudioGetTextWidth(text);
	StudioPrintText(x + w * 0.5f - textW * 0.5f, y, color, text);
}

static void StudioDrawButton(const StudioMenuButton& button, float mouseX, float mouseY, bool mouseDown)
{
	bool hovered = StudioPointInRect(mouseX, mouseY, button.x, button.y, button.w, button.h);
	bool pressed = hovered && mouseDown;

	r3dColor fillColor;
	r3dColor innerColor;
	r3dColor borderColor;
	r3dColor textColor;
	r3dColor arrowColor;

	if(button.primary)
	{
		fillColor = pressed ? r3dColor(8, 34, 54, 235) : hovered ? r3dColor(20, 82, 122, 235) : r3dColor(16, 75, 114, 230);
		innerColor = r3dColor(57, 168, 232, 210);
		borderColor = hovered ? r3dColor(255, 255, 255, 255) : r3dColor(57, 168, 232, 255);
		textColor = r3dColor(242, 247, 252, 255);
		arrowColor = r3dColor(57, 168, 232, 255);
	}
	else
	{
		fillColor = pressed ? r3dColor(8, 34, 54, 220) : hovered ? r3dColor(16, 75, 114, 220) : r3dColor(11, 48, 75, 190);
		innerColor = hovered ? r3dColor(57, 168, 232, 180) : r3dColor(35, 110, 158, 120);
		borderColor = hovered ? r3dColor(57, 168, 232, 255) : r3dColor(30, 85, 120, 180);
		textColor = hovered ? r3dColor(242, 247, 252, 255) : r3dColor(91, 155, 196, 230);
		arrowColor = hovered ? r3dColor(57, 168, 232, 255) : r3dColor(57, 168, 232, 180);
	}

	r3dDrawBox2D(button.x, button.y, button.w, button.h, r3dColor(5, 16, 26, 170));
	r3dDrawBox2D(button.x + 4, button.y + 4, button.w - 8, button.h - 8, fillColor);

	r3dDrawBox2D(button.x + 12, button.y + 8, button.w - 82, 2, borderColor);
	r3dDrawBox2D(button.x + 12, button.y + button.h - 10, button.w - 82, 2, borderColor);
	r3dDrawBox2D(button.x + 12, button.y + 8, 2, button.h - 16, borderColor);
	r3dDrawBox2D(button.x + button.w - 72, button.y + 8, 2, button.h - 16, borderColor);

	r3dDrawBox2D(button.x + button.w - 70, button.y + 4, 66, button.h - 8, r3dColor(10, 35, 55, 190));
	r3dDrawBox2D(button.x + button.w - 68, button.y + 6, 2, button.h - 12, innerColor);

	StudioDrawTextCentered(
		button.x,
		button.y + button.h * 0.5f - 8.0f,
		button.w - 70.0f,
		textColor,
		button.text
	);

	StudioPrintText(
		button.x + button.w - 44.0f,
		button.y + button.h * 0.5f - 12.0f,
		arrowColor,
		">"
	);
}

static void StudioDrawLogo(float sw, float sh, float scale)
{
	float logoY = sh * 0.5f - 310.0f * scale;
	float centerX = sw * 0.5f;

	float iconSize = 76.0f * scale;
	float iconX = centerX - 250.0f * scale;
	float iconY = logoY + 4.0f * scale;

	r3dDrawBox2D(iconX, iconY, iconSize, iconSize, r3dColor(57, 168, 232, 255));
	r3dDrawBox2D(iconX + 10.0f * scale, iconY + 10.0f * scale, iconSize - 20.0f * scale, iconSize - 20.0f * scale, r3dColor(10, 35, 55, 255));
	r3dDrawBox2D(iconX + 22.0f * scale, iconY + 22.0f * scale, iconSize - 44.0f * scale, iconSize - 44.0f * scale, r3dColor(57, 168, 232, 180));
	r3dDrawBox2D(iconX + 29.0f * scale, iconY + 29.0f * scale, iconSize - 58.0f * scale, iconSize - 58.0f * scale, r3dColor(10, 35, 55, 255));

	StudioPrintText(
		centerX - 130.0f * scale,
		logoY + 22.0f * scale,
		r3dColor(242, 247, 252, 255),
		"GAME"
	);

	StudioPrintText(
		centerX + 10.0f * scale,
		logoY + 22.0f * scale,
		r3dColor(57, 168, 232, 255),
		"STUDIO"
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

	float scale = StudioMinFloat(sw / 1920.0f, sh / 1080.0f);

	if(scale < 0.65f)
		scale = 0.65f;

	const float btnW = 720.0f * scale;
	const float btnH = 62.0f * scale;
	const float gap = 18.0f * scale;

	float startX = sw * 0.5f - btnW * 0.5f;
	float startY = sh * 0.5f - 105.0f * scale;

	StudioDrawLogo(sw, sh, scale);

	StudioMenuButton buttons[] =
	{
		{ startX, startY + (btnH + gap) * 0.0f, btnW, btnH, "PLAY GAME",        "BtnPlayGame",        Menu_AppSelect::bStartGamePublic,      1 },
		{ startX, startY + (btnH + gap) * 1.0f, btnW, btnH, "LEVEL EDITOR",     "BtnLevelEditor",     Menu_AppSelect::bStartLevelEditor,     0 },
		{ startX, startY + (btnH + gap) * 2.0f, btnW, btnH, "PARTICLE EDITOR",  "BtnParticleEditor",  Menu_AppSelect::bStartParticleEditor,  0 },
		{ startX, startY + (btnH + gap) * 3.0f, btnW, btnH, "PHYSICS EDITOR",   "BtnPhysicsEditor",   Menu_AppSelect::bStartPhysicsEditor,   0 },
		{ startX, startY + (btnH + gap) * 4.0f, btnW, btnH, "CHARACTER EDITOR", "BtnCharacterEditor", Menu_AppSelect::bStartCharacterEditor, 0 },
		{ startX, startY + (btnH + gap) * 5.0f, btnW, btnH, "EXIT",             "BtnExit",            Menu_AppSelect::bQuit,                 0 }
	};

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

	if(strcmp(command, "BtnPlayGame") == 0)
		return Menu_AppSelect::bStartGamePublic;

	if(strcmp(command, "BtnLevelEditor") == 0)
		return Menu_AppSelect::bStartLevelEditor;

	if(strcmp(command, "BtnParticleEditor") == 0)
		return Menu_AppSelect::bStartParticleEditor;

	if(strcmp(command, "BtnPhysicsEditor") == 0)
		return Menu_AppSelect::bStartPhysicsEditor;

	if(strcmp(command, "BtnCharacterEditor") == 0)
		return Menu_AppSelect::bStartCharacterEditor;

	if(strcmp(command, "BtnExit") == 0)
		return Menu_AppSelect::bQuit;

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

static void AppSelectUnloadNoesis(bool noesisReady)
{
	if(noesisReady && gNoesisGUI)
		gNoesisGUI->UnloadXaml();
}

int Menu_AppSelect::DoModal()
{
	AppSelectMode = 100;
	released_id = -1;

	Desktop().SetViewSize(r3dRenderer->ScreenW, r3dRenderer->ScreenH);

	r3dMouse::Show();

	bool noesisReady = false;

	if(gNoesisGUI)
	{
		gNoesisGUI->SetD3D9Device(r3dRenderer->pd3ddev);
		gNoesisGUI->SetSize((int)r3dRenderer->ScreenW, (int)r3dRenderer->ScreenH);
		noesisReady = gNoesisGUI->LoadXaml("Studio/AppSelect.xaml");
	}

	if(!noesisReady)
	{
		StudioGetFont();
		r3dOutToLog("AppSelect: Noesis is not ready, native fallback enabled\n");
	}
	else
	{
		r3dOutToLog("AppSelect: Noesis UI enabled\n");
	}

	while(1)
	{
		if(g_bExit)
		{
			AppSelectUnloadNoesis(noesisReady);
			return 0;
		}

		mUpdate();

		int noesisResult = ProcessAppSelectNoesisCommands();

		if(noesisResult != -1)
		{
			AppSelectUnloadNoesis(noesisReady);
			return noesisResult;
		}

		r3dStartFrame();

		mDrawStart();

		ClearFullScreen_Menu();

		r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);
		r3dSetFiltering(R3D_BILINEAR);
		r3dRenderer->SetMipMapBias(-6.0f, -1);

		if(noesisReady && gNoesisGUI && gNoesisGUI->IsLoaded())
		{
			gNoesisGUI->SetSize((int)r3dRenderer->ScreenW, (int)r3dRenderer->ScreenH);
			gNoesisGUI->Update(r3dGetTime());
			gNoesisGUI->Render();
		}
		else
		{
			int nativeResult = StudioDrawAndProcessMenu();

			if(nativeResult != -1)
			{
				mDrawEnd();
				r3dEndFrame();

				AppSelectUnloadNoesis(noesisReady);
				return nativeResult;
			}

			StudioPrintText(
				8,
				8,
				r3dColor(255, 160, 0),
				"Noesis AppSelect failed, drawing native fallback"
			);
		}

		r3dRenderer->pd3ddev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		r3dRenderer->SetRenderingMode(R3D_BLEND_NOALPHA | R3D_BLEND_NZ);

		mDrawEnd();

		r3dEndFrame();
	}

	AppSelectUnloadNoesis(noesisReady);
	return 0;
}