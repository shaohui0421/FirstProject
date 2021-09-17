#!/bin/bash
#
basename=$1
mount_dir=$2
if_same_base_exist=0
log_file="/var/log/rcc_client.log"

usb_devs=`ls -l /sys/block/ | grep usb | awk '{print $9}'`
echo "usb_devs: $usb_devs" >> $log_file

usb_dev_count=`ls -l /sys/block/ | grep usb | wc -l`
if [ ${usb_dev_count} -eq 0 ];then
    echo "DEV NOT EXIST"
    exit 0
fi

for usb_dev in $usb_devs
do
    mount_devs=`blkid | awk '{print $1}' | grep /dev/${usb_dev}`
    echo "mount_devs: $mount_devs" >> $log_file
    for mount_dev in $mount_devs
    do
        mount_dev=${mount_dev%%:*}
        mount_dev_name=${mount_dev##*/}
        fstype=`lsblk -f | grep ${mount_dev_name} | awk '{print $2}'`
        if [ ${fstype} != "ntfs" ];then	
            echo "ignore $mount_dev fstype: $fstype" >> $log_file
            continue
        fi
        mount ${mount_dev} ${mount_dir}
        files=`ls -l ${mount_dir}`
        echo "$mount_dev contains files: $files" >> $log_file
        if [ ! -f ${mount_dir}/${basename} ]; then
            echo "not find ${basename} in ${mount_dev}" >> $log_file
            umount ${mount_dir}
            continue
        else
            echo "find ${basename} in ${mount_dev}" >> $log_file
            umount ${mount_dir}
            if_same_base_exist=1
            break
        fi
    done
    if [ ${if_same_base_exist} -eq 1 ];then
        break
    fi
done

if [ ${if_same_base_exist} -eq 1 ];then
    echo $mount_dev
else
    echo "NOT EXIST"
fi
