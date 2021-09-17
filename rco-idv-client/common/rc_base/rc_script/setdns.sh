#!/bin/bash
dns1=$1
dns2=$2
rm -rf /etc/resolv.conf.tmp
[ ! -z "$dns1" ] && echo nameserver $dns1 >> /etc/resolv.conf.tmp 
[ ! -z "$dns2" ] && echo nameserver $dns2 >> /etc/resolv.conf.tmp

if [ -f /etc/resolv.conf.tmp ];then
#  cp -f /etc/resolv.conf.tmp /etc/resolv.conf
#  rm -rf /etc/resolv.conf.tmp
  mv -f /etc/resolv.conf.tmp /etc/resolv.conf
elif [ -z "$dns1" ] && [ -z "$dns2" ];then
  rm -rf /etc/resolv.conf
  ln -s ../run/resolvconf/resolv.conf /etc/resolv.conf
fi
sync
exit 0
