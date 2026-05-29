#pragma once

#include <string>
#include <vector>

class CkString
{
private:
	std::string value_;

	static const char* Base64Table()
	{
		return "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	}

	static int Base64Value(unsigned char c)
	{
		if(c >= 'A' && c <= 'Z')
			return c - 'A';

		if(c >= 'a' && c <= 'z')
			return c - 'a' + 26;

		if(c >= '0' && c <= '9')
			return c - '0' + 52;

		if(c == '+')
			return 62;

		if(c == '/')
			return 63;

		return -1;
	}

public:
	CkString()
	{
	}

	CkString(const char* s)
	{
		value_ = s ? s : "";
	}

	CkString& operator=(const char* s)
	{
		value_ = s ? s : "";
		return *this;
	}

	CkString& operator=(const CkString& rhs)
	{
		if(this != &rhs)
			value_ = rhs.value_;

		return *this;
	}

	void append(const char* s)
	{
		if(s)
			value_ += s;
	}

	void clear()
	{
		value_.clear();
	}

	const char* getUtf8() const
	{
		return value_.c_str();
	}

	const char* c_str() const
	{
		return value_.c_str();
	}

	int getLength() const
	{
		return (int)value_.length();
	}

	const char* getAnsi() const
	{
		return value_.c_str();
	}

	const char* ansi() const
	{
		return value_.c_str();
	}

	void base64Encode(const char*)
	{
		const unsigned char* data = (const unsigned char*)value_.data();
		size_t len = value_.size();

		std::string out;
		out.reserve(((len + 2) / 3) * 4);

		const char* table = Base64Table();

		for(size_t i = 0; i < len; i += 3)
		{
			unsigned int octet_a = i < len ? data[i] : 0;
			unsigned int octet_b = (i + 1) < len ? data[i + 1] : 0;
			unsigned int octet_c = (i + 2) < len ? data[i + 2] : 0;

			unsigned int triple = (octet_a << 16) | (octet_b << 8) | octet_c;

			out.push_back(table[(triple >> 18) & 0x3F]);
			out.push_back(table[(triple >> 12) & 0x3F]);
			out.push_back((i + 1) < len ? table[(triple >> 6) & 0x3F] : '=');
			out.push_back((i + 2) < len ? table[triple & 0x3F] : '=');
		}

		value_ = out;
	}

	bool base64Decode(const char*)
	{
		std::string clean;
		clean.reserve(value_.size());

		for(size_t i = 0; i < value_.size(); ++i)
		{
			unsigned char c = (unsigned char)value_[i];

			if(c == '\r' || c == '\n' || c == '\t' || c == ' ')
				continue;

			clean.push_back((char)c);
		}

		if(clean.empty())
		{
			value_.clear();
			return true;
		}

		if((clean.size() % 4) != 0)
			return false;

		std::string out;
		out.reserve((clean.size() / 4) * 3);

		for(size_t i = 0; i < clean.size(); i += 4)
		{
			int v0 = Base64Value((unsigned char)clean[i + 0]);
			int v1 = Base64Value((unsigned char)clean[i + 1]);

			if(v0 < 0 || v1 < 0)
				return false;

			int v2 = clean[i + 2] == '=' ? -2 : Base64Value((unsigned char)clean[i + 2]);
			int v3 = clean[i + 3] == '=' ? -2 : Base64Value((unsigned char)clean[i + 3]);

			if(v2 == -1 || v3 == -1)
				return false;

			unsigned int triple = ((unsigned int)v0 << 18) | ((unsigned int)v1 << 12);

			if(v2 >= 0)
				triple |= ((unsigned int)v2 << 6);

			if(v3 >= 0)
				triple |= (unsigned int)v3;

			out.push_back((char)((triple >> 16) & 0xFF));

			if(v2 >= 0)
				out.push_back((char)((triple >> 8) & 0xFF));

			if(v3 >= 0)
				out.push_back((char)(triple & 0xFF));
		}

		value_ = out;
		return true;
	}
};