#include <iostream>
#include <fstream>
#include <conio.h>
#include <string>
#include <direct.h>
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



#define FILEPATH_MAX 200
#define POST_INIT "init"
#define POST_ALIVE "alive"
#define POST_CONTROL "control"
#define POST_RESUME "resume"
#define POST_PRIORITY "priority"

#define TASK_INVALID -1
#define TASK_UNKNOWN 0

#define MAX_SEM_COUNT 1


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



 //#define MODE_SPLIT  //split large conections

/* notes
* 1. server must send data just in answer to post_control.
* 
* 
*/

HANDLE semaphore_alive;
HANDLE semaphore_resultfile;
HANDLE semaphore_checkresult;
DWORD dwWaitResult;
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

CURLcode get_skipssl(CURL *curl_handler, char *url, char download_filename[]) {

	curl_easy_setopt(curl_handler, CURLOPT_URL, url);
	//skip ssl cetification
	curl_easy_setopt(curl_handler, CURLOPT_SSL_VERIFYPEER, FALSE);

	FILE *fp = fopen(download_filename, "wb");
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

CURLcode post_data(CURL *curl_handler, char *url, char upload_filename[], char result_filename[]) {

	curl_easy_setopt(curl_handler, CURLOPT_URL, url);
	curl_easy_setopt(curl_handler, CURLOPT_POST, 1L);

	FILE *fpin = fopen(upload_filename, "rb");
	curl_easy_setopt(curl_handler, CURLOPT_READFUNCTION, read_data);
	curl_easy_setopt(curl_handler, CURLOPT_READDATA, fpin);

	/*dont save response
	FILE *fpout = fopen(result_filename, "ab+");
	curl_easy_setopt(curl_handler, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl_handler, CURLOPT_WRITEDATA, fpout);
	*/
	CURLcode res = curl_easy_perform(curl_handler); /* post away! */

	fclose(fpin);
	//fclose(fpout);

	return res;
}
CURLcode post_control_synchronous(CURL *curl_handler, char *url, char *post_data, char result_filename[]) {
	WaitForSingleObject(semaphore_resultfile, INFINITE);

	curl_easy_setopt(curl_handler, CURLOPT_URL, url);
	curl_easy_setopt(curl_handler, CURLOPT_POSTFIELDS, post_data);
	curl_easy_setopt(curl_handler, CURLOPT_POSTFIELDSIZE, (long)strlen(post_data)); 

	FILE *fpout = fopen(result_filename, "ab+");
	curl_easy_setopt(curl_handler, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl_handler, CURLOPT_WRITEDATA, fpout);

	CURLcode res = curl_easy_perform(curl_handler); /* post away! */

	fclose(fpout);

	ReleaseSemaphore(semaphore_resultfile, 1, NULL);

	return res;
}

#pragma endregion

int task_decode(string line);
bool task_do(int task);
void delete_done_task_synchronous(char result_filename[]);

void thread_alive_run(char *server_url, char result_filename[], int alive_delay_min)
{

	WaitForSingleObject(semaphore_alive,INFINITE);
	ReleaseSemaphore(semaphore_alive, 1, NULL);

	CURL *curl_handler;
	curl_handler = curl_easy_init();

	if (curl_handler) {


		while (alive)
		{
			post_control_synchronous(curl_handler, server_url, POST_ALIVE, result_filename);
			ReleaseSemaphore(semaphore_checkresult, 1, NULL);
			this_thread::sleep_for(chrono::minutes(alive_delay_min));
		}

		curl_easy_cleanup(curl_handler);
	}

}


void client_init(char init_filename[])
{

}


void add_header_file(string header_data, char in_filename[])
{
	
}

void check_tasks(char result_filename[]) {

	ifstream infile;
	infile.open(result_filename);
	string line;
	bool task_done = FALSE;
	int task = TASK_UNKNOWN;
	if (getline(infile, line)) {

		task = task_decode(line);
		if(task != TASK_INVALID)
			task_done = task_do(task);
	}

	infile.close();

	if (task==TASK_INVALID || task_done) {
		delete_done_task_synchronous(result_filename);
	}
	
}

int task_decode(string line){
	
	return 1;
}

bool task_do(int task) {

	if (task == TASK_INVALID || task == TASK_UNKNOWN)
		return FALSE;


	return TRUE;


}

void delete_done_task_synchronous(char result_filename[])
{
	char temp_filename[FILENAME_MAX] = "C:/ProgramData/tempo/temp.txt";
	string line;
	ifstream infile;
	ofstream tempfile;

	WaitForSingleObject(semaphore_resultfile, INFINITE);
	
	infile.open(result_filename);
	tempfile.open(temp_filename);
	for (int i =0; getline(infile, line); i++)
	{
		if (i != 0) {
			tempfile << line << "\n";
		}
	}
	infile.close();
	tempfile.close();
	remove(result_filename);
	rename(temp_filename,result_filename);
	remove(temp_filename);
	ReleaseSemaphore(semaphore_resultfile, 1, NULL);
}

#define DEBUG
#ifdef DEBUG
	int main()
#else
	int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
#endif
{

//#define _TEST
#ifdef _TEST
		CURL *curl_handlerm;
		curl_handler2 = curl_easy_init();
		char *server_url2 = "http://127.0.0.1:6789";
		char *data = POST_INIT;
		char result_filename2[FILENAME_MAX] = "C:/ProgramData/tempo/results.txt";
		forbid_reuse_connection(curl_handlerm, TRUE);
		post_control_synchronous(curl_handlerm, server_url2, POST_INIT, result_filename2);
		cout << "\nend.";
		_getch();
		return 0;
#endif

	char *server_url = "http://127.0.0.1:6789";
	int client_id = 10;
	int alive_delay_min = 2;
	//make a sub folder
	char outDir[FILENAME_MAX] = "C:/ProgramData/tempo";
	_mkdir(outDir);

	char result_filename[FILENAME_MAX] = "C:/ProgramData/tempo/results.txt";
	char download_filename[FILENAME_MAX] = "C:/ProgramData/tempo/download.txt";
	char upload_filename[FILENAME_MAX] = "C:/ProgramData/tempo/upload.txt";
	CURL *curl_handler;
	
	
	CURLcode get_res;
	CURLcode upload_res;
	char *urlsecure = "https://192.168.1.201/easy.txt";
	char *urllarge = "http://www.bdu.ac.in/centers/uic/docs/courseware/C_Programming_Notes.pdf";
	char *urllocal = "http://127.0.0.1:6789";
	char *url = "http://www.google.com";
	
	semaphore_alive = CreateSemaphore(
		NULL,           // default security attributes
		MAX_SEM_COUNT,  // initial count
		MAX_SEM_COUNT,  // maximum count
		NULL);			// unnamed semaphore
	semaphore_resultfile = CreateSemaphore(NULL, MAX_SEM_COUNT, MAX_SEM_COUNT, NULL);
	semaphore_checkresult = CreateSemaphore(NULL, 0, MAX_SEM_COUNT, NULL);

	dwWaitResult  = WaitForSingleObject(
		semaphore_alive,   // handle to semaphore
		INFINITE);		// time-out interval use 0L if dont want block
	/*if timer expires or semaphore signals:
	switch (dwWaitResult)
	{
		// The semaphore object was signaled.
	case WAIT_OBJECT_0:
		//...
		break;
		//The semaphore was nonsignaled, so a time-out occurred
	case WAIT_TIMEOUT:
		//...
		break;
	}
	*/

	thread thread_alive(thread_alive_run,
									server_url, result_filename, alive_delay_min);

	curl_handler = curl_easy_init();

	if (curl_handler) {

		forbid_reuse_connection(curl_handler, TRUE);
		
		//get init settings
		post_control_synchronous(curl_handler, server_url, POST_INIT, result_filename);
		client_init(result_filename);

		//start reporting alive
		ReleaseSemaphore(
			semaphore_alive,  // handle to semaphore
			1,            // increase count by one
			NULL);       // not interested in previous count
		while (alive)
		{
			WaitForSingleObject(semaphore_checkresult, INFINITE);
			check_tasks(result_filename);
		}
		
		/* always cleanup one time at end*/
		curl_easy_cleanup(curl_handler);
	}

	#ifdef _DEBUG
		cout << "\nend.";
		_getch();
	#endif

	//main thread must ends after end of other threads. if want main dont wait call thread_alive.detach()
	if (thread_alive.joinable())
	{
		thread_alive.join(); // main wait untill this thread finishes.
	}
	//close semophore handlers
	CloseHandle(semaphore_alive);
	CloseHandle(semaphore_resultfile);
	CloseHandle(semaphore_checkresult);

	return 0;
}