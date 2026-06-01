#include "WarNoesisD3D9RenderDevice.h"

#include <algorithm>
#include <string.h>

WarNoesisD3D9Texture::WarNoesisD3D9Texture(
	IDirect3DTexture9* texture,
	uint32_t width,
	uint32_t height,
	uint32_t levels,
	Noesis::TextureFormat::Enum format,
	bool hasAlpha
)
{
	Texture = texture;
	Width = width;
	Height = height;
	Levels = levels;
	Format = format;
	bHasAlpha = hasAlpha;
}

WarNoesisD3D9Texture::~WarNoesisD3D9Texture()
{
	if(Texture)
	{
		Texture->Release();
		Texture = NULL;
	}
}

uint32_t WarNoesisD3D9Texture::GetWidth() const
{
	return Width;
}

uint32_t WarNoesisD3D9Texture::GetHeight() const
{
	return Height;
}

bool WarNoesisD3D9Texture::HasMipMaps() const
{
	return Levels > 1;
}

bool WarNoesisD3D9Texture::IsInverted() const
{
	return false;
}

bool WarNoesisD3D9Texture::HasAlpha() const
{
	return bHasAlpha;
}

IDirect3DTexture9* WarNoesisD3D9Texture::GetD3DTexture() const
{
	return Texture;
}

Noesis::TextureFormat::Enum WarNoesisD3D9Texture::GetFormat() const
{
	return Format;
}

WarNoesisD3D9RenderTarget::WarNoesisD3D9RenderTarget(
	IDirect3DDevice9* device,
	uint32_t width,
	uint32_t height
)
{
	Surface = NULL;
	Width = width;
	Height = height;

	IDirect3DTexture9* d3dTexture = NULL;

	HRESULT hr = device->CreateTexture(
		width,
		height,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&d3dTexture,
		NULL
	);

	if(SUCCEEDED(hr) && d3dTexture)
	{
		d3dTexture->GetSurfaceLevel(0, &Surface);

		TargetTexture = *new WarNoesisD3D9Texture(
			d3dTexture,
			width,
			height,
			1,
			Noesis::TextureFormat::RGBA8,
			true
		);
	}
}

WarNoesisD3D9RenderTarget::~WarNoesisD3D9RenderTarget()
{
	if(Surface)
	{
		Surface->Release();
		Surface = NULL;
	}

	TargetTexture.Reset();
}

Noesis::Texture* WarNoesisD3D9RenderTarget::GetTexture()
{
	return TargetTexture;
}

IDirect3DSurface9* WarNoesisD3D9RenderTarget::GetSurface() const
{
	return Surface;
}

uint32_t WarNoesisD3D9RenderTarget::GetWidth() const
{
	return Width;
}

uint32_t WarNoesisD3D9RenderTarget::GetHeight() const
{
	return Height;
}

WarNoesisD3D9RenderDevice::WarNoesisD3D9RenderDevice(IDirect3DDevice9* device)
{
	Device = device;
	SavedState = NULL;
	CurrentTargetHeight = 0;

	if(Device)
		Device->AddRef();

	Caps.centerPixelOffset = 0.0f;
	Caps.linearRendering = false;
	Caps.subpixelRendering = false;
	Caps.depthRangeZeroToOne = true;
	Caps.clipSpaceYInverted = false;
}

WarNoesisD3D9RenderDevice::~WarNoesisD3D9RenderDevice()
{
	RestoreState();

	if(Device)
	{
		Device->Release();
		Device = NULL;
	}
}

const Noesis::DeviceCaps& WarNoesisD3D9RenderDevice::GetCaps() const
{
	return Caps;
}

Noesis::Ptr<Noesis::RenderTarget> WarNoesisD3D9RenderDevice::CreateRenderTarget(
	const char* label,
	uint32_t width,
	uint32_t height,
	uint32_t sampleCount,
	bool needsStencil
)
{
	WarNoesisD3D9RenderTarget* target = new WarNoesisD3D9RenderTarget(Device, width, height);
	return Noesis::Ptr<Noesis::RenderTarget>(*target);
}

Noesis::Ptr<Noesis::RenderTarget> WarNoesisD3D9RenderDevice::CloneRenderTarget(
	const char* label,
	Noesis::RenderTarget* surface
)
{
	WarNoesisD3D9RenderTarget* src = Noesis::DynamicCast<WarNoesisD3D9RenderTarget*>(surface);

	uint32_t width = 1;
	uint32_t height = 1;

	if(src)
	{
		width = src->GetWidth();
		height = src->GetHeight();
	}

	WarNoesisD3D9RenderTarget* target = new WarNoesisD3D9RenderTarget(Device, width, height);
	return Noesis::Ptr<Noesis::RenderTarget>(*target);
}

