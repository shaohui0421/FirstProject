#!/bin/bash
#

#cancel upgrade software version by running this script

pkill -9 rcc_upgrade
pkill -9 dpkg
pkill -9 apt
rm -f /var/cache/apt/archives/*.deb
rm -f /var/lib/dpkg/lock
rm -f /var/cache/apt/archives/lock

exit 0
