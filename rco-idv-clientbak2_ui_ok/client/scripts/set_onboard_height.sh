#!/bin/sh
#
height=$1
echo $height

sed -i '/dch$/d' /usr/lib/python3/dist-packages/Onboard/KbdWindow.py
sed -i '1300,1313s/return co.dock_width, co.dock_height/co.dock_height = '"$height "'#modify by dch\n        return co.dock_width, co.dock_height/g' /usr/lib/python3/dist-packages/Onboard/KbdWindow.py
sync
