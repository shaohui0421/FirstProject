#!/bin/sh
#

# TODO: Now client must be running on this dir, for UI icon bug.
CLIENT_RESDIR=/etc/RCC_Client/res
cd $CLIENT_RESDIR

cat > daemon.list << EOF
catchsegv /usr/local/bin/IDV_Client
/usr/local/bin/terminal_conf
EOF

CLIENT_SCRIPTDIR=/etc/RCC_Client/scripts
. $CLIENT_SCRIPTDIR/idv_function.sh

if [ -n "$HOME" ]; then
    IDV_DAEMON_DIR=$HOME/.idv_daemon
else
    IDV_DAEMON_DIR=$CLIENT_SCRIPTDIR/.idv_daemon
fi
mkdir -p $IDV_DAEMON_DIR

start_one_daemon()
{
    local cmd=$1
    local program=$(basename $cmd)

    shift
    rm -f $IDV_DAEMON_DIR/${program}_daemon.sh
    cat > $IDV_DAEMON_DIR/${program}_daemon.sh << EOF
#!/bin/sh
#

CLIENT_SCRIPTDIR=/etc/RCC_Client/scripts
. \$CLIENT_SCRIPTDIR/idv_function.sh

while :
do
    idv_logging "$program starting ..."
    $cmd $@ >> $IDVLOG 2>&1
    error=\$?
    idv_logging "[*] $program exited with error code $error !!!"
    sleep 0.05
    top -b -n 1 >> $IDVLOG 2>&1
    idv_logging "$program exited unexpectedly! System will restart it."
    sleep 0.1
done
EOF
    chmod +x $IDV_DAEMON_DIR/${program}_daemon.sh
    $IDV_DAEMON_DIR/${program}_daemon.sh &
}

# check if client started.
if [ -f /tmp/.client_started ]; then
    # client daemon started, just return.
    rm -f daemon.list
    exit 0
fi

# export for X11 client to start well.
export DISPLAY=:0
#sleep 1

# start all the daemon processes.
cat daemon.list | while read cmdline
do
    idv_logging "Starting daemon for $cmdline ..."
    start_one_daemon $cmdline
done
rm -f daemon.list
touch /tmp/.client_started