Noesis::Ptr<Noesis::Texture> WarNoesisD3D9RenderDevice::CreateTexture(
	const char* label,
	uint32_t width,
	uint32_t height,
	uint32_t numLevels,
	Noesis::TextureFormat::Enum format,
	const void** data
)
{
	IDirect3DTexture9* texture = NULL;

	HRESULT hr = Device->CreateTexture(
		width,
		height,
		numLevels,
		0,
		D3DFMT_A8R8G8B8,
		D3DPOOL_MANAGED,
		&texture,
		NULL
	);

	if(FAILED(hr) || !texture)
		return Noesis::Ptr<Noesis::Texture>();

	bool hasAlpha = format != Noesis::TextureFormat::RGBX8;

	WarNoesisD3D9Texture* result = new WarNoesisD3D9Texture(
		texture,
		width,
		height,
		numLevels,
		format,
		hasAlpha
	);

	if(data)
	{
		for(uint32_t i = 0; i < numLevels; ++i)
		{
			uint32_t mipWidth = std::max<uint32_t>(1, width >> i);
			uint32_t mipHeight = std::max<uint32_t>(1, height >> i);

			if(data[i])
				CopyToD3DTexture(result, i, 0, 0, mipWidth, mipHeight, data[i]);
		}
	}

	return Noesis::Ptr<Noesis::Texture>(*result);
}

void WarNoesisD3D9RenderDevice::UpdateTexture(
	Noesis::Texture* texture,
	uint32_t level,
	uint32_t x,
	uint32_t y,
	uint32_t width,
	uint32_t height,
	const void* data
)
{
	WarNoesisD3D9Texture* d3dTexture = Noesis::DynamicCast<WarNoesisD3D9Texture*>(texture);

	if(!d3dTexture || !data)
		return;

	CopyToD3DTexture(d3dTexture, level, x, y, width, height, data);
}

void WarNoesisD3D9RenderDevice::BeginOffscreenRender()
{
	CaptureState();
	SetDefaultStates();
}

void WarNoesisD3D9RenderDevice::EndOffscreenRender()
{
}

void WarNoesisD3D9RenderDevice::BeginOnscreenRender()
{
	CaptureState();
	SetDefaultStates();
}

void WarNoesisD3D9RenderDevice::EndOnscreenRender()
{
	Device->SetTexture(0, NULL);
	Device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	RestoreState();
}

void WarNoesisD3D9RenderDevice::SetRenderTarget(Noesis::RenderTarget* surface)
{
	WarNoesisD3D9RenderTarget* target = Noesis::DynamicCast<WarNoesisD3D9RenderTarget*>(surface);

	if(!target || !target->GetSurface())
		return;

	Device->SetRenderTarget(0, target->GetSurface());

	D3DVIEWPORT9 vp;
	vp.X = 0;
	vp.Y = 0;
	vp.Width = target->GetWidth();
	vp.Height = target->GetHeight();
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;

	Device->SetViewport(&vp);

	CurrentTargetHeight = target->GetHeight();
}

void WarNoesisD3D9RenderDevice::BeginTile(Noesis::RenderTarget* surface, const Noesis::Tile& tile)
{
	WarNoesisD3D9RenderTarget* target = Noesis::DynamicCast<WarNoesisD3D9RenderTarget*>(surface);

	uint32_t targetHeight = CurrentTargetHeight;

	if(target)
		targetHeight = target->GetHeight();

	RECT rc;
	rc.left = (LONG)tile.x;
	rc.right = (LONG)(tile.x + tile.width);
	rc.top = (LONG)(targetHeight - tile.y - tile.height);
	rc.bottom = (LONG)(targetHeight - tile.y);

	Device->SetScissorRect(&rc);
	Device->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
}

void WarNoesisD3D9RenderDevice::EndTile(Noesis::RenderTarget* surface)
{
	Device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
}

void WarNoesisD3D9RenderDevice::ResolveRenderTarget(
	Noesis::RenderTarget* surface,
	const Noesis::Tile* tiles,
	uint32_t numTiles
)
{
}

void* WarNoesisD3D9RenderDevice::MapVertices(uint32_t bytes)
{
	VertexBuffer.resize(bytes);
	return VertexBuffer.empty() ? NULL : &VertexBuffer[0];
}

void WarNoesisD3D9RenderDevice::UnmapVertices()
{
}

