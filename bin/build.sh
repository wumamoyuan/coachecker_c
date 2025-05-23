#!/bin/bash

# Clean built source files
if [[ $1 = 'clean' ]]; then
	echo "Cleaning built ..."
	cd ../ccl/src
	make clean
	cd ../../
	rm -rf lib
    rm -rf build
	exit 1
fi

echo "Compiling source files..."
# Build ccl library first
cd ../ccl/src
make libccl.a
cd ../../
mkdir lib
cp ccl/src/libccl.a lib

#Build coachecker
mkdir build
cd build
cmake ..
make clean
make

echo "Done."
echo "You can execute the coachecker in bin directory"


