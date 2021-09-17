#!/bin/sh
#

IDVLOG=/var/log/idv.log

idv_logging()
{
    local curtime="`date +'%Y-%m-%d %H:%M:%S'`"
    local logsize=0

    echo -n "$curtime " >> $IDVLOG
    echo $@ >> $IDVLOG
    logsize=`wc -c $IDVLOG | awk '{print $1}'`
    if [ $logsize -ge 4194304 ]; then
        # logfile size beyond 4MB, override the old one.
        mv $IDVLOG ${IDVLOG}.bak
    fi
}

