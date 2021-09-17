#!/bin/bash

pool_name=$1
server_ip=$2
version=$3
update_pool=$4
log_file=/tmp/oahalo_upgrade.log
pkg_name_estserver=oahalo

ret=0

echo "downgrade halo modules begin" >> $log_file 2>&1
echo "cmd: $0 $*" >> $log_file 2>&1
 
echo "downgrade oaHalo" >> $log_file 2>&1
install_result=$(dpkg -l | awk '{if ($2 == "oahalo") print $1}')
if [ -n "$install_result" ]; then
    dpkg -P $pkg_name_estserver
    ret=$?
    echo "downgrade oaHalo ret $ret" >> $log_file 2>&1
fi

echo "downgrade halo modules end" >> $log_file 2>&1

exit $ret
