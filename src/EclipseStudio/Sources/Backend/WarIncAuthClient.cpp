#include "r3dPCH.h"
#include "r3d.h"

#include "WarIncHttpClient.h"
#include "WarIncJson.h"
#include "WarIncAuthClient.h"

bool CWarIncAuthClient::Login(const char* username, const char* password, CWarIncAuthLoginResult& outResult)
{
	outResult.Reset();

	std::string safeUser = CWarIncJson::EscapeString(username);
	std::string safePass = CWarIncJson::EscapeString(password);

	char jsonBody[2048];
	sprintf(jsonBody,
		"{\"username\":\"%s\",\"password\":\"%s\"}",
		safeUser.c_str(),
		safePass.c_str());

	CWarIncHttpClient http;
	CWarIncHttpResponse response;

	if(!http.PostJson("/v1/auth/login", jsonBody, response))
	{
		outResult.Code = response.ErrorCode;
		r3dOutToLog("WarIncAuthClient::Login failed http=%d err=%d\n", response.StatusCode, response.ErrorCode);
		return false;
	}

	const char* json = response.Body.c_str();

	outResult.Ok = CWarIncJson::GetBool(json, "ok", false);
	outResult.Code = CWarIncJson::GetInt(json, "code", 0);
	CWarIncJson::GetString(json, "message", outResult.Message, sizeof(outResult.Message));

	outResult.CustomerID = CWarIncJson::GetUInt(json, "customerId", 0);
	outResult.SessionID = CWarIncJson::GetUInt(json, "sessionId", 0);
	outResult.AccountStatus = CWarIncJson::GetInt(json, "accountStatus", 0);
	CWarIncJson::GetString(json, "token", outResult.Token, sizeof(outResult.Token));

	if(!outResult.Ok)
	{
		r3dOutToLog("WarIncAuthClient::Login backend rejected code=%d msg=%s\n",
			outResult.Code,
			outResult.Message);
		return false;
	}

	if(outResult.CustomerID == 0 || outResult.SessionID == 0)
	{
		r3dOutToLog("WarIncAuthClient::Login bad json: missing customerId/sessionId\n");
		return false;
	}

	return true;
}

bool CWarIncAuthClient::Check(DWORD customerId, DWORD sessionId, const char* token)
{
	char jsonBody[2048];
	sprintf(jsonBody,
		"{\"customerId\":%u,\"sessionId\":%u,\"token\":\"%s\"}",
		(unsigned int)customerId,
		(unsigned int)sessionId,
		CWarIncJson::EscapeString(token).c_str());

	CWarIncHttpClient http;
	CWarIncHttpResponse response;

	if(!http.PostJson("/v1/auth/check", jsonBody, response))
		return false;

	const char* json = response.Body.c_str();

	bool ok = CWarIncJson::GetBool(json, "ok", false);
	int code = CWarIncJson::GetInt(json, "code", 0);

	if(!ok)
	{
		r3dOutToLog("WarIncAuthClient::Check failed code=%d body=%s\n", code, json);
		return false;
	}

	return true;
}

bool CWarIncAuthClient::Logout(DWORD customerId, DWORD sessionId, const char* token)
{
	char jsonBody[2048];
	sprintf(jsonBody,
		"{\"customerId\":%u,\"sessionId\":%u,\"token\":\"%s\"}",
		(unsigned int)customerId,
		(unsigned int)sessionId,
		CWarIncJson::EscapeString(token).c_str());

	CWarIncHttpClient http;
	CWarIncHttpResponse response;

	if(!http.PostJson("/v1/auth/logout", jsonBody, response))
		return false;

	const char* json = response.Body.c_str();
	return CWarIncJson::GetBool(json, "ok", false);
}