#include <iostream>
#include "SecureLibCurlJson.h"

int main()
{
    SecureLibCurlJson curlLib(true);

    std::string apiUrl = "https://api.publicapis.org/entries";
    std::string requestType = "GET";

    std::map<std::string, std::string> queryParameters;
    /*  EXAMPLE USAGE OF QUERY PARAMETERS
    *   queryParameters["key"] = "abcxyz123";
    *   queryParameters["query"] = "helloworld";
    */

    json bodyParameters;
    /*  EXAMPLE USAGE OF BODY PARAMETERS
    *   bodyParameters["value1"] = "one";
    *   bodyParameters["value2"] = "two";
    *   -- if json objects need to be nested --
    *   json nestedValues;
    *   nestedValues["nestedValue1"] = "n_one";
    *   nestedValues["nestedValue2"] = "n_two";
    *   bodyParameters["nested"] = nestedValues;    
    */

    json responseData = curlLib.MakeApiRequest(apiUrl, requestType, queryParameters, bodyParameters);

    if (!responseData.empty())
    {
        std::cout << "Response Data: " << responseData.dump(2) << std::endl;
    }
    else
    {
        std::cerr << "API request failed." << std::endl;
    }

    return 0;
}