#include "r3dPCH.h"

#include "r3d.h"
#include "GameCommon.h"

#include "UI/UIimEdit2.h"
#include "UI/RmlUiBackend.h"

#include "m_Main.h"
#include "GameLevel.h"
#include "TrueNature2/Terrain2.h"

#include <windows.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>

Menu_Main::Menu_Main()
{
}

Menu_Main::~Menu_Main()
{
}



void Menu_Main::Draw()
{
  
  return;
}


				  static int __CreateTerrain = 0;
				  static int __CreateTerrain2 = 0;
				  static int __CreateMesh = 0;
				  static int __TerrainSize = 0;
				  static float __TerrainSizeCell = 1.0f;
				  static int __TerrainSMapSize = 0;
				  static float __TerrainSizeHeight = 100.0f;
				  static float __TerrainStepY = 1.0f;
				  static char HMapName[64];
				  static char HMeshName[64];

char LevelEditName[256];

void SaveLevelData( char* Str );

static void* ____DummyObjectConstructor()
{
	return NULL;
}

bool gNewLevelCreated = false ;

bool CreateNewLevel()
{
#ifndef FINAL_BUILD
	char Path[256];
	char Str[256];

	gNewLevelCreated = true ;

	sprintf(Path, "Levels\\%s", LevelEditName );
	if ( mkdir(Path) == -1 )
	{
		if ( errno == EEXIST )
		{
			r3dOutToLog( "Map with name \"%s\" already exist.\n", LevelEditName );
			return false;
		}
	}

	if (__CreateTerrain)
	{
		r3d_assert(g_pPhysicsWorld == 0);
		g_pPhysicsWorld = new PhysXWorld();
		g_pPhysicsWorld->Init();

		r3dGameLevel::SetHomeDir( LevelEditName);

		if( __CreateTerrain2 )
		{
			r3dTerrain2::CreationParams params ;

			params.CellCountX = int( pow( 2.0f, 8.0f + __TerrainSize ) ) ;
			params.CellCountZ = params.CellCountX ;
			params.SplatSizeX = int( pow( 2.0f, 8.0f + __TerrainSMapSize ) ) ;
			params.SplatSizeZ = params.SplatSizeX ;

			params.CellSize = __TerrainSizeCell ;
			params.LevelDir = r3dString( "Levels\\" ) + LevelEditName + "\\" ;;
			
			r3dTerrain2 createTerra ;

			createTerra.SaveEmpty( params ) ;
		}
		else
		{
			sprintf(Str, "Levels\\%s\\Terrain", LevelEditName );
			mkdir(Str);

			int TerraWidth = int( pow(2.0f, 8.0f + __TerrainSize) );
			int SplatWidth = int(pow(2.0f, 8+__TerrainSMapSize));

			sprintf(Str, "Levels\\%s\\Terrain\\Mat-Splat.dds", LevelEditName );
			r3dTexture *TempTex = r3dRenderer->AllocateTexture();
			TempTex->Create(SplatWidth, SplatWidth, D3DFMT_A8R8G8B8, 1);
			TempTex->Save(Str);
			r3dRenderer->DeleteTexture(TempTex);

			g_pUndoHistory = new UndoHistory;
			
			extern r3dITerrain* Terrain ;
			extern r3dTerrain* Terrain1 ;
			Terrain1 = new CQuadTerrain;
			Terrain = Terrain1 ;
			
			Terrain1->Height = Terrain1->Width = float( TerraWidth );
			Terrain1->SetCellSize ( __TerrainSizeCell );
			Terrain1->__HeightmapBlend = 0.25f;
			Terrain1->__TerraLOD1 = 1;
			Terrain1->__TerraLOD2 = 2;
			Terrain1->__TerraLOD3 = 24;
			Terrain1->m_HeightmapSize = __TerrainSizeHeight;
			
			Terrain1->MatSplatTex[ 0 ] = r3dRenderer->LoadTexture( Str );
			Terrain1->NumMats = 1;
			r3dTerrain::Layer_t layer;
			for (int i = 0; i < 4 ; i++)
			{
				layer.pMapDiffuse = r3dRenderer->LoadTexture( "Data\\Shaders\\Texture\\MissingTexture.dds" );
				layer.pMapNormal = r3dRenderer->LoadTexture( "Data\\TerrainData\\Materials\\bump_sim.dds" );
				Terrain1->m_dMatLayers[ 0 ][ i ] = layer;
			}
			
			//Terrain->InitData();

			r3d_assert(_CrtCheckMemory());

			Terrain1->CreateDefaultPhysicsData();

			r3d_assert(_CrtCheckMemory());
			Terrain1->HeightFieldData.Resize( uint32_t(Terrain1->Width*Terrain1->Height) );

			r3d_assert(_CrtCheckMemory());
			for(int x=0; x<Terrain1->Width; ++x)
				for(int y=0; y<Terrain1->Height; ++y)
					Terrain1->HeightFieldData[ x*(int)Terrain1->Width + y ] = __TerrainSizeHeight;

			r3d_assert(_CrtCheckMemory());
			Terrain1->UpdatePhysHeightField();
			r3d_assert(_CrtCheckMemory());

			Terrain1->m_pColorData = new uint32_t[ Terrain1->HeightFieldData.Count() ];
			r3d_assert(_CrtCheckMemory());

			memset( Terrain1->m_pColorData, 0xff, sizeof( uint32_t) * uint32_t(Terrain1->Width) * uint32_t(Terrain1->Height) );

			r3d_assert(_CrtCheckMemory());

			Terrain1->m_tBaseLayer.pMapDiffuse = r3dRenderer->LoadTexture( "Data\\Shaders\\Texture\\MissingTexture.dds" );
			Terrain1->m_tBaseLayer.pMapNormal = r3dRenderer->LoadTexture( "Data\\TerrainData\\Materials\\bump_sim.dds" );
			Terrain1->m_tBaseLayer.fScale = 50.0f;
			r3d_assert(_CrtCheckMemory());

			Terrain1->RecalcParams();
			r3d_assert(_CrtCheckMemory());

			Terrain1->SaveData( Path, false );
			Terrain1->Unload();

			Terrain1->UpdateDesc() ;			
		}

		GameWorld().Init(1);
		obj_Terrain* pObjTerrain = new obj_Terrain();
		obj_Terrain& objTerrain = *pObjTerrain;
		objTerrain.DrawOrder      = OBJ_DRAWORDER_FIRST;
		objTerrain.ObjFlags      |= OBJFLAG_SkipCastRay;
		objTerrain.FileName = "terra1";
		AClass classData( NULL, "obj_Terrain", "Type", ____DummyObjectConstructor );
		classData.Name = "obj_Terrain";
		objTerrain.Class = &classData;
		GameWorld().AddObject( pObjTerrain );
		SaveLevelData( Str );
		GameWorld().Destroy();

		g_pPhysicsWorld->Destroy();
		SAFE_DELETE(g_pPhysicsWorld);

		delete g_pUndoHistory;
		g_pUndoHistory = NULL;
	}

#endif
	return true;
}


