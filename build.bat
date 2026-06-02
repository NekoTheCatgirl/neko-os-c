@echo off

docker image inspect iso-builder >nul 2>&1
if %errorlevel% neq 0 (
    echo Building Docker image...
    docker build -t iso-builder .
) else (
    echo Docker image already exists, skipping build.
)

if not exist build mkdir build

echo Configuring with cmake
docker run --rm -v %cd%:/src -w /src iso-builder cmake -S . -B build
if %errorlevel% neq 0 exit /b %errorlevel%

echo Building ISO...
docker run --rm -v %cd%:/src -w /src iso-builder cmake --build build --target iso
if %errorlevel% neq 0 exit /b %errorlevel%

echo Done!