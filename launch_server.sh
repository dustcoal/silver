#!/bin/sh
export LSAN_OPTIONS=suppressions=lsan_suppressions.txt
cd build
./server