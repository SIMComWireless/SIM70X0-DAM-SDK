@echo off
setlocal enabledelayedexpansion
set CLEAN=0
set PROJ_NAME=app
for %%i in (%*) do (
  if "!PROJ_TYPE!"=="1" (
    set "PROJ_NAME=%%i"
    set PROJ_TYPE=0
  )else (
    if "%%i"=="-c" (
      set "CLEAN=1"
    )else (
      if "%%i"=="-p" (
        set PROJ_TYPE=1
      )
    )
  )
)

set CUR_PATH=%~dp0
set TOOL_PATH=%CUR_PATH%\tools
set PATH=%TOOL_PATH%;%PATH%
set SENSOR_ELF_OUTPUT_PATH=bin
set DEMO_APP_OUTPUT_PATH=.\src\%PROJ_NAME%\build
set BUILD_LOG_FILE=build.log

if not exist %SENSOR_ELF_OUTPUT_PATH% (
  mkdir %SENSOR_ELF_OUTPUT_PATH%
)

if not exist %DEMO_APP_OUTPUT_PATH% (
  mkdir %DEMO_APP_OUTPUT_PATH%
)

if exist %BUILD_LOG_FILE% (
  del %BUILD_LOG_FILE%
)

rem if "%1%"=="-c" (
if "%CLEAN%" == "1" (
  echo "Cleaning...%SENSOR_ELF_OUTPUT_PATH%"
  rd /s /q %SENSOR_ELF_OUTPUT_PATH%
  echo "Cleaning...%DEMO_APP_OUTPUT_PATH%"
  rd /s /q %DEMO_APP_OUTPUT_PATH%
  
  echo "Done."
  pause 
  exit /b
)



make all OS=WINDOWS COMP=LLVM PROJ=%PROJ_NAME%
@echo on
pause 

