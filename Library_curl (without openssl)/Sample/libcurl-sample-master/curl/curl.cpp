/*
#include <iostream>
#include <exception>
#include <direct.h>
#include <stdio.h>
#include <sys/stat.h>
*/


#include <string.h>
#define _WIN32_WINNT 0x0500  // must be before Windows.h
#include <Windows.h>
/*
#include "include\curl\curl.h"
#pragma comment(lib, "D:/Develop/C-Cpp/VisualStudio/secdow4/libcurl.lib")
*/
#include "libcurl/include/curl/curl.h"

#ifdef _DEBUG
#pragma comment(lib, "libcurl/lib/libcurl_a_debug.lib")
#else
//#pragma comment(lib, "libcurl/lib/libcurl_a.lib")
#pragma comment(lib, "libcurl/lib/libcurl_a32.lib")
#endif

#define FILENAME_MAX 100

using namespace std;

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	CURL *curl;
	FILE *fp;
	CURLcode res;
	char *url = "https://google.com";
	//char *url = "https://192.168.1.201/easy.txt";
	//char outDir[FILENAME_MAX] = "C:/ProgramData/temp2";
	//_mkdir(outDir);
	char outfilename[FILENAME_MAX] = "C:/ProgramData/easy2.txt";

	curl = curl_easy_init();
	if (curl) {
		fp = fopen(outfilename, "wb");
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
		res = curl_easy_perform(curl);
		/* always cleanup */
		curl_easy_cleanup(curl);
		fclose(fp);
	}

	return 0;
}