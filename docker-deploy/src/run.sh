#! /bin/sh
make clean
make
echo 'Starting proxy server...'
./test &
while true ; do continue ; done