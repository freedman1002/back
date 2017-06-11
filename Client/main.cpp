#include <iostream>
#include <conio.h>
#include <string.h>
#include <Windows.h>
#include <stdio.h>
#include <thread>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <mutex>

#include "libcurl/include/curl/curl.h"

#ifdef _DEBUG
#pragma comment(lib, "libcurl/lib/libcurl_a_debug.lib")
#else
#pragma comment(lib, "libcurl/lib/libcurl_a32.lib")
#endif

#define FILEPATH_MAX 200

#define POST_INIT "init"
#define POST_ALIVE "alive"
#define POST_CONTROL "control"
#define POST_RESUME "resume"
#define POST_PRIORITY "priority"


using namespace std;
/*
There's basically only one thing to keep in mind when using C++ instead of C when interfacing libcurl:
The callbacks CANNOT be non-static class member functions
Example C++ code:
class AClass {
	static size_t write_data(void *ptr, size_t size, size_t nmemb, void *ourpointer)
	{
		// do what you want with the data 
	}
 }
 */


mutex mutex_push, mutex_alive, mutex_checkbuffer;
bool alive = TRUE;
int block_size_m = 1;


#pragma region curl
void forbid_reuse_connection(CURL *curl_handler, bool forbid) {
	if (curl_handler) {
		curl_easy_setopt(curl_handler, CURLOPT_FORBID_REUSE, forbid);
	}
}

