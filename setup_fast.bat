@echo off
setlocal enableextensions enabledelayedexpansion

title STL Bitz Box Setup (Fast)

for /f %%a in ('copy /Z "%~dpf0" nul') do set "CR=%%a"
for /f %%a in ('"prompt $H &echo on &for %%B in (1) do rem"') do set BS=%%a
for /f %%a in ('echo prompt $E ^| cmd') do set "ESC=%%a"

goto MainMenu

:MainMenu
goto InstallThumbs


:ExitMenu
goto End


:InstallServer
cls
	title STL Bitz Box Setup (Fast) - Server Installer
	
	echo Running install script . . .

	start "Running install script . . ." wsvcinst.bat

	echo\
	echo Launching server in background . . .

	start "Launching server . . ." STLBitzBoxServer.exe

	echo\
goto OBJtoSTL


:InstallThumbs
cls
	title STL Bitz Box Setup (Fast) - PNG Thumbnail Support Installer
	
	echo Adding thumbnails provider to registry . . .

	echo\
	echo Attempting to add registry keys for all users . . .

	reg add "HKCR\.stl\shellex\{E357FCCD-A995-4576-B01F-234630154E96}" /ve /t REG_SZ /d "{77257004-6F25-4521-B602-50ECC6EC62A6}" /f
	reg add "HKCR\CLSID\{2E2F83C0-00D8-4504-B84A-31D6A29BFD80}" /ve /t REG_SZ /d "Papa’s Best STL Thumbnails" /f
	reg add "HKCR\CLSID\{2E2F83C0-00D8-4504-B84A-31D6A29BFD80}" /v "DisableProcessIsolation" /t REG_DWORD /d "1" /f
	reg add "HKCR\CLSID\{2E2F83C0-00D8-4504-B84A-31D6A29BFD80}\InprocServer32" /ve /t REG_SZ /d "%CD%\tools\STL_Thumbnails_x64.dll" /f
	reg add "HKCR\CLSID\{2E2F83C0-00D8-4504-B84A-31D6A29BFD80}\InprocServer32" /v "ThreadingModel" /t REG_SZ /d "Apartment" /f
	reg add "HKCR\WOW6432Node\CLSID\{2E2F83C0-00D8-4504-B84A-31D6A29BFD80}" /ve /t REG_SZ /d "Papa’s Best STL Thumbnails" /f
	reg add "HKCR\WOW6432Node\CLSID\{2E2F83C0-00D8-4504-B84A-31D6A29BFD80}" /v "DisableProcessIsolation" /t REG_DWORD /d "1" /f
	reg add "HKCR\WOW6432Node\CLSID\{2E2F83C0-00D8-4504-B84A-31D6A29BFD80}\InprocServer32" /ve /t REG_SZ /d "%CD%\tools\STL_Thumbnails_x86.dll" /f
	reg add "HKCR\WOW6432Node\CLSID\{2E2F83C0-00D8-4504-B84A-31D6A29BFD80}\InprocServer32" /v "ThreadingModel" /t REG_SZ /d "Apartment" /f

	echo\
	echo Attempting to add registry keys for current user . . .

	reg add "HKCU\Software\Classes\.stl\shellex\{E357FCCD-A995-4576-B01F-234630154E96}" /ve /t REG_SZ /d "{77257004-6F25-4521-B602-50ECC6EC62A6}" /f
	reg add "HKCU\Software\Classes\CLSID\{2E2F83C0-00D8-4504-B84A-31D6A29BFD80}" /ve /t REG_SZ /d "Papa’s Best STL Thumbnails" /f
	reg add "HKCU\Software\Classes\CLSID\{2E2F83C0-00D8-4504-B84A-31D6A29BFD80}" /v "DisableProcessIsolation" /t REG_DWORD /d "1" /f
	reg add "HKCU\Software\Classes\CLSID\{2E2F83C0-00D8-4504-B84A-31D6A29BFD80}\InprocServer32" /ve /t REG_SZ /d "%CD%\tools\STL_Thumbnails_x64.dll" /f
	reg add "HKCU\Software\Classes\CLSID\{2E2F83C0-00D8-4504-B84A-31D6A29BFD80}\InprocServer32" /v "ThreadingModel" /t REG_SZ /d "Apartment" /f
	reg add "HKCU\Software\Classes\WOW6432Node\CLSID\{2E2F83C0-00D8-4504-B84A-31D6A29BFD80}" /ve /t REG_SZ /d "Papa’s Best STL Thumbnails" /f
	reg add "HKCU\Software\Classes\WOW6432Node\CLSID\{2E2F83C0-00D8-4504-B84A-31D6A29BFD80}" /v "DisableProcessIsolation" /t REG_DWORD /d "1" /f
	reg add "HKCU\Software\Classes\WOW6432Node\CLSID\{2E2F83C0-00D8-4504-B84A-31D6A29BFD80}\InprocServer32" /ve /t REG_SZ /d "%CD%\tools\STL_Thumbnails_x86.dll" /f
	reg add "HKCU\Software\Classes\WOW6432Node\CLSID\{2E2F83C0-00D8-4504-B84A-31D6A29BFD80}\InprocServer32" /v "ThreadingModel" /t REG_SZ /d "Apartment" /f

	echo\
