#!/bin/bash

. /etc/rc_script/function.sh

netifnamelist=`/etc/rc_script/getnetifnamelist.sh`
netifnum=`num_strunit $netifnamelist`
[ ! -z $netifnum ] && echo $netifnum
exit 0
