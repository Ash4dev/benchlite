#!/bin/bash

# TODO: take CPUs to use as input from user

sudo cpupower -c 2,4,6,8,10 frequency-set -g performance

# Build the executable
cmake --build ./build

# thread priority change: privileged operation
sudo setcap cap_sys_nice=eip ./build/bin/main

# Run the executable
./build/bin/main
EXIT_CODE=$?

sudo cpupower -c 2,4,6,8,10 frequency-set -g powersave
exit $EXIT_CODE
