#!/bin/bash

pool_name=$1
server_ip=$2
version=$3
update_pool=$4
log_file=/tmp/est_upgrade.log
pkg_name_estserver=estserver
pkg_name_estclient=estclient
pkg_name_usb=usb

ret=0

echo "downgrade est modules begin" >> $log_file 2>&1
echo "cmd: $0 $*" >> $log_file 2>&1
 
echo "downgrade ESTServer" >> $log_file 2>&1
install_result=$(dpkg -l | awk '{if ($2 == "estserver") print $1}')
if [ -n "$install_result" ]; then
    dpkg -P $pkg_name_estserver
    ret=$?
    echo "downgrade ESTServer ret $ret" >> $log_file 2>&1
fi

echo "downgrade ESTClient" >> $log_file 2>&1
install_result=$(dpkg -l | awk '{if ($2 == "estclient") print $1}')
if [ -n "$install_result" ]; then
    dpkg -P $pkg_name_estclient
    ret=$?
    echo "downgrade ESTClient ret $ret" >> $log_file 2>&1
fi

echo "downgrade usb" >> $log_file 2>&1
install_result=$(dpkg -l | awk '{if ($2 == "usb") print $1}')
if [ -n "$install_result" ]; then
    dpkg -P $pkg_name_usb
    ret=$?
    echo "downgrade usb ret $ret" >> $log_file 2>&1
fi

echo "downgrade est modules end" >> $log_file 2>&1

exit $ret
