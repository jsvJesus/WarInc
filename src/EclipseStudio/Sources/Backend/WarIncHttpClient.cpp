#include "r3dPCH.h"
#include "r3d.h"

#include "WarIncBackendConfig.h"
#include "WarIncHttpClient.h"

CWarIncHttpClient::CWarIncHttpClient()
{
	InitHttp();
}

CWarIncHttpClient::~CWarIncHttpClient()
{
}

bool CWarIncHttpClient::InitHttp()
{
	int success = http.UnlockComponent("ARKTOSHttp_decCLPWFQXmU");
	if(success != 1)
	{
		r3dOutToLog("WarIncHttpClient: failed to unlock CkHttp\n");
		return false;
	}

	http.put_ConnectTimeout(10);
	http.put_ReadTimeout(30);
	http.put_FollowRedirects(true);
	http.put_UserAgent("WarIncClient/1.0");

	return true;
}

void CWarIncHttpClient::BuildPath(const char* url, char* outPath, int outPathSize)
{
	const char* baseUrl = gWarIncBackendConfig.GetBaseUrl();

	if(!url || !url[0])
	{
		r3dscpy(outPath, "/");
		return;
	}

	if(url[0] == '/')
	{
		sprintf(outPath, "%s", url);
		return;
	}

	if(!baseUrl || !baseUrl[0])
	{
		sprintf(outPath, "/%s", url);
		return;
	}

	if(baseUrl[strlen(baseUrl) - 1] == '/')
		sprintf(outPath, "%s%s", baseUrl, url);
	else
		sprintf(outPath, "%s/%s", baseUrl, url);

	outPath[outPathSize - 1] = 0;
}

bool CWarIncHttpClient::ParseResponse(const char* debugName, CkHttpResponse* resp, CWarIncHttpResponse& outResponse)
{
	if(!resp)
	{
		outResponse.ErrorCode = 8;
		outResponse.StatusCode = 0;
		outResponse.Body.clear();

		r3dOutToLog("WarIncHttpClient: %s timeout/no response\n", debugName);
		return false;
	}

	outResponse.StatusCode = resp->get_StatusCode();

	CkByteData data;
	resp->get_Body(data);
	data.appendChar(0);

	if(data.getSize() > 0)
		outResponse.Body = (const char*)data.getData();
	else
		outResponse.Body.clear();

	if(outResponse.StatusCode != 200)
	{
		outResponse.ErrorCode = 8;

		r3dOutToLog("WarIncHttpClient: %s http status=%d body=%s\n",
			debugName,
			outResponse.StatusCode,
			outResponse.Body.c_str());

		return false;
	}

	outResponse.ErrorCode = 0;

#ifndef FINAL_BUILD
	r3dOutToLog("WarIncHttpClient: %s response=%s\n", debugName, outResponse.Body.c_str());
#endif

	return true;
}

bool CWarIncHttpClient::Get(const char* url, CWarIncHttpResponse& outResponse)
{
	char path[1024];
	BuildPath(url, path, sizeof(path));

	CkHttpRequest req;
	req.UseGet();
	req.put_Path(path);
	req.put_Utf8(true);

	float t1 = r3dGetTime();

	CkHttpResponse* resp = http.SynchronousRequest(
		gWarIncBackendConfig.GetHost(),
		gWarIncBackendConfig.GetPort(),
		gWarIncBackendConfig.IsSSL(),
		req);

#ifndef FINAL_BUILD
	r3dOutToLog("WarIncHttpClient GET %s time %.4f\n", path, r3dGetTime() - t1);
#endif

	bool ok = ParseResponse(path, resp, outResponse);
	SAFE_DELETE(resp);

	return ok;
}

bool CWarIncHttpClient::PostForm(const char* url, CkHttpRequest& req, CWarIncHttpResponse& outResponse)
{
	char path[1024];
	BuildPath(url, path, sizeof(path));

	req.UsePost();
	req.put_Path(path);
	req.put_Utf8(true);

	float t1 = r3dGetTime();

	CkHttpResponse* resp = http.SynchronousRequest(
		gWarIncBackendConfig.GetHost(),
		gWarIncBackendConfig.GetPort(),
		gWarIncBackendConfig.IsSSL(),
		req);

#ifndef FINAL_BUILD
	r3dOutToLog("WarIncHttpClient POST FORM %s time %.4f\n", path, r3dGetTime() - t1);
#endif

	bool ok = ParseResponse(path, resp, outResponse);
	SAFE_DELETE(resp);

	return ok;
}

bool CWarIncHttpClient::PostJson(const char* url, const char* jsonBody, CWarIncHttpResponse& outResponse)
{
	char path[1024];
	BuildPath(url, path, sizeof(path));

	CkHttpRequest req;
	req.UsePost();
	req.put_Path(path);
	req.put_Utf8(true);
	req.AddHeader("Content-Type", "application/json");
	req.AddHeader("Accept", "application/json");
	req.LoadBodyFromString(jsonBody ? jsonBody : "", "utf-8");

	float t1 = r3dGetTime();

	CkHttpResponse* resp = http.SynchronousRequest(
		gWarIncBackendConfig.GetHost(),
		gWarIncBackendConfig.GetPort(),
		gWarIncBackendConfig.IsSSL(),
		req);

#ifndef FINAL_BUILD
	r3dOutToLog("WarIncHttpClient POST JSON %s body=%s time %.4f\n",
		path,
		jsonBody ? jsonBody : "",
		r3dGetTime() - t1);
#endif

	bool ok = ParseResponse(path, resp, outResponse);
	SAFE_DELETE(resp);

	return ok;
}