#pragma once

#ifndef WARINC_CURL_COMPAT_H
#define WARINC_CURL_COMPAT_H

#ifndef CURL_STATICLIB
#define CURL_STATICLIB
#endif

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>

#include <curl/curl.h>

class CkByteData
{
public:
	std::vector<unsigned char> data_;

	CkByteData()
	{
	}

	CkByteData(const CkByteData& rhs)
	{
		data_ = rhs.data_;
	}

	CkByteData& operator=(const CkByteData& rhs)
	{
		if(this != &rhs)
			data_ = rhs.data_;

		return *this;
	}

	bool loadFile(const char* fileName)
	{
		data_.clear();

		FILE* f = fopen(fileName, "rb");
		if(!f)
			return false;

		fseek(f, 0, SEEK_END);
		long sz = ftell(f);
		fseek(f, 0, SEEK_SET);

		if(sz < 0)
		{
			fclose(f);
			return false;
		}

		data_.resize((size_t)sz);

		if(sz > 0)
		{
			size_t rd = fread(&data_[0], 1, (size_t)sz, f);
			fclose(f);
			return rd == (size_t)sz;
		}

		fclose(f);
		return true;
	}

	bool saveFile(const char* fileName) const
	{
		FILE* f = fopen(fileName, "wb");
		if(!f)
			return false;

		if(!data_.empty())
		{
			size_t wr = fwrite(&data_[0], 1, data_.size(), f);
			fclose(f);
			return wr == data_.size();
		}

		fclose(f);
		return true;
	}

	void clear()
	{
		data_.clear();
	}

	void appendChar(char c)
	{
		data_.push_back((unsigned char)c);
	}

	void append(const void* p, unsigned int len)
	{
		if(!p || len == 0)
			return;

		const unsigned char* b = (const unsigned char*)p;
		data_.insert(data_.end(), b, b + len);
	}

	void removeChunk(int offset, int len)
	{
		if(offset < 0 || len <= 0)
			return;

		if(offset >= (int)data_.size())
			return;

		int end = offset + len;
		if(end > (int)data_.size())
			end = (int)data_.size();

		data_.erase(data_.begin() + offset, data_.begin() + end);
	}

	int getSize() const
	{
		return (int)data_.size();
	}

	unsigned char getByte(int idx) const
	{
		if(idx < 0 || idx >= (int)data_.size())
			return 0;

		return data_[idx];
	}

	const unsigned char* getBytes() const
	{
		return data_.empty() ? NULL : &data_[0];
	}

	unsigned char* getBytes()
	{
		return data_.empty() ? NULL : &data_[0];
	}

	const unsigned char* getData() const
	{
		return getBytes();
	}

	unsigned char* getData()
	{
		return getBytes();
	}
};

class CkHttpProgress
{
public:
	virtual ~CkHttpProgress()
	{
	}

	virtual void PercentDone(int pctDone, bool* abort)
	{
		if(abort)
			*abort = false;
	}

	virtual void ProgressInfo(const char* name, const char* value)
	{
	}

	virtual void HttpBeginReceive(void)
	{
	}

	virtual void HttpEndReceive(bool success)
	{
	}

	virtual void ReceiveRate(unsigned long byteCount, unsigned long bytesPerSec)
	{
	}
};

class CkHttpRequest
{
public:
	struct FilePart
	{
		std::string name;
		std::string fileName;
		CkByteData data;
	};

	std::string path_;
	std::string fullUrl_;
	bool post_;
	bool upload_;
	bool utf8_;
	std::vector<std::pair<std::string, std::string> > params_;
	std::vector<std::pair<std::string, std::string> > headers_;
	std::vector<FilePart> files_;
	std::string lastError_;

	CkHttpRequest()
	{
		post_ = false;
		upload_ = false;
		utf8_ = false;
	}

	void UsePost()
	{
		post_ = true;
		upload_ = false;
	}

	void UseUpload()
	{
		post_ = true;
		upload_ = true;
	}

	void put_Path(const char* p)
	{
		path_ = p ? p : "";
	}

	void put_Utf8(bool v)
	{
		utf8_ = v;
	}

	void AddParam(const char* name, const char* val)
	{
		params_.push_back(std::make_pair(name ? name : "", val ? val : ""));
	}

	void AddHeader(const char* name, const char* val)
	{
		headers_.push_back(std::make_pair(name ? name : "", val ? val : ""));
	}

