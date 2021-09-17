#!/bin/bash

serialnum=`echo $(dmidecode -s system-serial-number)`
productid=`echo 0x$(dmidecode -s baseboard-product-name)`
productname=`echo $(dmidecode -s system-product-name)`
biosversion=`echo $(dmidecode -s bios-version)`
hdversion=`echo $(dmidecode -s system-version)`
osversion=`echo $(cat /etc/issue)`
cpu=`echo $(cat /proc/cpuinfo | grep "^model\s*name\s*:" | uniq | cut -d: -f2)`
memory=`echo $(cat /proc/meminfo | grep "^MemTotal:" | cut -d: -f2)`

rootdevnum=`mount | grep "on / type " | awk '{print $1}'`
rootdevp=$(echo $rootdevnum | sed 's/\/dev\///' | sed 's/[0-9]*$//')
if [[ "$rootdevp" =~ "mmcblk" ]];then
  rootdev=${rootdevp%p}
else
  rootdev=${rootdevp}
fi
[ -f /sys/class/block/${rootdev}/size ] && storage=`echo $(expr $(cat /sys/class/block/${rootdev}/size) \* 512 / 1000 / 1000 / 1000 )`
# storage should be the power of 2 (31.8 GB -> 32 GB)
storage=`echo ${storage} | awk '{print 2**int(log($storage)/log(2)+0.5)}'`
storage=`echo ${storage} GB`

[ ! -z "$serialnum" ] && echo serialnum=$serialnum
[ ! -z "$productid" ] && echo productid=$productid
[ ! -z "$productname" ] && echo productname=$productname
[ ! -z "$hdversion" ] && echo hdversion=$hdversion
[ ! -z "$osversion" ] && echo osversion=$osversion
[ ! -z "$biosversion" ] && echo biosversion=$biosversion
[ ! -z "$cpu" ] && echo cpu=$cpu
[ ! -z "$memory" ] && echo memory=$memory
[ ! -z "$storage" ] && echo storage=$storage
exit 0
