#include "WarNoesisMediaPlayer.h"

#include <windows.h>
#include <stdint.h>
#include <string>
#include <vector>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>

#include <NsCore/DynamicCast.h>
#include <NsCore/BaseComponent.h>
#include <NsCore/ReflectionImplementEmpty.h>
#include <NsGui/Uri.h>
#include <NsGui/DynamicTextureSource.h>
#include <NsRender/RenderDevice.h>
#include <NsRender/Texture.h>
#include <NsApp/MediaElement.h>
#include <NsApp/MediaPlayer.h>

static char gWarNoesisMediaRoot[MAX_PATH] = "";
static bool gWarNoesisMediaStarted = false;
static bool gWarNoesisComStarted = false;

static void WarMediaLog(const char* text)
{
	OutputDebugStringA(text);
	OutputDebugStringA("\n");
}

template<class T>
static void WarSafeRelease(T*& value)
{
	if(value)
	{
		value->Release();
		value = NULL;
	}
}

static bool WarFileExistsA(const char* path)
{
	if(!path || !path[0])
		return false;

	DWORD attr = GetFileAttributesA(path);

	return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

static bool WarIsAbsolutePath(const char* path)
{
	if(!path || !path[0])
		return false;

	if(path[0] == '\\' || path[0] == '/')
		return true;

	if(path[1] == ':')
		return true;

	return false;
}

static void WarNormalizeSlashes(char* text)
{
	if(!text)
		return;

	for(char* p = text; *p; ++p)
	{
		if(*p == '/')
			*p = '\\';
	}
}

static std::string WarBuildMediaPath(const Noesis::Uri& uri)
{
	const char* raw = uri.Str();

	if(!raw)
		raw = "";

	std::string src = raw;

	if(src.find("file:///") == 0)
		src = src.substr(8);

	if(src.find("pack://application:,,,/") == 0)
		src = src.substr(strlen("pack://application:,,,/"));

	while(!src.empty() && (src[0] == '/' || src[0] == '\\'))
		src.erase(src.begin());

	size_t component = src.find(";component/");
	if(component != std::string::npos)
		src = src.substr(component + strlen(";component/"));

	char candidate[MAX_PATH];

	if(WarIsAbsolutePath(src.c_str()))
	{
		_snprintf(candidate, sizeof(candidate) - 1, "%s", src.c_str());
		candidate[sizeof(candidate) - 1] = 0;
		WarNormalizeSlashes(candidate);
		return candidate;
	}

	_snprintf(candidate, sizeof(candidate) - 1, "%s\\%s", gWarNoesisMediaRoot, src.c_str());
	candidate[sizeof(candidate) - 1] = 0;
	WarNormalizeSlashes(candidate);

	if(WarFileExistsA(candidate))
		return candidate;

	_snprintf(candidate, sizeof(candidate) - 1, "%s\\Studio\\%s", gWarNoesisMediaRoot, src.c_str());
	candidate[sizeof(candidate) - 1] = 0;
	WarNormalizeSlashes(candidate);

	if(WarFileExistsA(candidate))
		return candidate;

	_snprintf(candidate, sizeof(candidate) - 1, "%s", src.c_str());
	candidate[sizeof(candidate) - 1] = 0;
	WarNormalizeSlashes(candidate);

	return candidate;
}

static std::wstring WarUtf8ToWide(const char* text)
{
	if(!text)
		text = "";

	int count = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);

	if(count <= 0)
		count = MultiByteToWideChar(CP_ACP, 0, text, -1, NULL, 0);

	if(count <= 0)
		return L"";

	std::wstring result;
	result.resize((size_t)count - 1);

	if(!result.empty())
	{
		if(MultiByteToWideChar(CP_UTF8, 0, text, -1, &result[0], count) <= 0)
			MultiByteToWideChar(CP_ACP, 0, text, -1, &result[0], count);
	}

	return result;
}

class WarNoesisMFMediaPlayer : public NoesisApp::MediaPlayer
{
public:
	WarNoesisMFMediaPlayer(NoesisApp::MediaElement* owner, const Noesis::Uri& uri)
	{
		Owner = owner;
		Path = WarBuildMediaPath(uri);
		Open();
	}

	~WarNoesisMFMediaPlayer()
	{
		Close();
	}

