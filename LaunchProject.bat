@echo off
echo Checking if Unreal Editor is already running...
tasklist /FI "IMAGENAME eq UnrealEditor.exe" 2>NUL | find /I /N "UnrealEditor.exe">NUL
if "%ERRORLEVEL%"=="0" (
    echo Unreal Editor is already running.
) else (
    echo Launching MCPGameProject...
    start "" "D:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" "D:\UGit\unreal-mcp\MCPGameProject\MCPGameProject.uproject"
    echo Unreal Editor launched successfully!
) 