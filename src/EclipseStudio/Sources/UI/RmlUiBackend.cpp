#include "r3dPCH.h"
#include "r3d.h"

#include "RmlUiBackend.h"

#include "GameCommon.h"
#include "..\\Menus\\m_AppSelect.h"
#include "r3dDX11.h"
#include "r3dDX11Geometry.h"
#include "r3dDX11State.h"
#include "r3dDX11ScaleformBridge.h"

#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#ifdef DrawIndexed
#undef DrawIndexed
#endif

#ifdef DrawIndexedPrimitive
#undef DrawIndexedPrimitive
#endif

#ifdef DrawIndexedPrimitiveUP
#undef DrawIndexedPrimitiveUP
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

// -----------------------------------------------------------------------
//  Forward declarations used by the backend
// -----------------------------------------------------------------------
extern int AppSelectMode;
void ClearFullScreen_Menu();

// -----------------------------------------------------------------------
//  Globals
// -----------------------------------------------------------------------
const DWORD RMLUI_D3D9_FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
static bool g_RmlUiFrameUsingDX11Bridge = false;

bool RmlUiBackend::s_initialized = false;
RmlUiSystemInterface*   RmlUiBackend::s_system  = NULL;
RmlUiRenderInterface*   RmlUiBackend::s_render  = NULL;
Rocket::Core::Context*  RmlUiBackend::s_context = NULL;
IDirect3DDevice9*       RmlUiBackend::s_device  = NULL;
ID3D11Device*           RmlUiBackend::s_dx11Device = NULL;
bool                    RmlUiBackend::s_usingDX11 = false;

// =======================================================================
//  RmlUiSystemInterface
// =======================================================================
float RmlUiSystemInterface::GetElapsedTime()
{
	return r3dGetTime();
}

bool RmlUiSystemInterface::LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message)
{
	const char* level = "Info";

	switch(type)
	{
	case Rocket::Core::Log::LT_ERROR:   level = "Error";   break;
	case Rocket::Core::Log::LT_WARNING: level = "Warning"; break;
	case Rocket::Core::Log::LT_DEBUG:   level = "Debug";   break;
	default: break;
	}

	r3dOutToLog("RmlUi %s: %s\n", level, message.CString());
	return true;
}

// =======================================================================
//  DX11 implementation details
// =======================================================================
namespace
{
	struct RmlUiBackendDX11Vertex
	{
		float x;
		float y;
		unsigned int color;
		float u;
		float v;
	};

	struct RmlUiBackendDX11Texture
	{
		ID3D11Texture2D* Texture;
		ID3D11ShaderResourceView* SRV;

		RmlUiBackendDX11Texture()
		: Texture(NULL)
		, SRV(NULL)
		{
		}
	};
}

