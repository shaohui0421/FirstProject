#!/bin/bash
#

#post-upgrade software version by running this script
#return:
#	0:success
#	1:upgrade failed

pool_name=$1
server_ip=$2
version=$3
update_pool=$4

SCRIPT_DIR=/etc/RCC_Client/scripts/post_upgrade
LOG_FILE=/var/log/rcc_client.log

for script_file in $SCRIPT_DIR/*.sh
do
    if [ -f $script_file ]; then
        bash $script_file $pool_name $server_ip $version $update_pool
        ret=$?
        if [ $ret -eq 0 ] || [ $ret -eq 1 ]; then
            echo "$script_file excute success" >> $LOG_FILE
        else
            echo "$script_file excute fail, ret = $ret" >> $LOG_FILE
            exit 1
        fi
    fi
done

#complete successful
exit 0
