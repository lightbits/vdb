@echo off

REM Build library for Visual Studio compiler. Run your copy of vcvars32.bat or vcvarsall.bat to setup command-line compiler.
REM To link with vdb, see test.cpp for an example

if not exist "dll" mkdir dll
pushd dll

set INC=-I..\include\ -I..\include\vdb\ -I..\include\sdl\ -I..\lib\freetype\include
set CF=%INC% -nologo -Oi -fp:fast -O2 -WX -W3 -wd4100 -wd4189 -wd4996 -wd4055
set LF=-DEF:exports.def ..\lib\sdl\vc2010-x86\SDL2.lib ..\lib\sdl\vc2010-x86\SDL2main.lib C:\Programming\lib-vdb\lib\freetype\win32\freetype.lib opengl32.lib user32.lib gdi32.lib shell32.lib

cl %CF% -LD ..\src\vdb.cpp /link %LF%

popd
