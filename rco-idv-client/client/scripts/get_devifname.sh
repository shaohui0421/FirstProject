#!/bin/sh
#

RCCSYS_SCRIPTDIR=/etc/rc_script
CLIENT_SCRIPTDIR=/etc/RCC_Client/scripts
. $CLIENT_SCRIPTDIR/idv_function.sh

def_netif=`$RCCSYS_SCRIPTDIR/getdefaultnetifname.sh`
dev_ifname=`brctl show | awk '{if ($1 == "'$def_netif'") print $4}'`
if [ $? -ne 0 ]; then
    idv_logging "Failed to get real physical netccard of bridge $def_netif"
    exit 1
elif [ -z "$dev_ifname" ]; then
    idv_logging "bridge $def_netif not found"
    exit 2
fi
echo $dev_ifname
exit 0
