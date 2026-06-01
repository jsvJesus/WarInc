#ifndef __R3D_DX11_RENDER_TARGET_H
#define __R3D_DX11_RENDER_TARGET_H

#ifndef WO_SERVER

#include <map>

struct IDirect3DSurface9;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;

enum
{
	R3D_DX11_MAX_RENDER_TARGETS = 4
};

class r3dD3DSurfaceTunnel;

class r3dDX11RenderTargetBridge
{
public:
	r3dDX11RenderTargetBridge();
	~r3dDX11RenderTargetBridge();

	bool Init();
	void Shutdown();

	bool IsInitialized() const;

	void ClearCurrentTargets();
	void ResetToBackBuffer();

	void RegisterRenderTarget(
		IDirect3DSurface9* surface,
		ID3D11RenderTargetView* rtv,
		unsigned int width,
		unsigned int height
	);

	void RegisterDepthStencil(
		IDirect3DSurface9* surface,
		ID3D11DepthStencilView* dsv,
		unsigned int width,
		unsigned int height
	);

	void UnregisterSurface(IDirect3DSurface9* surface);

	bool SetRenderTarget(int slot, IDirect3DSurface9* surface);
	bool SetDepthStencil(IDirect3DSurface9* surface);
	bool Clear(
		unsigned int rectCount,
		const D3DRECT* rects,
		unsigned int flags,
		unsigned int color,
		float z,
		unsigned int stencil
	);

	bool CreateRenderTargetMirror(
		r3dD3DSurfaceTunnel* surface,
		unsigned int width,
		unsigned int height,
		int d3dFormat,
		int multiSample,
		unsigned int multisampleQuality
	);

	bool CreateDepthStencilMirror(
		r3dD3DSurfaceTunnel* surface,
		unsigned int width,
		unsigned int height,
		int d3dFormat,
		int multiSample,
		unsigned int multisampleQuality
	);

private:
	struct RegisteredSurface
	{
		RegisteredSurface();

		ID3D11RenderTargetView* RTV;
		ID3D11DepthStencilView* DSV;
		unsigned int Width;
		unsigned int Height;
		bool HasRenderTarget;
		bool HasDepthStencil;
	};

	typedef std::map<IDirect3DSurface9*, RegisteredSurface> SurfaceMap;

	bool FindRenderTarget(
		IDirect3DSurface9* surface,
		ID3D11RenderTargetView** rtv,
		unsigned int* width,
		unsigned int* height
	) const;

	bool FindDepthStencil(
		IDirect3DSurface9* surface,
		ID3D11DepthStencilView** dsv,
		unsigned int* width,
		unsigned int* height
	) const;

	void Apply();

	bool Initialized;
	SurfaceMap Surfaces;

	ID3D11RenderTargetView* CurrentRTV[R3D_DX11_MAX_RENDER_TARGETS];
	ID3D11DepthStencilView* CurrentDSV;
	unsigned int CurrentWidth;
	unsigned int CurrentHeight;
};

extern r3dDX11RenderTargetBridge g_r3dDX11RenderTargets;

#endif

#endif
