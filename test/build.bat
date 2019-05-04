@REM Build for Visual Studio compiler.
@REM Run your copy of vcvars32.bat or vcvarsall.bat to setup command-line compiler.
@REM Ensure that the environment variables SDL2_DIR and VDB_DIR are correct.
set INCLUDES=/I..\include
set SOURCES=test.cpp
set LIBS=/libpath:%SDL2_DIR%\lib\x86 /libpath:%VDB_DIR%\lib vdb.lib SDL2.lib SDL2main.lib opengl32.lib
cl /nologo /Zi /MD %INCLUDES% test.cpp /link %LIBS% /subsystem:console