void ClearFullScreen_Menu();
void RestoreDX11MenuBackBuffer(const char* where);
extern bool g_bExit;

namespace
{
	const int MAIN_WAITING_FOR_COMMAND = -1;
	const int MAIN_MAX_LEVEL_ROWS = 512;

	enum MainMenuAction
	{
		MMA_None = 0,
		MMA_TabLive,
		MMA_TabEditor,
		MMA_TabCreate,
		MMA_SelectLive,
		MMA_SelectEditor,
		MMA_LoadLevel,
		MMA_CreateLevel,
		MMA_Quit,
		MMA_NameFocus,
		MMA_Refresh,
		MMA_ToggleTerrain,
		MMA_ToggleTerrain2,
		MMA_TerrainSizeMinus,
		MMA_TerrainSizePlus,
		MMA_SplatSizeMinus,
		MMA_SplatSizePlus,
		MMA_CellSizeMinus,
		MMA_CellSizePlus,
		MMA_HeightMinus,
		MMA_HeightPlus
	};

	struct MainStaticBinding
	{
		const char* ElementId;
		int Action;
		int Index;
	};

	static const MainStaticBinding g_MainStaticBindings[] =
	{
		{ "tab_live", MMA_TabLive, 0 },
		{ "tab_editor", MMA_TabEditor, 0 },
		{ "tab_create", MMA_TabCreate, 0 },
		{ "load_level", MMA_LoadLevel, 0 },
		{ "create_level", MMA_CreateLevel, 0 },
		{ "quit", MMA_Quit, 0 },
		{ "name_focus", MMA_NameFocus, 0 },
		{ "refresh", MMA_Refresh, 0 },
		{ "toggle_terrain", MMA_ToggleTerrain, 0 },
		{ "toggle_terrain2", MMA_ToggleTerrain2, 0 },
		{ "terrain_size_minus", MMA_TerrainSizeMinus, 0 },
		{ "terrain_size_plus", MMA_TerrainSizePlus, 0 },
		{ "splat_size_minus", MMA_SplatSizeMinus, 0 },
		{ "splat_size_plus", MMA_SplatSizePlus, 0 },
		{ "cell_size_minus", MMA_CellSizeMinus, 0 },
		{ "cell_size_plus", MMA_CellSizePlus, 0 },
		{ "height_minus", MMA_HeightMinus, 0 },
		{ "height_plus", MMA_HeightPlus, 0 }
	};

