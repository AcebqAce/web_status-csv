#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <curl/curl.h>

class URLStatusChecker {
public:
    URLStatusChecker(const std::string& inputFile, const std::string& outputFile)
        : inputFilename(inputFile), outputFilename(outputFile)
    {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
    }

    ~URLStatusChecker() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
    }

    void checkURLs() {
        if (!curl) {
            std::cerr << "Error: Failed to initialize libcurl." << std::endl;
            return;
        }

        std::ifstream inputFile(inputFilename);
        if (!inputFile.is_open()) {
            std::cerr << "Error: Failed to open input file '" << inputFilename << "'" << std::endl;
            return;
        }

        std::ofstream outputFile(outputFilename);
        if (!outputFile.is_open()) {
            std::cerr << "Error: Failed to open output file '" << outputFilename << "'" << std::endl;
            inputFile.close();
            return;
        }

        char url[1024];
        while (inputFile.getline(url, sizeof(url))) {
            std::string urlString(url);

            std::cout << "Checking status for \"" << urlString << "\"" << std::endl;

            if (checkURLStatus(urlString)) {
                outputFile << urlString << ",Active" << std::endl;
            } else {
                outputFile << urlString << ",Inactive" << std::endl;
            }
        }

        inputFile.close();
        outputFile.close();
    }

private:
    CURL* curl;
    std::string inputFilename;
    std::string outputFilename;

    static size_t writeDataCallback(void* buffer, size_t size, size_t nmemb, void* userdata) {
        // This callback function is required to perform the CURLOPT_WRITEFUNCTION operation,
        // but we don't need to write the response data anywhere, so we just return the received data size.
        return size * nmemb;
    }

    bool checkURLStatus(const std::string& url) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, NULL);

        CURLcode res = curl_easy_perform(curl);
        return (res == CURLE_OK);
    }
};

int main() {
    URLStatusChecker checker("input.csv", "output.csv");
    checker.checkURLs();
    return 0;
}

