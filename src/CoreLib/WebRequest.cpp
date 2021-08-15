// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/WebRequest.hpp>
#include <CoreLib/WebService.hpp>
#include <curl/curl.h>

namespace bw
{
	WebRequest::WebRequest() :
	m_isUserAgentSet(false)
	{
		m_curlHandle = curl_easy_init();
	}

	WebRequest::~WebRequest()
	{
		if (m_curlHandle)
			curl_easy_cleanup(m_curlHandle);

		if (m_headerList)
			curl_slist_free_all(m_headerList);
	}

	void WebRequest::SetJSonContent(const std::string_view& encodedJSon)
	{
		SetHeader("Content-Type", "application/json");
		curl_easy_setopt(m_curlHandle, CURLOPT_POSTFIELDSIZE_LARGE, curl_off_t(encodedJSon.size()));
		curl_easy_setopt(m_curlHandle, CURLOPT_COPYPOSTFIELDS, encodedJSon.data());
	}

	void WebRequest::SetMaximumFileSize(Nz::UInt64 maxFileSize)
	{
		curl_off_t maxSize = maxFileSize;
		curl_easy_setopt(m_curlHandle, CURLOPT_MAXFILESIZE_LARGE, maxSize);
	}

	void WebRequest::SetServiceName(const std::string_view& serviceName)
	{
		if (!serviceName.empty())
		{
			//TODO Nz::StackString?
			std::string userAgent = WebService::GetUserAgent();
			userAgent.reserve(userAgent.size() + 3 + serviceName.size());
			userAgent.append(" - ");
			userAgent.append(serviceName.data(), serviceName.size());

			curl_easy_setopt(m_curlHandle, CURLOPT_USERAGENT, userAgent.data());
		}
		else
			curl_easy_setopt(m_curlHandle, CURLOPT_USERAGENT, WebService::GetUserAgent().c_str());

		m_isUserAgentSet = true;
	}

	void WebRequest::SetURL(const std::string& url)
	{
		curl_easy_setopt(m_curlHandle, CURLOPT_URL, url.data());
	}

	void WebRequest::SetupGet()
	{
		curl_easy_setopt(m_curlHandle, CURLOPT_HTTPGET, long(1));
	}

	void WebRequest::SetupPost()
	{
		curl_easy_setopt(m_curlHandle, CURLOPT_POST, long(1));
	}

	std::unique_ptr<WebRequest> WebRequest::Get(const std::string& url, ResultCallback callback)
	{
		std::unique_ptr<WebRequest> request = std::make_unique<WebRequest>();
		request->SetURL(url);
		request->SetResultCallback(std::move(callback));
		request->SetupGet();

		return request;
	}

	std::unique_ptr<WebRequest> WebRequest::Post(const std::string& url, ResultCallback callback)
	{
		std::unique_ptr<WebRequest> request = std::make_unique<WebRequest>();
		request->SetURL(url);
		request->SetResultCallback(std::move(callback));
		request->SetupPost();

		return request;
	}

	CURL* WebRequest::Prepare()
	{
		if (!m_headers.empty())
		{
			for (auto&& [header, value] : m_headers)
			{
				std::string headerValue = (!value.empty()) ? header + ": " + value : header + ";";
				m_headerList = curl_slist_append(m_headerList, headerValue.c_str());
			}

			curl_easy_setopt(m_curlHandle, CURLOPT_HTTPHEADER, m_headerList.Get());
		}

		if (!m_isUserAgentSet)
			curl_easy_setopt(m_curlHandle, CURLOPT_USERAGENT, WebService::GetUserAgent().c_str());

		return m_curlHandle;
	}
}