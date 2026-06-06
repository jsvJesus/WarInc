#include "r3dPCH.h"
#include "r3d.h"

#ifndef WO_SERVER

#include "r3dDX9UIBridge.h"
#include "r3dDX11.h"
#include "r3dDX11State.h"
#include "r3dDX11Geometry.h"

#pragma warning(push)
#pragma warning(disable: 4005)
#pragma warning(disable: 4061)
#pragma warning(disable: 4062)
#pragma warning(disable: 4191)
#pragma warning(disable: 4365)
#pragma warning(disable: 4668)

#include <d3d9.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#ifdef SetRenderTarget
#undef SetRenderTarget
#endif

#ifdef GetRenderTarget
#undef GetRenderTarget
#endif

#ifdef SetDepthStencilSurface
#undef SetDepthStencilSurface
#endif

#ifdef GetDepthStencilSurface
#undef GetDepthStencilSurface
#endif

#ifdef SetViewport
#undef SetViewport
#endif

#ifdef GetViewport
#undef GetViewport
#endif

#pragma warning(pop)

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#ifdef Draw
#undef Draw
#endif

#ifdef DrawIndexed
#undef DrawIndexed
#endif

#ifdef IASetPrimitiveTopology
#undef IASetPrimitiveTopology
#endif

#ifdef IASetVertexBuffers
#undef IASetVertexBuffers
#endif

#ifdef IASetIndexBuffer
#undef IASetIndexBuffer
#endif

namespace
{
	struct DX11UIVertex
	{
		float x;
		float y;
		float u;
		float v;
	};

	template<class T>
	void SafeReleaseLocal(T*& p)
	{
		if(p)
		{
			p->Release();
			p = NULL;
		}
	}

	static IDirect3DDevice9* GetDX9Device()
	{
		if(!r3dRenderer)
			return NULL;

		return r3dRenderer->pd3ddev;
	}

	class r3dDX9UIBridgeImpl
	{
	public:
		r3dDX9UIBridgeImpl()
		{
			Width = 0;
			Height = 0;
			Capturing = false;

			UITexture9 = NULL;
			UISurface9 = NULL;
			SystemSurface9 = NULL;

			OldRT9 = NULL;
			OldDS9 = NULL;
			ZeroMemory(&OldVP9, sizeof(OldVP9));

			UITexture11 = NULL;
			UISRV11 = NULL;

			VS = NULL;
			PS = NULL;
			InputLayout = NULL;
			VB = NULL;
			Sampler = NULL;
			BlendState = NULL;
			DepthState = NULL;
			RasterState = NULL;
		}

		~r3dDX9UIBridgeImpl()
		{
			Close();
		}

		bool Init()
		{
			if(!r3dRenderer || !r3dRenderer->pd3ddev)
				return false;

			if(!g_r3dDX11.IsInitialized() || !g_r3dDX11.GetDevice() || !g_r3dDX11.GetContext())
				return false;

			const int w = R3D_MAX(g_r3dDX11.GetWidth(), 1);
			const int h = R3D_MAX(g_r3dDX11.GetHeight(), 1);

			if(w != Width || h != Height || !UITexture9 || !SystemSurface9 || !UITexture11 || !UISRV11)
			{
				ReleaseD3D9Resources();
				ReleaseD3D11Texture();

				Width = w;
				Height = h;

				if(!CreateD3D9Resources())
					return false;

				if(!CreateD3D11Texture())
					return false;
			}

			if(!CreateD3D11Pipeline())
				return false;

			return true;
		}

		void Close()
		{
			Capturing = false;

			RestoreD3D9Target();

			ReleaseD3D9Resources();
			ReleaseD3D11Texture();
			ReleaseD3D11Pipeline();

			Width = 0;
			Height = 0;
		}

