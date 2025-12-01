@echo off

pushd %~dp0

mkdir ..\build-em

@REM xcopy /E /I /Y "..\assets\" "..\build-em\assets\"

cd ..\build-em

IF "%1"=="async" (
    echo using asyncify
    call emcmake cmake .. -DPLATFORM=Web -DUSE_SDL3=ON -DUSE_SDL2=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXECUTABLE_SUFFIX=".html" -DASYNCIFY=1
) ELSE IF "%1"=="debug" (
    call emcmake cmake .. -DPLATFORM=Web -DUSE_SDL3=ON -DUSE_SDL2=OFF -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXECUTABLE_SUFFIX=".html" -DASYNCIFY=1
) ELSE IF NOT "%1"=="" (
    echo Unknown argument: %1
    exit /b 1
) ELSE (
    call emcmake cmake .. -DPLATFORM=Web -DUSE_SDL3=ON -DUSE_SDL2=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXECUTABLE_SUFFIX=".html" -DASYNCIFY=1
)

call emmake make