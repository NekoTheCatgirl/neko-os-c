#!/usr/bin/env sh

if ! docker image inspect iso-builder > /dev/null 2>&1; then
  echo "Building Docker image..."
  docker build -t iso-builder .
else
  echo "Docker image already exists, skipping build."
fi

mkdir -p build

echo "Configuring with CMake..."
docker run --rm -v "${PWD}:/src" -w /src iso-builder cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-x86_64.cmake || exit 1

echo "Building ISO..."
docker run --rm -v "${PWD}:/src" -w /src iso-builder cmake --build build --target iso || exit 1

sudo chown -R $(id -u):$(id -g) build

echo "Done!"