void* WarNoesisD3D9RenderDevice::MapIndices(uint32_t bytes)
{
	IndexBuffer.resize(bytes);
	return IndexBuffer.empty() ? NULL : &IndexBuffer[0];
}

void WarNoesisD3D9RenderDevice::UnmapIndices()
{
}

void WarNoesisD3D9RenderDevice::DrawBatch(const Noesis::Batch& batch)
{
	if(VertexBuffer.empty() || IndexBuffer.empty())
		return;

	if(batch.numIndices < 3 || batch.numVertices == 0)
		return;

	uint8_t vertexType = Noesis::VertexForShader[batch.shader.v];
	uint8_t format = Noesis::FormatForVertex[vertexType];
	uint8_t stride = Noesis::SizeForFormat[format];

	const unsigned char* vertexBase = &VertexBuffer[0] + batch.vertexOffset;
	const unsigned short* indices = (const unsigned short*)(&IndexBuffer[0]) + batch.startIndex;

	std::vector<ParsedVertex> parsed;
	parsed.resize(batch.numVertices);

	for(uint32_t i = 0; i < batch.numVertices; ++i)
	{
		parsed[i] = ParseVertex(vertexBase + i * stride, format);
	}

	DrawVertices.clear();
	DrawVertices.reserve(batch.numIndices);

	for(uint32_t i = 0; i < batch.numIndices; ++i)
	{
		uint16_t index = indices[i];

		if(index >= parsed.size())
			continue;

		const ParsedVertex& src = parsed[index];

		unsigned char a = (unsigned char)((src.color >> 24) & 0xFF);
		unsigned char r = (unsigned char)((src.color >> 16) & 0xFF);
		unsigned char g = (unsigned char)((src.color >> 8) & 0xFF);
		unsigned char b = (unsigned char)(src.color & 0xFF);

		float coverage = src.coverage;

		if(coverage < 0.0f)
			coverage = 0.0f;

		if(coverage > 1.0f)
			coverage = 1.0f;

		a = (unsigned char)((float)a * coverage);

		D3D9Vertex out;
		out.x = src.x - 0.5f;
		out.y = src.y - 0.5f;
		out.z = 0.0f;
		out.rhw = 1.0f;
		out.color = D3DCOLOR_ARGB(a, r, g, b);
		out.u = src.u;
		out.v = src.v;

		DrawVertices.push_back(out);
	}

	if(DrawVertices.size() < 3)
		return;

	ApplyRenderState(batch.renderState);

	Noesis::Texture* texture = SelectBatchTexture(batch);
	Noesis::SamplerState sampler = SelectBatchSampler(batch, texture);

	ApplySampler(0, texture, sampler);
	ApplyTexture(0, texture);

	Device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	Device->DrawPrimitiveUP(
		D3DPT_TRIANGLELIST,
		(UINT)(DrawVertices.size() / 3),
		&DrawVertices[0],
		sizeof(D3D9Vertex)
	);
}

void WarNoesisD3D9RenderDevice::CaptureState()
{
	if(SavedState)
		return;

	Device->CreateStateBlock(D3DSBT_ALL, &SavedState);

	if(SavedState)
		SavedState->Capture();
}

void WarNoesisD3D9RenderDevice::RestoreState()
{
	if(!SavedState)
		return;

	SavedState->Apply();
	SavedState->Release();
	SavedState = NULL;
}

void WarNoesisD3D9RenderDevice::SetDefaultStates()
{
	Device->SetRenderState(D3DRS_LIGHTING, FALSE);
	Device->SetRenderState(D3DRS_ZENABLE, FALSE);
	Device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	Device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	Device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
	Device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	Device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	Device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	Device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	Device->SetPixelShader(NULL);
	Device->SetVertexShader(NULL);
}

void WarNoesisD3D9RenderDevice::ApplyRenderState(const Noesis::RenderState& state)
{
	Device->SetRenderState(D3DRS_COLORWRITEENABLE, state.f.colorEnable ? 0x0000000F : 0);

	if(state.f.wireframe)
		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	else
		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	switch(state.f.blendMode)
	{
	case Noesis::BlendMode::Src:
		Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		break;

	case Noesis::BlendMode::SrcOver_Additive:
		Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		break;

	default:
		Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		break;
	}

	Device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
	Device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
}

