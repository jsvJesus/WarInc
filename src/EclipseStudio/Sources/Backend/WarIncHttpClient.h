#pragma once

#include "CkHttpRequest.h"
#include "CkHttp.h"
#include "CkHttpResponse.h"
#include "CkByteData.h"

class CWarIncHttpResponse
{
public:
    int StatusCode;
    int ErrorCode;
    std::string Body;

public:
    CWarIncHttpResponse()
    {
        StatusCode = 0;
        ErrorCode = 0;
        Body.clear();
    }

    bool IsOk() const
    {
        return ErrorCode == 0 && StatusCode == 200;
    }
};

class CWarIncHttpClient
{
private:
    CkHttp http;

private:
    bool InitHttp();
    void BuildPath(const char* url, char* outPath, int outPathSize);
    bool ParseResponse(const char* debugName, CkHttpResponse* resp, CWarIncHttpResponse& outResponse);

public:
    CWarIncHttpClient();
    ~CWarIncHttpClient();

    bool Get(const char* url, CWarIncHttpResponse& outResponse);
    bool PostForm(const char* url, CkHttpRequest& req, CWarIncHttpResponse& outResponse);
    bool PostJson(const char* url, const char* jsonBody, CWarIncHttpResponse& outResponse);
};