		bool Begin()
		{
			if(Capturing)
				return true;

			if(!r3dRenderer || !r3dRenderer->IsDX9UIEnabled())
				return false;

			if(!Init())
				return false;

			IDirect3DDevice9* dev9 = GetDX9Device();
			if(!dev9 || !UISurface9)
				return false;

			RestoreD3D9Target();

			dev9->GetRenderTarget(0, &OldRT9);
			dev9->GetDepthStencilSurface(&OldDS9);
			dev9->GetViewport(&OldVP9);

			D3DVIEWPORT9 vp;
			ZeroMemory(&vp, sizeof(vp));
			vp.X = 0;
			vp.Y = 0;
			vp.Width = (DWORD)Width;
			vp.Height = (DWORD)Height;
			vp.MinZ = 0.0f;
			vp.MaxZ = 1.0f;

			dev9->SetRenderTarget(0, UISurface9);
			dev9->SetDepthStencilSurface(NULL);
			dev9->SetViewport(&vp);

			dev9->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
			dev9->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dev9->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			dev9->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			dev9->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			dev9->SetRenderState(D3DRS_ZENABLE, FALSE);
			dev9->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

			dev9->Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0);

			Capturing = true;
			return true;
		}

		void End(bool compositeToDX11)
		{
			if(!Capturing)
				return;

			IDirect3DDevice9* dev9 = GetDX9Device();

			if(dev9 && UISurface9 && SystemSurface9)
				dev9->GetRenderTargetData(UISurface9, SystemSurface9);

			RestoreD3D9Target();

			Capturing = false;

			if(compositeToDX11)
			{
				UploadToDX11();
				CompositeToDX11();
			}
		}

