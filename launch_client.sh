#!/bin/sh
export LSAN_OPTIONS=suppressions=../lsan_suppressions.txt
export DEBUG=1
cd build
./craft