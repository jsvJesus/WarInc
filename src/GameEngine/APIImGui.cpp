#include "r3dPCH.h"
#include "r3d.h"
#include "APIImGui.h"

#ifndef WO_SERVER

#pragma warning(push)
#pragma warning(disable: 4005)
#pragma warning(disable: 4100)
#pragma warning(disable: 4127)
#pragma warning(disable: 4244)
#pragma warning(disable: 4312)
#pragma warning(disable: 4996)

#include <d3d11.h>

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif

#pragma warning(pop)

#include "r3dDX11.h"
#include "r3dDX11State.h"
#include "r3dDX11Geometry.h"

typedef bool (*Win32MsgProc_fn)(UINT uMsg, WPARAM wParam, LPARAM lParam);

extern void RegisterMsgProc(Win32MsgProc_fn proc);
extern void UnregisterMsgProc(Win32MsgProc_fn proc);

APIImGui* gImGuiGUI = NULL;

static bool r3dImGuiMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(!gImGuiGUI)
		return false;

	return gImGuiGUI->ProcessWindowMessage(uMsg, wParam, lParam);
}

static float r3dImGuiMin(float a, float b)
{
	return a < b ? a : b;
}

static ImU32 r3dImGuiColor32(int r, int g, int b, int a)
{
	return IM_COL32(r, g, b, a);
}

APIImGui::APIImGui()
{
	Initialized = false;
	MsgProcRegistered = false;
	FrameStarted = false;
	DX11Ready = false;

	D3D11Device = NULL;
	D3D11Context = NULL;
}

APIImGui::~APIImGui()
{
	Shutdown();
}

bool APIImGui::Init()
{
	if(Initialized)
		return true;

	if(!win::hWnd)
	{
		r3dOutToLog("ImGui: Init failed, hwnd is NULL\n");
		return false;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = NULL;
	io.LogFilename = NULL;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;
	style.ChildRounding = 0.0f;
	style.FrameRounding = 0.0f;
	style.PopupRounding = 0.0f;
	style.ScrollbarRounding = 0.0f;
	style.GrabRounding = 0.0f;
	style.TabRounding = 0.0f;
	style.WindowBorderSize = 0.0f;
	style.FrameBorderSize = 0.0f;
	style.WindowPadding = ImVec2(0.0f, 0.0f);
	style.FramePadding = ImVec2(12.0f, 8.0f);
	style.ItemSpacing = ImVec2(8.0f, 8.0f);

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(win::hWnd);

	RegisterMsgProc(r3dImGuiMsgProc);
	MsgProcRegistered = true;

	Initialized = true;

	r3dOutToLog("ImGui: Init OK, DX11 only\n");
	return true;
}

void APIImGui::ShutdownRenderer()
{
	if(DX11Ready)
	{
		ImGui_ImplDX11_Shutdown();
	}

	DX11Ready = false;
	D3D11Device = NULL;
	D3D11Context = NULL;
}

void APIImGui::Shutdown()
{
	if(FrameStarted)
	{
		ImGui::EndFrame();
		FrameStarted = false;
	}

	ShutdownRenderer();

	if(MsgProcRegistered)
	{
		UnregisterMsgProc(r3dImGuiMsgProc);
		MsgProcRegistered = false;
	}

	if(Initialized)
	{
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		Initialized = false;
	}

	r3dOutToLog("ImGui: Shutdown OK\n");
}

bool APIImGui::SetD3D11Device(void* device, void* context)
{
	if(!device || !context)
		return false;

	if(!Init())
		return false;

	if(DX11Ready && D3D11Device == device && D3D11Context == context)
		return true;

	ShutdownRenderer();

	D3D11Device = device;
	D3D11Context = context;

	if(!ImGui_ImplDX11_Init((ID3D11Device*)D3D11Device, (ID3D11DeviceContext*)D3D11Context))
	{
		D3D11Device = NULL;
		D3D11Context = NULL;
		DX11Ready = false;
		r3dOutToLog("ImGui: DX11 backend init failed\n");
		return false;
	}

	DX11Ready = true;

	r3dOutToLog("ImGui: DX11 backend OK\n");
	return true;
}

bool APIImGui::BeginFrame()
{
	if(!Initialized)
		return false;

	if(!DX11Ready)
		return false;

	if(FrameStarted)
		return true;

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	FrameStarted = true;
	return true;
}

void APIImGui::EndFrame()
{
	if(!FrameStarted)
		return;

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	FrameStarted = false;
}

bool APIImGui::IsInitialized() const
{
	return Initialized;
}

bool APIImGui::IsRendererReady() const
{
	return Initialized && DX11Ready;
}

bool APIImGui::ProcessWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(!Initialized)
		return false;

	ImGui_ImplWin32_WndProcHandler(win::hWnd, uMsg, wParam, lParam);

	ImGuiIO& io = ImGui::GetIO();

	switch(uMsg)
	{
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
		return io.WantCaptureMouse;

	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		return io.WantCaptureKeyboard;

	case WM_CHAR:
		return io.WantTextInput || io.WantCaptureKeyboard;
	}

	return false;
}

