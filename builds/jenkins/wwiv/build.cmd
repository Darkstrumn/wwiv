@rem **************************************************************************
@rem WWIV Build Script.
@rem
@rem Required Variables:
@rem WORKSPACE - git root directory
@rem BUILD_NUMBER - Jenkins Build number
@rem
@rem Installed Software:
@rem   7-Zip [C:\Program Files\7-Zip\7z.exe]
@rem   VS 2013 [C:\Program Files (x86)\Microsoft Visual Studio 12.0]
@rem   msbuild [in PATH, set by vcvarsall.bat]
@rem   sed [in PATH]
@rem 
@rem **************************************************************************

setlocal

del wwiv-*.zip

@if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" (
  call "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
)

@if exist "%ProgramFiles%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" (
  call "%ProgramFiles%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
)

set ZIP_EXE="C:\Program Files\7-Zip\7z.exe"
set RELEASE_ZIP=%WORKSPACE%\wwiv-win-5.3.%BUILD_NUMBER%.zip
set STAGE_DIR=%WORKSPACE%\staging
set WWIV_CMAKE_DIR=%WORKSPACE%\_build
echo =============================================================================
echo Workspace:       %WORKSPACE%         
echo Build Number:    %BUILD_NUMBER%
echo Archive:         %RELEASE_ZIP%
echo Staging Dir:     %STAGE_DIR%
echo WWIV CMake Root: %WWIV_CMAKE_DIR%
echo =============================================================================
echo Release Notes:
echo %RELEASE_NOTES%
echo =============================================================================


if not exist %WWIV_CMAKE_DIR% (
  echo Creating %WWIV_CMAKE_DIR%
  mkdir %WWIV_CMAKE_DIR%
)


@rem build Cryptlib 1st.
echo:
echo * Building Cryptlib (zip/unzip)
cd %WORKSPACE%\deps\cl342
msbuild crypt32.vcxproj  /t:Build /p:Configuration=Release /p:Platform=Win32 || exit /b

@rem Build BBS, wwivconfig, telnetserver
echo:
echo * Updating the Build Number in version.cpp
cd %WORKSPACE%\core

%SED% -i -e "s@.development@.%BUILD_NUMBER%@" version.cpp

echo:
echo * Building WWIV
cd %WWIV_CMAKE_DIR%
cmake -G "Ninja" -DCMAKE_BUILD_TYPE:STRING=MinSizeRel %WORKSPACE%   || exit /b
cmake --build .   || exit /b

@rem Build .NET Components
echo:
echo: * Building .NET Components
cd %WORKSPACE%\WWIV5TelnetServer\WWIV5TelnetServer
msbuild wwiv-server.csproj /t:Build /p:Configuration=Release /p:Platform=AnyCPU
cd %WORKSPACE%\windows-wwiv-update
msbuild windows-wwiv-update.csproj /t:Build /p:Configuration=Release /p:Platform=AnyCPU


@rem build WINS
echo:
echo * Building WINS
cd %WORKSPACE%\wins

msbuild exp\exp.vcxproj /t:Build /p:Configuration=Release /p:Platform=Win32 || exit /b
msbuild networkp\networkp.vcxproj /t:Build /p:Configuration=Release /p:Platform=Win32 || exit /b
msbuild news\news.vcxproj /t:Build /p:Configuration=Release /p:Platform=Win32 || exit /b
msbuild pop\pop.vcxproj /t:Build /p:Configuration=Release /p:Platform=Win32 || exit /b
msbuild pppurge\pppurge.vcxproj /t:Build /p:Configuration=Release /p:Platform=Win32 || exit /b
msbuild ppputil\ppputil.vcxproj /t:Build /p:Configuration=Release /p:Platform=Win32 || exit /b
msbuild qotd\qotd.vcxproj /t:Build /p:Configuration=Release /p:Platform=Win32 || exit /b
msbuild uu\uu.vcxproj /t:Build /p:Configuration=Release /p:Platform=Win32 || exit /b

@rem Building bits from the build tree.
@rem build InfoZIP Zip/UnZip
echo:
echo * Building INFOZIP (zip/unzip)
cd %WORKSPACE%\deps\infozip

msbuild unzip60\win32\vc8\unzip.vcxproj /t:Build /p:Configuration=Release /p:Platform=Win32 || exit /b
msbuild zip30\win32\vc6\zip.vcxproj /t:Build /p:Configuration=Release /p:Platform=Win32 || exit /b

cd %WORKSPACE%\
if not exist %STAGE_DIR% (
  echo Creating %STAGE_DIR%
  mkdir %STAGE_DIR%
)
del /q %STAGE_DIR%
del wwiv-*.zip

echo:
echo * Creating inifiles.zip
cd %WORKSPACE%\bbs\admin\inifiles
%ZIP_EXE% a -tzip -r %STAGE_DIR%\inifiles.zip *

echo:
echo * Creating data.zip
cd %WORKSPACE%\bbs\admin\data
%ZIP_EXE% a -tzip -r %STAGE_DIR%\data.zip *

echo:
echo * Creating gfiles.zip
cd %WORKSPACE%\bbs\admin\gfiles
%ZIP_EXE% a -tzip -r %STAGE_DIR%\gfiles.zip *

