#!/bin/sh
#

CLIENT_SCRIPTDIR=/etc/RCC_Client/scripts
. $CLIENT_SCRIPTDIR/idv_function.sh

idv_logging "X server stopping for VM starting ..."
# create file .xserver_forbidden, to notify rcc_startx.sh not to restart X server.
touch /tmp/.xserver_forbidden
sleep 0.5
pkill xinit
pkill Xorg
sleep 0.5
exit 0

# XXX: Should we check if X server exited?
while :
do
    xstate=`ps aux | grep -e xinit -e Xorg | grep -v grep | awk '{print $8}'`
    if [ -z "$xstate" ]; then
        # X server has exited, just return
        idv_logging "X server has been stopped."
        break
    elif echo $xstate | grep -q '^D'; then
        # X server deep sleep, we could only reboot system to solve the problem
        idv_logging "X server fell into deep sleep, system will reboot soon."
        touch /root/.rcc_exception_reboot
        sync
        sleep 1
        reboot
    else
        idv_logging "X server state: \n$xstate"
        idv_logging "Kill X server with SIGKILL again."
        pkill -9 xinit
        pkill -9 Xorg
        sleep 0.5
        continue
    fi
done