void r3dImGuiCreate()
{
	if(gImGuiGUI)
		return;

	gImGuiGUI = new APIImGui();

	if(!gImGuiGUI->Init())
	{
		SAFE_DELETE(gImGuiGUI);
	}
}

void r3dImGuiDestroy()
{
	if(!gImGuiGUI)
		return;

	gImGuiGUI->Shutdown();
	SAFE_DELETE(gImGuiGUI);
}

void r3dImGuiResetDX11()
{
	if(!gImGuiGUI)
		return;

	if(!g_r3dDX11.IsInitialized())
		return;

	gImGuiGUI->SetD3D11Device(
		g_r3dDX11.GetDevice(),
		g_r3dDX11.GetContext()
	);
}

bool r3dImGuiBeginFrame()
{
	if(!gImGuiGUI)
		return false;

	return gImGuiGUI->BeginFrame();
}

void r3dImGuiEndFrame()
{
	if(!gImGuiGUI)
		return;

	gImGuiGUI->EndFrame();
}

static void r3dImGuiAddTextCentered(
	ImDrawList* draw,
	const char* text,
	float x,
	float y,
	float w,
	ImU32 color,
	float fontSize
)
{
	if(!draw || !text)
		return;

	ImFont* font = ImGui::GetFont();
	ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);

	draw->AddText(
		font,
		fontSize,
		ImVec2(x + w * 0.5f - textSize.x * 0.5f, y),
		color,
		text
	);
}

static bool r3dImGuiStudioButton(
	ImDrawList* draw,
	const char* id,
	const char* text,
	float x,
	float y,
	float w,
	float h,
	bool primary,
	float scale
)
{
	ImGui::SetCursorScreenPos(ImVec2(x, y));
	ImGui::InvisibleButton(id, ImVec2(w, h));

	bool hovered = ImGui::IsItemHovered();
	bool active = ImGui::IsItemActive();
	bool clicked = ImGui::IsItemClicked(0);

	ImU32 fillColor;
	ImU32 innerColor;
	ImU32 borderColor;
	ImU32 textColor;
	ImU32 arrowColor;

	if(primary)
	{
		fillColor = active ? r3dImGuiColor32(8, 34, 54, 235) : hovered ? r3dImGuiColor32(20, 82, 122, 235) : r3dImGuiColor32(16, 75, 114, 230);
		innerColor = r3dImGuiColor32(57, 168, 232, 210);
		borderColor = hovered ? r3dImGuiColor32(255, 255, 255, 255) : r3dImGuiColor32(57, 168, 232, 255);
		textColor = r3dImGuiColor32(242, 247, 252, 255);
		arrowColor = r3dImGuiColor32(57, 168, 232, 255);
	}
	else
	{
		fillColor = active ? r3dImGuiColor32(8, 34, 54, 220) : hovered ? r3dImGuiColor32(16, 75, 114, 220) : r3dImGuiColor32(11, 48, 75, 190);
		innerColor = hovered ? r3dImGuiColor32(57, 168, 232, 180) : r3dImGuiColor32(35, 110, 158, 120);
		borderColor = hovered ? r3dImGuiColor32(57, 168, 232, 255) : r3dImGuiColor32(30, 85, 120, 180);
		textColor = hovered ? r3dImGuiColor32(242, 247, 252, 255) : r3dImGuiColor32(91, 155, 196, 230);
		arrowColor = hovered ? r3dImGuiColor32(57, 168, 232, 255) : r3dImGuiColor32(57, 168, 232, 180);
	}

	draw->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), r3dImGuiColor32(5, 16, 26, 170));
	draw->AddRectFilled(ImVec2(x + 4.0f * scale, y + 4.0f * scale), ImVec2(x + w - 4.0f * scale, y + h - 4.0f * scale), fillColor);

	draw->AddRectFilled(ImVec2(x + 12.0f * scale, y + 8.0f * scale), ImVec2(x + w - 82.0f * scale, y + 10.0f * scale), borderColor);
	draw->AddRectFilled(ImVec2(x + 12.0f * scale, y + h - 10.0f * scale), ImVec2(x + w - 82.0f * scale, y + h - 8.0f * scale), borderColor);
	draw->AddRectFilled(ImVec2(x + 12.0f * scale, y + 8.0f * scale), ImVec2(x + 14.0f * scale, y + h - 8.0f * scale), borderColor);
	draw->AddRectFilled(ImVec2(x + w - 72.0f * scale, y + 8.0f * scale), ImVec2(x + w - 70.0f * scale, y + h - 8.0f * scale), borderColor);

	draw->AddRectFilled(ImVec2(x + w - 70.0f * scale, y + 4.0f * scale), ImVec2(x + w - 4.0f * scale, y + h - 4.0f * scale), r3dImGuiColor32(10, 35, 55, 190));
	draw->AddRectFilled(ImVec2(x + w - 68.0f * scale, y + 6.0f * scale), ImVec2(x + w - 66.0f * scale, y + h - 6.0f * scale), innerColor);

	r3dImGuiAddTextCentered(
		draw,
		text,
		x,
		y + h * 0.5f - 10.0f * scale,
		w - 70.0f * scale,
		textColor,
		20.0f * scale
	);

	draw->AddText(
		ImGui::GetFont(),
		26.0f * scale,
		ImVec2(x + w - 44.0f * scale, y + h * 0.5f - 14.0f * scale),
		arrowColor,
		">"
	);

	return clicked;
}