		bool CompositeToDX11()
		{
			if(!UITexture11 || !UISRV11)
				return false;

			if(!CreateD3D11Pipeline())
				return false;

			ID3D11DeviceContext* ctx = g_r3dDX11.GetContext();
			if(!ctx)
				return false;

			g_r3dDX11.ResetBackBufferTarget();

			ID3D11RenderTargetView* oldRTV = NULL;
			ID3D11DepthStencilView* oldDSV = NULL;
			ID3D11BlendState* oldBlend = NULL;
			ID3D11DepthStencilState* oldDepth = NULL;
			ID3D11RasterizerState* oldRaster = NULL;
			ID3D11VertexShader* oldVS = NULL;
			ID3D11PixelShader* oldPS = NULL;
			ID3D11GeometryShader* oldGS = NULL;
			ID3D11InputLayout* oldLayout = NULL;
			ID3D11Buffer* oldVB = NULL;
			ID3D11ShaderResourceView* oldSRV = NULL;
			ID3D11SamplerState* oldSampler = NULL;

			FLOAT oldBlendFactor[4] = {0, 0, 0, 0};
			UINT oldSampleMask = 0xffffffff;
			UINT oldStencilRef = 0;
			UINT oldStride = 0;
			UINT oldOffset = 0;
			D3D11_PRIMITIVE_TOPOLOGY oldTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

			UINT oldViewportCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
			D3D11_VIEWPORT oldViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

			ctx->OMGetRenderTargets(1, &oldRTV, &oldDSV);
			ctx->OMGetBlendState(&oldBlend, oldBlendFactor, &oldSampleMask);
			ctx->OMGetDepthStencilState(&oldDepth, &oldStencilRef);
			ctx->RSGetState(&oldRaster);
			ctx->RSGetViewports(&oldViewportCount, oldViewports);

			ctx->VSGetShader(&oldVS, NULL, NULL);
			ctx->PSGetShader(&oldPS, NULL, NULL);
			ctx->GSGetShader(&oldGS, NULL, NULL);
			ctx->IAGetInputLayout(&oldLayout);
			ctx->IAGetVertexBuffers(0, 1, &oldVB, &oldStride, &oldOffset);
			ctx->IAGetPrimitiveTopology(&oldTopology);
			ctx->PSGetShaderResources(0, 1, &oldSRV);
			ctx->PSGetSamplers(0, 1, &oldSampler);

			D3D11_VIEWPORT vp;
			ZeroMemory(&vp, sizeof(vp));
			vp.TopLeftX = 0.0f;
			vp.TopLeftY = 0.0f;
			vp.Width = (float)R3D_MAX(Width, 1);
			vp.Height = (float)R3D_MAX(Height, 1);
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;

			UINT stride = sizeof(DX11UIVertex);
			UINT offset = 0;
			FLOAT blendFactor[4] = {0, 0, 0, 0};

			ID3D11RenderTargetView* backRTV = g_r3dDX11.GetBackBufferRTV();

			ctx->OMSetRenderTargets(1, &backRTV, NULL);
			ctx->RSSetViewports(1, &vp);
			ctx->RSSetState(RasterState);
			ctx->OMSetBlendState(BlendState, blendFactor, 0xffffffff);
			ctx->OMSetDepthStencilState(DepthState, 0);

			ctx->IASetInputLayout(InputLayout);
			ctx->IASetVertexBuffers(0, 1, &VB, &stride, &offset);
			ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			ctx->VSSetShader(VS, NULL, 0);
			ctx->GSSetShader(NULL, NULL, 0);
			ctx->PSSetShader(PS, NULL, 0);
			ctx->PSSetSamplers(0, 1, &Sampler);
			ctx->PSSetShaderResources(0, 1, &UISRV11);

			ctx->Draw(4, 0);

			ID3D11ShaderResourceView* nullSRV = NULL;
			ctx->PSSetShaderResources(0, 1, &nullSRV);

			ctx->IASetInputLayout(oldLayout);
			ctx->IASetVertexBuffers(0, 1, &oldVB, &oldStride, &oldOffset);
			ctx->IASetPrimitiveTopology(oldTopology);

			ctx->VSSetShader(oldVS, NULL, 0);
			ctx->GSSetShader(oldGS, NULL, 0);
			ctx->PSSetShader(oldPS, NULL, 0);
			ctx->PSSetShaderResources(0, 1, &oldSRV);
			ctx->PSSetSamplers(0, 1, &oldSampler);

			if(oldViewportCount > 0)
				ctx->RSSetViewports(oldViewportCount, oldViewports);

			ctx->RSSetState(oldRaster);
			ctx->OMSetDepthStencilState(oldDepth, oldStencilRef);
			ctx->OMSetBlendState(oldBlend, oldBlendFactor, oldSampleMask);
			ctx->OMSetRenderTargets(1, &oldRTV, oldDSV);

			SafeReleaseLocal(oldRTV);
			SafeReleaseLocal(oldDSV);
			SafeReleaseLocal(oldBlend);
			SafeReleaseLocal(oldDepth);
			SafeReleaseLocal(oldRaster);
			SafeReleaseLocal(oldVS);
			SafeReleaseLocal(oldPS);
			SafeReleaseLocal(oldGS);
			SafeReleaseLocal(oldLayout);
			SafeReleaseLocal(oldVB);
			SafeReleaseLocal(oldSRV);
			SafeReleaseLocal(oldSampler);

			g_r3dDX11State.InvalidateCache();
			g_r3dDX11Geometry.InvalidateCache();

			return true;
		}

		bool IsCapturing() const
		{
			return Capturing;
		}

	private:
		bool CreateD3D9Resources()
		{
			IDirect3DDevice9* dev9 = GetDX9Device();
			if(!dev9)
				return false;

			HRESULT hr = dev9->CreateTexture(
				Width,
				Height,
				1,
				D3DUSAGE_RENDERTARGET,
				D3DFMT_A8R8G8B8,
				D3DPOOL_DEFAULT,
				&UITexture9,
				NULL
			);

			if(FAILED(hr))
			{
				r3dOutToLog("DX9UIBridge: CreateTexture UITexture9 failed 0x%08x\n", hr);
				return false;
			}

			hr = UITexture9->GetSurfaceLevel(0, &UISurface9);
			if(FAILED(hr))
			{
				r3dOutToLog("DX9UIBridge: GetSurfaceLevel failed 0x%08x\n", hr);
				return false;
			}

			hr = dev9->CreateOffscreenPlainSurface(
				Width,
				Height,
				D3DFMT_A8R8G8B8,
				D3DPOOL_SYSTEMMEM,
				&SystemSurface9,
				NULL
			);

			if(FAILED(hr))
			{
				r3dOutToLog("DX9UIBridge: CreateOffscreenPlainSurface failed 0x%08x\n", hr);
				return false;
			}

			return true;
		}

