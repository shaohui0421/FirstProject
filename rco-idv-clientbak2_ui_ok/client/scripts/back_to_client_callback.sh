#!/bin/sh
#
rm /tmp/.client_started -f
rm /tmp/.xserver_forbidden -f
rm /tmp/user_status.ini -f
kill `pidof IDV_Client`