static void r3dImGuiDrawAppSelectBackground(ImDrawList* draw, float sw, float sh)
{
	draw->AddRectFilled(ImVec2(0.0f, 0.0f), ImVec2(sw, sh), r3dImGuiColor32(3, 10, 16, 255));
	draw->AddRectFilled(ImVec2(0.0f, 0.0f), ImVec2(sw, sh), r3dImGuiColor32(0, 20, 32, 120));

	for(int i = 0; i < 18; ++i)
	{
		float x = (float)i * (sw / 18.0f);

		draw->AddLine(
			ImVec2(x, 0.0f),
			ImVec2(x + sw * 0.15f, sh),
			r3dImGuiColor32(18, 62, 88, 35),
			1.0f
		);
	}

	draw->AddRectFilled(
		ImVec2(0.0f, sh - 120.0f),
		ImVec2(sw, sh),
		r3dImGuiColor32(0, 0, 0, 120)
	);
}

static void r3dImGuiDrawAppSelectLogo(ImDrawList* draw, float sw, float sh, float scale)
{
	float logoY = sh * 0.5f - 315.0f * scale;
	float centerX = sw * 0.5f;

	float iconSize = 76.0f * scale;
	float iconX = centerX - 270.0f * scale;
	float iconY = logoY + 4.0f * scale;

	draw->AddRectFilled(ImVec2(iconX, iconY), ImVec2(iconX + iconSize, iconY + iconSize), r3dImGuiColor32(57, 168, 232, 255));
	draw->AddRectFilled(ImVec2(iconX + 10.0f * scale, iconY + 10.0f * scale), ImVec2(iconX + iconSize - 10.0f * scale, iconY + iconSize - 10.0f * scale), r3dImGuiColor32(10, 35, 55, 255));
	draw->AddRectFilled(ImVec2(iconX + 22.0f * scale, iconY + 22.0f * scale), ImVec2(iconX + iconSize - 22.0f * scale, iconY + iconSize - 22.0f * scale), r3dImGuiColor32(57, 168, 232, 180));
	draw->AddRectFilled(ImVec2(iconX + 29.0f * scale, iconY + 29.0f * scale), ImVec2(iconX + iconSize - 29.0f * scale, iconY + iconSize - 29.0f * scale), r3dImGuiColor32(10, 35, 55, 255));

	draw->AddText(
		ImGui::GetFont(),
		42.0f * scale,
		ImVec2(centerX - 165.0f * scale, logoY + 17.0f * scale),
		r3dImGuiColor32(242, 247, 252, 255),
		"WARINC"
	);

	draw->AddText(
		ImGui::GetFont(),
		42.0f * scale,
		ImVec2(centerX + 10.0f * scale, logoY + 17.0f * scale),
		r3dImGuiColor32(57, 168, 232, 255),
		"STUDIO"
	);

	draw->AddText(
		ImGui::GetFont(),
		15.0f * scale,
		ImVec2(centerX - 165.0f * scale, logoY + 70.0f * scale),
		r3dImGuiColor32(91, 155, 196, 210),
		"DX11 IMGUI FRONTEND"
	);
}

