#!/bin/sh
#

CLIENT_SCRIPTDIR=/etc/RCC_Client/scripts
. $CLIENT_SCRIPTDIR/idv_function.sh

while :
do
    if ps -ef | grep openbox | grep -v grep > /dev/null; then
        # X server started, client continues running.
        idv_logging "X server has started, continue running client."
        sleep 0.5
        touch /tmp/client_start_over
        break
    else
        # X server has not started, just wait.
        sleep 0.1
    fi
done