	Noesis::ImageSource* GetTextureSource() const override
	{
		return TextureSource.GetPtr();
	}

	uint32_t GetWidth() const override
	{
		return Width;
	}

	uint32_t GetHeight() const override
	{
		return Height;
	}

	bool CanPause() const override
	{
		return true;
	}

	bool HasAudio() const override
	{
		return false;
	}

	bool HasVideo() const override
	{
		return Width > 0 && Height > 0;
	}

	float GetBufferingProgress() const override
	{
		return 1.0f;
	}

	float GetDownloadProgress() const override
	{
		return 1.0f;
	}

	double GetDuration() const override
	{
		return Duration;
	}

	double GetPosition() const override
	{
		if(!Playing)
			return PausedPosition;

		double now = (double)(GetTickCount64() - StartTick) / 1000.0;
		return PausedPosition + now * (double)SpeedRatio;
	}

	void SetPosition(double position) override
	{
		Seek(position);
	}

	float GetSpeedRatio() const override
	{
		return SpeedRatio;
	}

	void SetSpeedRatio(float speedRatio) override
	{
		if(speedRatio <= 0.0f)
			speedRatio = 1.0f;

		PausedPosition = GetPosition();
		SpeedRatio = speedRatio;
		StartTick = GetTickCount64();
	}

	float GetVolume() const override
	{
		return Volume;
	}

	void SetVolume(float volume) override
	{
		if(volume < 0.0f)
			volume = 0.0f;

		if(volume > 1.0f)
			volume = 1.0f;

		Volume = volume;
	}

	float GetBalance() const override
	{
		return Balance;
	}

	void SetBalance(float balance) override
	{
		Balance = balance;
	}

	bool GetIsMuted() const override
	{
		return Muted;
	}

	void SetIsMuted(bool muted) override
	{
		Muted = muted;
	}

	bool GetScrubbingEnabled() const override
	{
		return Scrubbing;
	}

	void SetScrubbingEnabled(bool scrubbing) override
	{
		Scrubbing = scrubbing;
	}

	void Play() override
	{
		if(!Reader)
			return;

		if(!Playing)
		{
			Playing = true;
			StartTick = GetTickCount64();
		}
	}

	void Pause() override
	{
		if(Playing)
		{
			PausedPosition = GetPosition();
			Playing = false;
		}
	}

	void Stop() override
	{
		Seek(0.0);
		Playing = false;
		PausedPosition = 0.0;
	}

private:
	static Noesis::Texture* TextureRenderCallback(Noesis::RenderDevice* device, void* user)
	{
		WarNoesisMFMediaPlayer* self = (WarNoesisMFMediaPlayer*)user;

		if(!self)
			return NULL;

		return self->TextureRender(device);
	}

	Noesis::Texture* TextureRender(Noesis::RenderDevice* device)
	{
		if(!device || Width == 0 || Height == 0 || FrameRGBA.empty())
			return NULL;

		if(Playing)
		{
			ULONGLONG now = GetTickCount64();

			if(ForceFrame || now - LastFrameTick >= FrameDelayMs)
			{
				ReadFrame();
				LastFrameTick = now;
				ForceFrame = false;
			}
		}
		else if(Scrubbing && ForceFrame)
		{
			ReadFrame();
			ForceFrame = false;
		}

		if(!Texture)
		{
			const void* levels[1];
			levels[0] = &FrameRGBA[0];

			Texture = device->CreateTexture(
				"WarNoesisVideoFrame",
				Width,
				Height,
				1,
				Noesis::TextureFormat::RGBA8,
				levels
			);

			DirtyFrame = false;
		}
		else if(DirtyFrame)
		{
			device->UpdateTexture(
				Texture.GetPtr(),
				0,
				0,
				0,
				Width,
				Height,
				&FrameRGBA[0]
			);

			DirtyFrame = false;
		}

		return Texture.GetPtr();
	}

	void Open()
	{
		Close();

		std::wstring widePath = WarUtf8ToWide(Path.c_str());

		if(widePath.empty())
		{
			mMediaFailed("Invalid media path");
			return;
		}

		IMFAttributes* attributes = NULL;

		HRESULT hr = MFCreateAttributes(&attributes, 2);

		if(SUCCEEDED(hr))
			attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);

		hr = MFCreateSourceReaderFromURL(widePath.c_str(), attributes, &Reader);

