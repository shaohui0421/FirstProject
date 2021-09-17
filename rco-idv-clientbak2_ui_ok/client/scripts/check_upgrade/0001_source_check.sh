#!/bin/bash

pool_name=$1
server_ip=$2
version=$3
update_pool=$4
package_deb="bittornado qemu vmmanager"
package_cbb="estserver estclient usb"
ret=0

echo "check udpgrade source begin"
echo "cmd: $0 $*"

for package_name in $package_deb
do
    if [ -z "$(dpkg -l | grep ${package_name} | grep ${version} | grep ii)" ] ;then
        echo "$package_name install failed, need update source.list"
        ret=1
        break
    fi
done

for package_name in $package_cbb
do
    if [ -z "$(dpkg -l | grep -w ${package_name} | grep ii)" ] ;then
        echo "$package_name install failed, need update source.list"
        ret=1
        break
    fi
done

if [ $ret -eq 0 ] ;then
    echo "package install success, not update source.list"
    exit 0
fi

rm -f /var/lib/apt/lists/*Packages
rm -f /var/lib/apt/lists/partial/*.gz
sourcelist_content="deb http://${server_ip} ${pool_name}/"
echo "${sourcelist_content}" > /etc/apt/sources.list
apt-get update

exit 0
