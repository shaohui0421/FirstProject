#!/usr/bash
#

#update to the new system format
os_version=`cat /etc/issue | head -n 1`
if [ -n "$(echo $os_version | sed -n '/^RCO-IDV-Rain_*/p' | sed -n '/_NS$/p')" ] ; then
    main_version=`echo $os_version | awk -F "_" '{print $3}' | awk -F "." '{printf("%04d", $2)}'`
    temp_version=`echo $os_version | awk -F "_" '{print $3}' | awk -F "." '{print $1}' | cut -c 3-`
    new_os_version="IDV-RainOS_${main_version}${temp_version}"
    echo "$new_os_version" > /etc/issue
    sync
fi

exit 0
