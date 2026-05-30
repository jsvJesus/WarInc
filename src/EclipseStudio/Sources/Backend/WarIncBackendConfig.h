#pragma once

class CWarIncBackendConfig
{
public:
    char Host[256];
    int Port;
    bool UseSSL;
    char BaseUrl[256];

public:
    CWarIncBackendConfig();

    void SetDefaults();
    bool LoadFromFile(const char* fileName);

    const char* GetHost() const;
    int GetPort() const;
    bool IsSSL() const;
    const char* GetBaseUrl() const;
};

extern CWarIncBackendConfig gWarIncBackendConfig;