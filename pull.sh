#!/bin/sh
git pull
cd Debug/
make clean
make
cp freecooling2 ..
cd ..
