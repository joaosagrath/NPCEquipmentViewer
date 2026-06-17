@echo off
setlocal
cd /d "%~dp0"

if "%VCPKG_ROOT%"=="" (
    echo ERRO: a variavel de ambiente VCPKG_ROOT nao esta configurada.
    echo Exemplo: C:\Ferramentas\vcpkg
    exit /b 1
)

if not exist "%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" (
    echo ERRO: VCPKG_ROOT aponta para uma pasta invalida: "%VCPKG_ROOT%"
    exit /b 1
)

where cmake >nul 2>nul
if errorlevel 1 (
    echo ERRO: CMake nao foi encontrado no PATH.
    exit /b 1
)

where ninja >nul 2>nul
if errorlevel 1 (
    echo ERRO: Ninja nao foi encontrado no PATH.
    exit /b 1
)

cmake --preset release
if errorlevel 1 exit /b 1

cmake --build --preset release
if errorlevel 1 exit /b 1

echo.
echo Compilacao concluida.
echo Pacote gerado em: build\release\package
endlocal
