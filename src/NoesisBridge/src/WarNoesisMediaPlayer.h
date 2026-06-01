#pragma once

#include <NsCore/Noesis.h>
#include <NsCore/Ptr.h>

namespace Noesis
{
    struct Uri;
}

namespace NoesisApp
{
    class MediaElement;
    class MediaPlayer;
}

void WarNoesisMediaPlayer_Init(const char* rootPath);
void WarNoesisMediaPlayer_Shutdown();

Noesis::Ptr<NoesisApp::MediaPlayer> WarNoesisMediaPlayer_Create(
    NoesisApp::MediaElement* owner,
    const Noesis::Uri& uri,
    void* user
);