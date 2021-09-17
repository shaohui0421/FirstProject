#!/bin/sh

sleep 3

while true
do
    rcc=`pidof TCH_Client_daemon`
    if [ -z "$rcc" ]
    then
        now=`date +"%F %T"`
        echo "$now start TCH_Client_daemon" >> /var/log/RCC_Client.log
        /usr/local/bin/TCH_Client_daemon
        now=`date +"%F %T"`
        echo "$now restart TCH_Client_daemon" >> /var/log/RCC_Client.log
        sleep 1
    fi
    sleep 1
done