	bool AddBytesForUpload(const char* name, const char* fileName, const CkByteData& data)
	{
		FilePart fp;
		fp.name = name ? name : "";
		fp.fileName = fileName ? fileName : "";
		fp.data = data;
		files_.push_back(fp);
		return true;
	}

	void SetFromUrl(const char* url)
	{
		fullUrl_ = url ? url : "";

		const char* p = strstr(fullUrl_.c_str(), "://");
		if(p)
		{
			p += 3;
			const char* slash = strchr(p, '/');
			path_ = slash ? slash : "/";
		}
		else
		{
			path_ = fullUrl_;
		}
	}

	const char* lastErrorText() const
	{
		return lastError_.c_str();
	}
};

class CkHttpResponse
{
public:
	int statusCode_;
	CkByteData body_;
	std::string bodyText_;
	std::map<std::string, std::string> headers_;

	CkHttpResponse()
	{
		statusCode_ = 0;
	}

	int get_StatusCode() const
	{
		return statusCode_;
	}

	DWORD get_ContentLength() const
	{
		return (DWORD)body_.getSize();
	}

	void get_Body(CkByteData& out) const
	{
		out = body_;
	}

	const char* bodyStr()
	{
		bodyText_.clear();

		if(body_.getSize() > 0 && body_.getData())
			bodyText_.assign((const char*)body_.getData(), body_.getSize());

		bodyText_.push_back(0);
		return bodyText_.c_str();
	}

	const char* getHeaderField(const char* name)
	{
		if(!name)
			return NULL;

		std::map<std::string, std::string>::iterator it = headers_.find(name);
		if(it != headers_.end())
			return it->second.c_str();

		for(it = headers_.begin(); it != headers_.end(); ++it)
		{
			if(_stricmp(it->first.c_str(), name) == 0)
				return it->second.c_str();
		}

		return NULL;
	}
};

class CkHttp
{
private:
	bool unlocked_;
	long connectTimeout_;
	long readTimeout_;
	bool followRedirects_;
	CkHttpProgress* progress_;
	std::string lastError_;
	std::string domainScratch_;

	static void EnsureCurl()
	{
		static bool initialized = false;
		if(!initialized)
		{
			curl_global_init(CURL_GLOBAL_DEFAULT);
			initialized = true;
		}
	}

	static size_t WriteBody(void* ptr, size_t size, size_t nmemb, void* userdata)
	{
		CkByteData* data = (CkByteData*)userdata;
		size_t bytes = size * nmemb;
		data->append(ptr, (unsigned int)bytes);
		return bytes;
	}

	static size_t WriteHeader(char* buffer, size_t size, size_t nitems, void* userdata)
	{
		CkHttpResponse* resp = (CkHttpResponse*)userdata;
		size_t bytes = size * nitems;

		std::string line(buffer, bytes);
		size_t colon = line.find(':');

		if(colon != std::string::npos)
		{
			std::string key = line.substr(0, colon);
			std::string val = line.substr(colon + 1);

			while(!val.empty() && (val[0] == ' ' || val[0] == '\t'))
				val.erase(val.begin());

			while(!val.empty() && (val[val.size() - 1] == '\r' || val[val.size() - 1] == '\n'))
				val.erase(val.end() - 1);

			resp->headers_[key] = val;
		}

		return bytes;
	}

	static std::string Escape(CURL* curl, const std::string& s)
	{
		char* e = curl_easy_escape(curl, s.c_str(), (int)s.size());
		if(!e)
			return "";

		std::string out = e;
		curl_free(e);
		return out;
	}

	static std::string BuildPostFields(CURL* curl, const CkHttpRequest& req)
	{
		std::string out;

		for(size_t i = 0; i < req.params_.size(); ++i)
		{
			if(i > 0)
				out += "&";

			out += Escape(curl, req.params_[i].first);
			out += "=";
			out += Escape(curl, req.params_[i].second);
		}

		return out;
	}

	static std::string BuildUrl(const char* domain, int port, bool ssl, const CkHttpRequest& req)
	{
		if(!req.fullUrl_.empty())
			return req.fullUrl_;

		std::string url = ssl ? "https://" : "http://";
		url += domain ? domain : "";

		if((ssl && port != 443) || (!ssl && port != 80))
		{
			char p[32];
			sprintf(p, ":%d", port);
			url += p;
		}

		if(req.path_.empty() || req.path_[0] != '/')
			url += "/";

		url += req.path_;
		return url;
	}

public:
	CkHttp()
	{
		EnsureCurl();
		unlocked_ = true;
		connectTimeout_ = 30;
		readTimeout_ = 60;
		followRedirects_ = true;
		progress_ = NULL;
	}