		bool CreateD3D11Texture()
		{
			ID3D11Device* dev11 = g_r3dDX11.GetDevice();
			if(!dev11)
				return false;

			D3D11_TEXTURE2D_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.Width = (UINT)Width;
			desc.Height = (UINT)Height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			HRESULT hr = dev11->CreateTexture2D(&desc, NULL, &UITexture11);
			if(FAILED(hr))
			{
				r3dOutToLog("DX9UIBridge: CreateTexture2D UITexture11 failed 0x%08x\n", hr);
				return false;
			}

			hr = dev11->CreateShaderResourceView(UITexture11, NULL, &UISRV11);
			if(FAILED(hr))
			{
				r3dOutToLog("DX9UIBridge: CreateShaderResourceView UISRV11 failed 0x%08x\n", hr);
				return false;
			}

			return true;
		}

		bool CreateD3D11Pipeline()
		{
			if(VS && PS && InputLayout && VB && Sampler && BlendState && DepthState && RasterState)
				return true;

			ID3D11Device* dev11 = g_r3dDX11.GetDevice();
			if(!dev11)
				return false;

			ReleaseD3D11Pipeline();

			static const char* vsCode =
				"struct VS_IN"
				"{"
				"	float2 Pos : POSITION;"
				"	float2 UV  : TEXCOORD0;"
				"};"
				"struct VS_OUT"
				"{"
				"	float4 Pos : SV_POSITION;"
				"	float2 UV  : TEXCOORD0;"
				"};"
				"VS_OUT main(VS_IN input)"
				"{"
				"	VS_OUT output;"
				"	output.Pos = float4(input.Pos, 0.0f, 1.0f);"
				"	output.UV = input.UV;"
				"	return output;"
				"}";

			static const char* psCode =
				"Texture2D UITexture : register(t0);"
				"SamplerState UISampler : register(s0);"
				"struct PS_IN"
				"{"
				"	float4 Pos : SV_POSITION;"
				"	float2 UV  : TEXCOORD0;"
				"};"
				"float4 main(PS_IN input) : SV_TARGET"
				"{"
				"	return UITexture.Sample(UISampler, input.UV);"
				"}";

			ID3DBlob* vsBlob = NULL;
			ID3DBlob* psBlob = NULL;
			ID3DBlob* errBlob = NULL;

			HRESULT hr = D3DCompile(vsCode, strlen(vsCode), "DX9UIBridgeVS", NULL, NULL, "main", "vs_4_0", 0, 0, &vsBlob, &errBlob);
			if(FAILED(hr))
			{
				if(errBlob)
					r3dOutToLog("DX9UIBridge VS compile error: %s\n", (const char*)errBlob->GetBufferPointer());

				SafeReleaseLocal(errBlob);
				return false;
			}

			SafeReleaseLocal(errBlob);

			hr = D3DCompile(psCode, strlen(psCode), "DX9UIBridgePS", NULL, NULL, "main", "ps_4_0", 0, 0, &psBlob, &errBlob);
			if(FAILED(hr))
			{
				if(errBlob)
					r3dOutToLog("DX9UIBridge PS compile error: %s\n", (const char*)errBlob->GetBufferPointer());

				SafeReleaseLocal(vsBlob);
				SafeReleaseLocal(errBlob);
				return false;
			}

			hr = dev11->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &VS);
			if(FAILED(hr))
			{
				SafeReleaseLocal(vsBlob);
				SafeReleaseLocal(psBlob);
				SafeReleaseLocal(errBlob);
				return false;
			}