		WarSafeRelease(attributes);

		if(FAILED(hr) || !Reader)
		{
			char buffer[1024];
			_snprintf(buffer, sizeof(buffer) - 1, "Media open failed: %s", Path.c_str());
			buffer[sizeof(buffer) - 1] = 0;

			WarMediaLog(buffer);
			mMediaFailed("Media open failed");
			return;
		}

		SourceHasAlpha = false;

		IMFMediaType* type = NULL;

		hr = MFCreateMediaType(&type);

		if(SUCCEEDED(hr))
		{
			type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
			type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32);

			HRESULT setHr = Reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, type);

			if(SUCCEEDED(setHr))
			{
				SourceHasAlpha = true;
			}
			else
			{
				WarSafeRelease(type);

				hr = MFCreateMediaType(&type);

				if(SUCCEEDED(hr))
				{
					type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
					type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
					Reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, type);
				}

				SourceHasAlpha = false;
			}
		}

		WarSafeRelease(type);

		Reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
		Reader->SetStreamSelection(MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE);

		IMFMediaType* currentType = NULL;

		hr = Reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &currentType);

		if(FAILED(hr) || !currentType)
		{
			mMediaFailed("Media type failed");
			return;
		}

		UINT32 w = 0;
		UINT32 h = 0;

		MFGetAttributeSize(currentType, MF_MT_FRAME_SIZE, &w, &h);

		Width = w;
		Height = h;

		UINT32 frameRateNum = 30;
		UINT32 frameRateDen = 1;

		if(SUCCEEDED(MFGetAttributeRatio(currentType, MF_MT_FRAME_RATE, &frameRateNum, &frameRateDen)))
		{
			if(frameRateNum > 0)
			{
				FrameDelayMs = (DWORD)((1000ULL * frameRateDen) / frameRateNum);

				if(FrameDelayMs < 1)
					FrameDelayMs = 1;
			}
		}

		WarSafeRelease(currentType);

		PROPVARIANT durationValue;
		PropVariantInit(&durationValue);

		if(SUCCEEDED(Reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &durationValue)))
		{
			if(durationValue.vt == VT_UI8 || durationValue.vt == VT_I8)
				Duration = (double)durationValue.hVal.QuadPart / 10000000.0;
		}

		PropVariantClear(&durationValue);

		if(Width == 0 || Height == 0)
		{
			mMediaFailed("Invalid video size");
			return;
		}

		FrameRGBA.resize((size_t)Width * (size_t)Height * 4);
		memset(&FrameRGBA[0], 0, FrameRGBA.size());

		TextureSource = *new Noesis::DynamicTextureSource(
			Width,
			Height,
			&WarNoesisMFMediaPlayer::TextureRenderCallback,
			this
		);

		ReadFrame();

		PausedPosition = 0.0;
		StartTick = GetTickCount64();
		LastFrameTick = 0;
		Playing = true;

		mMediaOpened();
	}

	void Close()
	{
		Texture.Reset();
		TextureSource.Reset();

		WarSafeRelease(Reader);

		FrameRGBA.clear();

		Width = 0;
		Height = 0;
		Duration = 0.0;
		PausedPosition = 0.0;
		Playing = false;
		DirtyFrame = false;
		ForceFrame = true;
		SourceHasAlpha = false;
	}

	void Seek(double position)
	{
		if(!Reader)
			return;

		if(position < 0.0)
			position = 0.0;

		PROPVARIANT value;
		PropVariantInit(&value);

		value.vt = VT_I8;
		value.hVal.QuadPart = (LONGLONG)(position * 10000000.0);

		Reader->SetCurrentPosition(GUID_NULL, value);

		PropVariantClear(&value);

		PausedPosition = position;
		StartTick = GetTickCount64();
		ForceFrame = true;
		DirtyFrame = true;
	}

	void Restart()
	{
		Seek(0.0);
		Playing = true;
		mMediaEnded();
	}

	bool ReadFrame()
	{
		if(!Reader || Width == 0 || Height == 0)
			return false;

		DWORD streamIndex = 0;
		DWORD flags = 0;
		LONGLONG timestamp = 0;
		IMFSample* sample = NULL;

		HRESULT hr = Reader->ReadSample(
			MF_SOURCE_READER_FIRST_VIDEO_STREAM,
			0,
			&streamIndex,
			&flags,
			&timestamp,
			&sample
		);

		if(FAILED(hr))
			return false;

		if(flags & MF_SOURCE_READERF_ENDOFSTREAM)
		{
			WarSafeRelease(sample);
			Restart();
			return false;
		}

		if(!sample)
			return false;

		IMFMediaBuffer* buffer = NULL;

		hr = sample->ConvertToContiguousBuffer(&buffer);

		if(FAILED(hr) || !buffer)
		{
			WarSafeRelease(sample);
			return false;
		}

		BYTE* src = NULL;
		DWORD maxLen = 0;
		DWORD curLen = 0;

		hr = buffer->Lock(&src, &maxLen, &curLen);

		if(SUCCEEDED(hr) && src && !FrameRGBA.empty())
		{
			const uint32_t pixelCount = Width * Height;
			const uint32_t needed = pixelCount * 4;

			if(curLen >= needed)
			{
				unsigned char* dst = &FrameRGBA[0];

				for(uint32_t i = 0; i < pixelCount; ++i)
				{
					unsigned char b = src[i * 4 + 0];
					unsigned char g = src[i * 4 + 1];
					unsigned char r = src[i * 4 + 2];
					unsigned char a = SourceHasAlpha ? src[i * 4 + 3] : 255;

					dst[i * 4 + 0] = r;
					dst[i * 4 + 1] = g;
					dst[i * 4 + 2] = b;
					dst[i * 4 + 3] = a;
				}

				DirtyFrame = true;
			}

			buffer->Unlock();
		}

		WarSafeRelease(buffer);
		WarSafeRelease(sample);

		return true;
	}

