#!/bin/bash

pool_name=$1
server_ip=$2
ret=0

echo "pre:upgrade source list"
echo "cmd: $0 $*"

rm -f /var/lib/apt/lists/*Packages
rm -f /var/lib/apt/lists/partial/*.gz
sourcelist_content="deb http://${server_ip} ${pool_name}/"
echo "${sourcelist_content}" > /etc/apt/sources.list
apt-get update

exit 0
