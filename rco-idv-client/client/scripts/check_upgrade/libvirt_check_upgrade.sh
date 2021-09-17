#!/bin/bash

pool_name=$1
server_ip=$2
package_name=libvirt
version=$3
update_pool=$4

main_version="`cat /etc/issue | head -n 1 | cut -d _ -f 2 | cut -d V -f 2 | cut -d . -f 1`"
second_version="`cat /etc/issue | head -n 1 | cut -d _ -f 2 | cut -d V -f 2 | cut -d . -f 2`"
os_version_head=`cat /etc/issue | head -n 1 | sed -n '/^IDV-RainOS_*/p'`

ret=0

if [ -n "$os_version_head" ] ;then
    echo "check upgrade libivrt.(new os)"
    /etc/RCC_Client/scripts/rcc_upgrade.sh $pool_name $server_ip $package_name $version $update_pool
    ret=$?
elif [ -n $main_version -a -n $second_version ] ;then
    if [ $main_version -eq 3 -a $second_version -ge 1 ] || [ $main_version -gt 3 ] ;then
        echo "check upgrade libivrt."
        /etc/RCC_Client/scripts/rcc_upgrade.sh $pool_name $server_ip $package_name $version $update_pool
        ret=$?
    fi    
else
    echo "This system version does not need to upgrade libvirt. "    
fi

exit $ret
