#pragma once

#ifndef WARINC_SCALEFORM_AUTOLINK
#define WARINC_SCALEFORM_AUTOLINK 1
#endif

#if WARINC_SCALEFORM_AUTOLINK

#if defined(_WIN64)

#pragma comment(lib, "..\\External\\Scaleform\\Lib\\x64\\Msvc10\\Release\\libgfx.lib")
#pragma comment(lib, "..\\External\\Scaleform\\Lib\\x64\\Msvc10\\Release\\libgfx_as2.lib")
#pragma comment(lib, "..\\External\\Scaleform\\Lib\\x64\\Msvc10\\Release\\libgfxplatform_d3d9.lib")
#pragma comment(lib, "..\\External\\Scaleform\\Lib\\x64\\Msvc10\\Release\\libgfxrender_d3d9.lib")
#pragma comment(lib, "..\\External\\Scaleform\\Lib\\x64\\Msvc10\\Release\\libgfxexpat.lib")

#pragma comment(lib, "..\\External\\Scaleform\\3rdParty\\zlib-1.2.7\\Lib\\x64\\Msvc10\\Release\\zlib.lib")
#pragma comment(lib, "..\\External\\Scaleform\\3rdParty\\jpeg-8d\\Lib\\x64\\Msvc10\\Release\\libjpeg.lib")
#pragma comment(lib, "..\\External\\Scaleform\\3rdParty\\libpng-1.5.13\\Lib\\x64\\Msvc10\\Release\\libpng.lib")
#pragma comment(lib, "..\\External\\Scaleform\\3rdParty\\pcre\\Lib\\x64\\Msvc10\\Release\\pcre.lib")
#pragma comment(lib, "..\\External\\Scaleform\\3rdParty\\expat-2.1.0\\Lib\\x64\\Msvc10\\Release\\expat.lib")

#else

#pragma comment(lib, "libpng.lib")
#pragma comment(lib, "libjpeg.lib")

#if defined(_DEBUG)
#pragma comment(lib, "libgfx_DebugOpt.lib")
#pragma comment(lib, "libgfx_as2_DebugOpt.lib")
#elif defined(FINAL_BUILD)
#pragma comment(lib, "libgfx_Shipping.lib")
#pragma comment(lib, "libgfx_as2_Shipping.lib")
#else
#pragma comment(lib, "libgfx.lib")
#pragma comment(lib, "libgfx_as2.lib")
#endif

#endif

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "version.lib")

#endif