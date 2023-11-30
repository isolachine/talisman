#!/bin/bash

CURDIR=$(pwd)

cd ../../../

if [[ $1 == clean ]]; then
    make clean
fi

make

cd $CURDIR
