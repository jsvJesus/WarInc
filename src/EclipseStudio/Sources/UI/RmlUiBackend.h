#pragma once

#ifndef STATIC_LIB
#define STATIC_LIB
#endif

#include <Rocket/Core.h>
#include <Rocket/Core/Context.h>
#include <Rocket/Core/Element.h>
#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Core/EventListener.h>
#include <Rocket/Core/Font.h>
#include <Rocket/Core/FontDatabase.h>
#include <Rocket/Core/Input.h>
#include <Rocket/Core/RenderInterface.h>
#include <Rocket/Core/SystemInterface.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct RmlUiRenderInterfaceDX11;

#ifdef SetViewport
#undef SetViewport
#endif

#ifdef SetTexture
#undef SetTexture
#endif

#ifdef DrawIndexedPrimitiveUP
#undef DrawIndexedPrimitiveUP
#endif

// -----------------------------------------------------------------------
//  RmlUi System Interface  -  wraps r3dGetTime() and r3dOutToLog()
// -----------------------------------------------------------------------
class RmlUiSystemInterface : public Rocket::Core::SystemInterface
{
public:
	virtual float GetElapsedTime();
	virtual bool LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message);
};

// -----------------------------------------------------------------------
//  RmlUi Render Interface  -  native DX11 rendering
// -----------------------------------------------------------------------
class RmlUiRenderInterface : public Rocket::Core::RenderInterface
{
public:
	RmlUiRenderInterface();
	virtual ~RmlUiRenderInterface();

	void SetDX11Device(ID3D11Device* device, ID3D11DeviceContext* context, int width, int height);
	bool UsesDX11() const;
	void PrepareState();
	void RestoreState();

	// Rocket::Core::RenderInterface
	virtual void RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture, const Rocket::Core::Vector2f& translation);
	virtual void EnableScissorRegion(bool enable);
	virtual void SetScissorRegion(int x, int y, int width, int height);
	virtual bool GenerateTexture(Rocket::Core::TextureHandle& texture_handle, const Rocket::Core::byte* source, const Rocket::Core::Vector2i& source_dimensions);
	virtual void ReleaseTexture(Rocket::Core::TextureHandle texture);

private:
	int m_width;
	int m_height;
	RmlUiRenderInterfaceDX11* m_dx11;
};

// Forward declaration - struct defined in m_AppSelect.h
struct AppSelectCommandBinding;

// Escape key result constant matching Menu_AppSelect::bQuit
// (defined as 1009 in m_AppSelect.h START_MENU_ENUMS)
const int RMLUI_BACKEND_QUIT_RESULT = 1009;

// -----------------------------------------------------------------------
//  RmlUi Backend  -  singleton-style lifecycle manager
// -----------------------------------------------------------------------
class RmlUiBackend
{
public:
	// Initialise Rocket::Core, create system/render interfaces.
	// Must be called once before any context/document work.
	static bool Initialize();

	// Shut down Rocket::Core, release all resources.
	static void Shutdown();

	// True after successful Initialize()
	static bool IsInitialized();

	// Get the singleton context (created on first call to Initialize).
	static Rocket::Core::Context* GetContext();

	// Convenience: load a document from string into the singleton context.
	static Rocket::Core::ElementDocument* LoadDocumentFromMemory(const Rocket::Core::String& rml);

	// Convenience: load a document from file into the singleton context.
	static Rocket::Core::ElementDocument* LoadDocumentFromFile(const char* filePath);

	// ----------------------------------------------------------------
	//  Frame lifecycle  -  call once per frame from your menu loop
	// ----------------------------------------------------------------
	static void BeginFrame();
	static void EndFrame();

	// ----------------------------------------------------------------
	//  Input helpers
	// ----------------------------------------------------------------
	static void ProcessMouse();
	static int  ProcessHotkeys(const struct AppSelectCommandBinding* commands, int numCommands);
	static int  ProcessEscapeKey();

	// ----------------------------------------------------------------
	//  Utility
	// ----------------------------------------------------------------
	static int  GetKeyModifierState();
	static float GetScreenWidth();
	static float GetScreenHeight();

private:
	static bool s_initialized;
	static RmlUiSystemInterface* s_system;
	static RmlUiRenderInterface* s_render;
	static Rocket::Core::Context* s_context;
	static ID3D11Device* s_dx11Device;
	static bool s_usingDX11;
};
