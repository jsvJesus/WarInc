#pragma once

#include <windows.h>
#include <d3d9.h>
#include <stdint.h>
#include <vector>

#include <NsCore/Noesis.h>
#include <NsCore/Ptr.h>
#include <NsCore/DynamicCast.h>
#include <NsCore/ReflectionImplementEmpty.h>

#include <NsRender/RenderDevice.h>
#include <NsRender/Texture.h>
#include <NsRender/RenderTarget.h>

class WarNoesisD3D9Texture : public Noesis::Texture
{
public:
	WarNoesisD3D9Texture(
		IDirect3DTexture9* texture,
		uint32_t width,
		uint32_t height,
		uint32_t levels,
		Noesis::TextureFormat::Enum format,
		bool hasAlpha
	);

	~WarNoesisD3D9Texture();

	uint32_t GetWidth() const override;
	uint32_t GetHeight() const override;
	bool HasMipMaps() const override;
	bool IsInverted() const override;
	bool HasAlpha() const override;

	IDirect3DTexture9* GetD3DTexture() const;
	Noesis::TextureFormat::Enum GetFormat() const;

private:
	IDirect3DTexture9* Texture;
	uint32_t Width;
	uint32_t Height;
	uint32_t Levels;
	Noesis::TextureFormat::Enum Format;
	bool bHasAlpha;

	NS_IMPLEMENT_INLINE_REFLECTION_(WarNoesisD3D9Texture, Noesis::Texture)
};

class WarNoesisD3D9RenderTarget : public Noesis::RenderTarget
{
public:
	WarNoesisD3D9RenderTarget(
		IDirect3DDevice9* device,
		uint32_t width,
		uint32_t height
	);

	~WarNoesisD3D9RenderTarget();

	Noesis::Texture* GetTexture() override;
	IDirect3DSurface9* GetSurface() const;

	uint32_t GetWidth() const;
	uint32_t GetHeight() const;

private:
	Noesis::Ptr<WarNoesisD3D9Texture> TargetTexture;
	IDirect3DSurface9* Surface;
	uint32_t Width;
	uint32_t Height;

	NS_IMPLEMENT_INLINE_REFLECTION_(WarNoesisD3D9RenderTarget, Noesis::RenderTarget)
};

class WarNoesisD3D9RenderDevice : public Noesis::RenderDevice
{
public:
	WarNoesisD3D9RenderDevice(IDirect3DDevice9* device);
	~WarNoesisD3D9RenderDevice();

	const Noesis::DeviceCaps& GetCaps() const override;

	Noesis::Ptr<Noesis::RenderTarget> CreateRenderTarget(
		const char* label,
		uint32_t width,
		uint32_t height,
		uint32_t sampleCount,
		bool needsStencil
	) override;

	Noesis::Ptr<Noesis::RenderTarget> CloneRenderTarget(
		const char* label,
		Noesis::RenderTarget* surface
	) override;

	Noesis::Ptr<Noesis::Texture> CreateTexture(
		const char* label,
		uint32_t width,
		uint32_t height,
		uint32_t numLevels,
		Noesis::TextureFormat::Enum format,
		const void** data
	) override;

	void UpdateTexture(
		Noesis::Texture* texture,
		uint32_t level,
		uint32_t x,
		uint32_t y,
		uint32_t width,
		uint32_t height,
		const void* data
	) override;

	void BeginOffscreenRender() override;
	void EndOffscreenRender() override;

	void BeginOnscreenRender() override;
	void EndOnscreenRender() override;

	void SetRenderTarget(Noesis::RenderTarget* surface) override;

	void BeginTile(Noesis::RenderTarget* surface, const Noesis::Tile& tile) override;
	void EndTile(Noesis::RenderTarget* surface) override;

	void ResolveRenderTarget(Noesis::RenderTarget* surface, const Noesis::Tile* tiles, uint32_t numTiles) override;

	void* MapVertices(uint32_t bytes) override;
	void UnmapVertices() override;

	void* MapIndices(uint32_t bytes) override;
	void UnmapIndices() override;

	void DrawBatch(const Noesis::Batch& batch) override;

private:
	struct D3D9Vertex
	{
		float x;
		float y;
		float z;
		float rhw;
		DWORD color;
		float u;
		float v;
	};

	struct ParsedVertex
	{
		float x;
		float y;
		DWORD color;
		float u;
		float v;
		float coverage;
	};

private:
	void CaptureState();
	void RestoreState();

	void SetDefaultStates();
	void ApplyRenderState(const Noesis::RenderState& state);
	void ApplySampler(uint32_t stage, Noesis::Texture* texture, const Noesis::SamplerState& sampler);
	void ApplyTexture(uint32_t stage, Noesis::Texture* texture);

	void CopyToD3DTexture(
		WarNoesisD3D9Texture* texture,
		uint32_t level,
		uint32_t x,
		uint32_t y,
		uint32_t width,
		uint32_t height,
		const void* data
	);

	void ConvertCopyRGBA8(unsigned char* dst, int dstPitch, const unsigned char* src, uint32_t width, uint32_t height);
	void ConvertCopyRGBX8(unsigned char* dst, int dstPitch, const unsigned char* src, uint32_t width, uint32_t height);
	void ConvertCopyR8(unsigned char* dst, int dstPitch, const unsigned char* src, uint32_t width, uint32_t height);

	ParsedVertex ParseVertex(const unsigned char* vertex, uint8_t format);
	Noesis::Texture* SelectBatchTexture(const Noesis::Batch& batch);
	Noesis::SamplerState SelectBatchSampler(const Noesis::Batch& batch, Noesis::Texture* texture);

private:
	IDirect3DDevice9* Device;
	IDirect3DStateBlock9* SavedState;

	Noesis::DeviceCaps Caps;

	std::vector<unsigned char> VertexBuffer;
	std::vector<unsigned char> IndexBuffer;
	std::vector<D3D9Vertex> DrawVertices;

	uint32_t CurrentTargetHeight;

	NS_IMPLEMENT_INLINE_REFLECTION_(WarNoesisD3D9RenderDevice, Noesis::RenderDevice)
};