	static int g_MainPendingAction = MMA_None;
	static int g_MainPendingIndex = -1;

	static int g_MainTab = 0;
	static int g_SelectedLiveLevel = -1;
	static int g_SelectedEditorLevel = -1;
	static bool g_NameInputActive = false;

	static std::vector<std::string> g_LiveLevels;
	static std::vector<std::string> g_EditorLevels;
	static std::string g_MainStatus = "Select map or create new level.";

	const char* g_MainTemplatePath = "Data\\UI\\m_Main.rml";
	const char* g_EditorBackgroundPath = "Data\\UI\\Assets\\Background.png";

	class MainMenuEventListener : public Rocket::Core::EventListener
	{
	public:
		MainMenuEventListener()
		: m_action(MMA_None)
		, m_index(-1)
		{
		}

		void Setup(int action, int index)
		{
			m_action = action;
			m_index = index;
		}

		virtual void ProcessEvent(Rocket::Core::Event& event)
		{
			g_MainPendingAction = m_action;
			g_MainPendingIndex = m_index;
		}

	private:
		int m_action;
		int m_index;
	};

	static bool MainIsKeyPressedOnce(int vk)
	{
		static unsigned char oldState[256] = {0};

		SHORT state = GetAsyncKeyState(vk);
		bool isDown = (state & 0x8000) != 0;
		bool wasDown = oldState[vk] != 0;

		oldState[vk] = isDown ? 1 : 0;

		return isDown && !wasDown;
	}

