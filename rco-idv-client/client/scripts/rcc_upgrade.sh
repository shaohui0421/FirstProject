#!/bin/bash
#

#upgrade software version by running this script
#return:
#	0:success
#	1:already installed
#	2:no packeage in dest pool
#	3:upgrade failed

pool_name=$1
server_ip=$2
package_name=$3
version=$4
update_pool=$5

if [ $update_pool -ne 0 ]
then
    rm -f /var/lib/apt/lists/*Packages
    rm -f /var/lib/apt/lists/partial/*.gz
    update_pool_count=0
    update_result=""
    while [ $update_pool_count -lt 6 ] && [ -z "$update_result" ]
    do
        #update new server's sources.list
        sourcelist_content="deb http://${server_ip} ${pool_name}/"
        echo "${sourcelist_content}" > /etc/apt/sources.list
        apt-get update

        #check whether our packeage is in the pool
        update_result=$(apt-cache showpkg ${package_name} |grep ${server_ip} |grep ${version})
        update_pool_count=$(($update_pool_count+1))
        echo "update pool count $update_pool_count"
        if [ -z "$update_result" ]
        then
            if [ $update_pool_count -eq 6 ]
            then
                echo "there is no package in server's pool"
                exit 2
            else
                sleep 15
            fi
        fi
    done
fi

# check whether our version is latest
install_result=$(dpkg -l | awk -v pkgname=$package_name -v ver=$version '{if ($2 == pkgname && $3 == ver) print $1}')
if [ -n "$install_result" -a "$install_result" = "ii" ]; then
    echo "version latest"
    exit 1
fi

upgrade_status=0
while [ $upgrade_status -ne 2 ]
do
    #prepare downloading
    pkill -9 apt
    pkill -9 dpkg
    rm -f /var/lib/dpkg/lock
    rm -f /var/cache/apt/archives/lock

    #download
    apt-get -f --allow-downgrades -y --allow-unauthenticated install $package_name=$version

    #check whether download is successful
    upgrade_result=$(dpkg -l |grep ${package_name} |grep ${version}| grep ii)
    if [ -z "$upgrade_result" ];
    then
        echo "upgrade failed, status=$upgrade_status"
        if [ $upgrade_status -eq 0 ]
        then
            upgrade_status=1
            dpkg --configure -a
        elif [ $upgrade_status -eq 1 ]
        then
            upgrade_status=9
            apt-get install -f -y --allow-unauthenticated $package_name --reinstall
        else
            exit 3
        fi
    else
        upgrade_status=2
    fi
done

#complete successful
exit 0
