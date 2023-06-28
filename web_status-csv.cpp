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

        std::string url;
        while (std::getline(inputFile, url)) {
            std::cout << "Checking status for \"" << url << "\"" << std::endl;

            std::string status;
            std::string newUrl;
            if (checkURLStatus(url, status, newUrl)) {
                if (!newUrl.empty() && isDomainChanged(url, newUrl)) {
                    outputFile << url << ",Redirect to " << newUrl << std::endl;
                } else {
                    outputFile << url << ",Active" << std::endl;
                }
            } else {
                outputFile << url << ",Inactive" << std::endl;
            }
        }

        inputFile.close();
        outputFile.close();
    }

private:
    CURL* curl;
    std::string inputFilename;
    std::string outputFilename;

    bool isDomainChanged(const std::string& url, const std::string& newUrl) {
        std::string domain1 = extractDomain(url);
        std::string domain2 = extractDomain(newUrl);

        return (domain1 != domain2);
    }

    std::string extractDomain(const std::string& url) {
        std::string domain;

        size_t start = url.find("://");
        if (start != std::string::npos) {
            start += 3;
            size_t end = url.find('/', start);
            if (end != std::string::npos) {
                domain = url.substr(start, end - start);
            } else {
                domain = url.substr(start);
            }
        }

        return domain;
    }

    bool checkURLStatus(const std::string& url, std::string& status, std::string& newUrl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            long httpCode;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
            if (httpCode >= 200 && httpCode < 300) {
                char* effectiveUrl;
                res = curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &effectiveUrl);
                if (res == CURLE_OK && effectiveUrl) {
                    newUrl = effectiveUrl;
                }
                return true;
            }
        } else if (res == CURLE_OPERATION_TIMEDOUT) {
            status = "Error: Request timed out";
            return false;
        }

        status = "Error: Failed to perform request";
        return false;
    }
};

int main() {
    URLStatusChecker checker("input.csv", "output.csv");
    checker.checkURLs();
    return 0;
}

