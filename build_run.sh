#!/bin/sh

make clean
make -j $(nproc)
./fspdplusplus
