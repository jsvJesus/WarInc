#pragma once

#ifndef __API_NOESIS_GUI_H__
#define __API_NOESIS_GUI_H__

#include "r3d.h"

#define WARINC_USE_NOESIS_GUI 1

#if WARINC_USE_NOESIS_GUI

#include <NsCore/Noesis.h>
#include <NsCore/Ptr.h>
#include <NsCore/Log.h>
#include <NsCore/Init.h>

#include <NsGui/IntegrationAPI.h>
#include <NsGui/FrameworkElement.h>
#include <NsGui/IView.h>
#include <NsGui/IRenderer.h>
#include <NsGui/InputEnums.h>
#include <NsGui/Uri.h>

#include <NsApp/LocalXamlProvider.h>
#include <NsApp/LocalTextureProvider.h>
#include <NsApp/LocalFontProvider.h>

#endif

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

    bool MouseMove(int x, int y);
    bool MouseButtonDown(int x, int y, int button);
    bool MouseButtonUp(int x, int y, int button);
    bool MouseWheel(int x, int y, int wheelDelta);

    bool KeyDown(int vk);
    bool KeyUp(int vk);
    bool Char(unsigned int ch);

private:
    bool Initialized;
    bool Loaded;
    bool RendererInitialized;

    int Width;
    int Height;

#if WARINC_USE_NOESIS_GUI
    Noesis::Ptr<Noesis::FrameworkElement> Root;
    Noesis::Ptr<Noesis::IView> View;

    static void NoesisLogHandler(const char* file, uint32_t line, uint32_t level, const char* channel, const char* message);

    static Noesis::MouseButton ConvertMouseButton(int button);
    static Noesis::Key ConvertKey(int vk);
#endif
};

extern APINoesisGUI* gNoesisGUI;

void r3dNoesisGUICreate();
void r3dNoesisGUIDestroy();
void r3dNoesisGUIReset();

#endif