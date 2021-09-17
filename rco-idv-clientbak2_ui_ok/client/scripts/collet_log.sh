#!/bin/bash
#

host_name=$1
host_ip=$2
server_ip=$3
ftpuser=$4
ftppwd=$5

FTP_PORT=21

rm -rf /tmp/log
mkdir /tmp/log
cp -rf /var/log/* /tmp/log
tar zcvf /tmp/log/$host_name"@"$host_ip.tar.gz /tmp/log/*
pkill -9 ftp
# check ftp port is connected
`nc -w 2 -z $server_ip $FTP_PORT`
if [ $? -eq 0 ]; then
    timeout 100 wput --basename=/tmp/log/ /tmp/log/*.tar.gz ftp://$ftpuser:$ftppwd@$server_ip/CLOG/
fi
