#!/bin/bash

set -e

if [ ! -d "build" ]; then
  mkdir build
fi

cd build

echo "--- Configuring ---"
cmake .. -DCMAKE_BUILD_TYPE=Release

echo "--- Building ---"
cmake --build . -j$(nproc) 

echo "--- Running Game ---"
./AlgorithmicArena
