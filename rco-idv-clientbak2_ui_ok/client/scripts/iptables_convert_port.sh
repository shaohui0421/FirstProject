#!/bin/bash

#public network ip
lanip=$1       
dport=$2
wanip=$3
proto=$4
del_or_add=$5

#dnaport maybe empty
dnatport=$6



add_iptables () {
    if [ -n "$dnatport" ]; then
        iptables -t nat -A OUTPUT -d "${lanip}" -p "${proto}" --dport "${dport}" -j DNAT --to-destination "${wanip}:${dnatport}"
    else
        iptables -t nat -A OUTPUT -d "${lanip}" -p "${proto}" -m multiport --dport "${dport}" -j DNAT --to-destination "${wanip}"
    fi
}

del_iptables () {
    if [ -n "$dnatport" ]; then
        iptables -t nat -D OUTPUT -d "${lanip}" -p "${proto}" --dport "${dport}" -j DNAT --to-destination "${wanip}:${dnatport}"
    else
        iptables -t nat -D OUTPUT -d "${lanip}" -p "${proto}" -m multiport --dport "${dport}" -j DNAT --to-destination "${wanip}"
    fi
}


if [ $# -lt 1 ]; then
    exit 1
fi

if [ "$del_or_add" = 0 ]; then

    iptables_flag=$(iptables -t nat -nL |grep $wanip | grep $lanip | grep $dport | grep $proto)
    if [ -n "$iptables_flag" ]; then
        exit 2
    fi
    add_iptables
elif [ "$del_or_add" = 1 ]; then
    iptables_flag=$(iptables -t nat -nL |grep $wanip | grep $lanip | grep $dport | grep $proto)
    if [ -z "$iptables_flag" ]; then
        exit 2
    fi

    del_iptables
else
    exit 3
fi

exit