CURLcode modify_headers(CURL *curl_handler) {

	struct curl_slist *headers = NULL; /* init to NULL is important */

	headers = curl_slist_append(headers, "Hey-server-hey: how are you?");
	headers = curl_slist_append(headers, "X-silly-content: yes");

	/* and if you think some of the internally generated headers, such as Accept : or Host :
	don't contain the data you want them to contain, you can replace them by simply setting them too:
	*/
	headers = curl_slist_append(headers, "Accept: Agent-007");
	headers = curl_slist_append(headers, "Host: munged.host.line");
	/*If you replace an existing header with one with no contents, you will prevent the header from being sent.
	For instance, if you want to completely prevent the "Accept:" header from being sent,
	you can disable it with code similar to this:
	*/
	headers = curl_slist_append(headers, "Accept:");


	/* pass our list of custom made headers */
	curl_easy_setopt(curl_handler, CURLOPT_HTTPHEADER, headers);

	/*other settings and transfer http */
	//... 
	CURLcode res = curl_easy_perform(curl_handler);

	curl_slist_free_all(headers); /* free the header list */

	return res;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {

	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}
size_t read_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {

	if (size*nmemb < 1)
		return 0;

	size_t read = fread(ptr, size, nmemb, stream);
	return read;
}

CURLcode get_skipssl(CURL *curl_handler, string url, char outfilename[]) {

	curl_easy_setopt(curl_handler, CURLOPT_URL, url);
	//skip ssl cetification
	curl_easy_setopt(curl_handler, CURLOPT_SSL_VERIFYPEER, FALSE);

	FILE *fp = fopen(outfilename, "wb");
	curl_easy_setopt(curl_handler, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl_handler, CURLOPT_WRITEDATA, fp);

	//		cause the library to spew out entire protocol detailes
	//curl_easy_setopt(curl_handler, CURLOPT_VERBOSE, TRUE);
	//		if you want see header in output:
	//curl_easy_setopt(curl_handler, CURLOPT_HEADER, TRUE);

	//this function blocks and calls callback functoin(write_data) frequently
	CURLcode res = curl_easy_perform(curl_handler);

	fclose(fp);

	return res;
}

CURLcode post_data_synchronous(CURL *curl_handler, string url, char read_filename[], char result_filename[]) {
	mutex_push.lock();

	curl_easy_setopt(curl_handler, CURLOPT_URL, url);
	curl_easy_setopt(curl_handler, CURLOPT_POST, 1L);

	FILE *fpin = fopen(read_filename, "rb");
	curl_easy_setopt(curl_handler, CURLOPT_READFUNCTION, read_data);
	curl_easy_setopt(curl_handler, CURLOPT_READDATA, fpin);

	//save response
	FILE *fpout = fopen(result_filename, "wb");
	curl_easy_setopt(curl_handler, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl_handler, CURLOPT_WRITEDATA, fpout);

	CURLcode res = curl_easy_perform(curl_handler); /* post away! */

	fclose(fpin);
	fclose(fpout);

	mutex_push.unlock();

	return res;
}
CURLcode post_control_synchronous(CURL *curl_handler, string url, string post_data, char result_filename[]) {
	mutex_push.lock();

	curl_easy_setopt(curl_handler, CURLOPT_URL, url);
	curl_easy_setopt(curl_handler, CURLOPT_POSTFIELDS, post_data);
	curl_easy_setopt(curl_handler, CURLOPT_POSTFIELDSIZE, post_data.length);  //if post_data is char* use: (long)strlen(post_data)

	FILE *fpout = fopen(result_filename, "wb");
	curl_easy_setopt(curl_handler, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl_handler, CURLOPT_WRITEDATA, fpout);

	CURLcode res = curl_easy_perform(curl_handler); /* post away! */

	fclose(fpout);

	mutex_push.unlock();

	return res;
}

CURLcode post_data_asynchronous(CURL *curl_handler, string url, char read_filename[], char result_filename[]) {

	//post
}
CURLcode post_control_asynchronous(CURL *curl_handler, string url, string post_data, char result_filename[]) {

	//post
}
#pragma endregion


void thread_run(CURL *curl_handler, string server_url, char result_filename[], int alive_delay_min)
{
	mutex_alive.lock();
	mutex_alive.unlock();

	while (alive)
	{
		post_control_synchronous(curl_handler, server_url, POST_ALIVE, result_filename);
		mutex_checkbuffer.unlock();
		this_thread::sleep_for(chrono::minutes(alive_delay_min));
	}

}

void client_init(char init_filename[])
{

}


void check_orders() {


	//just one large data post when use one connection
}


#ifdef _DEBUG
	int main()
#else
	int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
#endif
{

	string server_url = "http://127.0.0.1:6789";
	int client_id = 10;
	int alive_delay_min = 2;
	CURL *curl_handler;
	char result_filename[FILENAME_MAX] = "C:/ProgramData/easy1.txt";
	char outfilename[FILEPATH_MAX] = "E:\cb5a5782_resize.jpg";
	
	/*
	CURLcode get_res;
	CURLcode upload_res;
	char *urlsecure = "https://192.168.1.201/easy.txt";
	char *urllarge = "http://www.bdu.ac.in/centers/uic/docs/courseware/C_Programming_Notes.pdf";
	char *urllocal = "http://127.0.0.1:6789";
	char *url = "http://www.google.com";
	*/
	

	mutex_alive.lock();
	thread thread_timing(thread_run,
									curl_handler, server_url, result_filename, alive_delay_min);

	curl_handler = curl_easy_init();

	if (curl_handler) {

		forbid_reuse_connection(curl_handler, TRUE);

		//get init settings
		post_control_synchronous(curl_handler, server_url, POST_INIT, result_filename);
		client_init(result_filename);

		//start reporting alive
		mutex_checkbuffer.lock();
		mutex_alive.unlock();

		while (alive)
		{
			mutex_checkbuffer.lock();
			check_orders();
		}

		/* always cleanup one time at end*/
		curl_easy_cleanup(curl_handler);
	}

	#ifdef _DEBUG
		cout << "\nend.";
		_getch();
	#endif

	//main thread must ends after end of other threads. if want main dont wait call thread_timing.detach()
	if (thread_timing.joinable())
	{
		thread_timing.join(); // main wait untill this thread finishes.
	}
	return 0;
}