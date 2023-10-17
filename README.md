# Secure [LibCurl](https://curl.se) Wrapper in C++ for HTTP(S) API Calls with [Nlohmann Json](https://github.com/nlohmann/json) Support

This project uses 2 simple files to create the wrapper with `SecureLibCurlJson.h` containing the definitions and `SecureLibCurlJson.cpp` containing all the implementaions.

## Windows (Visual Studio 2022)

### Project Settings for Libcurl

Note : These steps have already been performed in the included visual studio project included in this repo, but, they are documented if you need to setup a new project.

- Download [Curl for Windows](https://curl.se/windows/)
- Extract:
  - The `include` and `lib` folders and keep them in a new folder called `[your-curl-dir]` (it is named `curl-8.4.0_3-win64-mingw` in this repo's case)
  - The `bin/libcurl-x64.dll` file inside and keep it in your project directory.
- Download [CA certificate extracted from Mozilla](https://curl.se/docs/caextract.html) `cacert.pem` and place it inside your project directory.
- Add your-curl-dir/include to Configuration Properties -> VC++ Directories -> Include Directories.
- Add your-curl-dir/lib to Configuration Properties -> VC++ Directories -> Library Directories.
- In Configuration Properties -> Linker -> Input -> Additional Dependencies, add these followings lines:
  - your-curl-dir/lib/libcurl_a.lib
  - Ws2_32.lib
  - Wldap32.lib
  - Crypt32.lib
  - Normaliz.lib
- In Configuration Properties -> Build Events -> Post-Build Event -> Command Line add the following lines to copy necessary files when the .exe is generated:
  - copy "libcurl-x64.dll" "\$(SolutionDir)$(Platform)\$(Configuration)\libcurl-x64.dll"
  - copy "cacert.pem" "\$(SolutionDir)$(Platform)\$(Configuration)\cacert.pem"

### Project Settings for Nlohmann Json

- Download `json.hpp` from [Nlohmann Json](https://github.com/nlohmann/json/releases) and keep it in a folder called `nlohmann` in your project directory
- You can now use the library as follows:

```cpp
#include "nlohmann/json.hpp"

using json = nlohmann::json;
```

### Directory Structure

This is what the directory structure should look like after setting up

```bash
.
├── curl-8.4.0_3-win64-mingw/
│   ├── include/
│   │   └── [all the files inside include as-is]
│   └── lib/
│       └── [all the files lib include as-is]
├── nlohmann/
│   └── json.hpp
├── cacert.pem
├── libcurl-x64.dll
├── SecureLibCurlJson.h
├── SecureLibCurlJson.cpp
└── Source.cpp
```

### Usage

Example usage is provided in `Source.cpp`

```cpp
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
```
