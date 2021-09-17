#!/bin/bash

pool_name=$1
server_ip=$2
estserver_version="3.0-1"
estclient_version="3.0.0"
usb_version="0-1"
update_pool=$4
log_file=/tmp/est_upgrade.log
pkg_name_estserver=estserver
pkg_name_estclient=estclient
pkg_name_usb=usb

ret=0

echo "upgrade est modules begin" >> $log_file 2>&1
echo "cmd: $0 $*" >> $log_file 2>&1

echo "upgrade ESTServer" >> $log_file 2>&1
/etc/RCC_Client/scripts/rcc_upgrade.sh $pool_name $server_ip $pkg_name_estserver $estserver_version $update_pool
ret=$?
echo "upgrade ESTServer ret $ret" >> $log_file 2>&1

echo "upgrade ESTClient" >> $log_file 2>&1
/etc/RCC_Client/scripts/rcc_upgrade.sh $pool_name $server_ip $pkg_name_estclient $estclient_version $update_pool
ret=$?
echo "upgrade ESTClient ret $ret" >> $log_file 2>&1

echo "upgrade usb" >> $log_file 2>&1
/etc/RCC_Client/scripts/rcc_upgrade.sh $pool_name $server_ip $pkg_name_usb $usb_version $update_pool
ret=$?
echo "upgrade usb ret $ret" >> $log_file 2>&1

echo "upgrade est modules end" >> $log_file 2>&1

exit $ret
