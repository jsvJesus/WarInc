#pragma once

#ifdef WAR_NOESIS_BRIDGE_EXPORTS
#define WAR_NOESIS_API extern "C" __declspec(dllexport)
#else
#define WAR_NOESIS_API extern "C" __declspec(dllimport)
#endif

typedef void (__cdecl* WarNoesisCommandCallback)(const char* command, const char* value);

WAR_NOESIS_API int  __cdecl WarNoesis_Init(const char* rootPath);
WAR_NOESIS_API void __cdecl WarNoesis_Shutdown();

WAR_NOESIS_API int  __cdecl WarNoesis_LoadXamlFile(const char* filename);
WAR_NOESIS_API void __cdecl WarNoesis_UnloadXaml();

WAR_NOESIS_API void __cdecl WarNoesis_SetSize(int width, int height);
WAR_NOESIS_API void __cdecl WarNoesis_Update(double timeSeconds);
WAR_NOESIS_API void __cdecl WarNoesis_Render();

WAR_NOESIS_API int  __cdecl WarNoesis_IsLoaded();

WAR_NOESIS_API void __cdecl WarNoesis_SetCommandCallback(WarNoesisCommandCallback callback);

WAR_NOESIS_API int __cdecl WarNoesis_MouseMove(int x, int y);
WAR_NOESIS_API int __cdecl WarNoesis_MouseButtonDown(int x, int y, int button);
WAR_NOESIS_API int __cdecl WarNoesis_MouseButtonUp(int x, int y, int button);
WAR_NOESIS_API int __cdecl WarNoesis_MouseWheel(int x, int y, int delta);

WAR_NOESIS_API int __cdecl WarNoesis_KeyDown(int vk);
WAR_NOESIS_API int __cdecl WarNoesis_KeyUp(int vk);
WAR_NOESIS_API int __cdecl WarNoesis_Char(unsigned int ch);