	static bool MainIsShiftDown()
	{
		return (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
	}

	static bool MainIsValidLevelChar(char c)
	{
		if(c >= 'a' && c <= 'z') return true;
		if(c >= 'A' && c <= 'Z') return true;
		if(c >= '0' && c <= '9') return true;
		if(c == '_' || c == '-' || c == '.') return true;
		return false;
	}

	static void MainAppendLevelChar(char c)
	{
		if(!MainIsValidLevelChar(c))
			return;

		int len = (int)strlen(LevelEditName);
		if(len >= 240)
			return;

		LevelEditName[len] = c;
		LevelEditName[len + 1] = 0;
	}

	static void MainProcessNameInput(bool& reloadDocument)
	{
		if(!g_NameInputActive)
			return;

		if(MainIsKeyPressedOnce(VK_RETURN))
		{
			g_NameInputActive = false;
			reloadDocument = true;
			return;
		}

		if(MainIsKeyPressedOnce(VK_ESCAPE))
		{
			g_NameInputActive = false;
			reloadDocument = true;
			return;
		}

		if(MainIsKeyPressedOnce(VK_BACK))
		{
			int len = (int)strlen(LevelEditName);
			if(len > 0)
			{
				LevelEditName[len - 1] = 0;
				reloadDocument = true;
			}
		}

		for(int vk = 'A'; vk <= 'Z'; ++vk)
		{
			if(MainIsKeyPressedOnce(vk))
			{
				char c = MainIsShiftDown() ? (char)vk : (char)tolower(vk);
				MainAppendLevelChar(c);
				reloadDocument = true;
			}
		}

		for(int vk = '0'; vk <= '9'; ++vk)
		{
			if(MainIsKeyPressedOnce(vk))
			{
				MainAppendLevelChar((char)vk);
				reloadDocument = true;
			}
		}

		if(MainIsKeyPressedOnce(VK_SPACE))
		{
			MainAppendLevelChar('_');
			reloadDocument = true;
		}

		if(MainIsKeyPressedOnce(VK_OEM_MINUS))
		{
			MainAppendLevelChar(MainIsShiftDown() ? '_' : '-');
			reloadDocument = true;
		}

		if(MainIsKeyPressedOnce(VK_OEM_PERIOD))
		{
			MainAppendLevelChar('.');
			reloadDocument = true;
		}
	}

	static std::string MainEscapeRml(const std::string& text)
	{
		std::string out;
		out.reserve(text.size() + 16);

		for(size_t i = 0; i < text.size(); ++i)
		{
			switch(text[i])
			{
			case '&': out += "&amp;"; break;
			case '<': out += "&lt;"; break;
			case '>': out += "&gt;"; break;
			case '"': out += "&quot;"; break;
			case '\'': out += "&apos;"; break;
			default: out += text[i]; break;
			}
		}

		return out;
	}

	static bool MainIsDotsDir(const char* name)
	{
		return strcmp(name, ".") == 0 || strcmp(name, "..") == 0;
	}

	static void MainCollectLevelDirs(const char* root, std::vector<std::string>& outLevels)
	{
		outLevels.clear();

		std::string mask = root;
		mask += "\\*";

		WIN32_FIND_DATAA fd;
		HANDLE hFind = FindFirstFileA(mask.c_str(), &fd);

		if(hFind == INVALID_HANDLE_VALUE)
			return;

		do
		{
			if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
				continue;

			if(MainIsDotsDir(fd.cFileName))
				continue;

			outLevels.push_back(fd.cFileName);
		}
		while(FindNextFileA(hFind, &fd));

		FindClose(hFind);

		std::sort(outLevels.begin(), outLevels.end(), [](const std::string& a, const std::string& b)
		{
			return _stricmp(a.c_str(), b.c_str()) < 0;
		});
	}

	static void MainRefreshLevelLists()
	{
		MainCollectLevelDirs("Levels", g_LiveLevels);
		MainCollectLevelDirs("Levels\\WorkInProgress", g_EditorLevels);

		for(int i = (int)g_LiveLevels.size() - 1; i >= 0; --i)
		{
			if(_stricmp(g_LiveLevels[i].c_str(), "WorkInProgress") == 0)
				g_LiveLevels.erase(g_LiveLevels.begin() + i);
		}

		if(g_SelectedLiveLevel >= (int)g_LiveLevels.size())
			g_SelectedLiveLevel = (int)g_LiveLevels.size() - 1;

		if(g_SelectedEditorLevel >= (int)g_EditorLevels.size())
			g_SelectedEditorLevel = (int)g_EditorLevels.size() - 1;

		if(g_SelectedLiveLevel < 0 && !g_LiveLevels.empty())
			g_SelectedLiveLevel = 0;

		if(g_SelectedEditorLevel < 0 && !g_EditorLevels.empty())
			g_SelectedEditorLevel = 0;
	}

	static void MainSetCurrentLevelFromSelection()
	{
		if(g_MainTab == 0)
		{
			if(g_SelectedLiveLevel >= 0 && g_SelectedLiveLevel < (int)g_LiveLevels.size())
				r3dscpy(LevelEditName, g_LiveLevels[g_SelectedLiveLevel].c_str());
		}
		else if(g_MainTab == 1)
		{
			if(g_SelectedEditorLevel >= 0 && g_SelectedEditorLevel < (int)g_EditorLevels.size())
				sprintf(LevelEditName, "WorkInProgress\\%s", g_EditorLevels[g_SelectedEditorLevel].c_str());
		}
	}

	static std::string MainMakeMapRows(const std::vector<std::string>& levels, const char* idPrefix, int selectedIndex)
	{
		if(levels.empty())
			return "<div class='empty'>No maps found</div>";

		std::string out;

		int count = (int)levels.size();
		if(count > MAIN_MAX_LEVEL_ROWS)
			count = MAIN_MAX_LEVEL_ROWS;

		for(int i = 0; i < count; ++i)
		{
			char id[64];
			sprintf(id, "%s%d", idPrefix, i);

			out += "<button id='";
			out += id;
			out += "' class='map-row";

			if(i == selectedIndex)
				out += " selected";

			out += "' onclick='dummy'>";
			out += MainEscapeRml(levels[i]);
			out += "</button>";
		}

		return out;
	}

	static void MainAppendValueButton(std::string& out, const char* minusId, const char* plusId, const char* label, const char* value)
	{
		out += "<div class='setting-row'>";
		out += "<div class='setting-label'>";
		out += label;
		out += "</div>";
		out += "<button id='";
		out += minusId;
		out += "' class='small-button' onclick='dummy'>-</button>";
		out += "<div class='setting-value'>";
		out += value;
		out += "</div>";
		out += "<button id='";
		out += plusId;
		out += "' class='small-button' onclick='dummy'>+</button>";
		out += "</div>";
	}

	static std::string MainBuildCreatePanel()
	{
		char value[128];
		std::string out;

		out += "<div class='create-block'>";
		out += "<div class='field-title'>MAP NAME</div>";
		out += "<button id='name_focus' class='text-input";
		if(g_NameInputActive)
			out += " active";
		out += "' onclick='dummy'>";
		out += MainEscapeRml(LevelEditName[0] ? LevelEditName : "NewLevel");
		if(g_NameInputActive)
			out += "_";
		out += "</button>";

		out += "<button id='toggle_terrain' class='option-button' onclick='dummy'>HAVE TERRAIN: ";
		out += __CreateTerrain ? "ON" : "OFF";
		out += "</button>";

		if(__CreateTerrain)
		{
			out += "<button id='toggle_terrain2' class='option-button' onclick='dummy'>CREATE TERRAIN V2: ";
			out += __CreateTerrain2 ? "ON" : "OFF";
			out += "</button>";

			const char* terrainSizes[] = { "256", "512", "1024", "2048", "4096" };

			int terrainIndex = R3D_MAX(0, R3D_MIN(__TerrainSize, 4));
			sprintf(value, "%s", terrainSizes[terrainIndex]);
			MainAppendValueButton(out, "terrain_size_minus", "terrain_size_plus", "Terrain Size", value);

			__TerrainSMapSize = R3D_MIN(__TerrainSMapSize, terrainIndex);
			int splatIndex = R3D_MAX(0, R3D_MIN(__TerrainSMapSize, terrainIndex));
			sprintf(value, "%s", terrainSizes[splatIndex]);
			MainAppendValueButton(out, "splat_size_minus", "splat_size_plus", "Splat Map Size", value);

			sprintf(value, "%0.2f", __TerrainSizeCell);
			MainAppendValueButton(out, "cell_size_minus", "cell_size_plus", "Cell Size", value);

			sprintf(value, "%0.2f", __TerrainSizeHeight);
			MainAppendValueButton(out, "height_minus", "height_plus", "Terrain Height", value);
		}

		out += "<button id='create_level' class='primary-button' onclick='dummy'>Create Level</button>";
		out += "</div>";

		return out;
	}

	static bool MainLoadTextFile(const char* filePath, std::string& out)
	{
		out.clear();

		r3dFile* file = r3d_open(filePath, "rb");
		if(!file)
			return false;

		if(file->size > 0)
		{
			out.resize(file->size);
			fread(&out[0], 1, file->size, file);
		}

		fclose(file);
		return true;
	}

	static void MainReplaceAll(std::string& text, const char* token, const std::string& value)
	{
		size_t pos = 0;
		const size_t tokenLen = strlen(token);

		while((pos = text.find(token, pos)) != std::string::npos)
		{
			text.replace(pos, tokenLen, value);
			pos += value.size();
		}
	}

	static std::string MainBuildContentPanel()
	{
		std::string out;

		if(g_MainTab == 0)
		{
			out += "<div class='map-list'>";
			out += MainMakeMapRows(g_LiveLevels, "live_", g_SelectedLiveLevel);
			out += "</div>";

			out += "<div id='side'>";
			out += "<button id='load_level' class='primary-button' onclick='dummy'>Load Level</button>";
			out += "<button id='refresh' class='side-button' onclick='dummy'>Refresh</button>";
			out += "<button id='quit' class='side-button' onclick='dummy'>Exit</button>";
			out += "</div>";
		}
		else if(g_MainTab == 1)
		{
			out += "<div class='map-list'>";
			out += MainMakeMapRows(g_EditorLevels, "editor_", g_SelectedEditorLevel);
			out += "</div>";

			out += "<div id='side'>";
			out += "<button id='load_level' class='primary-button' onclick='dummy'>Load Level</button>";
			out += "<button id='refresh' class='side-button' onclick='dummy'>Refresh</button>";
			out += "<button id='quit' class='side-button' onclick='dummy'>Exit</button>";
			out += "</div>";
		}
		else
		{
			out += MainBuildCreatePanel();
		}

		return out;
	}

	static std::string MainBuildRml()
	{
		std::string out;

		if(!MainLoadTextFile(g_MainTemplatePath, out))
		{
			r3dOutToLog("RmlUi Main: failed to load template '%s'\n", g_MainTemplatePath);
			return std::string();
		}

		MainReplaceAll(out, "{{tab_live_class}}", g_MainTab == 0 ? "tab-button active" : "tab-button");
		MainReplaceAll(out, "{{tab_editor_class}}", g_MainTab == 1 ? "tab-button active" : "tab-button");
		MainReplaceAll(out, "{{tab_create_class}}", g_MainTab == 2 ? "tab-button active" : "tab-button");
		MainReplaceAll(out, "{{content}}", MainBuildContentPanel());
		MainReplaceAll(out, "{{status}}", MainEscapeRml(g_MainStatus));

		return out;
	}

	static void MainAttachEvent(Rocket::Core::ElementDocument* document, const char* id, MainMenuEventListener* listener, int action, int index)
	{
		if(!document || !id || !listener)
			return;

		listener->Setup(action, index);

		Rocket::Core::Element* element = document->GetElementById(id);
		if(element)
			element->AddEventListener("click", listener);
	}

	static void MainDetachEvent(Rocket::Core::ElementDocument* document, const char* id, MainMenuEventListener* listener)
	{
		if(!document || !id || !listener)
			return;

		Rocket::Core::Element* element = document->GetElementById(id);
		if(element)
			element->RemoveEventListener("click", listener);
	}

	static void MainDetachDocumentEvents(
		Rocket::Core::ElementDocument* document,
		MainMenuEventListener* staticListeners,
		MainMenuEventListener* liveListeners,
		MainMenuEventListener* editorListeners)
	{
		if(!document)
			return;

		for(int i = 0; i < R3D_ARRAYSIZE(g_MainStaticBindings); ++i)
			MainDetachEvent(document, g_MainStaticBindings[i].ElementId, &staticListeners[i]);

		for(int i = 0; i < MAIN_MAX_LEVEL_ROWS; ++i)
		{
			char id[64];
			sprintf(id, "live_%d", i);
			MainDetachEvent(document, id, &liveListeners[i]);

			sprintf(id, "editor_%d", i);
			MainDetachEvent(document, id, &editorListeners[i]);
		}
	}

	static Rocket::Core::ElementDocument* MainLoadDocument(
		MainMenuEventListener* staticListeners,
		MainMenuEventListener* liveListeners,
		MainMenuEventListener* editorListeners)
	{
		std::string rml = MainBuildRml();
		if(rml.empty())
			return NULL;

		Rocket::Core::ElementDocument* document = RmlUiBackend::LoadDocumentFromMemory(rml.c_str());
		if(!document)
		{
			r3dOutToLog("RmlUi Main: LoadDocumentFromMemory failed\n");
			return NULL;
		}

		for(int i = 0; i < R3D_ARRAYSIZE(g_MainStaticBindings); ++i)
		{
			MainAttachEvent(
				document,
				g_MainStaticBindings[i].ElementId,
				&staticListeners[i],
				g_MainStaticBindings[i].Action,
				g_MainStaticBindings[i].Index);
		}

		int liveCount = (int)g_LiveLevels.size();
		if(liveCount > MAIN_MAX_LEVEL_ROWS)
			liveCount = MAIN_MAX_LEVEL_ROWS;

		for(int i = 0; i < liveCount; ++i)
		{
			char id[64];
			sprintf(id, "live_%d", i);
			MainAttachEvent(document, id, &liveListeners[i], MMA_SelectLive, i);
		}

		int editorCount = (int)g_EditorLevels.size();
		if(editorCount > MAIN_MAX_LEVEL_ROWS)
			editorCount = MAIN_MAX_LEVEL_ROWS;

		for(int i = 0; i < editorCount; ++i)
		{
			char id[64];
			sprintf(id, "editor_%d", i);
			MainAttachEvent(document, id, &editorListeners[i], MMA_SelectEditor, i);
		}

		document->Show();

		return document;
	}

	static r3dTexture* MainLoadBackground()
	{
		const char* bgPath = g_EditorBackgroundPath;

		if(!r3dFileExists(bgPath))
		{
			r3dOutToLog("RmlUi Main: background file not found: %s\n", bgPath);
			return NULL;
		}

		r3dTexture* tex = r3dRenderer->LoadTexture(bgPath);

		if(!tex)
		{
			r3dOutToLog("RmlUi Main: LoadTexture returned NULL for %s\n", bgPath);
			return NULL;
		}

		if(!tex->IsValid())
		{
			r3dOutToLog("RmlUi Main: background texture is invalid: %s\n", bgPath);
			return NULL;
		}

#ifndef WO_SERVER
		if(g_r3dDX11.IsInitialized() && !tex->HasDX11Texture())
		{
			r3dOutToLog("RmlUi Main: background has no DX11 SRV: %s\n", bgPath);
		}
#endif

		r3dOutToLog("RmlUi Main: background loaded: %s\n", bgPath);
		return tex;
	}

	static void MainDrawBackground(r3dTexture* backgroundTexture)
	{
		r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);

		ClearFullScreen_Menu();

		float x = 0.0f;
		float y = 0.0f;
		float w = 0.0f;
		float h = 0.0f;

		r3dRenderer->GetBackBufferViewport(&x, &y, &w, &h);

		if(backgroundTexture)
			r3dDrawBox2D(x, y, w, h, r3dColor24::white, backgroundTexture);
		else
			r3dDrawBox2D(x, y, w, h, r3dColor(12, 14, 18));
	}

