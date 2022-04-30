#!/bin/bash
make clean
make
echo 'Starting Server...'
./proxy &
while true ; do continue ; done
