#ifndef __apps_H
#define __apps_H

#include "ui/r3dMenu.h"

// Command binding structure shared with RmlUiBackend
struct AppSelectCommandBinding
{
	const char* ElementId;
	const char* Label;
	int Result;
	int HotKey;
};

class Menu_AppSelect : public MenuLayer
{
public:
	START_MENU_ENUMS
		bUpdateDB,
		bStartGamePublic,
		bStartGameSVN,

		bStartLevelEditor,
		bStartParticleEditor,
		bStartPhysicsEditor,
		bStartCharacterEditor,

		bQuit
	};

protected:
void Draw();

public:
Menu_AppSelect();
~Menu_AppSelect();

int DoModal();
};

#endif