#!/bin/bash
devname=`echo $1`
status=`LC_ALL=c ifconfig | grep "Link encap" | grep "^${devname}\s"`
status2=`LC_ALL=c ifconfig -a | grep "Link encap" | grep "^${devname}\s"`
iptype=`grep "iface.*\s$devname\s.*inet\s" /etc/network/interfaces | awk '{print $4}'`
if [ "$iptype" = "static" ]; then
    address=$(LC_ALL=c sed -n "/iface $devname inet/,/iface/p" /etc/network/interfaces | grep "address" | awk '{print $2}')
    netmask=$(LC_ALL=c sed -n "/iface $devname inet/,/iface/p" /etc/network/interfaces | grep "netmask" | awk '{print $2}')
    gateway=$(LC_ALL=c sed -n "/iface $devname inet/,/iface/p" /etc/network/interfaces | grep "gateway" | awk '{print $2}')
else
    address=`LC_ALL=c ifconfig $devname | grep "inet addr:" | grep -v "127.0.0.1" | cut -d: -f2 | awk '{print $1}'`
    netmask=`LC_ALL=c ifconfig $devname | grep "inet addr:" | grep -v "127.0.0.1" | cut -d: -f4 | awk '{print $1}'`
    gateway=`LC_ALL=c route -n | grep "\s${devname}$" | grep UG | grep "^0.0.0.0\s" | awk '{print $2}'`
fi
addressavahi=`LC_ALL=c ifconfig ${devname}:avahi | grep "inet addr:169" | grep -v "127.0.0.1" | cut -d: -f2 | awk '{print $1}'`

mac=`ip -o link | grep $devname | awk -F/ '{printf $2}' | awk '{printf $2}'`
if [ -L /etc/resolv.conf ];then
dnstype=dhcp
else
dnstype=static
fi
dns1=`awk '/^name/{print $2}' /etc/resolv.conf | sed -n "1,1p"`
dns2=`awk '/^name/{print $2}' /etc/resolv.conf | sed -n "2,2p"`
if [ "$devname" = "br0" ];then
linkstatus=`echo $(ethtool eth0 | grep "Link detected:" | cut -d: -f2)`
speed=`cat /sys/class/net/eth0/speed`
else
linkstatus=`echo $(ethtool $devname | grep "Link detected:" | cut -d: -f2)`
speed=`cat /sys/class/net/$devname/speed`
fi
#hostname=`hostname`

[ -z "$status" ] && [ -z "$status2" ] && echo status=0
[ -z "$status" ] && [ ! -z "$status2" ] && echo status=1
[ ! -z "$status" ] && [ ! -z "$status2" ] && echo status=2
[ ! -z "$iptype" ] && echo iptype=$iptype
if [ ! -z "$address" ]; then
  echo ip=$address
elif [ ! -z "$addressavahi" ]; then
  echo ip=$addressavahi
fi
[ ! -z "$netmask" ] && echo netmask=$netmask
[ ! -z "$gateway" ] && echo gateway=$gateway
[ ! -z "$mac" ] && echo mac=$mac
[ ! -z "$dnstype" ] && echo dnstype=$dnstype
[ ! -z "$dns1" ] && echo dns1=$dns1
[ ! -z "$dns2" ] && echo dns2=$dns2
[ ! -z "$linkstatus" ] && echo linkstatus=$linkstatus
[ ! -z "$speed" ] && echo speed=$speed
#[ ! -z "$hostname" ] && echo hostname=$hostname

exit 0
