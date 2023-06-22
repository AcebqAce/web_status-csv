#include <stdio.h>
#include <string>
#include <cstring>
#include <curl/curl.h>

int main(void) {
    CURL *curl;
    FILE *fp;
    CURLcode res;
    long http_code;

    const char *infilename = "input.csv";
    const char *outfilename = "output.csv";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        fp = fopen(outfilename, "wb");
        if (!fp) {
            fprintf(stderr, "Error: Failed to open output file '%s'\n", outfilename);
            return 1;
        }

        FILE *csvfile = fopen(infilename, "rb");
        if (!csvfile) {
            fprintf(stderr, "Error: Failed to open input file '%s'\n", infilename);
            fclose(fp);
            return 1;
        }

	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);

        char url[1024];
        char *newUrl = NULL;
        size_t len = sizeof(url);

        while (fgets(url, len, csvfile)) {
            // Remove trailing newline character if present
            if (url[strcspn(url, "\n")] != '\0') {
                url[strcspn(url, "\n")] = '\0';
            }

            printf("Checking status for \"%s\"\n", url);

            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, NULL);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, NULL);

            res = curl_easy_perform(curl);

            if (res == CURLE_OK) {
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
                if (http_code >= 200 && http_code < 300) {
                    newUrl = NULL;
                    if (curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &newUrl) == CURLE_OK && newUrl != NULL && strcmp(newUrl, url) != 0) {
                        fprintf(fp, "%s,Redirect to %s\n", url, newUrl);       
                    } else {
                        fprintf(fp, "%s,Active\n", url);
                    }
                } else {
                    fprintf(fp, "%s,Inactive\n", url);
                }
	        } else if (res == CURLE_OPERATION_TIMEDOUT) {
		        fprintf(fp, "%s,Error: Request timed out\n", url);
            } else {
                fprintf(fp, "%s,Error: Failed to perform request\n", url);
            }
        }

        fclose(csvfile);
        fclose(fp);
        curl_easy_cleanup(curl);
    }
    return 0;
}