private:
	NoesisApp::MediaElement* Owner = NULL;
	IMFSourceReader* Reader = NULL;

	std::string Path;

	Noesis::Ptr<Noesis::DynamicTextureSource> TextureSource;
	Noesis::Ptr<Noesis::Texture> Texture;

	std::vector<unsigned char> FrameRGBA;

	uint32_t Width = 0;
	uint32_t Height = 0;

	double Duration = 0.0;
	double PausedPosition = 0.0;

	float SpeedRatio = 1.0f;
	float Volume = 0.5f;
	float Balance = 0.0f;

	bool Muted = false;
	bool Scrubbing = false;
	bool Playing = false;
	bool DirtyFrame = false;
	bool ForceFrame = true;
	bool SourceHasAlpha = false;

	DWORD FrameDelayMs = 33;
	ULONGLONG StartTick = 0;
	ULONGLONG LastFrameTick = 0;

	NS_IMPLEMENT_INLINE_REFLECTION_(WarNoesisMFMediaPlayer, Noesis::BaseComponent)
};

void WarNoesisMediaPlayer_Init(const char* rootPath)
{
	gWarNoesisMediaRoot[0] = 0;

	if(rootPath && rootPath[0])
	{
		_snprintf(gWarNoesisMediaRoot, sizeof(gWarNoesisMediaRoot) - 1, "%s", rootPath);
		gWarNoesisMediaRoot[sizeof(gWarNoesisMediaRoot) - 1] = 0;
	}

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if(SUCCEEDED(hr))
		gWarNoesisComStarted = true;

	hr = MFStartup(MF_VERSION, MFSTARTUP_LITE);

	if(SUCCEEDED(hr))
		gWarNoesisMediaStarted = true;

	NoesisApp::MediaElement::SetCreateMediaPlayerCallback(&WarNoesisMediaPlayer_Create, NULL);
}

void WarNoesisMediaPlayer_Shutdown()
{
	NoesisApp::MediaElement::SetCreateMediaPlayerCallback(NULL, NULL);

	if(gWarNoesisMediaStarted)
	{
		MFShutdown();
		gWarNoesisMediaStarted = false;
	}

	if(gWarNoesisComStarted)
	{
		CoUninitialize();
		gWarNoesisComStarted = false;
	}
}

Noesis::Ptr<NoesisApp::MediaPlayer> WarNoesisMediaPlayer_Create(
	NoesisApp::MediaElement* owner,
	const Noesis::Uri& uri,
	void* user
)
{
	return Noesis::Ptr<NoesisApp::MediaPlayer>(*new WarNoesisMFMediaPlayer(owner, uri));
}