	static void MainCloseDocument(
		Rocket::Core::ElementDocument*& document,
		MainMenuEventListener* staticListeners,
		MainMenuEventListener* liveListeners,
		MainMenuEventListener* editorListeners)
	{
		if(document)
		{
			MainDetachDocumentEvents(document, staticListeners, liveListeners, editorListeners);

			document->Close();
			document->RemoveReference();
			document = NULL;

			Rocket::Core::Context* ctx = RmlUiBackend::GetContext();
			if(ctx)
				ctx->Update();
		}
	}

	static void MainClampCreateValues()
	{
		__TerrainSize = R3D_MAX(0, R3D_MIN(__TerrainSize, 4));
		__TerrainSMapSize = R3D_MAX(0, R3D_MIN(__TerrainSMapSize, __TerrainSize));

		if(__TerrainSizeCell < 1.0f)
			__TerrainSizeCell = 1.0f;

		if(__TerrainSizeCell > 100.0f)
			__TerrainSizeCell = 100.0f;

		if(__TerrainSizeHeight < 0.0f)
			__TerrainSizeHeight = 0.0f;

		if(__TerrainSizeHeight > 1200.0f)
			__TerrainSizeHeight = 1200.0f;
	}

	static void MainHandleAction(int action, int index, int& modalResult, bool& reloadDocument)
	{
		switch(action)
		{
		case MMA_TabLive:
			g_MainTab = 0;
			g_NameInputActive = false;
			MainSetCurrentLevelFromSelection();
			g_MainStatus = "Live maps.";
			reloadDocument = true;
			break;

		case MMA_TabEditor:
			g_MainTab = 1;
			g_NameInputActive = false;
			MainSetCurrentLevelFromSelection();
			g_MainStatus = "Editor WorkInProgress maps.";
			reloadDocument = true;
			break;

		case MMA_TabCreate:
			if(g_MainTab != 2)
			{
				r3dscpy(LevelEditName, "NewLevel");
				g_NameInputActive = false;
			}
			g_MainTab = 2;
			g_MainStatus = "Create new map.";
			reloadDocument = true;
			break;

		case MMA_SelectLive:
			if(index >= 0 && index < (int)g_LiveLevels.size())
			{
				g_SelectedLiveLevel = index;
				g_MainTab = 0;
				r3dscpy(LevelEditName, g_LiveLevels[index].c_str());
				g_MainStatus = std::string("Selected: ") + g_LiveLevels[index];
				reloadDocument = true;
			}
			break;

		case MMA_SelectEditor:
			if(index >= 0 && index < (int)g_EditorLevels.size())
			{
				g_SelectedEditorLevel = index;
				g_MainTab = 1;
				sprintf(LevelEditName, "WorkInProgress\\%s", g_EditorLevels[index].c_str());
				g_MainStatus = std::string("Selected: WorkInProgress\\") + g_EditorLevels[index];
				reloadDocument = true;
			}
			break;

		case MMA_LoadLevel:
			MainSetCurrentLevelFromSelection();

			if(LevelEditName[0])
				modalResult = Menu_Main::bEditor;
			else
				g_MainStatus = "No level selected.";

			reloadDocument = true;
			break;

		case MMA_CreateLevel:
			if(!LevelEditName[0])
				r3dscpy(LevelEditName, "NewLevel");

			MainClampCreateValues();

			if(CreateNewLevel())
				modalResult = Menu_Main::bEditor;
			else
				g_MainStatus = "Create level failed. Check log.";

			reloadDocument = true;
			break;

		case MMA_Quit:
			modalResult = Menu_Main::bQuit;
			break;

		case MMA_NameFocus:
			g_MainTab = 2;
			g_NameInputActive = true;
			g_MainStatus = "Typing map name. ENTER - finish, ESC - cancel typing.";
			reloadDocument = true;
			break;

		case MMA_Refresh:
			MainRefreshLevelLists();
			MainSetCurrentLevelFromSelection();
			g_MainStatus = "Map lists refreshed.";
			reloadDocument = true;
			break;

		case MMA_ToggleTerrain:
			__CreateTerrain = __CreateTerrain ? 0 : 1;
			MainClampCreateValues();
			reloadDocument = true;
			break;

		case MMA_ToggleTerrain2:
			__CreateTerrain2 = __CreateTerrain2 ? 0 : 1;
			MainClampCreateValues();
			reloadDocument = true;
			break;

		case MMA_TerrainSizeMinus:
			__TerrainSize--;
			MainClampCreateValues();
			reloadDocument = true;
			break;

		case MMA_TerrainSizePlus:
			__TerrainSize++;
			MainClampCreateValues();
			reloadDocument = true;
			break;

		case MMA_SplatSizeMinus:
			__TerrainSMapSize--;
			MainClampCreateValues();
			reloadDocument = true;
			break;

		case MMA_SplatSizePlus:
			__TerrainSMapSize++;
			MainClampCreateValues();
			reloadDocument = true;
			break;

		case MMA_CellSizeMinus:
			__TerrainSizeCell -= 1.0f;
			MainClampCreateValues();
			reloadDocument = true;
			break;

		case MMA_CellSizePlus:
			__TerrainSizeCell += 1.0f;
			MainClampCreateValues();
			reloadDocument = true;
			break;

		case MMA_HeightMinus:
			__TerrainSizeHeight -= 10.0f;
			MainClampCreateValues();
			reloadDocument = true;
			break;

		case MMA_HeightPlus:
			__TerrainSizeHeight += 10.0f;
			MainClampCreateValues();
			reloadDocument = true;
			break;
		}
	}
}

