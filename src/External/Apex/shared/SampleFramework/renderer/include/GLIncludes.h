#ifndef GL_INCLUDES_H
#define GL_INCLUDES_H

#if defined(RENDERER_WINDOWS)
    #include <windows/WindowsGLIncludes.h>
#elif defined(RENDERER_PS3)
    #include <ps3/PS3GLIncludes.h>
#elif defined(RENDERER_MACOSX)
    #include <osx/OSXGLIncludes.h>
#elif defined(RENDERER_XBOX360)
    #include <xbox360/Xbox360GLIncludes.h>
#else
#error Undefined platform.
#endif


#endif