goto InstallServer


:OBJtoSTL
cls
	title STL Bitz Box Setup (Fast) - OBJ to STL Converter
	
	echo Creating new ^.STL files from ^.OBJ files found ^. ^. ^.!ESC![^?25l
	
	set "reportconversion=reports\Report_FileConverted"
	del "!reportconversion!.txt" >nul 2>&1

	echo Creating new ^.STL files from ^.OBJ files found . . . >> !reportconversion!.txt
	echo\  >> !reportconversion!.txt 

	set /a "convertedcount=0"

	set "spinner=|"
	
	for /r %%i in (*.obj) do (
		set "str=%%i"
	  	set str=!str:~0,-4!
	  	set "stlstr=!str!.stl"
	
		if "!spinner!" == "\" (
			set "spinner=|"
		)
		if "!spinner!" == "-" (
			set "spinner=\"
		)
		if "!spinner!" == "/" (
			set "spinner=-"
		)
		if "!spinner!" == "|" (
			set "spinner=/"
		)
		echo\
		echo !ESC![0KThis may take a few minutes . . .  !spinner!!ESC![0K
		echo\

	  	if NOT exist !stlstr! (
	  		tools\assimp export "%%i" "!stlstr!" >> !reportconversion!.txt
	  		set /a convertedcount += 1
	  		echo Created new STL !stlstr!!ESC![0K
	  		echo from file^: %%i!ESC![0K
	  		echo Created new STL !stlstr! >> !reportconversion!.txt
	  		echo from file^: %%i >> !reportconversion!.txt
	  	)
	  	echo !ESC![H
	)
	echo\
	echo !convertedcount! ^.OBJ files required converting.
	echo !convertedcount! ^.OBJ files required converting. >> !reportconversion!.txt
	echo\
	
	echo\  >> !reportconversion!.txt
	echo End of report. >> !reportconversion!.txt

	echo Report generated.
	echo See !reportconversion!.txt for more details.

	echo\
goto RebuildAll

:RebuildAll
cls
	title STL Bitz Box Setup (Fast) - Thumbnail Generator
	
	echo Generating thumbnails^.!ESC![^?25l

	set /a "thumbcount=0"
	
	set "spinner=|"
	
	for /r %%i in (*.stl) do (
		if "!spinner!" == "\" (
			set "spinner=|"
		)
		if "!spinner!" == "-" (
			set "spinner=\"
		)
		if "!spinner!" == "/" (
			set "spinner=-"
		)
		if "!spinner!" == "|" (
			set "spinner=/"
		)
		echo !ESC![0KThis may take a few minutes . . .  !spinner!!ESC![0K

		if NOT exist %%i.png (
			tools\stltopng "%%i" -o "%%i.png" >nul 2>&1
			set /a thumbcount += 1
			echo\
			echo Created %%i.png!ESC![0K
		)
		echo !ESC![H
	)
	
	echo\
	echo\
	echo\
	echo\
	echo !thumbcount! thumbnails required generation.
	
	echo\
goto RebuildDB


:RebuildDB
cls
	title STL Library Setup

	set timestart=%time%
	
	set /a count = 0
	set /a cntpat = 0
	set /a cntpro = 0
	set /a countother = -1
	
	set "dbfile=files\db"
	set "db=!dbfile!"
	set "top=files\top"
	set "bot=files\bot"
	set "index=index"
	set "scriptpath=%~dp0"
	set "tmphttp="
	set "pa=files\db_pta"
	set "pb=files\db_ptb"
	set "pc=files\db_ptc"
	set "pd=files\db_ptd"
	set "pe=files\db_pte"
	set "pf=files\db_ptf"
	set "pg=files\db_ptg"
	set "ph=files\db_pth"
	set "pi=files\db_pti"
	set "pj=files\db_ptj"
	
	echo Removing old files . . .
	echo\
	echo Deleting index and DB files . . .
	del !dbfile!.sdb >nul 2>&1
	del !db!.sdb >nul 2>&1
	del !pa!.sdb >nul 2>&1
	del !pb!.sdb >nul 2>&1
	del !pc!.sdb >nul 2>&1
	del !pd!.sdb >nul 2>&1
	del !pe!.sdb >nul 2>&1
	del !pf!.sdb >nul 2>&1
	del !pg!.sdb >nul 2>&1
	del !ph!.sdb >nul 2>&1
	del !pi!.sdb >nul 2>&1
	del !pj!.sdb >nul 2>&1
	del !index!.html >nul 2>&1
	timeout /T 1 >nul
	echo\
	
	echo Getting file totals . . .
	for /r %%a in ("*.stl") do (
		set /a cntpat += 1
	)
	set /a "parts1=!cntpat!/10"
	set /a "parts2=!cntpat!/10*2"
	set /a "parts3=!cntpat!/10*3"
	set /a "parts4=!cntpat!/10*4"
	set /a "parts5=!cntpat!/10*5"
	set /a "parts6=!cntpat!/10*6"
	set /a "parts7=!cntpat!/10*7"
	set /a "parts8=!cntpat!/10*8"
	set /a "parts9=!cntpat!/10*9"
	echo STL Files^: !cntpat!
	echo\
	timeout /T 1 >nul
	
	set /A "est=!cntpat! / 60"
	set /A "esthr=!est! / 60"
	set /A "estmin= !est! - (60 * !esthr!)"
	echo Estimated run time: !esthr!h !estmin!m.
	title STL Library Setup - Started: %time:~,5% ^| Est. run-time: !esthr!h !estmin!m
	echo\
	timeout /T 1 >nul
	
	echo Generating index HTML . . .
	
	echo !ESC![^?25l
	
	timeout /T 3 >nul
	cls
	
	for /r %%i in ("*.stl") do (
	  set /a count += 1
	
	  set "file=%%i"
	  call set "result=%%file:%scriptpath%=%%"
	  set "var1=!result!"
	  set "var2=!var1:*\=!"
	  for /f "delims=\" %%z in ("!var2!") do (set "lic=%%z")
		set "str=!result!"
	  set str=!!result:~0,-4!
	  set "pngstr=!str!.stl.png"
	  set "txtstr=!str!"
	  set "txtstr=!txtstr:*\=!"
	  set "txtstr=!txtstr:*\=!"
	  for /f "delims=\" %%y in ("!txtstr!") do (set "dirstr=%%y")
	  set "readmetxt=!result!\..\..\README.txt"
	  set "licensetxt=!result!\..\..\LICENSE.txt"
	  set "tagstxt=!result!\..\..\TAGS.txt"
	  set "setdir=!result!\..\..\"
	  set "setfiles=!result!\..\"
	  set "txtstr=!txtstr:*\=!"
	
	  if !count! leq !cntpat! (
	    set "dbfile=!pj!"
	  )
	  if !count! leq !parts9! (
	    set "dbfile=!pi!"
	  )
	  if !count! leq !parts8! (
	    set "dbfile=!ph!"
	  )
	  if !count! leq !parts7! (
	    set "dbfile=!pg!"
	  )
	  if !count! leq !parts6! (
	    set "dbfile=!pf!"
	  )
	  if !count! leq !parts5! (
	    set "dbfile=!pe!"
	  )
	  if !count! leq !parts4! (
	    set "dbfile=!pd!"
	  )
	  if !count! leq !parts3! (
	    set "dbfile=!pc!"
	  )
	  if !count! leq !parts2! (
	    set "dbfile=!pb!"
	  )
	  if !count! leq !parts1! (
	    set "dbfile=!pa!"
	  )
	
	  echo !ESC![0K!ESC![1A!ESC![0KRebuilding database . . .!ESC![0K
	  
	  echo !ESC![0K
	
	  echo !ESC![0KAdding entry^: !count! of !cntpat!!ESC![0K
	
	  echo !ESC![0KSTL file^: !txtstr!.stl!ESC![0K
	
	  echo ^<tr^> >> !dbfile!.sdb
	
	  echo ^<td^> >> !dbfile!.sdb
	  echo ^<div class="tooltip-item"^>^<a href="#"^>^<img src="" data-src="!pngstr!" width="96" height="96"^>^</a^>^</div^> >> !dbfile!.sdb
	  echo ^</td^> >> !dbfile!.sdb
	
	  echo ^<td^> >> !dbfile!.sdb
	  echo ^<a href="!result!" target="_blank"^>^<p^> 💾 .STL^</p^>^</a^> >> !dbfile!.sdb

	  set "viewpath=!result!"
	  set "viewpath=!viewpath:\=/!"

	  echo ^<a href="viewer.html" target="_blank" onClick="loadSTLView('!viewpath!')"^>^<p^> 🔍 3D VIEW^</p^>^</a^> >> !dbfile!.sdb
	
	  if exist !readmetxt! (
	    echo ^<a href="!readmetxt!" target="_blank"^>^<p^> 📝 README^</p^>^</a^> >> !dbfile!.sdb
	  ) else (
	    echo ^<p^> 🚫 README^</p^> >> !dbfile!.sdb
	  )
	
	  if exist !readmetxt! (
	    for /f "usebackq delims=" %%h in ("!readmetxt!") do for %%l in (%%h) do (
	      set "varhttp=%%~l"
	      set "varhttp=!varhttp:>=!"
	      set "varhttp=!varhttp:"=!"
	      if NOT "!varhttp!" == "!tmphttp!" (
	        if "!varhttp:~0,4!" == "http" (
	          echo !ESC![0KLink^: !varhttp!!ESC![0K
	          echo ^<a href="!varhttp!" target="_blank"^>^<p^> 🌐 LINK^</p^>^</a^> >> !dbfile!.sdb
	          set "tmphttp=!varhttp!"
	        )
	      ) else (
	        echo !ESC![0KLink^: !varhttp!!ESC![0K
	        echo ^<a href="!varhttp!" target="_blank"^>^<p^> 🌐 LINK^</p^>^</a^> >> !dbfile!.sdb
	      )
	    )
	  ) else (
	    set "varhttp="
	  )
	
	  if NOT "!varhttp:~0,4!" == "http" (
	    echo !ESC![0KLink^: No link present.!ESC![0K
	    echo ^<p^> 🚫 LINK^</p^> >> !dbfile!.sdb
	  )
	
	  echo ^</td^> >> !dbfile!.sdb
	
	  echo ^<td^> >> !dbfile!.sdb
	
	  if exist "!licensetxt!" (
	    echo ^<a href="!licensetxt!" target="_blank"^> !lic! ^</a^> >> !dbfile!.sdb
	  ) else (
	    echo ^<p^> !lic! ^</p^> >> !dbfile!.sdb
	  )
	
	  echo ^</td^> >> !dbfile!.sdb
	
	  echo ^<td^>^<a href="!setdir!" target="_blank"^>!dirstr!^</a^>^</td^> >> !dbfile!.sdb
	
	  echo ^<td^>^<a href="!setfiles!" target="_blank"^>!txtstr!^</a^>^</td^> >> !dbfile!.sdb

	  if exist "!tagstxt!" (
	  	for /f "usebackq delims=:" %%x in ("!tagstxt!") do (
	 		echo ^<td^>%%x^</td^> >> !dbfile!.sdb
  	  	)
	  ) else (
	    echo ^<td^>none^</td^> >> !dbfile!.sdb
	  )
	
	  echo ^</tr^> >> !dbfile!.sdb
	
	  rem pathping 127.0.0.1 -n -q 1 -p 150 >nul
	
	  echo !ESC![^?25l
	  echo !ESC![H
	
	  echo !ESC![4B!ESC![0J
	  echo !ESC![H
	)
	
	title STL Library Setup - Finalising...
	
	copy /b !pa!.sdb +!pb!.sdb +!pc!.sdb +!pd!.sdb +!pe!.sdb +!pf!.sdb +!pg!.sdb +!ph!.sdb +!pi!.sdb +!pj!.sdb !db!.sdb >nul 2>&1
	copy /b !top!.html +!db!.sdb +!bot!.html !index!.html >nul 2>&1
	
	cls
	
	echo Database rebuild complete.
	set timeend=%time%
	set chunks="tokens=1-4 delims=:.,"
	for /f %chunks% %%a in ("!timestart!") do (
	  set "starthr=%%a"
	  set /a "startm=100%%b %% 100"
	)
	for /f %chunks% %%a in ("!timeend!") do (
	  set "endhr=%%a"
	  set /a "endm=100%%b %% 100"
	)
	set /a "hrs=!endhr! - !starthr!"
	set /a "mins=!endm! - !startm!"
	if !secs! lss 0 (
	  set /a "mins= !mins! - 1"
	)
	if !mins! lss 0 (
	  set /a "hrs= !hrs! - 1"
	  set /a "mins = 60!mins!"
	)
	if !hrs! lss 0 (
	  set /a "hrs= 24!hrs!"
	)
	echo\
	echo Execution completed in !hrs!h !mins!m
	echo\
	
	rundll32.exe cmdext.dll,MessageBeepStub
	
	echo\
	
	echo Launching page . . .
	timeout 2 >nul
	start http://stlbitzbox.local

goto End


:End