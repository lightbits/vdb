@echo off

REM Build library for Visual Studio compiler. Run your copy of vcvars32.bat or vcvarsall.bat to setup command-line compiler.
REM To link with vdb, see test.cpp for an example

if not exist "lib" mkdir lib
pushd lib

set INC=-I..\include\ -I..\include\vdb\ -I..\include\sdl\ -I..\lib\freetype\include
set CF=%INC% -Zi -Oi -fp:fast -nologo -O2 -WX -W3 -wd4100 -wd4189 -wd4996 -wd4055
set LF=

REM If you want to statically link auxilliary libraries
REM set SDLLIB=..\lib\sdl\vc2010-x86\SDL2.lib ..\lib\sdl\vc2010-x86\SDL2main.lib
REM set LF=%SDLLIB% opengl32.lib user32.lib gdi32.lib shell32.lib

cl %CF% -c ..\src\vdb.cpp
lib *.obj %LF% -nologo -out:vdb.lib

popd
