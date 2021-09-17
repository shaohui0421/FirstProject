#!/bin/sh
#

CLIENT_SCRIPTDIR=/etc/RCC_Client/scripts
. $CLIENT_SCRIPTDIR/idv_function.sh

# exclude ini list,
# not to be cleared, while client initializing.
cat > $CLIENT_SCRIPTDIR/exclude.list << EOF
version_client_idv.ini
vmmode.ini
mina.ini
vm_network_info.ini
logic_configured.ini
EOF

ini_found()
{
    found=1
    for i in `cat $CLIENT_SCRIPTDIR/exclude.list`
    do
        if [ "$i" = "$1" ]; then
            found=0
            break
        fi
    done
    return $found
}

# clear all the config ini,
# to reset all the client configurations to initial status.
idv_logging "Clear all the config ini for client reset init ..."
cd /opt/lessons/RCC_Client
for ini in `ls *.ini`
do
    if ! ini_found $ini; then
        rm -f $ini
    fi
done
sync
idv_logging "All the client config ini cleared, excluding files:"
cat $CLIENT_SCRIPTDIR/exclude.list >> $IDVLOG
rm -f $CLIENT_SCRIPTDIR/exclude.list
