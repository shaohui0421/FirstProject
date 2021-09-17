#!/bin/bash
#

workdir=$(cd $(dirname $0); pwd)
echo $workdir

#sh $workdir/scripts/version_update.sh 4.1.592 R1P7 
sh scripts/version_update.sh 4.1.592 R1P7 
#sh $workdir/scripts/build.sh -j8
sh scripts/build.sh -j8
