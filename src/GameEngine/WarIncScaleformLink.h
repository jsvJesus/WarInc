#pragma once

#ifndef WARINC_SCALEFORM_AUTOLINK
#define WARINC_SCALEFORM_AUTOLINK 1
#endif

#ifndef WARINC_SCALEFORM_USE_SPLIT_D3D9_LIBS
#define WARINC_SCALEFORM_USE_SPLIT_D3D9_LIBS 0
#endif

#if WARINC_SCALEFORM_AUTOLINK

#pragma comment(lib, "libpng.lib")
#pragma comment(lib, "libjpeg.lib")

#if defined(_DEBUG)

#pragma comment(lib, "libgfx_DebugOpt.lib")
#pragma comment(lib, "libgfx_as2_DebugOpt.lib")

#if WARINC_SCALEFORM_USE_SPLIT_D3D9_LIBS
#pragma comment(lib, "libgfxplatform_d3d9_DebugOpt.lib")
#pragma comment(lib, "libgfxrender_d3d9_DebugOpt.lib")
#endif

#elif defined(FINAL_BUILD)

#pragma comment(lib, "libgfx_Shipping.lib")
#pragma comment(lib, "libgfx_as2_Shipping.lib")

#if WARINC_SCALEFORM_USE_SPLIT_D3D9_LIBS
#pragma comment(lib, "libgfxplatform_d3d9_Shipping.lib")
#pragma comment(lib, "libgfxrender_d3d9_Shipping.lib")
#endif

#else

#pragma comment(lib, "libgfx_Release.lib")
#pragma comment(lib, "libgfx_as2_Release.lib")

#if WARINC_SCALEFORM_USE_SPLIT_D3D9_LIBS
#pragma comment(lib, "libgfxplatform_d3d9_Release.lib")
#pragma comment(lib, "libgfxrender_d3d9_Release.lib")
#endif

#endif

#endif