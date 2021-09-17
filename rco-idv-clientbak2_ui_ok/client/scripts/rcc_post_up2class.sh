#!/bin/bash
#

#post-upgrade software version by running this script
#return:
#	0:success
#	1:upgrade failed

pool_name=$1
server_ip=$2
version=$3
update_pool=$4

#do rcc update.sh
#

#remove rcos-abslayer
if [ -n "$(dpkg -l | grep rcos-abslayer)" ]; then
    apt-get remove -y --purge rcos-abslayer
    echo "remove rcos-abslayer ret:$?"
fi

#upgrade class libvirt
libvirt_version=$(apt-cache showpkg libvirt | grep \(/var/lib/apt/lists/${server_ip} | awk '{print $1}')
if [ -n "$libvirt_version" ]; then
    apt-get -f --allow-downgrades -y --allow-unauthenticated install libvirt=$libvirt_version
    if [ -z "$(dpkg -l | grep libvirt | grep ii)" ]; then
        echo "install libvirt failed"
        exit 1
    fi
    echo "install libvirt success"
fi

#uninstall rjsyscore and stop wpa service
apt-get remove -y --purge rjsyscore
if [ -n "$(dpkg -l | grep rjsyscore)" ] ;then
    echo "remove rjsyscore failed"
    exit 1
fi
echo "remove rjsyscore success"

sed -i -e '/wlan0/d' -e '/wpa/d' /etc/network/interfaces
sync

#save class config file
hostname=`awk '/hostname/{print $3}' /opt/lessons/RCC_Client/logic_configured.ini | awk '/^[a-zA-Z0-9\-]+$/{printf("%.15s\n", $1)}'`
if [ -z "$hostname" ] ;then
    hostname=rcd
fi

if [ -n "$(file /etc/resolv.conf | grep 'symbolic link')" ] ;then
    dnstype=Auto
else
    dnstype=Static
fi

cat > /etc/RCC-Client/RCC_Client_Config.ini << EOF
[CLIENTCONF]
ip=
netmask=
gateway=
dns=
hostname=$hostname
adminpwd=ruijie.com
DNSType=$dnstype
role=0

[AUTOLOGING]
login=1

[SERVERCONF]
mainServer=$server_ip
backServer=
serverPort=9109

[TEACHERCONF]
cm_tch_ip=0.0.0.0
EOF
sync

#delete rco config file
if [ ! -f /opt/lessons/UPGRADE_FROM_RCO ] ; then
    rm -f /opt/lessons/data.teacher.disk
    rm -f /opt/lessons/*.base
    rm -f /opt/lessons/*.img
    rm -f /opt/lessons/*.layer
    rm -f /opt/lessons/w.disk
    rm -f /opt/lessons/*.ini
    rm -f /opt/lessons/*.conf
    rm -f /opt/lessons/*.xml
    rm -f /opt/lessons/*.torrent
fi

rm -rf /opt/lessons/layer/
rm -rf /opt/lessons/RCC_Client/
rm -rf /opt/lessons/wpa_supplicant/
rm -rf /etc/RCC_Client/
rm -f  /etc/RCC_Client_os_upgrade_rule.ini

rm -f /etc/AutoStart.rclocal/0900B_RCO_VMMESSAGE.bash
rm -f /etc/AutoStart.login/0800B_RCO_CLIENT.bash
rm -f /etc/AutoStart.xinit/0900F_rco_hotplug.bash
rm -f /etc/AutoStart.xinit/0910B_HotplugDaemon.bash
sync

#copy vmmanager script
cp -f /etc/rcc-bak/0900B_RCC_VM_DATA_DISK_CREATE.bash /etc/AutoStart.rclocal/
sync

#complete successful
exit 0