void WarNoesisD3D9RenderDevice::ApplySampler(uint32_t stage, Noesis::Texture* texture, const Noesis::SamplerState& sampler)
{
	DWORD address = D3DTADDRESS_CLAMP;

	switch(sampler.f.wrapMode)
	{
	case Noesis::WrapMode::Repeat:
		address = D3DTADDRESS_WRAP;
		break;

	case Noesis::WrapMode::Mirror:
	case Noesis::WrapMode::MirrorU:
	case Noesis::WrapMode::MirrorV:
		address = D3DTADDRESS_MIRROR;
		break;

	case Noesis::WrapMode::ClampToZero:
		address = D3DTADDRESS_BORDER;
		Device->SetSamplerState(stage, D3DSAMP_BORDERCOLOR, D3DCOLOR_ARGB(0, 0, 0, 0));
		break;

	default:
		address = D3DTADDRESS_CLAMP;
		break;
	}

	Device->SetSamplerState(stage, D3DSAMP_ADDRESSU, address);
	Device->SetSamplerState(stage, D3DSAMP_ADDRESSV, address);

	DWORD minMag = sampler.f.minmagFilter == Noesis::MinMagFilter::Nearest ? D3DTEXF_POINT : D3DTEXF_LINEAR;

	Device->SetSamplerState(stage, D3DSAMP_MINFILTER, minMag);
	Device->SetSamplerState(stage, D3DSAMP_MAGFILTER, minMag);

	DWORD mip = D3DTEXF_NONE;

	if(sampler.f.mipFilter == Noesis::MipFilter::Nearest)
		mip = D3DTEXF_POINT;
	else if(sampler.f.mipFilter == Noesis::MipFilter::Linear)
		mip = D3DTEXF_LINEAR;

	Device->SetSamplerState(stage, D3DSAMP_MIPFILTER, mip);
}

