if can't download curl:
	download it manually and rename it to curl.zip and put it behind build.bat
	delete this lines from build.bat:
		%RM% -rf curl.zip
		
		REM Get download url .Look under <blockquote><a type='application/zip' href='xxx'>
		echo Get download url...
		%XIDEL% http://curl.haxx.se/download.html -e "//a[@type='application/zip' and ends-with(@href, '.zip')]/@href" > tmp_url
		set /p url=<tmp_url

		REM exit on errors, else continue
		if %errorlevel% neq 0 exit /b %errorlevel%

		REM Download latest curl and rename to curl.zip
		echo Downloading latest curl...
		%WGET% "http://curl.haxx.se%url%" -O curl.zip
		

if want static and release vertions and /MT put "set RTLIBCFG=static"(whitout "") in a line before "nmake /f ..."
	