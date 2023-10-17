#include "SecureLibCurlJson.h"
#include <iostream>
#include <fstream>

using json = nlohmann::json;

// *********************************** PUBLIC METHODS ***********************************

SecureLibCurlJson::SecureLibCurlJson()
{
    curl = curl_easy_init();
    if (!curl)
        std::cerr << "Failed to initialize libcurl." << std::endl;
    if (this->DownloadUpdatedCert() != 0)
        std::cerr << "Failed to download or update certificate" << std::endl;
}

SecureLibCurlJson::~SecureLibCurlJson()
{
    if (curl)
        curl_easy_cleanup(curl);
}

int SecureLibCurlJson::DownloadUpdatedCert()
{
    if (!curl) {
        std::cerr << "libcurl not initialized." << std::endl;
        return 1;
    }

    CURLcode res;

    // Set the URL to the remote file
    curl_easy_setopt(curl, CURLOPT_URL, "https://curl.se/ca/cacert.pem");

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 2);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);
    curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");

    // Set the output file and callback function
    std::ofstream output("cacert_updated.pem", std::ios::binary);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FileWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);

    // Perform the download
    res = curl_easy_perform(curl);

    output.close();

    // Check for errors
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return 2;
    }
    else {
        std::cout << "Updated Certificate File Download successful!" << std::endl;
        // If the download was successful, overwrite the original file
        if (std::remove("cacert.pem") != 0) {
            std::cerr << "Failed to remove the original file." << std::endl;
            return 3;
        }
        else if (std::rename("cacert_updated.pem", "cacert.pem") != 0) {
            std::cerr << "Failed to rename the updated file to the original." << std::endl;
            return 4;
        }
        else {
            std::cout << "Certificates updated successfully." << std::endl;
        }
    }

    return 0;
}

json SecureLibCurlJson::MakeApiRequest(const std::string& url, const std::string& requestType,
    const std::map<std::string, std::string>& queryParameters,
    const json& bodyParameters) 
{
    if (!curl) {
        std::cerr << "libcurl not initialized." << std::endl;
        return json();
    }

    CURLcode res;
    std::string response;
    std::string fullUrl = url;

    // Construct the URL with query parameters
    if (!queryParameters.empty()) 
    {
        fullUrl += "?";
        for (const auto& param : queryParameters) 
        {
            fullUrl += param.first + "=" + param.second + "&";
        }
        fullUrl = fullUrl.substr(0, fullUrl.length() - 1);  // Remove the trailing '&'
    }

    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());

    // Set the custom request type (GET, POST, PUT, DELETE, etc.)
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, requestType.c_str());

    // Set headers and body data as needed
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 2);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);
    curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Set the body data as a JSON string
    const std::string bodyString = bodyParameters.dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyString.c_str());

    // Set the write callback function
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    // Pass the response string as userdata
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    res = curl_easy_perform(curl);

    // Clean up headers
    curl_slist_free_all(headers);

    if (res != CURLE_OK) 
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    else
    {
        // The response is now stored in the 'response' string
        json data;
        try {
            data = json::parse(response);
        }
        catch (const json::parse_error& e) {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
        }
        return data;
    }

    return json();
}

// *********************************** PRIVATE STATIC METHODS ***********************************

size_t SecureLibCurlJson::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

size_t SecureLibCurlJson::FileWriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* file = static_cast<std::ofstream*>(userp);
    if (file) {
        file->write(static_cast<const char*>(contents), size * nmemb);
        return size * nmemb;
    }
    return 0;
}
