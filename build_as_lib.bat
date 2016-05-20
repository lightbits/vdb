@echo off
if not exist "bin" mkdir bin
pushd bin
cl -c -nologo -Oi ../gdebug.cpp
lib -nologo gdebug.obj
popd
