#!/bin/bash

pool_name=$1
version=$3
package_name=rcos-abslayer

function is_update()
{
    local loc_ver=$1
    local max_ver=$(echo "$@" | tr " " "\n" | sort -rV | head -1)
    if [[ "$loc_ver" == "$max_ver" ]]; then
        echo 0
    else
        echo 1
    fi
}

echo "pre:update rcos-abslayer begin"
echo "cmd: $0 $*"

local_version=$(dpkg-query -W -f='${Version}' $package_name)
if [ -z "$local_version" ]; then
    echo "pre:local don't exist rcos-abslayer, upgrade rcos-abslayer by depends"
    exit 0
fi

cache_version=$(apt-cache show $package_name | grep Version: | grep -v $local_version | head -1 | awk '{print $2}')
echo "pre:local version=$local_version, cache version=$cache_version"
if [ -z "$cache_version" ]; then
    echo "pre:remote version no exist or already laster, no need downgrade"
    exit 0
fi

#download rcd deb
apt-get -f --allow-downgrades -y --allow-unauthenticated -d install rcd=$version
if [ $? -ne 0 ]; then
    echo "pre:download rcd package failed, ret:$ret"
    exit $?
fi

#download depends deb
apt-get -f --allow-downgrades -y --allow-unauthenticated -d install $package_name=$cache_version
if [ $? -ne 0 ]; then
    echo "pre:download $package_name package failed, ret:$ret"
    exit $?
fi

#check is need update depends deb
if [ $(is_update $local_version $cache_version) -eq 0 ]; then
   echo "pre:degrade don't install $package_name"
   exit 0
fi

apt-get -f --allow-downgrades -y --allow-unauthenticated install $package_name=$cache_version
#check whether download is successful
upgrade_result=$(dpkg -l | grep ${package_name} | grep ${cache_version} | grep ii)
if [ -z "$upgrade_result" ];
then
    dpkg --configure -a
    apt-get install -f --allow-downgrades -y --allow-unauthenticated $package_name --reinstall
    echo "pre:upgrade $package_name ret $?"
    exit $?
fi

exit 0
