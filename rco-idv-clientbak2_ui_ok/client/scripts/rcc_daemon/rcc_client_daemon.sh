#!/bin/bash
#

#/usr/bin/openbox &

/usr/local/bin/RCC_daemon &
#exec 2>/tmp/os.log

xset s 0 0
xset dpms 0 0 0

sh /usr/setdisplay.sh

tail -1 /var/log/Xorg.0.log | grep "Cannot get EDID information" && xrandr -s 1024x768

shutdown_flag=`cat /usr/shutdown_flag | grep 1`
if [ -n "$shutdown_flag" ]
then
    rm /usr/shutdown_flag
    shutdown -h now
fi

/usr/local/bin/RCC_Client &

while true
do
    # pulseaudio will be killed while factory_test running on IDV.
    video=`ps ax | grep pulseaudio| grep -v grep |awk '{print $5 }'`
    factory=`ps -ef | grep factory_test | grep -v grep | awk '{print $2}'`
    if [[ -z "$video" && -z "$factory" ]]
    then
        pulseaudio --start --log-target=syslog &
        echo `date` start pulseaudio>>/tmp/std.log
        sleep 0.5
    fi

    rcc=`pidof RCC_Client`
    if [ -z "$rcc" ]
    then
        /usr/local/bin/RCC_Client &
        echo `date` start RCC_Client>>/tmp/std.log
        sleep 1.5
    fi

    video=`pidof video_player`
    if [ -z "$video" ]
    then
        /usr/local/bin/video_player &
        echo `date` start video_player>>/tmp/std.log
        sleep 0.5
    fi

    rcc_daemon=`pidof RCC_daemon`
    if [ -z "$rcc_daemon" ]
    then
        /usr/local/bin/RCC_daemon &
        echo `date` start RCC_daemon>>/tmp/std.log
        sleep 0.5
    fi

    #ps -ef | grep rj_webservice | grep -v grep || sh /usr/local/bin/cef/qt_rjwebservice/rj_webservice.sh &

    #exec 2>/tmp/os.log

    sleep 0.5

done
