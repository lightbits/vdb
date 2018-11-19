:: Build for Visual Studio compiler. Run your copy of vcvars32.bat or vcvarsall.bat to setup command-line compiler.
:: Place a copy of SDL2 for your platform (x86 or x64) in the directory of your final linked executable
:: Prebuilt binaries can be found in src/sdl/
@echo off

:: Create a .build directory (hidden)
if not exist "lib" mkdir lib
pushd lib

:: -Od: Turns off optimizations and speeds up compilation
:: -Zi: Generates debug symbols
:: -WX: Treat warnings as errors
:: -MD: Use DLL run-time library
set INC=-I..\include\ -I..\include\vdb\ -I..\include\sdl\
set CF=%INC% -Zi -nologo -O2 -WX -W3 -wd4100 -wd4189 -wd4996 -wd4055
:: set CF=%INC% -Zi -nologo -O2 -WX -W3 -wd4100 -wd4189 -wd4996 -wd4055

:: -subsystem:console: Open a console
:: -debug: Create debugging information into .pdb
set SDLLIB=..\src\sdl\vc2010-x86\SDL2.lib ..\src\sdl\vc2010-x86\SDL2main.lib
set LF=%SDLLIB% opengl32.lib user32.lib gdi32.lib shell32.lib

cl ..\src\vdb.cpp -c %CF%
lib vdb.obj %LF% -nologo -out:vdb.lib

popd