struct RmlUiRenderInterfaceDX11
{
	RmlUiRenderInterfaceDX11()
	: Device(NULL)
	, Context(NULL)
	, Width(0)
	, Height(0)
	, VS(NULL)
	, PS(NULL)
	, InputLayout(NULL)
	, Sampler(NULL)
	, BlendState(NULL)
	, DepthState(NULL)
	, RasterState(NULL)
	, RasterScissorState(NULL)
	, ConstantBuffer(NULL)
	, VertexBuffer(NULL)
	, IndexBuffer(NULL)
	, VertexBufferCapacity(0)
	, IndexBufferCapacity(0)
	, WhiteTexture(NULL)
	, WhiteSRV(NULL)
	, StateSaved(false)
	, OldRTV(NULL)
	, OldDSV(NULL)
	, OldBlend(NULL)
	, OldSampleMask(0)
	, OldDepth(NULL)
	, OldStencilRef(0)
	, OldRaster(NULL)
	, OldViewportCount(0)
	, OldScissorCount(0)
	, OldVS(NULL)
	, OldPS(NULL)
	, OldGS(NULL)
	, OldSRV(NULL)
	, OldSampler(NULL)
	, OldConstantBuffer(NULL)
	, OldLayout(NULL)
	, OldVB(NULL)
	, OldIB(NULL)
	, OldStride(0)
	, OldOffset(0)
	, OldIndexFormat(DXGI_FORMAT_UNKNOWN)
	, OldIndexOffset(0)
	, OldTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED)
	{
		ZeroMemory(OldBlendFactor, sizeof(OldBlendFactor));
		ZeroMemory(OldViewports, sizeof(OldViewports));
		ZeroMemory(OldScissors, sizeof(OldScissors));
	}

	~RmlUiRenderInterfaceDX11()
	{
		RestoreState();
		ReleaseResources();
	}

	void SetDevice(ID3D11Device* device, ID3D11DeviceContext* context, int width, int height)
	{
		if(Device != device || Context != context)
			ReleaseResources();

		Device = device;
		Context = context;
		Width = width;
		Height = height;
	}

	bool IsReady()
	{
		return Device && Context && EnsureResources();
	}

	bool EnsureResources()
	{
		if(!Device || !Context)
			return false;

		if(VS && PS && InputLayout && Sampler && BlendState && DepthState && RasterState &&
			RasterScissorState && ConstantBuffer && WhiteTexture && WhiteSRV)
		{
			return true;
		}

		ReleaseResources();

		static const char* vsCode =
			"cbuffer RmlConstants : register(b0)"
			"{"
			"	float2 ViewSize;"
			"	float2 Padding;"
			"};"
			"struct VS_IN"
			"{"
			"	float2 Pos : POSITION;"
			"	float4 Color : COLOR0;"
			"	float2 UV : TEXCOORD0;"
			"};"
			"struct VS_OUT"
			"{"
			"	float4 Pos : SV_POSITION;"
			"	float4 Color : COLOR0;"
			"	float2 UV : TEXCOORD0;"
			"};"
			"VS_OUT main(VS_IN input)"
			"{"
			"	VS_OUT output;"
			"	float2 clip;"
			"	clip.x = input.Pos.x / ViewSize.x * 2.0f - 1.0f;"
			"	clip.y = 1.0f - input.Pos.y / ViewSize.y * 2.0f;"
			"	output.Pos = float4(clip, 0.0f, 1.0f);"
			"	output.Color = input.Color;"
			"	output.UV = input.UV;"
			"	return output;"
			"}";

		static const char* psCode =
			"Texture2D RmlTexture : register(t0);"
			"SamplerState RmlSampler : register(s0);"
			"struct PS_IN"
			"{"
			"	float4 Pos : SV_POSITION;"
			"	float4 Color : COLOR0;"
			"	float2 UV : TEXCOORD0;"
			"};"
			"float4 main(PS_IN input) : SV_TARGET"
			"{"
			"	return RmlTexture.Sample(RmlSampler, input.UV) * input.Color;"
			"}";

		ID3DBlob* vsBlob = NULL;
		ID3DBlob* psBlob = NULL;
		ID3DBlob* errBlob = NULL;

		HRESULT hr = D3DCompile(vsCode, strlen(vsCode), "RmlUiDX11VS", NULL, NULL, "main", "vs_4_0", 0, 0, &vsBlob, &errBlob);
		if(FAILED(hr))
		{
			if(errBlob)
				r3dOutToLog("RmlUi DX11 VS compile error: %s\n", (const char*)errBlob->GetBufferPointer());
			SAFE_RELEASE(errBlob);
			return false;
		}
		SAFE_RELEASE(errBlob);

		hr = D3DCompile(psCode, strlen(psCode), "RmlUiDX11PS", NULL, NULL, "main", "ps_4_0", 0, 0, &psBlob, &errBlob);
		if(FAILED(hr))
		{
			if(errBlob)
				r3dOutToLog("RmlUi DX11 PS compile error: %s\n", (const char*)errBlob->GetBufferPointer());
			SAFE_RELEASE(vsBlob);
			SAFE_RELEASE(errBlob);
			return false;
		}
		SAFE_RELEASE(errBlob);

		hr = Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &VS);
		if(FAILED(hr))
		{
			SAFE_RELEASE(vsBlob);
			SAFE_RELEASE(psBlob);
			return false;
		}

		hr = Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &PS);
		if(FAILED(hr))
		{
			SAFE_RELEASE(vsBlob);
			SAFE_RELEASE(psBlob);
			return false;
		}

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(RmlUiBackendDX11Vertex, x), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(RmlUiBackendDX11Vertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(RmlUiBackendDX11Vertex, u), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		hr = Device->CreateInputLayout(layout, R3D_ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &InputLayout);
		SAFE_RELEASE(vsBlob);
		SAFE_RELEASE(psBlob);

		if(FAILED(hr))
			return false;

		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		hr = Device->CreateSamplerState(&samplerDesc, &Sampler);
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

		hr = Device->CreateBlendState(&blendDesc, &BlendState);
		if(FAILED(hr))
			return false;

		D3D11_DEPTH_STENCIL_DESC depthDesc;
		ZeroMemory(&depthDesc, sizeof(depthDesc));
		depthDesc.DepthEnable = FALSE;
		depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		depthDesc.StencilEnable = FALSE;

		hr = Device->CreateDepthStencilState(&depthDesc, &DepthState);
		if(FAILED(hr))
			return false;

		if(!CreateRasterState(false, &RasterState) || !CreateRasterState(true, &RasterScissorState))
			return false;

		D3D11_BUFFER_DESC cbDesc;
		ZeroMemory(&cbDesc, sizeof(cbDesc));
		cbDesc.ByteWidth = 16;
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		hr = Device->CreateBuffer(&cbDesc, NULL, &ConstantBuffer);
		if(FAILED(hr))
			return false;

		unsigned int white = 0xffffffff;
		D3D11_TEXTURE2D_DESC whiteDesc;
		ZeroMemory(&whiteDesc, sizeof(whiteDesc));
		whiteDesc.Width = 1;
		whiteDesc.Height = 1;
		whiteDesc.MipLevels = 1;
		whiteDesc.ArraySize = 1;
		whiteDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		whiteDesc.SampleDesc.Count = 1;
		whiteDesc.Usage = D3D11_USAGE_IMMUTABLE;
		whiteDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA whiteData;
		ZeroMemory(&whiteData, sizeof(whiteData));
		whiteData.pSysMem = &white;
		whiteData.SysMemPitch = sizeof(white);

		hr = Device->CreateTexture2D(&whiteDesc, &whiteData, &WhiteTexture);
		if(FAILED(hr))
			return false;

		hr = Device->CreateShaderResourceView(WhiteTexture, NULL, &WhiteSRV);
		if(FAILED(hr))
			return false;

		return true;
	}

	bool CreateRasterState(bool scissor, ID3D11RasterizerState** outState)
	{
		D3D11_RASTERIZER_DESC rasterDesc;
		ZeroMemory(&rasterDesc, sizeof(rasterDesc));
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.CullMode = D3D11_CULL_NONE;
		rasterDesc.FrontCounterClockwise = FALSE;
		rasterDesc.DepthClipEnable = FALSE;
		rasterDesc.ScissorEnable = scissor ? TRUE : FALSE;
		rasterDesc.MultisampleEnable = FALSE;
		rasterDesc.AntialiasedLineEnable = FALSE;

		return SUCCEEDED(Device->CreateRasterizerState(&rasterDesc, outState));
	}

	void ReleaseResources()
	{
		SAFE_RELEASE(WhiteSRV);
		SAFE_RELEASE(WhiteTexture);
		SAFE_RELEASE(IndexBuffer);
		SAFE_RELEASE(VertexBuffer);
		SAFE_RELEASE(ConstantBuffer);
		SAFE_RELEASE(RasterScissorState);
		SAFE_RELEASE(RasterState);
		SAFE_RELEASE(DepthState);
		SAFE_RELEASE(BlendState);
		SAFE_RELEASE(Sampler);
		SAFE_RELEASE(InputLayout);
		SAFE_RELEASE(PS);
		SAFE_RELEASE(VS);

		VertexBufferCapacity = 0;
		IndexBufferCapacity = 0;
	}

	bool EnsureDynamicBuffer(ID3D11Buffer** buffer, unsigned int& capacity, unsigned int requiredBytes, unsigned int bindFlags)
	{
		if(*buffer && capacity >= requiredBytes)
			return true;

		SAFE_RELEASE(*buffer);

		capacity = R3D_MAX(requiredBytes, 4096u);

		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.ByteWidth = capacity;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = bindFlags;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		return SUCCEEDED(Device->CreateBuffer(&desc, NULL, buffer));
	}

	void SaveState()
	{
		if(StateSaved || !Context)
			return;

		Context->OMGetRenderTargets(1, &OldRTV, &OldDSV);
		Context->OMGetBlendState(&OldBlend, OldBlendFactor, &OldSampleMask);
		Context->OMGetDepthStencilState(&OldDepth, &OldStencilRef);
		Context->RSGetState(&OldRaster);

		OldViewportCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
		Context->RSGetViewports(&OldViewportCount, OldViewports);

		OldScissorCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
		Context->RSGetScissorRects(&OldScissorCount, OldScissors);

		Context->VSGetShader(&OldVS, NULL, NULL);
		Context->PSGetShader(&OldPS, NULL, NULL);
		Context->GSGetShader(&OldGS, NULL, NULL);
		Context->PSGetShaderResources(0, 1, &OldSRV);
		Context->PSGetSamplers(0, 1, &OldSampler);
		Context->VSGetConstantBuffers(0, 1, &OldConstantBuffer);
		Context->IAGetInputLayout(&OldLayout);
		Context->IAGetVertexBuffers(0, 1, &OldVB, &OldStride, &OldOffset);
		Context->IAGetIndexBuffer(&OldIB, &OldIndexFormat, &OldIndexOffset);
		Context->IAGetPrimitiveTopology(&OldTopology);

		StateSaved = true;
	}

	void Prepare()
	{
		if(!IsReady())
			return;

		SaveState();

		D3D11_MAPPED_SUBRESOURCE mapped;
		if(SUCCEEDED(Context->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
		{
			float* values = static_cast<float*>(mapped.pData);
			values[0] = (float)R3D_MAX(Width, 1);
			values[1] = (float)R3D_MAX(Height, 1);
			values[2] = 0.0f;
			values[3] = 0.0f;
			Context->Unmap(ConstantBuffer, 0);
		}

		D3D11_VIEWPORT vp;
		ZeroMemory(&vp, sizeof(vp));
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.Width = (float)R3D_MAX(Width, 1);
		vp.Height = (float)R3D_MAX(Height, 1);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;

		FLOAT blendFactor[4] = {0, 0, 0, 0};
		Context->RSSetViewports(1, &vp);
		Context->RSSetState(RasterState);
		Context->OMSetBlendState(BlendState, blendFactor, 0xffffffff);
		Context->OMSetDepthStencilState(DepthState, 0);
		Context->IASetInputLayout(InputLayout);
		Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Context->VSSetShader(VS, NULL, 0);
		Context->GSSetShader(NULL, NULL, 0);
		Context->PSSetShader(PS, NULL, 0);
		Context->PSSetSamplers(0, 1, &Sampler);
		Context->VSSetConstantBuffers(0, 1, &ConstantBuffer);
	}

	void RestoreState()
	{
		if(!StateSaved || !Context)
			return;

		ID3D11ShaderResourceView* nullSRV = NULL;
		Context->PSSetShaderResources(0, 1, &nullSRV);

		Context->IASetInputLayout(OldLayout);
		Context->IASetVertexBuffers(0, 1, &OldVB, &OldStride, &OldOffset);
		Context->IASetIndexBuffer(OldIB, OldIndexFormat, OldIndexOffset);
		Context->IASetPrimitiveTopology(OldTopology);

		Context->VSSetShader(OldVS, NULL, 0);
		Context->GSSetShader(OldGS, NULL, 0);
		Context->PSSetShader(OldPS, NULL, 0);
		Context->PSSetShaderResources(0, 1, &OldSRV);
		Context->PSSetSamplers(0, 1, &OldSampler);
		Context->VSSetConstantBuffers(0, 1, &OldConstantBuffer);

		if(OldViewportCount > 0)
			Context->RSSetViewports(OldViewportCount, OldViewports);
		if(OldScissorCount > 0)
			Context->RSSetScissorRects(OldScissorCount, OldScissors);

		Context->RSSetState(OldRaster);
		Context->OMSetDepthStencilState(OldDepth, OldStencilRef);
		Context->OMSetBlendState(OldBlend, OldBlendFactor, OldSampleMask);
		Context->OMSetRenderTargets(1, &OldRTV, OldDSV);

		SAFE_RELEASE(OldRTV);
		SAFE_RELEASE(OldDSV);
		SAFE_RELEASE(OldBlend);
		SAFE_RELEASE(OldDepth);
		SAFE_RELEASE(OldRaster);
		SAFE_RELEASE(OldVS);
		SAFE_RELEASE(OldPS);
		SAFE_RELEASE(OldGS);
		SAFE_RELEASE(OldSRV);
		SAFE_RELEASE(OldSampler);
		SAFE_RELEASE(OldConstantBuffer);
		SAFE_RELEASE(OldLayout);
		SAFE_RELEASE(OldVB);
		SAFE_RELEASE(OldIB);

		StateSaved = false;

		g_r3dDX11State.InvalidateCache();
		g_r3dDX11Geometry.InvalidateCache();
	}

	void RenderGeometry(Rocket::Core::Vertex* vertices, int numVertices, int* indices, int numIndices, Rocket::Core::TextureHandle texture, const Rocket::Core::Vector2f& translation)
	{
		if(!IsReady() || !vertices || !indices || numVertices <= 0 || numIndices <= 0)
			return;

		const unsigned int vbBytes = (unsigned int)(numVertices * sizeof(RmlUiBackendDX11Vertex));
		const unsigned int ibBytes = (unsigned int)(numIndices * sizeof(unsigned int));

		if(!EnsureDynamicBuffer(&VertexBuffer, VertexBufferCapacity, vbBytes, D3D11_BIND_VERTEX_BUFFER) ||
			!EnsureDynamicBuffer(&IndexBuffer, IndexBufferCapacity, ibBytes, D3D11_BIND_INDEX_BUFFER))
		{
			return;
		}

		D3D11_MAPPED_SUBRESOURCE mapped;
		if(FAILED(Context->Map(VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
			return;

		RmlUiBackendDX11Vertex* dstVertices = static_cast<RmlUiBackendDX11Vertex*>(mapped.pData);
		for(int i = 0; i < numVertices; ++i)
		{
			const Rocket::Core::Vertex& src = vertices[i];
			RmlUiBackendDX11Vertex& dst = dstVertices[i];
			dst.x = src.position.x + translation.x;
			dst.y = src.position.y + translation.y;
			dst.color =
				((unsigned int)src.colour.red) |
				((unsigned int)src.colour.green << 8) |
				((unsigned int)src.colour.blue << 16) |
				((unsigned int)src.colour.alpha << 24);
			dst.u = src.tex_coord.x;
			dst.v = src.tex_coord.y;
		}
		Context->Unmap(VertexBuffer, 0);

		if(FAILED(Context->Map(IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
			return;

		memcpy(mapped.pData, indices, ibBytes);
		Context->Unmap(IndexBuffer, 0);

		UINT stride = sizeof(RmlUiBackendDX11Vertex);
		UINT offset = 0;
		Context->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
		Context->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		Context->IASetInputLayout(InputLayout);
		Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Context->VSSetShader(VS, NULL, 0);
		Context->PSSetShader(PS, NULL, 0);
		Context->VSSetConstantBuffers(0, 1, &ConstantBuffer);
		Context->PSSetSamplers(0, 1, &Sampler);

		RmlUiBackendDX11Texture* rmlTexture = reinterpret_cast<RmlUiBackendDX11Texture*>(texture);
		ID3D11ShaderResourceView* srv = rmlTexture && rmlTexture->SRV ? rmlTexture->SRV : WhiteSRV;
		Context->PSSetShaderResources(0, 1, &srv);
		Context->DrawIndexed((UINT)numIndices, 0, 0);
	}

	bool GenerateTexture(Rocket::Core::TextureHandle& textureHandle, const Rocket::Core::byte* source, const Rocket::Core::Vector2i& sourceDimensions)
	{
		if(!Device || !source || sourceDimensions.x <= 0 || sourceDimensions.y <= 0)
			return false;

		RmlUiBackendDX11Texture* texture = new RmlUiBackendDX11Texture();

		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = (UINT)sourceDimensions.x;
		desc.Height = (UINT)sourceDimensions.y;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA data;
		ZeroMemory(&data, sizeof(data));
		data.pSysMem = source;
		data.SysMemPitch = (UINT)(sourceDimensions.x * 4);

		HRESULT hr = Device->CreateTexture2D(&desc, &data, &texture->Texture);
		if(FAILED(hr))
		{
			delete texture;
			r3dOutToLog("RmlUi DX11: CreateTexture2D failed hr=0x%08x\n", hr);
			return false;
		}

		hr = Device->CreateShaderResourceView(texture->Texture, NULL, &texture->SRV);
		if(FAILED(hr))
		{
			SAFE_RELEASE(texture->Texture);
			delete texture;
			r3dOutToLog("RmlUi DX11: CreateShaderResourceView failed hr=0x%08x\n", hr);
			return false;
		}

		textureHandle = reinterpret_cast<Rocket::Core::TextureHandle>(texture);
		return true;
	}

	void ReleaseTexture(Rocket::Core::TextureHandle textureHandle)
	{
		RmlUiBackendDX11Texture* texture = reinterpret_cast<RmlUiBackendDX11Texture*>(textureHandle);
		if(!texture)
			return;

		SAFE_RELEASE(texture->SRV);
		SAFE_RELEASE(texture->Texture);
		delete texture;
	}

	ID3D11Device* Device;
	ID3D11DeviceContext* Context;
	int Width;
	int Height;

	ID3D11VertexShader* VS;
	ID3D11PixelShader* PS;
	ID3D11InputLayout* InputLayout;
	ID3D11SamplerState* Sampler;
	ID3D11BlendState* BlendState;
	ID3D11DepthStencilState* DepthState;
	ID3D11RasterizerState* RasterState;
	ID3D11RasterizerState* RasterScissorState;
	ID3D11Buffer* ConstantBuffer;
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
	unsigned int VertexBufferCapacity;
	unsigned int IndexBufferCapacity;
	ID3D11Texture2D* WhiteTexture;
	ID3D11ShaderResourceView* WhiteSRV;

	bool StateSaved;
	ID3D11RenderTargetView* OldRTV;
	ID3D11DepthStencilView* OldDSV;
	ID3D11BlendState* OldBlend;
	FLOAT OldBlendFactor[4];
	UINT OldSampleMask;
	ID3D11DepthStencilState* OldDepth;
	UINT OldStencilRef;
	ID3D11RasterizerState* OldRaster;
	UINT OldViewportCount;
	D3D11_VIEWPORT OldViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	UINT OldScissorCount;
	D3D11_RECT OldScissors[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	ID3D11VertexShader* OldVS;
	ID3D11PixelShader* OldPS;
	ID3D11GeometryShader* OldGS;
	ID3D11ShaderResourceView* OldSRV;
	ID3D11SamplerState* OldSampler;
	ID3D11Buffer* OldConstantBuffer;
	ID3D11InputLayout* OldLayout;
	ID3D11Buffer* OldVB;
	ID3D11Buffer* OldIB;
	UINT OldStride;
	UINT OldOffset;
	DXGI_FORMAT OldIndexFormat;
	UINT OldIndexOffset;
	D3D11_PRIMITIVE_TOPOLOGY OldTopology;
};

// =======================================================================
//  RmlUiRenderInterface
// =======================================================================
RmlUiRenderInterface::RmlUiRenderInterface()
: m_device(NULL)
, m_width(0)
, m_height(0)
, m_dx11(new RmlUiRenderInterfaceDX11())
{
}

RmlUiRenderInterface::~RmlUiRenderInterface()
{
	delete m_dx11;
	m_dx11 = NULL;
}

void RmlUiRenderInterface::SetDevice(IDirect3DDevice9* device, int width, int height)
{
	m_device = device;
	m_width  = width;
	m_height = height;
	if(m_dx11)
		m_dx11->SetDevice(NULL, NULL, width, height);
}

void RmlUiRenderInterface::SetDX11Device(ID3D11Device* device, ID3D11DeviceContext* context, int width, int height)
{
	m_device = NULL;
	m_width  = width;
	m_height = height;
	if(m_dx11)
		m_dx11->SetDevice(device, context, width, height);
}

bool RmlUiRenderInterface::UsesDX11() const
{
	return m_dx11 && m_dx11->Device != NULL && m_dx11->Context != NULL;
}

void RmlUiRenderInterface::PrepareState()
{
	if(UsesDX11())
	{
		m_dx11->Prepare();
		return;
	}

	if(!m_device)
		return;

	D3DVIEWPORT9 vp;
	vp.X      = 0;
	vp.Y      = 0;
	vp.Width  = (DWORD)R3D_MAX(m_width, 1);
	vp.Height = (DWORD)R3D_MAX(m_height, 1);
	vp.MinZ   = 0.0f;
	vp.MaxZ   = 1.0f;

	m_device->SetViewport(&vp);
	m_device->SetFVF(RMLUI_D3D9_FVF);
	m_device->SetVertexShader(NULL);
	m_device->SetPixelShader(NULL);

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE,  FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE,  D3DCULL_NONE);
	m_device->SetRenderState(D3DRS_LIGHTING,  FALSE);
	m_device->SetRenderState(D3DRS_ZENABLE,   FALSE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	m_device->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	m_device->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
	m_device->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);

	m_device->SetSamplerState(0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP);
	m_device->SetSamplerState(0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP);
	m_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
}

void RmlUiRenderInterface::RestoreState()
{
	if(UsesDX11())
		m_dx11->RestoreState();
}

void RmlUiRenderInterface::RenderGeometry(
	Rocket::Core::Vertex* vertices,
	int num_vertices,
	int* indices,
	int num_indices,
	Rocket::Core::TextureHandle texture,
	const Rocket::Core::Vector2f& translation)
{
	if(UsesDX11())
	{
		m_dx11->RenderGeometry(vertices, num_vertices, indices, num_indices, texture, translation);
		return;
	}

	if(!m_device || !vertices || !indices || num_vertices <= 0 || num_indices <= 0)
		return;

	m_vertices.resize(num_vertices);

	for(int i = 0; i < num_vertices; ++i)
	{
		const Rocket::Core::Vertex& src = vertices[i];
		RmlUiBackendVertex& dst = m_vertices[i];

		dst.x     = src.position.x + translation.x;
		dst.y     = src.position.y + translation.y;
		dst.z     = 0.0f;
		dst.rhw   = 1.0f;
		dst.color = D3DCOLOR_ARGB(src.colour.alpha, src.colour.red, src.colour.green, src.colour.blue);
		dst.u     = src.tex_coord.x;
		dst.v     = src.tex_coord.y;
	}

	m_device->SetTexture(0, reinterpret_cast<IDirect3DTexture9*>(texture));
	m_device->SetFVF(RMLUI_D3D9_FVF);
	m_device->DrawIndexedPrimitiveUP(
		D3DPT_TRIANGLELIST,
		0,
		num_vertices,
		num_indices / 3,
		indices,
		D3DFMT_INDEX32,
		&m_vertices[0],
		sizeof(RmlUiBackendVertex));
}

void RmlUiRenderInterface::EnableScissorRegion(bool enable)
{
	if(UsesDX11())
	{
		m_dx11->Context->RSSetState(enable ? m_dx11->RasterScissorState : m_dx11->RasterState);
		return;
	}

	if(m_device)
		m_device->SetRenderState(D3DRS_SCISSORTESTENABLE, enable ? TRUE : FALSE);
}

void RmlUiRenderInterface::SetScissorRegion(int x, int y, int width, int height)
{
	if(UsesDX11())
	{
		D3D11_RECT rect;
		rect.left = x;
		rect.top = y;
		rect.right = x + width;
		rect.bottom = y + height;
		m_dx11->Context->RSSetScissorRects(1, &rect);
		return;
	}

	if(!m_device)
		return;

	RECT rect;
	rect.left   = x;
	rect.top    = y;
	rect.right  = x + width;
	rect.bottom = y + height;

	m_device->SetScissorRect(&rect);
}

bool RmlUiRenderInterface::GenerateTexture(
	Rocket::Core::TextureHandle& texture_handle,
	const Rocket::Core::byte* source,
	const Rocket::Core::Vector2i& source_dimensions)
{
	if(UsesDX11())
		return m_dx11->GenerateTexture(texture_handle, source, source_dimensions);

	if(!m_device || !source || source_dimensions.x <= 0 || source_dimensions.y <= 0)
		return false;

	IDirect3DTexture9* texture = NULL;
	HRESULT hr = m_device->CreateTexture(
		source_dimensions.x,
		source_dimensions.y,
		1,
		D3DUSAGE_DYNAMIC,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&texture,
		NULL);

	if(FAILED(hr) || !texture)
	{
		r3dOutToLog("RmlUi: CreateTexture failed hr=0x%08x\n", hr);
		return false;
	}

	D3DLOCKED_RECT locked;
	ZeroMemory(&locked, sizeof(locked));

	hr = texture->LockRect(0, &locked, NULL, D3DLOCK_DISCARD);
	if(FAILED(hr))
	{
		texture->Release();
		r3dOutToLog("RmlUi: font texture LockRect failed hr=0x%08x\n", hr);
		return false;
	}

	for(int y = 0; y < source_dimensions.y; ++y)
	{
		unsigned char* dst = static_cast<unsigned char*>(locked.pBits) + locked.Pitch * y;
		const unsigned char* src = source + source_dimensions.x * y * 4;

		for(int x = 0; x < source_dimensions.x; ++x)
		{
			dst[x * 4 + 0] = src[x * 4 + 2];
			dst[x * 4 + 1] = src[x * 4 + 1];
			dst[x * 4 + 2] = src[x * 4 + 0];
			dst[x * 4 + 3] = src[x * 4 + 3];
		}
	}

	texture->UnlockRect(0);
	texture_handle = reinterpret_cast<Rocket::Core::TextureHandle>(texture);
	return true;
}

void RmlUiRenderInterface::ReleaseTexture(Rocket::Core::TextureHandle texture)
{
	if(UsesDX11())
	{
		m_dx11->ReleaseTexture(texture);
		return;
	}

	IDirect3DTexture9* d3d_texture = reinterpret_cast<IDirect3DTexture9*>(texture);

	if(d3d_texture)
		d3d_texture->Release();
}

// =======================================================================
//  Helpers (anonymous namespace in cpp)
// =======================================================================
namespace
{
	bool UseNativeDX11Rml()
	{
		return r3dRenderer &&
			!r3dRenderer->GetUseD3D9Present() &&
			g_r3dDX11.IsInitialized() &&
			g_r3dDX11.GetDevice() &&
			g_r3dDX11.GetContext();
	}

	IDirect3DDevice9* GetRmlD3D9Device()
	{
		if(r3dRenderer && r3dRenderer->GetUseD3D9Present())
			return r3dRenderer->pd3ddev;

		if(g_r3dDX11ScaleformBridge.IsReady())
			return r3dGetScaleformD3D9Device();

		if(!r3dRenderer)
			return NULL;

		return r3dRenderer->pd3ddev;
	}

	bool LoadDefaultFont()
	{
		const char* fonts[] =
		{
			"C:/Windows/Fonts/arial.ttf",
			"C:/Windows/Fonts/segoeui.ttf",
			"C:/Windows/Fonts/tahoma.ttf",
		};

		for(int i = 0; i < R3D_ARRAYSIZE(fonts); ++i)
		{
			if(Rocket::Core::FontDatabase::LoadFontFace(fonts[i], "Arial",
				Rocket::Core::Font::STYLE_NORMAL,
				Rocket::Core::Font::WEIGHT_NORMAL))
			{
				r3dOutToLog("RmlUi: loaded font %s\n", fonts[i]);
				return true;
			}
		}

		r3dOutToLog("RmlUi: failed to load a Windows UI font\n");
		return false;
	}
}

// =======================================================================
//  RmlUiBackend  implementation
// =======================================================================
bool RmlUiBackend::Initialize()
{
	const bool useDX11 = UseNativeDX11Rml();
	ID3D11Device* dx11Device = useDX11 ? g_r3dDX11.GetDevice() : NULL;
	ID3D11DeviceContext* dx11Context = useDX11 ? g_r3dDX11.GetContext() : NULL;
	IDirect3DDevice9* device = useDX11 ? NULL : GetRmlD3D9Device();

	if(!useDX11 && !device)
	{
		r3dOutToLog("RmlUi Backend: no D3D9 device available\n");
		return false;
	}

	if(s_initialized)
	{
		if(s_usingDX11 == useDX11 && ((useDX11 && s_dx11Device == dx11Device) || (!useDX11 && s_device == device)))
			return true;

		r3dOutToLog("RmlUi Backend: render device changed, recreating backend\n");
		Shutdown();
	}

	s_system = new RmlUiSystemInterface();
	s_render = new RmlUiRenderInterface();

	if(useDX11)
		s_render->SetDX11Device(dx11Device, dx11Context, (int)r3dRenderer->ScreenW, (int)r3dRenderer->ScreenH);
	else
		s_render->SetDevice(device, (int)r3dRenderer->ScreenW, (int)r3dRenderer->ScreenH);

	s_device = device;
	s_dx11Device = dx11Device;
	s_usingDX11 = useDX11;

	Rocket::Core::SetSystemInterface(s_system);
	Rocket::Core::SetRenderInterface(s_render);

	if(!Rocket::Core::Initialise())
	{
		r3dOutToLog("RmlUi Backend: Rocket::Core::Initialise failed\n");
		delete s_system; s_system = NULL;
		delete s_render; s_render = NULL;
		return false;
	}

	s_initialized = true;

	LoadDefaultFont();

	s_context = Rocket::Core::CreateContext(
		"WarIncRmlUi",
		Rocket::Core::Vector2i((int)r3dRenderer->ScreenW, (int)r3dRenderer->ScreenH));

	if(!s_context)
	{
		r3dOutToLog("RmlUi Backend: CreateContext failed\n");
		Shutdown();
		return false;
	}

	r3dOutToLog("RmlUi Backend: initialized (%dx%d, %s)\n",
		(int)r3dRenderer->ScreenW, (int)r3dRenderer->ScreenH, useDX11 ? "DX11" : "D3D9");
	return true;
}

void RmlUiBackend::Shutdown()
{
	if(s_context)
	{
		s_context->RemoveReference();
		s_context = NULL;
	}

	if(s_initialized)
	{
		Rocket::Core::Shutdown();
		s_initialized = false;
	}

	delete s_system;  s_system  = NULL;
	delete s_render;  s_render  = NULL;
	s_device = NULL;
	s_dx11Device = NULL;
	s_usingDX11 = false;

	r3dOutToLog("RmlUi Backend: shut down\n");
}

bool RmlUiBackend::IsInitialized()
{
	return s_initialized;
}

Rocket::Core::Context* RmlUiBackend::GetContext()
{
	return s_context;
}

Rocket::Core::ElementDocument* RmlUiBackend::LoadDocumentFromMemory(const Rocket::Core::String& rml)
{
	if(!s_context)
		return NULL;

	return s_context->LoadDocumentFromMemory(rml);
}

Rocket::Core::ElementDocument* RmlUiBackend::LoadDocumentFromFile(const char* filePath)
{
	if(!s_context)
		return NULL;

	return s_context->LoadDocument(filePath);
}

void RmlUiBackend::BeginFrame()
{
	g_RmlUiFrameUsingDX11Bridge = false;

	if(!s_initialized || !s_render || !r3dRenderer)
		return;

	if(UseNativeDX11Rml())
	{
		s_render->SetDX11Device(g_r3dDX11.GetDevice(), g_r3dDX11.GetContext(), (int)r3dRenderer->ScreenW, (int)r3dRenderer->ScreenH);
		s_render->PrepareState();
		return;
	}

	IDirect3DDevice9* device = GetRmlD3D9Device();
	if(!device)
		return;

	if(!r3dRenderer->GetUseD3D9Present() && g_r3dDX11ScaleformBridge.IsReady())
		g_RmlUiFrameUsingDX11Bridge = g_r3dDX11ScaleformBridge.BeginScaleformRender();

	s_render->SetDevice(device, (int)r3dRenderer->ScreenW, (int)r3dRenderer->ScreenH);
	s_render->PrepareState();
}

void RmlUiBackend::EndFrame()
{
	if(s_render)
		s_render->RestoreState();

	if(g_RmlUiFrameUsingDX11Bridge)
	{
		g_r3dDX11ScaleformBridge.EndScaleformRender();
		g_r3dDX11ScaleformBridge.DrawDX11();
		g_RmlUiFrameUsingDX11Bridge = false;
	}
}

void RmlUiBackend::ProcessMouse()
{
	if(!s_context)
		return;

	const int modifiers = GetKeyModifierState();

	int mx = 0;
	int my = 0;
	Mouse->GetXY(mx, my);

	s_context->ProcessMouseMove(mx, my, modifiers);

	if(Mouse->WasPressed(r3dMouse::mLeftButton))
		s_context->ProcessMouseButtonDown(0, modifiers);

	if(Mouse->WasReleased(r3dMouse::mLeftButton))
		s_context->ProcessMouseButtonUp(0, modifiers);
}

int RmlUiBackend::GetKeyModifierState()
{
	int modifiers = 0;

	if(Keyboard->IsPressed(kbsLeftShift) || Keyboard->IsPressed(kbsRightShift))
		modifiers |= Rocket::Core::Input::KM_SHIFT;

	if(Keyboard->IsPressed(kbsLeftControl) || Keyboard->IsPressed(kbsRightControl))
		modifiers |= Rocket::Core::Input::KM_CTRL;

	if(Keyboard->IsPressed(kbsLeftAlt) || Keyboard->IsPressed(kbsRightAlt))
		modifiers |= Rocket::Core::Input::KM_ALT;

	return modifiers;
}

int RmlUiBackend::ProcessEscapeKey()
{
	if(Keyboard->WasPressed(kbsEsc))
		return RMLUI_BACKEND_QUIT_RESULT;

	return 0;
}

int RmlUiBackend::ProcessHotkeys(const struct AppSelectCommandBinding* commands, int numCommands)
{
	for(int i = 0; i < numCommands; ++i)
	{
		if(Keyboard->WasPressed(commands[i].HotKey))
			return commands[i].Result;
	}

	return 0;
}

float RmlUiBackend::GetScreenWidth()
{
	return r3dRenderer ? (float)r3dRenderer->ScreenW : 1024.0f;
}

float RmlUiBackend::GetScreenHeight()
{
	return r3dRenderer ? (float)r3dRenderer->ScreenH : 768.0f;
}
