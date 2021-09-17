#!/bin/bash
#

TOPDIR=$PWD
VERSION_FILE=$TOPDIR/client/scripts/version_client_idv.ini
VERSION_RELEASE=R0

if [ ! -f $VERSION_FILE ]; then
    cat > $VERSION_FILE << EOF
[LINUX-RCC-Client]
ruijie.product=RCC-Client
ruijie.rcc.client.mainVersion=1
ruijie.rcc.client.minorVersion=0
ruijie.rcc.client.threeVersion=0
ruijie.rcc.client.extra1=_R0
ruijie.rcc.client.fourVersion=1
ruijie.rcc.client.extra2=
ruijie.rcc.client.buildDate=2017-03-31
EOF
fi
VERSION_RELEASE=$(cat $VERSION_FILE | awk -F= '/extra1/{print $2}' | tr -d '\r')
VERSION_RELEASE=${VERSION_RELEASE##*_}

if [ -n "$1" ]; then
    if [ "$1" = "revert" ]; then
        git checkout -- $VERSION_FILE
        exit 0
    fi
    if echo $1 | grep -q '\.'; then
        mainVersion=$(echo $1 | awk -F. '{print $1}')
        minorVersion=$(echo $1 | awk -F. '{print $2}')
        patchVersion=$(echo $1 | awk -F. '{print $3}')
    else
        mainVersion=$(cat $VERSION_FILE | awk -F= '/mainVersion/{print $2}' | tr -d '\r')
        minorVersion=$(cat $VERSION_FILE | awk -F= '/minorVersion/{print $2}' | tr -d '\r')
        patchVersion=$1
    fi
else
    mainVersion=$(cat $VERSION_FILE | awk -F= '/mainVersion/{print $2}' | tr -d '\r')
    minorVersion=$(cat $VERSION_FILE | awk -F= '/minorVersion/{print $2}' | tr -d '\r')
    patchVersion=$(cat $VERSION_FILE | awk -F= '/fourVersion/{print $2}' | tr -d '\r')
fi
if [ -n "$2" ]; then
    release_ver=$2
    VERSION_RELEASE=${release_ver##*_}
fi
buildDate=$(date +%F)
HEADcommit=$(git log -1 | awk '/^commit/ {print $2}')
curBranch=$(git branch | awk '/^\*/ {print $2}')

cat > $VERSION_FILE.$$ << EOF
[LINUX-RCC-Client]
ruijie.product=RCC-Client
ruijie.rcc.client.mainVersion=$mainVersion
ruijie.rcc.client.minorVersion=$minorVersion
ruijie.rcc.client.threeVersion=0
ruijie.rcc.client.extra1=_$VERSION_RELEASE
ruijie.rcc.client.fourVersion=$patchVersion
ruijie.rcc.client.extra2=
ruijie.rcc.client.buildDate=$buildDate
ruijie.rcc.client.buildHEAD=$HEADcommit
ruijie.rcc.client.buildBranch=client-4.1R1P7
EOF

mv $VERSION_FILE.$$ $VERSION_FILE
cat $VERSION_FILE
