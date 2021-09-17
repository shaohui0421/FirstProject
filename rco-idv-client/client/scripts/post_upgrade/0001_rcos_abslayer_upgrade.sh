#!/bin/bash

pool_name=$1
server_ip=$2
package_name=rcos-abslayer

echo "post:degrade $package_name"
echo "cmd: $0 $*"

local_version=$(dpkg-query -W -f='${Version}' $package_name)
if [ -z "$local_version" ]; then
    echo "post:local don't exist rcos-abslayer, no need downgrade"
    exit 0
fi

cache_version=$(apt-cache show $package_name | grep Version: | grep -v $local_version | head -1 | awk '{print $2}')
if [ -z "$cache_version" ]; then
    echo "post:remote version no exist or already laster, no need downgrade"
    exit 0
fi

apt-get -f --allow-downgrades -y --allow-unauthenticated install $package_name=$cache_version
#check whether download is successful
upgrade_result=$(dpkg -l | grep ${package_name} | grep ${cache_version} | grep ii)
if [ -z "$upgrade_result" ];
then
    dpkg --configure -a
    apt-get install -f --allow-downgrades -y --allow-unauthenticated $package_name --reinstall
    echo "post:upgrade $package_name ret $?"
    exit $?
fi

exit 0
