#!/bin/sh

clear
make clean
make -j $(nproc)
./fspdplusplus
