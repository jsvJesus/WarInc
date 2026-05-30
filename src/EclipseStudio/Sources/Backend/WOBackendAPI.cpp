#include "r3dPCH.h"
#include "r3d.h"
#include "CkGzip.h"

#include "GameCode/UserProfile.h"
#include "WOBackendAPI.h"
#include "WarIncBackendConfig.h"

const char* gDomainBaseUrl = "/";
int gDomainPort = 8080;
bool gDomainUseSSL = false;

static bool gBackendConfigLoaded = false;

static void EnsureWarIncBackendConfig()
{
	if(gBackendConfigLoaded)
		return;

	gWarIncBackendConfig.LoadFromFile("Data/Config/backend.ini");

	gDomainBaseUrl = gWarIncBackendConfig.GetBaseUrl();
	gDomainPort = gWarIncBackendConfig.GetPort();
	gDomainUseSSL = gWarIncBackendConfig.IsSSL();

	gBackendConfigLoaded = true;
}

static const char* GetWarIncBackendHost()
{
	EnsureWarIncBackendConfig();
	return gWarIncBackendConfig.GetHost();
}

CWOBackendReq::CWOBackendReq(const char* url)
{
	Init(url);
}

CWOBackendReq::CWOBackendReq(const CUserProfile* prof, const char* url)
{
	Init(url);
	AddSessionInfo(prof->CustomerID, prof->SessionID);
}

void CWOBackendReq::Init(const char* url)
{
	resp_ = NULL;
	EnsureWarIncBackendConfig();

	savedUrl_ = url;
	resultCode_ = 0;
	bodyStr_ = "";
	bodyLen_ = 0;

	int success = http.UnlockComponent("ARKTOSHttp_decCLPWFQXmU");
	if(success != 1)
	{
		r3dError("failed to unlock CkHttp");
		return;
	}

	http.put_ConnectTimeout(10);
	http.put_ReadTimeout(30);

	char fullUrl[512];

	if(url[0] != '/')
		sprintf(fullUrl, "%s%s", gDomainBaseUrl, url);
	else
		sprintf(fullUrl, "%s", url);

	req.UsePost();
	req.put_Path(fullUrl);
	req.put_Utf8(true);

	r3dOutToLog("WOApi init: host=%s port=%d path=%s ssl=%d\n",
		GetWarIncBackendHost(),
		gDomainPort,
		fullUrl,
		gDomainUseSSL ? 1 : 0);
}

CWOBackendReq::~CWOBackendReq()
{
	SAFE_DELETE(resp_);
}

void CWOBackendReq::AddSessionInfo(DWORD id, DWORD sid)
{
	r3d_assert(id);

	AddParam("s_id", (int)id);
	AddParam("s_key", (int)sid);
}

void CWOBackendReq::AddParam(const char* name, const char* val)
{
	req.AddParam(name, val);
}

void CWOBackendReq::AddParam(const char* name, int val)
{
	char buf[1024];
	sprintf(buf, "%d", val);
	AddParam(name, buf);
}

int CWOBackendReq::ParseResult(CkHttpResponse* resp)
{
	if(resp == NULL)
	{
		r3dOutToLog("WO_API: http timeout\n");
		return 8;
	}

	if(resp->get_StatusCode() != 200)
	{
		r3dOutToLog("WO_API: returned http%d\n", resp->get_StatusCode());
		return 8;
	}

	resp->get_Body(data_);
	data_.appendChar(0);

	if(data_.getSize() >= 2 && data_.getByte(0) == '$' && data_.getByte(1) == '1')
	{
		CkGzip gzip;

		if(gzip.UnlockComponent("ARKTOSZIP_cRvE6e7mpSqD") == false)
			r3dError("failed to unlock gzip");

		CkByteData udata;
		data_.removeChunk(0, 2);

		if(!gzip.UncompressMemory(data_, udata))
		{
			r3dOutToLog("WO_API: decompress failed\n");
			return 9;
		}

		data_ = udata;
		data_.appendChar(0);
	}

	bodyStr_ = (const char*)data_.getData();
	bodyLen_ = data_.getSize() - 1;

	if(bodyLen_ <= 0)
	{
		r3dOutToLog("WO_API: empty answer\n");
		return 9;
	}

	if(bodyStr_[0] == '{' || bodyStr_[0] == '[')
	{
		r3dOutToLog("WO_API: json answer: %s\n", bodyStr_);
		return 0;
	}

	if(bodyLen_ >= 3 && bodyStr_[0] == '<' && bodyStr_[1] == '?' && bodyStr_[2] == 'x')
	{
		return 0;
	}

	if(bodyLen_ < 4)
	{
		r3dOutToLog("WO_API: too small answer: %s\n", bodyStr_);
		return 9;
	}

	if(bodyStr_[0] != 'W' || bodyStr_[1] != 'O' || bodyStr_[2] != '_')
	{
		r3dOutToLog("WO_API: wrong header: %s\n", bodyStr_);
		return 9;
	}

	int resultCode = bodyStr_[3] - '0';

	bodyStr_ += 4;
	bodyLen_ -= 4;

	switch(resultCode)
	{
	case 0:
		break;

	case 1:
		r3dOutToLog("WO_API: session disconnected\n");
		break;

	default:
		r3dOutToLog("WO_API: failed with error code %d %s\n", resultCode, bodyStr_);
		break;
	}

	return resultCode;
}

bool CWOBackendReq::Issue()
{
	SAFE_DELETE(resp_);

	const char* host = GetWarIncBackendHost();

	float t1 = r3dGetTime();

	resp_ = http.SynchronousRequest(
		host,
		gDomainPort,
		gDomainUseSSL,
		req);

#ifndef FINAL_BUILD
	r3dOutToLog("WOApi: %s NETWORK time: %.4f\n", savedUrl_, r3dGetTime() - t1);
#endif

	resultCode_ = ParseResult(resp_);
	return resultCode_ == 0;
}

void CWOBackendReq::ParseXML(pugi::xml_document& xmlFile)
{
	pugi::xml_parse_result parseResult = xmlFile.load_buffer_inplace((void*)bodyStr_, bodyLen_);

	if(!parseResult)
		r3dError("Failed to parse server XML, error: %s", parseResult.description());
}