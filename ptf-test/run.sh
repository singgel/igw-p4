#!/bin/bash

#python mytests/switch_initialize.py

echo "Run the test case that read/write directly to Database"
ptf --test-dir mytests  --platform-dir platforms --relax -P remote 

echo "Run the test case done ........"

