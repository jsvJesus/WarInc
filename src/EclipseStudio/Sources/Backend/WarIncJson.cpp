#include "r3dPCH.h"
#include "r3d.h"

#include "WarIncJson.h"

static const char* FindJsonField(const char* json, const char* fieldName)
{
	if(!json || !fieldName || !fieldName[0])
		return NULL;

	char pattern[256];
	sprintf(pattern, "\"%s\"", fieldName);

	const char* p = strstr(json, pattern);
	if(!p)
		return NULL;

	p += strlen(pattern);

	while(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
		p++;

	if(*p != ':')
		return NULL;

	p++;

	while(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
		p++;

	return p;
}

std::string CWarIncJson::EscapeString(const char* value)
{
	std::string out;

	if(!value)
		return out;

	for(const char* p = value; *p; ++p)
	{
		char c = *p;

		if(c == '\\')
			out += "\\\\";
		else if(c == '"')
			out += "\\\"";
		else if(c == '\n')
			out += "\\n";
		else if(c == '\r')
			out += "\\r";
		else if(c == '\t')
			out += "\\t";
		else
			out += c;
	}

	return out;
}

bool CWarIncJson::HasField(const char* json, const char* fieldName)
{
	return FindJsonField(json, fieldName) != NULL;
}

bool CWarIncJson::GetBool(const char* json, const char* fieldName, bool defaultValue)
{
	const char* p = FindJsonField(json, fieldName);
	if(!p)
		return defaultValue;

	if(strnicmp(p, "true", 4) == 0)
		return true;

	if(strnicmp(p, "false", 5) == 0)
		return false;

	if(*p == '1')
		return true;

	if(*p == '0')
		return false;

	return defaultValue;
}

int CWarIncJson::GetInt(const char* json, const char* fieldName, int defaultValue)
{
	const char* p = FindJsonField(json, fieldName);
	if(!p)
		return defaultValue;

	return atoi(p);
}

unsigned int CWarIncJson::GetUInt(const char* json, const char* fieldName, unsigned int defaultValue)
{
	const char* p = FindJsonField(json, fieldName);
	if(!p)
		return defaultValue;

	return (unsigned int)strtoul(p, NULL, 10);
}

bool CWarIncJson::GetString(const char* json, const char* fieldName, char* outValue, int outValueSize)
{
	if(!outValue || outValueSize <= 0)
		return false;

	outValue[0] = 0;

	const char* p = FindJsonField(json, fieldName);
	if(!p)
		return false;

	if(*p != '"')
		return false;

	p++;

	int outPos = 0;

	while(*p && *p != '"' && outPos < outValueSize - 1)
	{
		if(*p == '\\')
		{
			p++;

			if(*p == 0)
				break;

			if(*p == 'n')
				outValue[outPos++] = '\n';
			else if(*p == 'r')
				outValue[outPos++] = '\r';
			else if(*p == 't')
				outValue[outPos++] = '\t';
			else
				outValue[outPos++] = *p;

			p++;
			continue;
		}

		outValue[outPos++] = *p;
		p++;
	}

	outValue[outPos] = 0;
	return true;
}