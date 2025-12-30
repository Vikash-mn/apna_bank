#!/bin/bash

echo "Building Banking System..."
make

if [ $? -eq 0 ]; then
    echo "Build successful! Starting application..."
    ./banking_system
else
    echo "Build failed!"
    exit 1
fi