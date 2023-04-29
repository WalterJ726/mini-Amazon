#!/bin/bash

make clean

make

# ls -al

echo "get ups host addr"
echo $UPS_HOST_ADDR

./daemon $UPS_HOST_ADDR

# while true
# do
#     sleep 1
# done