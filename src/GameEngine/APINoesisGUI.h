#pragma once

#ifndef __API_NOESIS_GUI_H__
#define __API_NOESIS_GUI_H__

#include "r3d.h"
#include <windows.h>

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

	bool ProcessWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	bool MouseMove(int x, int y);
	bool MouseButtonDown(int x, int y, int button);
	bool MouseButtonUp(int x, int y, int button);
	bool MouseWheel(int x, int y, int delta);

	bool KeyDown(int vk);
	bool KeyUp(int vk);
	bool Char(unsigned int ch);

	bool PopEditorCommand(char* command, int commandSize, char* value, int valueSize);

	void SetD3D9Device(void* device);

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
	typedef void (__cdecl* FN_WarNoesisCommandCallback)(const char* command, const char* value);
	typedef void (__cdecl* FN_WarNoesis_SetCommandCallback)(FN_WarNoesisCommandCallback callback);
	typedef void (__cdecl* FN_WarNoesis_SetD3D9Device)(void* device);

	typedef int (__cdecl* FN_WarNoesis_MouseMove)(int x, int y);
	typedef int (__cdecl* FN_WarNoesis_MouseButtonDown)(int x, int y, int button);
	typedef int (__cdecl* FN_WarNoesis_MouseButtonUp)(int x, int y, int button);
	typedef int (__cdecl* FN_WarNoesis_MouseWheel)(int x, int y, int delta);
	typedef int (__cdecl* FN_WarNoesis_KeyDown)(int vk);
	typedef int (__cdecl* FN_WarNoesis_KeyUp)(int vk);
	typedef int (__cdecl* FN_WarNoesis_Char)(unsigned int ch);

	FN_WarNoesis_Init FnInit;
	FN_WarNoesis_Shutdown FnShutdown;
	FN_WarNoesis_LoadXamlFile FnLoadXamlFile;
	FN_WarNoesis_UnloadXaml FnUnloadXaml;
	FN_WarNoesis_SetSize FnSetSize;
	FN_WarNoesis_Update FnUpdate;
	FN_WarNoesis_Render FnRender;
	FN_WarNoesis_IsLoaded FnIsLoaded;
	FN_WarNoesis_SetCommandCallback FnSetCommandCallback;
	FN_WarNoesis_SetD3D9Device FnSetD3D9Device;

	FN_WarNoesis_MouseMove FnMouseMove;
	FN_WarNoesis_MouseButtonDown FnMouseButtonDown;
	FN_WarNoesis_MouseButtonUp FnMouseButtonUp;
	FN_WarNoesis_MouseWheel FnMouseWheel;
	FN_WarNoesis_KeyDown FnKeyDown;
	FN_WarNoesis_KeyUp FnKeyUp;
	FN_WarNoesis_Char FnChar;

	bool Initialized;
	bool Loaded;
	bool MsgProcRegistered;
};

extern APINoesisGUI* gNoesisGUI;

void r3dNoesisGUICreate();
void r3dNoesisGUIDestroy();
void r3dNoesisGUIReset();

const char* r3dNoesisGetLastEditorCommand();
bool r3dNoesisPopEditorCommand(char* command, int commandSize, char* value, int valueSize);

#endif