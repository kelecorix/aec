#!/bin/sh
git pull
cd Debug/
make clean
make
cp freecooling ..
cd ..

