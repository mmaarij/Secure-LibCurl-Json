#pragma once

#include <curl/curl.h>
#include <string>
#include "nlohmann/json.hpp"
#include <map>

using json = nlohmann::json;

class SecureLibCurlJson 
{

public:

    /**
     * @brief Constructor for the SecureLibCurlJson class.
     * 
     * @param _secure A boolean flag indicating whether to use secure mode. 
     * If _secure = true then an updated certificate file is automatically downloaded and all future HTTP/HTTPS requests use this certificate.
     * If _secure = false then the updated certificate file will not be downloaded and only HTTP requests will be able to complete with this object.
     * 
     */
    SecureLibCurlJson(const bool _secure);

    /**
     * @brief Destructor for the SecureLibCurlJson class.
     */
    ~SecureLibCurlJson();

    /**
     * @brief Downloads an updated SSL certificate file.
     *
     * This method downloads an updated SSL certificate file from a remote source.
     * Note that it is automatically called when the class is constructed with _secure = true
     *
     * @return 0 on success, non-zero on failure.
     */
    int DownloadUpdatedCert();

    /**
     * @brief Makes an API request and returns the response data as JSON.
     *
     * This method sends an HTTP request to the specified URL, using the provided request type,
     * query parameters, and request body. It then returns the response data as a JSON object.
     *
     * @param url The URL to make the API request to.
     * @param requestType The HTTP request method (e.g., GET, POST).
     * @param queryParameters A map of query parameters to include in the request.
     * @param bodyParameters The JSON data to include in the request body.
     *
     * @return The response data as a JSON object.
     */
    json MakeApiRequest(const std::string& url, const std::string& requestType,
        const std::map<std::string, std::string>& queryParameters,
        const json& bodyParameters);

private:
    CURL* curl; // A pointer to the libcurl handle.
    bool secure; // A flag indicating whether to use secure mode

    /**
     * @brief Static callback function to write response data to a string.
     *
     * This function is used as a callback by libcurl to write response data to a string.
     *
     * @param contents Pointer to the response data.
     * @param size The size of each data element.
     * @param nmemb The number of data elements.
     * @param output Pointer to the string where response data is stored.
     *
     * @return The total size of data written.
     */
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);

    /**
    * @brief Static callback function to write data to a file.
    *
    * This function is used as a callback by libcurl to write data to a file.
    *
    * @param contents Pointer to the data to write.
    * @param size The size of each data element.
    * @param nmemb The number of data elements.
    * @param userp Pointer to the file stream where data is written.
    *
    * @return The total size of data written.
    */
    static size_t FileWriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

};