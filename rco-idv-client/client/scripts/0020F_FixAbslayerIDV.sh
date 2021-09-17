#!/bin/bash

# platform item will not execute this script
runningtype=rcd
if [ -f /boot/efi/runningtype ];then
    runningtype=`cat /boot/efi/runningtype`
fi
if [[ "$runningtype" != "rcd" ]]; then
    exit 0
fi
unset runningtype

package_name=rcos-abslayer
logfile=/var/log/fix_abslayer.log
deb_dir=/var/cache/apt/archives

# if log size exceed 1M ,reduce it.
logsize=$(stat -c "%s" $logfile)
if [[ "logsize" -gt 1048576 ]];then 
    echo `date` "logrotate!!!" > $logfile
fi

local_version=$(dpkg-query -W -f='${Version}' ${package_name} 2>/dev/null)
if [ -z "local_version" ];then
    exit 0
fi

# if local deb exist, install it.
cache_deb=$(ls -r $deb_dir/${package_name}* 2>/dev/null | head -n 1)

install_result=$(dpkg -l | awk -v pkgname=$package_name '{if ($2 == pkgname) print $1}')
if [[ -e $cache_deb && "$install_result" != "ii" ]]; then
    echo `date` "install $cache_deb from local archives, because deb status is not ii" >> $logfile
    dpkg --configure -a
    dpkg -i --force-overwrite $cache_deb >> $logfile
    if [[ $? = 0 ]];then
        echo `date` "install $cache_deb from local archives success" >> $logfile
    fi
    ldconfig
    exit 0
fi
