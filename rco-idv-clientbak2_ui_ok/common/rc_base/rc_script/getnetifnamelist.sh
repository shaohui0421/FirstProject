#!/bin/bash

. /etc/rc_script/function.sh

cd /etc/network/rc_inet/

namelist=`ls /sys/class/net/ | grep -v "vir"`

for brfile in `ls  | grep mk_`
do
  brto=`echo $brfile | cut -d_ -f2`
  brfrom=`echo $brfile | cut -d_ -f3`
  find_strunit "$brfrom"  "$namelist"
  if [ $? = 1 ];then
    namelist=`add_strunit "$brto" "$namelist"`
    namelist=`del_strunit "$brfrom" "$namelist"`
  fi
done

[ ! -z "$namelist" ] && echo $namelist
exit 0
