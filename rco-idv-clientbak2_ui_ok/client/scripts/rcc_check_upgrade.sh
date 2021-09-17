#!/bin/bash
#

#check-upgrade software version by running this script
#return:
#	0:success

pool_name=$1
server_ip=$2
version=$3
update_pool=$4

SCRIPT_DIR=/etc/RCC_Client/scripts/check_upgrade
LOG_FILE=/var/log/rcc_client.log

for script_file in $SCRIPT_DIR/*.sh
do
    if [ -f $script_file ]; then
        bash $script_file $pool_name $server_ip $version $update_pool
        ret=$?
        if [ $ret -eq 0 ] || [ $ret -eq 1 ]; then
            #ret: 0 means upgrade success, 1 means no need upgrade
            echo "$script_file excute success" >> $LOG_FILE
        else
            #if upgrade failed, continue execute script_file
            echo "$script_file excute fail, ret = $ret" >> $LOG_FILE
        fi
    fi
done

#complete successful
exit 0
