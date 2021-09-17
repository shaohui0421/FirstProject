#!/bin/bash
#

rco_daemon_script=/etc/RCC_Client/scripts/client_daemon.sh
if [ -f "$rco_daemon_script" ] ; then
    $rco_daemon_script &
fi
