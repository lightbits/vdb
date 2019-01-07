@echo off

REM Build library for Visual Studio compiler. Run your copy of vcvars32.bat or vcvarsall.bat to setup command-line compiler.
REM To link with vdb, see test.cpp for an example

if not exist "lib" mkdir lib
pushd lib

set INC=-I..\include\ -I..\include\vdb\ -I..\include\sdl\ -I..\lib\freetype\include
set CF=%INC% -Zi -nologo -Od -WX -W3 -wd4100 -wd4189 -wd4996 -wd4055 -DVDB_DEBUG
set LF=

REM If you want to statically link auxilliary libraries
REM set SDLLIB=..\lib\sdl\vc2010-x86\SDL2.lib ..\lib\sdl\vc2010-x86\SDL2main.lib
REM set LF=%SDLLIB% opengl32.lib user32.lib gdi32.lib shell32.lib

cl ..\src\vdb.cpp -c %CF%
lib vdb.obj %LF% -nologo -out:vdb.lib

popd
