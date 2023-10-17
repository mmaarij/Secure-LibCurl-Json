#pragma once

#include <curl/curl.h>
#include <string>
#include "nlohmann/json.hpp"
#include <map>

using json = nlohmann::json;

class SecureLibCurlJson 
{

public:
    SecureLibCurlJson();
    ~SecureLibCurlJson();
    int DownloadUpdatedCert();
    json MakeApiRequest(const std::string& url, const std::string& requestType,
        const std::map<std::string, std::string>& queryParameters,
        const json& bodyParameters);

private:
    CURL* curl;
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);
    static size_t FileWriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

};