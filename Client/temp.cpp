#include <iostream>
#include <conio.h>
#include <string.h>
#include <Windows.h>
#include <stdio.h>
#include <thread>
#include <iomanip>
#include <chrono>
#include <ctime>

#include "libcurl/include/curl/curl.h"

#ifdef _DEBUG
#pragma comment(lib, "libcurl/lib/libcurl_a_debug.lib")
#else
#pragma comment(lib, "libcurl/lib/libcurl_a32.lib")
#endif

using namespace std;
using std::chrono::system_clock;

CURLcode upload_put(CURL *curl_handler, char *url, char outfilename[]) {

	FILE *fp = fopen(outfilename, "rb");
	curl_easy_setopt(curl_handler, CURLOPT_URL, url);
	//curl_easy_setopt(curl_handler, CURLOPT_READFUNCTION, read_data); //provides an alternative way to get data
	curl_easy_setopt(curl_handler, CURLOPT_READDATA, fp);
	curl_easy_setopt(curl_handler, CURLOPT_SSL_VERIFYPEER, FALSE);
	//tell we want upload and uses put
	curl_easy_setopt(curl_handler, CURLOPT_UPLOAD, 1L);

	/*A few protocols won't behave properly when uploads are done without any prior knowledge of the expected file size
	in this example, file_size must be an curl_off_t variable */
	//curl_easy_setopt(curl_handler, CURLOPT_INFILESIZE_LARGE, file_size);

	CURLcode res = curl_easy_perform(curl_handler);

	/* now extract transfer info
	double speed_upload, total_time;
	curl_easy_getinfo(curl_handler, CURLINFO_SPEED_UPLOAD, &speed_upload);
	curl_easy_getinfo(curl_handler, CURLINFO_TOTAL_TIME, &total_time);
	fprintf(stderr, "Speed: %.3f bytes/sec during %.3f seconds\n",speed_upload, total_time);
	*/

	fclose(fp);

	return res;
}

CURLcode post_binary(CURL *curl_handler, char *url, char outfilename[]) {

	//save response
	FILE *fp = fopen(outfilename, "wb");
	curl_easy_setopt(curl_handler, CURLOPT_URL, url);
	curl_easy_setopt(curl_handler, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl_handler, CURLOPT_WRITEDATA, fp);

	//must change header
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Content-Type: text/xml");

	/* post binary data */
	curl_easy_setopt(curl_handler, CURLOPT_POSTFIELDS, fp); //debug

															/* set the size of the postfields data */
	curl_easy_setopt(curl_handler, CURLOPT_POSTFIELDSIZE, 23L);

	/* pass our list of custom made headers */
	curl_easy_setopt(curl_handler, CURLOPT_HTTPHEADER, headers);

	CURLcode res = curl_easy_perform(curl_handler); /* post away! */

	curl_slist_free_all(headers); /* free the header list */

	fclose(fp);
	return res;
}
CURLcode post_text(CURL *curl_handler, char *url, char *data, char outfilename[]) {

	//save response
	FILE *fp = fopen(outfilename, "wb");
	curl_easy_setopt(curl_handler, CURLOPT_URL, url);
	curl_easy_setopt(curl_handler, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl_handler, CURLOPT_WRITEDATA, fp);


	curl_easy_setopt(curl_handler, CURLOPT_URL, url);
	curl_easy_setopt(curl_handler, CURLOPT_POSTFIELDS, data);

	curl_easy_setopt(curl_handler, CURLOPT_POSTFIELDSIZE, (long)strlen(data));

	CURLcode res = curl_easy_perform(curl_handler); /* post away! */
	fclose(fp);
	return res;
}
CURLcode post_text_ok(CURL *curl_handler, char *url, char *data, char outfilename[]) {


	FILE *fp = fopen(outfilename, "rb");
	curl_easy_setopt(curl_handler, CURLOPT_URL, url);
	curl_easy_setopt(curl_handler, CURLOPT_READFUNCTION, read_data);
	curl_easy_setopt(curl_handler, CURLOPT_READDATA, fp);


	FILE *fpp = fopen("C:/ProgramData/easy2.txt", "wb");
	curl_easy_setopt(curl_handler, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl_handler, CURLOPT_WRITEDATA, fpp);

	curl_easy_setopt(curl_handler, CURLOPT_POST, 1L);
	curl_easy_setopt(curl_handler, CURLOPT_URL, url);

	//curl_easy_setopt(curl_handler, CURLOPT_POSTFIELDS, data);
	//curl_easy_setopt(curl_handler, CURLOPT_POSTFIELDSIZE, (long)strlen(data));

	CURLcode res = curl_easy_perform(curl_handler); /* post away! */
	fclose(fp);
	fclose(fpp);
	return res;
}


int  main()
{



	/*Timing*/
		// get current time
		time_t timet = system_clock::to_time_t(system_clock::now());
		//convert it to tm struct
		struct tm * time = localtime(&timet);
		cout << "Current time: " << put_time(time, "%X") << '\n';
		cout << "Waiting for the next minute to begin...\n";
		time->tm_min++; time->tm_sec = 0;
		//sleep until next minute is not reached
		this_thread::sleep_until(system_clock::from_time_t(mktime(time)));
		cout << put_time(time, "%X") << " reached!\n";
		//sleep for 5 seconds
		this_thread::sleep_for(chrono::seconds(5));
		//get current time
		timet = system_clock::to_time_t(system_clock::now());
		//convert it to tm struct
		time = localtime(&timet);
		cout << "Current time: " << put_time(time, "%X") << '\n';


}