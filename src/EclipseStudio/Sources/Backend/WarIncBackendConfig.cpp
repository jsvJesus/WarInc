#include "r3dPCH.h"
#include "r3d.h"

#include "WarIncBackendConfig.h"

CWarIncBackendConfig gWarIncBackendConfig;

static void TrimLine(char* s)
{
	if(!s)
		return;

	char* p = s;
	while(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
		p++;

	if(p != s)
		memmove(s, p, strlen(p) + 1);

	int len = (int)strlen(s);
	while(len > 0)
	{
		char c = s[len - 1];
		if(c != ' ' && c != '\t' && c != '\r' && c != '\n')
			break;

		s[len - 1] = 0;
		len--;
	}
}

CWarIncBackendConfig::CWarIncBackendConfig()
{
	SetDefaults();
}

void CWarIncBackendConfig::SetDefaults()
{
	r3dscpy(Host, "127.0.0.1");
	Port = 8080;
	UseSSL = false;
	r3dscpy(BaseUrl, "/");
}

bool CWarIncBackendConfig::LoadFromFile(const char* fileName)
{
	SetDefaults();

	FILE* f = fopen(fileName, "rt");
	if(!f)
	{
		r3dOutToLog("WarIncBackendConfig: config not found: %s, using defaults\n", fileName);
		return false;
	}

	char line[1024];
	bool inBackend = false;

	while(fgets(line, sizeof(line), f))
	{
		TrimLine(line);

		if(line[0] == 0)
			continue;

		if(line[0] == ';' || line[0] == '#')
			continue;

		if(line[0] == '[')
		{
			inBackend = (stricmp(line, "[Backend]") == 0);
			continue;
		}

		if(!inBackend)
			continue;

		char* eq = strchr(line, '=');
		if(!eq)
			continue;

		*eq = 0;

		char* key = line;
		char* value = eq + 1;

		TrimLine(key);
		TrimLine(value);

		if(stricmp(key, "Host") == 0)
		{
			if(value[0])
				r3dscpy(Host, value);
		}
		else if(stricmp(key, "Port") == 0)
		{
			Port = atoi(value);
			if(Port <= 0)
				Port = 8080;
		}
		else if(stricmp(key, "UseSSL") == 0)
		{
			UseSSL = atoi(value) != 0;
		}
		else if(stricmp(key, "BaseUrl") == 0)
		{
			if(value[0])
				r3dscpy(BaseUrl, value);
		}
	}

	fclose(f);

	if(BaseUrl[0] == 0)
		r3dscpy(BaseUrl, "/");

	r3dOutToLog("WarIncBackendConfig: host=%s port=%d ssl=%d baseUrl=%s\n",
		Host,
		Port,
		UseSSL ? 1 : 0,
		BaseUrl);

	return true;
}

const char* CWarIncBackendConfig::GetHost() const
{
	return Host;
}

int CWarIncBackendConfig::GetPort() const
{
	return Port;
}

bool CWarIncBackendConfig::IsSSL() const
{
	return UseSSL;
}

const char* CWarIncBackendConfig::GetBaseUrl() const
{
	return BaseUrl;
}