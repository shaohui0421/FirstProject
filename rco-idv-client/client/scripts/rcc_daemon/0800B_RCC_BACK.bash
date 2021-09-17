#!/bin/bash
#

#[ -e /opt/lessons/vm_data_disk_create.sh ] && /opt/lessons/vm_data_disk_create.sh  ## del after 20170308

tch_client_daemon=/usr/local/bin/tch_client_daemon.sh
client_daemon_process=`ps -ef | grep tch_client_daemon.sh | grep -v grep | awk '{print $2}'`
[ -z "$client_daemon_process" ] && [ -f "$tch_client_daemon" ] && sh $tch_client_daemon &
