#!/bin/bash
hostname=$1

rm -rf /etc/hostname.tmp
[ ! -z "$hostname" ] && echo $hostname > /etc/hostname.tmp

if [ -f /etc/hostname.tmp ];then
  hostname $hostname
  if [ $? = 0 ]; then
    mv -f /etc/hostname.tmp /etc/hostname
    sync
    exit 0
  fi
fi
exit 1

