@echo off
title STL Bitz Box Setup - Service Setup

:-------------------------------------
:: Check for admin
    IF "%PROCESSOR_ARCHITECTURE%" EQU "amd64" (
>nul 2>&1 "%SYSTEMROOT%\SysWOW64\cacls.exe" "%SYSTEMROOT%\SysWOW64\config\system"
) ELSE (
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
)

:: If error then we're not admin
if '%errorlevel%' NEQ '0' (
    echo Requesting administrative privileges...
    goto UACPrompt
) else ( goto gotAdmin )

:: Get admin
:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    set params= %*
    echo UAC.ShellExecute "cmd.exe", "/c ""%~s0"" %params:"=""%", "", "runas", 1 >> "%temp%\getadmin.vbs"

    "%temp%\getadmin.vbs"
    del "%temp%\getadmin.vbs"
    exit

:gotAdmin
    pushd "%CD%"
    CD /D "%~dp0"
:--------------------------------------    

echo\
echo Adding alias for local browser in hosts . . .

SET NEWLINE=^& echo.

FIND /C /I "stlbitzbox.local" %WINDIR%\system32\drivers\etc\hosts
IF %ERRORLEVEL% NEQ 0 ECHO %NEWLINE%^127.77.78.88 stlbitzbox.local>>%WINDIR%\System32\drivers\etc\hosts

echo\
echo Adding server startup key to registry . . .

echo\
echo Attempting to add registry keys for all users . . .

reg add HKLM\Software\Microsoft\Windows\CurrentVersion\Run /v STLBitzBoxServer /d "%CD%\STLBitzBoxServer.exe"

echo\
echo Attempting to add registry keys for current user . . .

reg add HKCU\Software\Microsoft\Windows\CurrentVersion\Run /v STLBitzBoxServer /d "%CD%\STLBitzBoxServer.exe"

exit