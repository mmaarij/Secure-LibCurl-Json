#include "SecureLibCurlJson.h"
#include <iostream>
#include <ctime>
#include <sstream>
#include <filesystem>


using json = nlohmann::json;

// *********************************** PUBLIC METHODS ***********************************

SecureLibCurlJson::SecureLibCurlJson(const bool _secure, const bool _logging)
{
    this->secure = _secure;
    curl = curl_easy_init();
    if (!curl)
        std::cerr << "Failed to initialize libcurl." << std::endl;
    if (_secure)
    {
        if (this->DownloadUpdatedCert() != 0)
            std::cerr << "Failed to download or update certificate" << std::endl;
    }

    if (_logging)
    {
        this->StartLogging();
    }
}

SecureLibCurlJson::~SecureLibCurlJson()
{
    if (curl)
        curl_easy_cleanup(curl);

    if (logging) {
        logFile.close();
    }
}

int SecureLibCurlJson::DownloadUpdatedCert()
{
    if (!curl) {
        std::cerr << "libcurl not initialized." << std::endl;
        return 1;
    }

    if (!secure) {
        std::cerr << "Object was constructed using _secure as false. Certificates cannot be updated." << std::endl;
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

    if (this->secure)
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 2);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Set the body data as a JSON string
    const std::string bodyString = bodyParameters.dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyString.c_str());

    // Set the write callback function
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    // Pass the response string as userdata
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Make the request
    res = curl_easy_perform(curl);

    // Log the API request
    std::string requestLog = "API Request: " + requestType + " " + url;
    LogMessage(requestLog);

    // Clean up headers
    curl_slist_free_all(headers);

    if (res != CURLE_OK) 
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        // Log the Error
        std::string responseLog = "API Request Error: " + std::string(curl_easy_strerror(res)) + "\n";
        LogMessage(responseLog);
    }
    else
    {
        // The response is now stored in the 'response' string
        json data;
        try {
            data = json::parse(response);
            // Log the API response
            std::string responseLog = "API Response: " + response + "\n";
            LogMessage(responseLog);
        }
        catch (const json::parse_error& e) {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
            // Log the Error
            std::string responseLog = "JSON Parse Error: " + std::string(e.what()) + "\n";
            LogMessage(responseLog);
        }
        return data;
    }

    return json();
}


void SecureLibCurlJson::StartLogging() {
    if (!logging) {
        // Specify the directory path for the "logs" folder
        std::string logDirectory = "logs/";
        std::string logFileName = logDirectory + GetCurrentTimestamp() + ".log";

        // Create the "logs" directory if it doesn't exist
        std::filesystem::create_directories(logDirectory);

        // Open the log file with the complete path
        logFile.open(logFileName, std::ios::out);

        logging = true;
        std::cout << "Logging Started." << std::endl;
        LogMessage("Logging started. Date|Time Format: DD-MM-YY-HH-MM-SS\n");
    }
}

// *********************************** PRIVATE STATIC METHODS ***********************************

void SecureLibCurlJson::LogMessage(const std::string& message) {
    if (logging) {
        logFile << GetCurrentTimestamp() << " " << message << std::endl;
    }
}

std::string SecureLibCurlJson::GetCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    struct std::tm timeInfo;
    localtime_s(&timeInfo, &now);

    std::ostringstream oss;
    oss << timeInfo.tm_mday << '-';
    oss << (timeInfo.tm_mon + 1) << '-';
    oss << (timeInfo.tm_year + 1900) << '-';
    oss << timeInfo.tm_hour << '-';
    oss << timeInfo.tm_min << '-';
    oss << timeInfo.tm_sec;

    return oss.str();
}

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
