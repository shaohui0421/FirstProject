#!/bin/bash
#

# building process num, for make processes.
BUILD_PROCESSES=8

# Deal with parameters for the script.
ARGS=$(getopt -o j: --long jobs: -- "$@")
eval set -- "$ARGS"
while :
do
    case "$1" in
        -j | --jobs)
            BUILD_PROCESSES=$2
            shift 2
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "Internal error!"
            exit 1
            ;;
    esac
done

curdirname=$(basename $PWD)
if [ "$curdirname" = "scripts" ]; then
    cd ..
fi

# clear old outputs.
rm -f *.deb
rm -f backup-*.tgz

# We start IDV client building with project top dir.
TOPDIR=$PWD
pkgName=rcd

CLIENT_VERSION_FILE=$TOPDIR/client/scripts/version_client_idv.ini
mainVersion=`cat $CLIENT_VERSION_FILE | awk -F= '/client.mainVersion/{print $2}' | tr -d '\r'`
minorVersion=`cat $CLIENT_VERSION_FILE | awk -F= '/client.minorVersion/{print $2}' | tr -d '\r'`
patchVersion=`cat $CLIENT_VERSION_FILE | awk -F= '/client.fourVersion/{print $2}' | tr -d '\r'`
releaseVersion=`cat $CLIENT_VERSION_FILE | awk -F= '/client.extra1/{print $2}' | tr -d '\r'`
pkgVersion=$mainVersion.$minorVersion.$patchVersion
pkgRelease=${releaseVersion##*_}

#depends="vmmanager,qemu,bittornado"

# do compiling.
make clean
make -j$BUILD_PROCESSES $@
if [ $? -ne 0 ]; then
    echo -e "\\033[1;31m"
    echo "Error occured while compiling. Stop!"
    echo -e "\\033[0m"
    exit 1
fi

# make a package.
sudo make uninstall $@
sudo checkinstall --pkgname=$pkgName --pkgversion=$pkgVersion --pkgrelease=$pkgRelease -y --install=no --strip=no
cp ${pkgName}*.deb ${pkgName}.deb

# re-package the rcd.deb as an uncompressed package.
dpkg -X ${pkgName}.deb ${pkgName}_tmp
echo
pushd ${pkgName}_tmp
dpkg -e $TOPDIR/${pkgName}.deb
if [ -d $TOPDIR/DEBIAN/ ]; then
    cp -rf $TOPDIR/DEBIAN/* ./DEBIAN/
    echo "Depends: rjsyscore, rcos-abslayer" >> ./DEBIAN/control
    chmod +x DEBIAN/*
fi
popd
rm -f ${pkgName}.deb
dpkg-deb -b -Znone ${pkgName}_tmp ${pkgName}.deb
rm -rf ${pkgName}_tmp
echo
echo "$pkgName packaging OK! find output: ${pkgName}.deb"
echo
