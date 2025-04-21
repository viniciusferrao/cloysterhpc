#!/bin/sh

(cmake -S . -B build/Debug -DCMAKE_BUILD_TYPE=Debug -G Ninja  && cmake --build build/Debug && cd build/Debug && ctest) 
