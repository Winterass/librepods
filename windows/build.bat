@echo off
REM Build script for LibrePods Windows

echo ===============================================
echo LibrePods for Windows - Build Script
echo ===============================================
echo.

REM Check if dotnet is installed
where dotnet >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: .NET SDK not found!
    echo Please install .NET 8.0 SDK from https://dotnet.microsoft.com/download
    pause
    exit /b 1
)

echo Checking .NET version...
dotnet --version
echo.

REM Set configuration (Debug or Release)
set CONFIG=Release
if "%1" NEQ "" set CONFIG=%1

echo Building LibrePods in %CONFIG% configuration...
echo.

REM Clean previous builds
echo Cleaning previous builds...
dotnet clean -c %CONFIG%
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Clean failed!
    pause
    exit /b 1
)

REM Restore NuGet packages
echo Restoring NuGet packages...
dotnet restore
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Restore failed!
    pause
    exit /b 1
)

REM Build LibrePods.Core
echo.
echo Building LibrePods.Core...
dotnet build LibrePods.Core\LibrePods.Core.csproj -c %CONFIG%
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed for LibrePods.Core!
    pause
    exit /b 1
)

REM Build LibrePods.Windows
echo.
echo Building LibrePods.Windows...
dotnet build LibrePods.Windows\LibrePods.Windows.csproj -c %CONFIG%
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed for LibrePods.Windows!
    pause
    exit /b 1
)

REM Publish for distribution
echo.
echo Publishing for distribution...
dotnet publish LibrePods.Windows\LibrePods.Windows.csproj -c %CONFIG% -r win-x64 --self-contained false -o publish\win-x64
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Publish failed!
    pause
    exit /b 1
)

echo.
echo ===============================================
echo Build completed successfully!
echo.
echo Output location: publish\win-x64
echo ===============================================
echo.

pause
