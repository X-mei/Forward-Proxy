#! /bin/sh
make clean
make
echo 'Starting proxy server...'
./proxy &
while true ; do continue ; done