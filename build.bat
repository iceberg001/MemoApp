@echo off
setlocal EnableDelayedExpansion

REM ============================================================
REM  MemoApp one-click build script
REM  Usage:
REM    build.bat              Build Release (default), VS2022 x64
REM    build.bat Debug        Build Debug
REM    build.bat clean        Delete build dir and full reconfigure
REM ============================================================

REM -- Clear duplicated proxy env vars (breaks MSBuild/.NET) --
set "HTTPS_PROXY="
set "https_proxy="
set "HTTP_PROXY="
set "http_proxy="
set "ALL_PROXY="
set "all_proxy="

set "BUILD_TYPE=%~1"
if "%BUILD_TYPE%"=="" set "BUILD_TYPE=Release"

set "ROOT=%~dp0"
pushd "%ROOT%"

echo ============================================================
echo  MemoApp  BuildType=%BUILD_TYPE%  VS2022 x64 (v143)
echo ============================================================

REM -- [0/5] Pre-check: close running app or the final copy fails --
tasklist /FI "IMAGENAME eq MemoApp.exe" 2>nul | findstr /I /C:"MemoApp.exe" >nul
if not errorlevel 1 (
    echo.
    echo  [ERROR] 检测到 MemoApp.exe 正在运行！
    echo          程序运行时会锁定 out\MemoApp.exe，导致编译产物无法复制。
    echo          请先退出程序（托盘图标右键退出），再重新运行本脚本。
    echo.
    popd
    exit /b 1
)

REM -- [1/5] Package skin.zip (MUST use forward-slash entries) --
echo [1/5] Packaging skin.zip ...
python -c "import zipfile,os;src=r'%ROOT%resources\skin';z=zipfile.ZipFile(r'%ROOT%resources\skin.zip','w',zipfile.ZIP_DEFLATED);[z.write(os.path.join(r,f),os.path.relpath(os.path.join(r,f),src).replace(os.sep,'/')) for r,d,fs in os.walk(src) for f in fs];z.close()"
if errorlevel 1 (
    echo [ERROR] skin.zip packaging failed - python is required
    popd & exit /b 1
)

REM -- [2/5] Clean if requested --
if /i "%BUILD_TYPE%"=="clean" (
    echo [2/5] Cleaning build directory ...
    if exist build rmdir /s /q build
    set "BUILD_TYPE=Release"
)
if not exist build mkdir build

cd build

REM -- [3/5] CMake configure --
echo.
echo [3/5] CMake configure ...
if not exist CMakeCache.txt (
    cmake -G "Visual Studio 17 2022" -A x64 -T v143 ..
) else (
    cmake ..
)
if errorlevel 1 (
    echo [ERROR] CMake configure failed
    popd & exit /b 1
)

REM -- [4/5] Build (DuiLib static + sqlite3 + app, single exe) --
echo.
echo [4/5] Building (this may take a few minutes on first run) ...
cmake --build . --config %BUILD_TYPE% --parallel 4
if errorlevel 1 (
    echo [ERROR] Build failed
    popd & exit /b 1
)

REM -- [5/5] Output --
echo.
echo [5/5] Output check ...
if not exist "%ROOT%out" mkdir "%ROOT%out"
if exist "%CD%\src\%BUILD_TYPE%\MemoApp.exe" (
    copy /y "%CD%\src\%BUILD_TYPE%\MemoApp.exe" "%ROOT%out\MemoApp.exe" >nul
    if errorlevel 1 (
        echo [ERROR] 复制 MemoApp.exe 到 out\ 失败，请确认程序已退出后重试。
        popd & exit /b 1
    )
) else (
    echo [ERROR] MemoApp.exe not found
    popd & exit /b 1
)

echo.
echo ============================================================
echo  [OK] Build finished:  %ROOT%out\MemoApp.exe
echo  Run:  directly double-click out\MemoApp.exe
echo        (data\reports.db is auto-created next to the exe)
echo ============================================================
popd
endlocal
