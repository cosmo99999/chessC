#!/bin/bash

arg1=$1
if [[ $arg1 == release ]]; then
  cmake --preset conan-release
  cmake --build --preset conan-release
else
  cmake --preset conan-debug
  cmake --build --preset conan-debug
fi
