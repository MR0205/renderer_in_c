@echo off
if not defined DevEnvDir (
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat" > nul
)
pushd D:\\c_proj\build
cl /Zi D:/c_proj/src/main.cpp User32.lib Gdi32.lib
popd
