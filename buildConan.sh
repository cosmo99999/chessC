#!/bin/bash

arg1=$1
if [[ $arg1 == release ]]; then
  conan install . --build=missing -pr release
elif [[ $arg1 == windows ]]; then
  conan install . --build=missing -pr windows_release
else
  conan install . --build=missing -pr debug
fi