void WarNoesisD3D9RenderDevice::ApplyTexture(uint32_t stage, Noesis::Texture* texture)
{
	WarNoesisD3D9Texture* d3dTexture = Noesis::DynamicCast<WarNoesisD3D9Texture*>(texture);

	if(d3dTexture && d3dTexture->GetD3DTexture())
	{
		Device->SetTexture(stage, d3dTexture->GetD3DTexture());

		Device->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
		Device->SetTextureStageState(stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		Device->SetTextureStageState(stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		Device->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		Device->SetTextureStageState(stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		Device->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	}
	else
	{
		Device->SetTexture(stage, NULL);

		Device->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		Device->SetTextureStageState(stage, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		Device->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		Device->SetTextureStageState(stage, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	}
}

void WarNoesisD3D9RenderDevice::CopyToD3DTexture(
	WarNoesisD3D9Texture* texture,
	uint32_t level,
	uint32_t x,
	uint32_t y,
	uint32_t width,
	uint32_t height,
	const void* data
)
{
	if(!texture || !texture->GetD3DTexture() || !data)
		return;

	RECT rect;
	rect.left = (LONG)x;
	rect.top = (LONG)y;
	rect.right = (LONG)(x + width);
	rect.bottom = (LONG)(y + height);

	D3DLOCKED_RECT locked;

	HRESULT hr = texture->GetD3DTexture()->LockRect(level, &locked, &rect, 0);

	if(FAILED(hr))
		return;

	unsigned char* dst = (unsigned char*)locked.pBits;
	const unsigned char* src = (const unsigned char*)data;

	if(texture->GetFormat() == Noesis::TextureFormat::RGBA8)
		ConvertCopyRGBA8(dst, locked.Pitch, src, width, height);
	else if(texture->GetFormat() == Noesis::TextureFormat::RGBX8)
		ConvertCopyRGBX8(dst, locked.Pitch, src, width, height);
	else
		ConvertCopyR8(dst, locked.Pitch, src, width, height);

	texture->GetD3DTexture()->UnlockRect(level);
}

void WarNoesisD3D9RenderDevice::ConvertCopyRGBA8(
	unsigned char* dst,
	int dstPitch,
	const unsigned char* src,
	uint32_t width,
	uint32_t height
)
{
	for(uint32_t y = 0; y < height; ++y)
	{
		unsigned char* d = dst + y * dstPitch;
		const unsigned char* s = src + y * width * 4;

		for(uint32_t x = 0; x < width; ++x)
		{
			unsigned char r = s[x * 4 + 0];
			unsigned char g = s[x * 4 + 1];
			unsigned char b = s[x * 4 + 2];
			unsigned char a = s[x * 4 + 3];

			d[x * 4 + 0] = b;
			d[x * 4 + 1] = g;
			d[x * 4 + 2] = r;
			d[x * 4 + 3] = a;
		}
	}
}

void WarNoesisD3D9RenderDevice::ConvertCopyRGBX8(
	unsigned char* dst,
	int dstPitch,
	const unsigned char* src,
	uint32_t width,
	uint32_t height
)
{
	for(uint32_t y = 0; y < height; ++y)
	{
		unsigned char* d = dst + y * dstPitch;
		const unsigned char* s = src + y * width * 4;

		for(uint32_t x = 0; x < width; ++x)
		{
			unsigned char r = s[x * 4 + 0];
			unsigned char g = s[x * 4 + 1];
			unsigned char b = s[x * 4 + 2];

			d[x * 4 + 0] = b;
			d[x * 4 + 1] = g;
			d[x * 4 + 2] = r;
			d[x * 4 + 3] = 255;
		}
	}
}

void WarNoesisD3D9RenderDevice::ConvertCopyR8(
	unsigned char* dst,
	int dstPitch,
	const unsigned char* src,
	uint32_t width,
	uint32_t height
)
{
	for(uint32_t y = 0; y < height; ++y)
	{
		unsigned char* d = dst + y * dstPitch;
		const unsigned char* s = src + y * width;

		for(uint32_t x = 0; x < width; ++x)
		{
			unsigned char a = s[x];

			d[x * 4 + 0] = 255;
			d[x * 4 + 1] = 255;
			d[x * 4 + 2] = 255;
			d[x * 4 + 3] = a;
		}
	}
}

WarNoesisD3D9RenderDevice::ParsedVertex WarNoesisD3D9RenderDevice::ParseVertex(
	const unsigned char* vertex,
	uint8_t format
)
{
	ParsedVertex out;
	out.x = 0.0f;
	out.y = 0.0f;
	out.color = D3DCOLOR_ARGB(255, 255, 255, 255);
	out.u = 0.0f;
	out.v = 0.0f;
	out.coverage = 1.0f;

	uint8_t attrs = Noesis::AttributesForFormat[format];

	uint32_t offset = 0;

	for(uint32_t attr = 0; attr < Noesis::Shader::Vertex::Format::Attr::Count; ++attr)
	{
		if((attrs & (1 << attr)) == 0)
			continue;

		uint8_t type = Noesis::TypeForAttr[attr];

		const unsigned char* p = vertex + offset;

		switch(attr)
		{
		case Noesis::Shader::Vertex::Format::Attr::Pos:
			{
				const float* f = (const float*)p;
				out.x = f[0];
				out.y = f[1];
			}
			break;

		case Noesis::Shader::Vertex::Format::Attr::Color:
			{
				unsigned char r = p[0];
				unsigned char g = p[1];
				unsigned char b = p[2];
				unsigned char a = p[3];

				out.color = D3DCOLOR_ARGB(a, r, g, b);
			}
			break;

		case Noesis::Shader::Vertex::Format::Attr::Tex0:
			{
				const float* f = (const float*)p;
				out.u = f[0];
				out.v = f[1];
			}
			break;

		case Noesis::Shader::Vertex::Format::Attr::Tex1:
			{
				const float* f = (const float*)p;
				out.u = f[0];
				out.v = f[1];
			}
			break;

		case Noesis::Shader::Vertex::Format::Attr::Coverage:
			{
				const float* f = (const float*)p;
				out.coverage = f[0];
			}
			break;

		default:
			break;
		}

		offset += Noesis::SizeForType[type];
	}

	return out;
}

Noesis::Texture* WarNoesisD3D9RenderDevice::SelectBatchTexture(const Noesis::Batch& batch)
{
	if(batch.image)
		return batch.image;

	if(batch.glyphs)
		return batch.glyphs;

	if(batch.pattern)
		return batch.pattern;

	if(batch.ramps)
		return batch.ramps;

	if(batch.shadow)
		return batch.shadow;

	return NULL;
}

Noesis::SamplerState WarNoesisD3D9RenderDevice::SelectBatchSampler(
	const Noesis::Batch& batch,
	Noesis::Texture* texture
)
{
	if(texture == batch.image)
		return batch.imageSampler;

	if(texture == batch.glyphs)
		return batch.glyphsSampler;

	if(texture == batch.pattern)
		return batch.patternSampler;

	if(texture == batch.ramps)
		return batch.rampsSampler;

	if(texture == batch.shadow)
		return batch.shadowSampler;

	Noesis::SamplerState state;
	state.v = 0;
	state.f.wrapMode = Noesis::WrapMode::ClampToEdge;
	state.f.minmagFilter = Noesis::MinMagFilter::Linear;
	state.f.mipFilter = Noesis::MipFilter::Disabled;
	return state;
}