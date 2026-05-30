#pragma once

class CWarIncAuthLoginResult
{
public:
    bool Ok;
    int Code;
    char Message[256];

    DWORD CustomerID;
    DWORD SessionID;
    int AccountStatus;
    char Token[512];

public:
    CWarIncAuthLoginResult()
    {
        Reset();
    }

    void Reset()
    {
        Ok = false;
        Code = 0;
        Message[0] = 0;
        CustomerID = 0;
        SessionID = 0;
        AccountStatus = 0;
        Token[0] = 0;
    }
};

class CWarIncAuthClient
{
public:
    bool Login(const char* username, const char* password, CWarIncAuthLoginResult& outResult);
    bool Check(DWORD customerId, DWORD sessionId, const char* token);
    bool Logout(DWORD customerId, DWORD sessionId, const char* token);
};