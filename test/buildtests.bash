#!/bin/bash

# Download asm files.
echo "BUILDTESTS: Downloading asm files..."
echo "------------------------------------"
wget https://raw.githubusercontent.com/rpendleton/lc3-2048/master/2048.asm

# Assemble
echo "BUILDTESTS: Assembling obj files..."
echo "-----------------------------------"
lc3as 2048.asm

# Clean
echo "BUILDTESTS: Cleaning..."
echo "-----------------------"
rm ./*.asm ./*.sym

echo "Done."