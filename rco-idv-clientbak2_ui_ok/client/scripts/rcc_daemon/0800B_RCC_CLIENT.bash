#!/bin/bash
#

rcc_daemon_script=/usr/local/bin/rcc_client_daemon.sh
client_daemon_process=`ps -ef | grep rcc_client_daemon.sh | grep -v grep | awk '{print $2}'`
[ -z "$client_daemon_process" ] && [ -f "$rcc_daemon_script" ] && bash $rcc_daemon_script &
