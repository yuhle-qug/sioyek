@echo off
setlocal

:: ============================================================================
:: Sioyek Enhanced Build & Package Script for Windows
:: ============================================================================
:: Automates finding tools, building the app, and packaging the release.
:: Designed to be run locally or by a CI/CD system like GitHub Actions.
:: ============================================================================

:: --- Step 1: Find Visual Studio Build Environment ---
echo Searching for Visual Studio build environment...
set VSWHERE_PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
if not exist "%VSWHERE_PATH%" (
    echo ERROR: Visual Studio is required. Please install VS 2019 or later
    echo with the "Desktop development with C++" workload.
    echo Download: https://visualstudio.microsoft.com/downloads/
    goto :Error
)
for /f "usebackq tokens=*" %%i in (`"%VSWHERE_PATH%" -latest -property installationPath`) do set VS_PATH=%%i
set VCVARS_PATH=%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat
if not exist "%VCVARS_PATH%" (
    echo ERROR: "Desktop development with C++" workload not found in Visual Studio.
    goto :Error
)
echo Found Visual Studio. Configuring environment...
call "%VCVARS_PATH%" x64

:: --- Step 2: Find Qt Installation ---
echo.
echo Searching for Qt installation (qmake.exe)...
set QMAKE_PATH=
:: First, check environment variable, useful for CI
if defined Qt5_Dir (
    set "QMAKE_PATH=%Qt5_Dir%\bin\qmake.exe"
)
if not exist "%QMAKE_PATH%" (
    :: Fallback to searching common paths
    for /f "delims=" %%F in ('where /r "C:\Qt" qmake.exe 2^>nul') do (
        if not defined QMAKE_PATH set "QMAKE_PATH=%%F"
    )
)
if not defined QMAKE_PATH (
    echo ERROR: Could not find qmake.exe. Please install Qt (MSVC version).
    echo Download: https://www.qt.io/download
    goto :Error
)
echo Found qmake at: %QMAKE_PATH%

:: --- Step 3: Pre-build dependencies ---
echo.
echo Building dependencies (MuPDF and ZLib)...
cd mupdf\platform\win32\
msbuild mupdf.sln /property:Configuration=Release /nologo /v:q
if %errorlevel% neq 0 ( echo ERROR: Failed to build MuPDF. & cd ..\..\.. & goto :Error )
cd ..\..\..
cd zlib
nmake -f win32/makefile.msc /nologo
if %errorlevel% neq 0 ( echo ERROR: Failed to build ZLib. & cd .. & goto :Error )
cd ..
echo Dependencies built successfully.

:: --- Step 4: Run the Main Build Process ---
echo.
echo Starting the Sioyek application build...
"%QMAKE_PATH%" pdf_viewer_build_config.pro -spec win32-msvc
if %errorlevel% neq 0 ( echo ERROR: qmake failed. & goto :Error )
nmake /f Makefile.Release
if %errorlevel% neq 0 ( echo ERROR: nmake compilation failed. & goto :Error )

:: --- Step 5: Package the release ---
echo.
echo Build successful. Packaging the application...
set RELEASE_DIR=sioyek-release-windows
rmdir /S /Q %RELEASE_DIR%
mkdir %RELEASE_DIR%
copy release\sioyek.exe %RELEASE_DIR%\sioyek.exe
copy pdf_viewer\keys.config %RELEASE_DIR%\keys.config
copy pdf_viewer\prefs.config %RELEASE_DIR%\prefs.config
xcopy /E /I /Q pdf_viewer\shaders %RELEASE_DIR%\shaders\
copy tutorial.pdf %RELEASE_DIR%\tutorial.pdf

echo Deploying Qt dependencies...
windeployqt %RELEASE_DIR%\sioyek.exe
if %errorlevel% neq 0 ( echo WARNING: windeployqt failed. Some DLLs might be missing. )

echo Copying required runtime DLLs...
copy windows_runtime\vcruntime140_1.dll %RELEASE_DIR%\vcruntime140_1.dll
copy windows_runtime\libssl-1_1-x64.dll %RELEASE_DIR%\libssl-1_1-x64.dll
copy windows_runtime\libcrypto-1_1-x64.dll %RELEASE_DIR%\libcrypto-1_1-x64.dll

echo Packaging complete. Files are in '%RELEASE_DIR%'.

:: --- Success ---
echo.
echo ==================================================
echo  PROCESS COMPLETE!
echo ==================================================
echo A ready-to-use version is located in the
echo '%RELEASE_DIR%' folder.
echo.
goto :End

:Error
echo.
echo ==================================================
echo  BUILD FAILED
echo ==================================================
echo Please review the error messages above.
echo.

:End
endlocal