int Menu_Main::DoModal()
{
	LevelEditName[0] = 0;

	g_MainTab = 0;
	g_SelectedLiveLevel = -1;
	g_SelectedEditorLevel = -1;
	g_NameInputActive = false;
	g_MainPendingAction = MMA_None;
	g_MainPendingIndex = -1;
	g_MainStatus = "Select map or create new level.";

	MainClampCreateValues();
	MainRefreshLevelLists();
	MainSetCurrentLevelFromSelection();

	released_id = MAIN_WAITING_FOR_COMMAND;

	Desktop().SetViewSize(r3dRenderer->ScreenW, r3dRenderer->ScreenH);
	r3dMouse::Show(true);

	if(!RmlUiBackend::Initialize())
	{
		r3dOutToLog("RmlUi Main: backend initialization failed\n");
		return 0;
	}

	Rocket::Core::Context* ctx = RmlUiBackend::GetContext();
	if(!ctx)
	{
		r3dOutToLog("RmlUi Main: no context\n");
		return 0;
	}

	MainMenuEventListener staticListeners[R3D_ARRAYSIZE(g_MainStaticBindings)];
	MainMenuEventListener liveListeners[MAIN_MAX_LEVEL_ROWS];
	MainMenuEventListener editorListeners[MAIN_MAX_LEVEL_ROWS];

	Rocket::Core::ElementDocument* document = MainLoadDocument(staticListeners, liveListeners, editorListeners);
	if(!document)
		return 0;

	r3dTexture* backgroundTexture = MainLoadBackground();

	while(released_id == MAIN_WAITING_FOR_COMMAND)
	{
		if(g_bExit)
		{
			released_id = Menu_Main::bQuit;
			break;
		}

		r3dProcessWindowMessages();
		r3dMouse::Show(true);

		RmlUiBackend::ProcessMouse();

		mUpdate();

		ctx->SetDimensions(Rocket::Core::Vector2i((int)r3dRenderer->ScreenW, (int)r3dRenderer->ScreenH));
		ctx->Update();

		bool reloadDocument = false;

		MainProcessNameInput(reloadDocument);

		if(Keyboard->WasPressed(kbsEsc))
		{
			if(g_NameInputActive)
			{
				g_NameInputActive = false;
				reloadDocument = true;
			}
			else
			{
				released_id = Menu_Main::bQuit;
				break;
			}
		}

		if(Keyboard->WasPressed(kbs1))
			MainHandleAction(MMA_TabLive, 0, released_id, reloadDocument);

		if(Keyboard->WasPressed(kbs2))
			MainHandleAction(MMA_TabEditor, 0, released_id, reloadDocument);

		if(Keyboard->WasPressed(kbs3))
			MainHandleAction(MMA_TabCreate, 0, released_id, reloadDocument);

		if(g_MainPendingAction != MMA_None)
		{
			int action = g_MainPendingAction;
			int index = g_MainPendingIndex;

			g_MainPendingAction = MMA_None;
			g_MainPendingIndex = -1;

			MainHandleAction(action, index, released_id, reloadDocument);
		}

		if(reloadDocument && released_id == MAIN_WAITING_FOR_COMMAND)
		{
			MainCloseDocument(document, staticListeners, liveListeners, editorListeners);
			document = MainLoadDocument(staticListeners, liveListeners, editorListeners);

			if(!document)
			{
				released_id = Menu_Main::bQuit;
				break;
			}
		}

		r3dStartFrame();

		if(r3dRenderer->DeviceAvailable)
		{
			r3dRenderer->StartRender(1);
			r3dRenderer->StartFrame();

			MainDrawBackground(backgroundTexture);

			r3dRenderer->Flush();

			RmlUiBackend::BeginFrame();

			if(ctx)
				ctx->Render();

			RmlUiBackend::EndFrame();

			RestoreDX11MenuBackBuffer("Main");
			r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);
			r3dRenderer->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

			r3dRenderer->EndFrame();
		}

		r3dRenderer->EndRender(true);
		r3dEndFrame();
	}

	MainCloseDocument(document, staticListeners, liveListeners, editorListeners);

	return released_id == MAIN_WAITING_FOR_COMMAND ? 0 : released_id;
}
