#!/usr/bin/env nix-shell
#! nix-shell -i bash --pure
#! nix-shell -p clang cmake git python lld zlib
#! nix-shell -I nixpkgs=https://github.com/NixOS/nixpkgs/archive/2a601aafdc5605a5133a2ca506a34a3a73377247.tar.gz

mkdir cppcoro/build && cd cppcoro/build

cmake -DCMAKE_INSTALL_PREFIX="../../" ..

make install -j

cd ../../

mkdir snappy/build && cd snappy

git submodule update --init

cd build

cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX="../../" ..

make install -j

cd ../../

mkdir build && cd build

cmake ..

make -j