int r3dImGuiRenderAppSelectMenu()
{
	if(!g_r3dDX11.IsInitialized())
		return R3D_IMGUI_APPSELECT_NONE;

	r3dImGuiResetDX11();

	if(!r3dImGuiBeginFrame())
		return R3D_IMGUI_APPSELECT_NONE;

	ImGuiIO& io = ImGui::GetIO();

	float sw = io.DisplaySize.x;
	float sh = io.DisplaySize.y;

	if(sw <= 0.0f)
		sw = 1280.0f;

	if(sh <= 0.0f)
		sh = 720.0f;

	float scale = r3dImGuiMin(sw / 1920.0f, sh / 1080.0f);

	if(scale < 0.65f)
		scale = 0.65f;

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(sw, sh), ImGuiCond_Always);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	ImGui::Begin("WarInc_AppSelect", NULL, flags);

	ImDrawList* draw = ImGui::GetWindowDrawList();

	r3dImGuiDrawAppSelectBackground(draw, sw, sh);
	r3dImGuiDrawAppSelectLogo(draw, sw, sh, scale);

	float btnW = 720.0f * scale;
	float btnH = 62.0f * scale;
	float gap = 18.0f * scale;

	float startX = sw * 0.5f - btnW * 0.5f;
	float startY = sh * 0.5f - 105.0f * scale;

	int result = R3D_IMGUI_APPSELECT_NONE;

	if(r3dImGuiStudioButton(draw, "BtnPlayGame", "PLAY GAME", startX, startY + (btnH + gap) * 0.0f, btnW, btnH, true, scale))
		result = R3D_IMGUI_APPSELECT_PLAY_GAME;

	if(r3dImGuiStudioButton(draw, "BtnLevelEditor", "LEVEL EDITOR", startX, startY + (btnH + gap) * 1.0f, btnW, btnH, false, scale))
		result = R3D_IMGUI_APPSELECT_LEVEL_EDITOR;

	if(r3dImGuiStudioButton(draw, "BtnParticleEditor", "PARTICLE EDITOR", startX, startY + (btnH + gap) * 2.0f, btnW, btnH, false, scale))
		result = R3D_IMGUI_APPSELECT_PARTICLE_EDITOR;

	if(r3dImGuiStudioButton(draw, "BtnPhysicsEditor", "PHYSICS EDITOR", startX, startY + (btnH + gap) * 3.0f, btnW, btnH, false, scale))
		result = R3D_IMGUI_APPSELECT_PHYSICS_EDITOR;

	if(r3dImGuiStudioButton(draw, "BtnCharacterEditor", "CHARACTER EDITOR", startX, startY + (btnH + gap) * 4.0f, btnW, btnH, false, scale))
		result = R3D_IMGUI_APPSELECT_CHARACTER_EDITOR;

	if(r3dImGuiStudioButton(draw, "BtnExit", "EXIT", startX, startY + (btnH + gap) * 5.0f, btnW, btnH, false, scale))
		result = R3D_IMGUI_APPSELECT_EXIT;

	draw->AddText(
		ImGui::GetFont(),
		14.0f * scale,
		ImVec2(18.0f * scale, sh - 32.0f * scale),
		r3dImGuiColor32(91, 155, 196, 180),
		"NoesisGUI removed. Dear ImGui DX11 frontend active."
	);

	if((GetAsyncKeyState(VK_ESCAPE) & 1) != 0)
		result = R3D_IMGUI_APPSELECT_EXIT;

	ImGui::End();

	ImGui::PopStyleVar(2);

	r3dImGuiEndFrame();

	return result;
}

#else

APIImGui* gImGuiGUI = NULL;

void r3dImGuiCreate() {}
void r3dImGuiDestroy() {}
void r3dImGuiResetDX11() {}
bool r3dImGuiBeginFrame() { return false; }
void r3dImGuiEndFrame() {}
int r3dImGuiRenderAppSelectMenu() { return R3D_IMGUI_APPSELECT_NONE; }

#endif