cd %WORKSPACE%\
echo:
echo * Copying BBS files to staging directory.
copy /v/y %WORKSPACE%\deps\cl342\Release\cl32.dll %STAGE_DIR%\cl32.dll || exit /b
copy /v/y %WWIV_CMAKE_DIR%\bbs\bbs.exe %STAGE_DIR%\bbs.exe || exit /b
@rem TODO: Make a workspace with these.
copy /v/y %WORKSPACE%\WWIV5TelnetServer\WWIV5TelnetServer\bin\Release\WWIVServer.exe %STAGE_DIR%\WWIVServer.exe || exit /b
copy /v/y %WORKSPACE%\windows-wwiv-update\bin\Release\wwiv-update.exe %STAGE_DIR%\wwiv-update.exe || exit /b
copy /v/y %WWIV_CMAKE_DIR%\wwivconfig\wwivconfig.exe %STAGE_DIR%\wwivconfig.exe || exit /b
copy /v/y %WWIV_CMAKE_DIR%\network\network.exe %STAGE_DIR%\network.exe || exit /b
copy /v/y %WWIV_CMAKE_DIR%\network1\network1.exe %STAGE_DIR%\network1.exe || exit /b
copy /v/y %WWIV_CMAKE_DIR%\network2\network2.exe %STAGE_DIR%\network2.exe || exit /b
copy /v/y %WWIV_CMAKE_DIR%\network3\network3.exe %STAGE_DIR%\network3.exe || exit /b
copy /v/y %WWIV_CMAKE_DIR%\networkb\networkb.exe %STAGE_DIR%\networkb.exe || exit /b
copy /v/y %WWIV_CMAKE_DIR%\networkc\networkc.exe %STAGE_DIR%\networkc.exe || exit /b
copy /v/y %WWIV_CMAKE_DIR%\networkf\networkf.exe %STAGE_DIR%\networkf.exe || exit /b
copy /v/y %WWIV_CMAKE_DIR%\wwivd\wwivd.exe %STAGE_DIR%\wwivd.exe || exit /b
copy /v/y %WWIV_CMAKE_DIR%\wwivutil\wwivutil.exe %STAGE_DIR%\wwivutil.exe || exit /b
copy /v/y %WORKSPACE%\bbs\admin\* %STAGE_DIR%\
copy /v/y %WORKSPACE%\bbs\admin\win32\* %STAGE_DIR%\

rem echo:
rem echo * Copying WINS files to staging area
set WINS=%WORKSPACE%\wins
copy /v/y %WINS%\exp\Release\exp.exe %STAGE_DIR%\exp.exe || exit /b
copy /v/y %WINS%\networkp\Release\networkp.exe %STAGE_DIR%\networkp.exe || exit /b
copy /v/y %WINS%\news\Release\news.exe %STAGE_DIR%\news.exe || exit /b
copy /v/y %WINS%\pop\Release\pop.exe %STAGE_DIR%\pop.exe || exit /b
copy /v/y %WINS%\pppurge\Release\pppurge.exe %STAGE_DIR%\pppurge.exe || exit /b
copy /v/y %WINS%\ppputil\Release\ppputil.exe %STAGE_DIR%\ppputil.exe || exit /b
copy /v/y %WINS%\qotd\Release\qotd.exe %STAGE_DIR%\qotd.exe || exit /b
copy /v/y %WINS%\uu\Release\uu.exe %STAGE_DIR%\uu.exe || exit /b

echo:
echo * Copying WINS sample filesto staging area
copy /v/y %WINS%\admin\* %STAGE_DIR%\

echo:
echo * Copying INFOZIP files to staging area
set INFOZIP=%WORKSPACE%\deps\infozip
dir %INFOZIP%\unzip60\win32\vc8\unzip__Win32_Release\unzip.exe
dir %INFOZIP%\zip30\win32\vc6\zip___Win32_Release\zip.exe
copy /v/y %INFOZIP%\unzip60\win32\vc8\unzip__Win32_Release\unzip.exe %STAGE_DIR%\unzip.exe
copy /v/y %INFOZIP%\zip30\win32\vc6\zip___Win32_Release\zip.exe %STAGE_DIR%\zip.exe


echo:
echo * Creating scripts.zip
cd %WORKSPACE%\bbs\admin\scripts
%ZIP_EXE% a -tzip -r %STAGE_DIR%\scripts.zip *

echo:
echo * Creating Regions
cd %WORKSPACE%\bbs\admin
%ZIP_EXE% a -tzip -r %STAGE_DIR%\zip-city.zip zip-city\*
%ZIP_EXE% a -tzip -r %STAGE_DIR%\regions.zip regions\*

echo:
echo * Creating build.nfo file
echo Build URL %BUILD_URL% > release\build.nfo
echo Build: 5.3.0.%BUILD_NUMBER% >> release\build.nfo

echo:
echo * Creating release archive: %RELEASE_ZIP%
cd %STAGE_DIR%
%ZIP_EXE% a -tzip -y %RELEASE_ZIP%

echo:
echo:
echo: **** SUCCESS ****
echo:
echo ** Archive File: %RELEASE_ZIP%
echo ** Archive contents:
%ZIP_EXE% l %RELEASE_ZIP%
endlocal

