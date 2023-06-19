#!/bin/bash

mkdir cppcoro/build && cd cppcoro/build

cmake -DCMAKE_INSTALL_PREFIX="../../" ..

make install -j

cd ../../

mkdir snappy/build && cd snappy

git submodule update --init

cd build

cmake -DCMAKE_INSTALL_PREFIX="../../" ..

make install -j

cd ../../

mkdir build && cd build

cmake ..

make -j
