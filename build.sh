#!/bin/bash

mkdir -p cmake-build-$1
rm -rf cmake-build-$1/*
if [ $2 == "gcc" ]; then
  CC=/usr/bin/gcc CXX=/usr/bin/g++ cmake -DCMAKE_BUILD_TYPE=$1 -S . -B cmake-build-$1
elif [ $2 == "clang" ]; then
  CC=/usr/bin/clang CXX=/usr/bin/clang++ cmake -DCMAKE_BUILD_TYPE=$1 -S . -B cmake-build-$1
else
  echo "Unknown compiler, use gcc or clang"
  exit 1
fi
cmake --build cmake-build-$1