	bool UnlockComponent(const char*)
	{
		unlocked_ = true;
		return true;
	}

	bool IsUnlocked() const
	{
		return unlocked_;
	}

	void put_ConnectTimeout(int seconds)
	{
		connectTimeout_ = seconds;
	}

	void put_ReadTimeout(int seconds)
	{
		readTimeout_ = seconds;
	}

	void put_EventCallbackObject(CkHttpProgress* cb)
	{
		progress_ = cb;
	}

	void put_UseBgThread(bool)
	{
	}

	void put_KeepEventLog(bool)
	{
	}

	void put_FollowRedirects(bool v)
	{
		followRedirects_ = v;
	}

	const char* lastErrorText() const
	{
		return lastError_.c_str();
	}

	const char* getDomain(const char* url)
	{
		domainScratch_.clear();

		if(!url)
			return NULL;

		const char* p = strstr(url, "://");
		p = p ? p + 3 : url;

		const char* end1 = strchr(p, '/');
		const char* end2 = strchr(p, ':');

		const char* end = NULL;

		if(end1 && end2)
			end = end1 < end2 ? end1 : end2;
		else if(end1)
			end = end1;
		else if(end2)
			end = end2;

		if(end)
			domainScratch_.assign(p, end - p);
		else
			domainScratch_ = p;

		if(domainScratch_.empty())
			return NULL;

		return domainScratch_.c_str();
	}

	CkHttpResponse* SynchronousRequest(const char* domain, int port, bool ssl, CkHttpRequest& req)
	{
		EnsureCurl();

		CURL* curl = curl_easy_init();
		if(!curl)
		{
			lastError_ = "curl_easy_init failed";
			return NULL;
		}

		CkHttpResponse* resp = new CkHttpResponse();

		std::string url = BuildUrl(domain, port, ssl, req);
		std::string postFields;

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connectTimeout_);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, readTimeout_);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CkHttp::WriteBody);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp->body_);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &CkHttp::WriteHeader);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, resp);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, followRedirects_ ? 1L : 0L);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		struct curl_slist* headers = NULL;

		for(size_t i = 0; i < req.headers_.size(); ++i)
		{
			std::string h = req.headers_[i].first;
			h += ": ";
			h += req.headers_[i].second;
			headers = curl_slist_append(headers, h.c_str());
		}

		if(headers)
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		curl_mime* mime = NULL;

		if(req.upload_)
		{
			mime = curl_mime_init(curl);

			for(size_t i = 0; i < req.params_.size(); ++i)
			{
				curl_mimepart* part = curl_mime_addpart(mime);
				curl_mime_name(part, req.params_[i].first.c_str());
				curl_mime_data(part, req.params_[i].second.c_str(), CURL_ZERO_TERMINATED);
			}

			for(size_t i = 0; i < req.files_.size(); ++i)
			{
				const CkHttpRequest::FilePart& fp = req.files_[i];

				curl_mimepart* part = curl_mime_addpart(mime);
				curl_mime_name(part, fp.name.c_str());
				curl_mime_filename(part, fp.fileName.c_str());

				if(fp.data.getSize() > 0 && fp.data.getData())
					curl_mime_data(part, (const char*)fp.data.getData(), fp.data.getSize());
				else
					curl_mime_data(part, "", 0);
			}

			curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
		}
		else if(req.post_)
		{
			postFields = BuildPostFields(curl, req);
			curl_easy_setopt(curl, CURLOPT_POST, 1L);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)postFields.size());
		}

		if(progress_)
			progress_->HttpBeginReceive();

		CURLcode code = curl_easy_perform(curl);

		if(progress_)
			progress_->HttpEndReceive(code == CURLE_OK);

		if(code != CURLE_OK)
		{
			lastError_ = curl_easy_strerror(code);

			if(mime)
				curl_mime_free(mime);

			if(headers)
				curl_slist_free_all(headers);

			curl_easy_cleanup(curl);
			delete resp;
			return NULL;
		}

		long httpCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
		resp->statusCode_ = (int)httpCode;

		if(mime)
			curl_mime_free(mime);

		if(headers)
			curl_slist_free_all(headers);

		curl_easy_cleanup(curl);
		return resp;
	}
};

#endif
