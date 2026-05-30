#pragma once

class CWarIncJson
{
public:
    static std::string EscapeString(const char* value);

    static bool HasField(const char* json, const char* fieldName);

    static bool GetBool(const char* json, const char* fieldName, bool defaultValue);
    static int GetInt(const char* json, const char* fieldName, int defaultValue);
    static unsigned int GetUInt(const char* json, const char* fieldName, unsigned int defaultValue);
    static bool GetString(const char* json, const char* fieldName, char* outValue, int outValueSize);
};