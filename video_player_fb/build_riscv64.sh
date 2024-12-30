#!/bin/bash

BuildDir="build"

if [ ! -d "$BuildDir" ]; then
    echo "正在创建目录 $BuildDir..."
    mkdir "$BuildDir"
    echo "目录 $BuildDir 创建成功。"
fi



cd $BuildDir
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../cmake/musl_riscv64.cmake ..
make -j16
make install