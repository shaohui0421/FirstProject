#!/bin/bash

pool_name=$1
server_ip=$2
version=$3
update_pool=$4

package_bt=bittornado
package_qemu=qemu
package_vmmanager=vmmanager

ret=0

echo "check upgrade deb modules begin"
echo "cmd: $0 $*"

test -n "`dpkg -l |grep spice.rain`" && dpkg -P spice.rain
test -n "`dpkg -l |grep rainconfig`" && dpkg -P rainconfig

echo "upgrade bittornado"
/etc/RCC_Client/scripts/rcc_upgrade.sh $pool_name $server_ip $package_bt $version $update_pool
ret=$?
echo "upgrade bittornado ret $ret"

echo "upgrade qemu"
/etc/RCC_Client/scripts/rcc_upgrade.sh $pool_name $server_ip $package_qemu $version $update_pool
ret=$?
echo "upgrade qemu ret $ret"

echo "upgrade vmmanager"
/etc/RCC_Client/scripts/rcc_upgrade.sh $pool_name $server_ip $package_vmmanager $version $update_pool
ret=$?
echo "upgrade vmmanager ret $ret"

echo "check upgrade deb modules end"

exit $ret
