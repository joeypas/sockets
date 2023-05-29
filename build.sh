#!/bin/bash

mkdir cppcoro/build && cd cppcoro/build

cmake -DCMAKE_INSTALL_PREFIX="../../" ..

make install

cd ../../

mkdir snappy/build && cd snappy/build

cmake -DCMAKE_INSTALL_PREFIX="../../" ..

make install

cd ../../

mkdir build && cd build

cmake ..

make