			hr = dev11->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &PS);
			if(FAILED(hr))
			{
				SafeReleaseLocal(vsBlob);
				SafeReleaseLocal(psBlob);
				SafeReleaseLocal(errBlob);
				return false;
			}

			D3D11_INPUT_ELEMENT_DESC layoutDesc[2];
			ZeroMemory(layoutDesc, sizeof(layoutDesc));

			layoutDesc[0].SemanticName = "POSITION";
			layoutDesc[0].SemanticIndex = 0;
			layoutDesc[0].Format = DXGI_FORMAT_R32G32_FLOAT;
			layoutDesc[0].InputSlot = 0;
			layoutDesc[0].AlignedByteOffset = 0;
			layoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			layoutDesc[0].InstanceDataStepRate = 0;

			layoutDesc[1].SemanticName = "TEXCOORD";
			layoutDesc[1].SemanticIndex = 0;
			layoutDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
			layoutDesc[1].InputSlot = 0;
			layoutDesc[1].AlignedByteOffset = 8;
			layoutDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			layoutDesc[1].InstanceDataStepRate = 0;

			hr = dev11->CreateInputLayout(layoutDesc, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &InputLayout);

			SafeReleaseLocal(vsBlob);
			SafeReleaseLocal(psBlob);
			SafeReleaseLocal(errBlob);

			if(FAILED(hr))
				return false;

			DX11UIVertex vertices[4];
			vertices[0].x = -1.0f; vertices[0].y =  1.0f; vertices[0].u = 0.0f; vertices[0].v = 0.0f;
			vertices[1].x =  1.0f; vertices[1].y =  1.0f; vertices[1].u = 1.0f; vertices[1].v = 0.0f;
			vertices[2].x = -1.0f; vertices[2].y = -1.0f; vertices[2].u = 0.0f; vertices[2].v = 1.0f;
			vertices[3].x =  1.0f; vertices[3].y = -1.0f; vertices[3].u = 1.0f; vertices[3].v = 1.0f;

			D3D11_BUFFER_DESC vbDesc;
			ZeroMemory(&vbDesc, sizeof(vbDesc));
			vbDesc.ByteWidth = sizeof(vertices);
			vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
			vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

			D3D11_SUBRESOURCE_DATA vbData;
			ZeroMemory(&vbData, sizeof(vbData));
			vbData.pSysMem = vertices;

			hr = dev11->CreateBuffer(&vbDesc, &vbData, &VB);
			if(FAILED(hr))
				return false;

			D3D11_SAMPLER_DESC samplerDesc;
			ZeroMemory(&samplerDesc, sizeof(samplerDesc));
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

			hr = dev11->CreateSamplerState(&samplerDesc, &Sampler);
			if(FAILED(hr))
				return false;

			D3D11_BLEND_DESC blendDesc;
			ZeroMemory(&blendDesc, sizeof(blendDesc));
			blendDesc.RenderTarget[0].BlendEnable = TRUE;
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			hr = dev11->CreateBlendState(&blendDesc, &BlendState);
			if(FAILED(hr))
				return false;

			D3D11_DEPTH_STENCIL_DESC depthDesc;
			ZeroMemory(&depthDesc, sizeof(depthDesc));
			depthDesc.DepthEnable = FALSE;
			depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
			depthDesc.StencilEnable = FALSE;

			hr = dev11->CreateDepthStencilState(&depthDesc, &DepthState);
			if(FAILED(hr))
				return false;

			D3D11_RASTERIZER_DESC rasterDesc;
			ZeroMemory(&rasterDesc, sizeof(rasterDesc));
			rasterDesc.FillMode = D3D11_FILL_SOLID;
			rasterDesc.CullMode = D3D11_CULL_NONE;
			rasterDesc.DepthClipEnable = FALSE;
			rasterDesc.ScissorEnable = FALSE;
			rasterDesc.MultisampleEnable = FALSE;

			hr = dev11->CreateRasterizerState(&rasterDesc, &RasterState);
			if(FAILED(hr))
				return false;

			return true;
		}

		bool UploadToDX11()
		{
			if(!SystemSurface9 || !UITexture11)
				return false;

			ID3D11DeviceContext* ctx = g_r3dDX11.GetContext();
			if(!ctx)
				return false;

			D3DLOCKED_RECT src;
			ZeroMemory(&src, sizeof(src));

			HRESULT hr = SystemSurface9->LockRect(&src, NULL, D3DLOCK_READONLY);
			if(FAILED(hr))
				return false;

			D3D11_MAPPED_SUBRESOURCE dst;
			ZeroMemory(&dst, sizeof(dst));

			hr = ctx->Map(UITexture11, 0, D3D11_MAP_WRITE_DISCARD, 0, &dst);
			if(FAILED(hr))
			{
				SystemSurface9->UnlockRect();
				return false;
			}

			const unsigned char* srcBytes = (const unsigned char*)src.pBits;
			unsigned char* dstBytes = (unsigned char*)dst.pData;
			const int rowBytes = Width * 4;

			for(int y = 0; y < Height; ++y)
			{
				memcpy(
					dstBytes + y * dst.RowPitch,
					srcBytes + y * src.Pitch,
					rowBytes
				);
			}

			ctx->Unmap(UITexture11, 0);
			SystemSurface9->UnlockRect();

			return true;
		}

		void RestoreD3D9Target()
		{
			if(!r3dRenderer || !r3dRenderer->pd3ddev)
				return;

			IDirect3DDevice9* dev9 = GetDX9Device();

			if(OldRT9)
				dev9->SetRenderTarget(0, OldRT9);

			if(OldDS9)
				dev9->SetDepthStencilSurface(OldDS9);
			else
				dev9->SetDepthStencilSurface(NULL);

			if(OldVP9.Width > 0 && OldVP9.Height > 0)
				dev9->SetViewport(&OldVP9);

			SafeReleaseLocal(OldRT9);
			SafeReleaseLocal(OldDS9);

			ZeroMemory(&OldVP9, sizeof(OldVP9));
		}

		void ReleaseD3D9Resources()
		{
			SafeReleaseLocal(SystemSurface9);
			SafeReleaseLocal(UISurface9);
			SafeReleaseLocal(UITexture9);
		}

		void ReleaseD3D11Texture()
		{
			SafeReleaseLocal(UISRV11);
			SafeReleaseLocal(UITexture11);
		}

		void ReleaseD3D11Pipeline()
		{
			SafeReleaseLocal(RasterState);
			SafeReleaseLocal(DepthState);
			SafeReleaseLocal(BlendState);
			SafeReleaseLocal(Sampler);
			SafeReleaseLocal(VB);
			SafeReleaseLocal(InputLayout);
			SafeReleaseLocal(PS);
			SafeReleaseLocal(VS);
		}

	private:
		int Width;
		int Height;
		bool Capturing;

		IDirect3DTexture9* UITexture9;
		IDirect3DSurface9* UISurface9;
		IDirect3DSurface9* SystemSurface9;

		IDirect3DSurface9* OldRT9;
		IDirect3DSurface9* OldDS9;
		D3DVIEWPORT9 OldVP9;

		ID3D11Texture2D* UITexture11;
		ID3D11ShaderResourceView* UISRV11;

		ID3D11VertexShader* VS;
		ID3D11PixelShader* PS;
		ID3D11InputLayout* InputLayout;
		ID3D11Buffer* VB;
		ID3D11SamplerState* Sampler;
		ID3D11BlendState* BlendState;
		ID3D11DepthStencilState* DepthState;
		ID3D11RasterizerState* RasterState;
	};

	r3dDX9UIBridgeImpl gDX9UIBridge;
}

bool r3dDX9UIBridge_Init()
{
	return gDX9UIBridge.Init();
}

void r3dDX9UIBridge_Close()
{
	gDX9UIBridge.Close();
}

bool r3dDX9UIBridge_Begin()
{
	return gDX9UIBridge.Begin();
}

void r3dDX9UIBridge_End(bool compositeToDX11)
{
	gDX9UIBridge.End(compositeToDX11);
}

bool r3dDX9UIBridge_CompositeToDX11()
{
	return gDX9UIBridge.CompositeToDX11();
}

bool r3dDX9UIBridge_IsCapturing()
{
	return gDX9UIBridge.IsCapturing();
}

#endif