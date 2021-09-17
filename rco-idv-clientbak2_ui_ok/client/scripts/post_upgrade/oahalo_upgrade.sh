#!/bin/bash

pool_name=$1
server_ip=$2
oahalo_version="0.1"
update_pool=$4
log_file=/tmp/oahalo_upgrade.log
pkg_name_oahalo=oahalo

ret=0

echo "upgrade halo modules begin" >> $log_file 2>&1
echo "cmd: $0 $*" >> $log_file 2>&1

echo "upgrade oaHalo" >> $log_file 2>&1
/etc/RCC_Client/scripts/rcc_upgrade.sh $pool_name $server_ip $pkg_name_oahalo $oahalo_version $update_pool
ret=$?
echo "upgrade oaHalo ret $ret" >> $log_file 2>&1

echo "upgrade halo modules end" >> $log_file 2>&1

exit $ret
