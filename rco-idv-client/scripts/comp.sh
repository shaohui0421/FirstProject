#!/bin/bash
#

workdir=$(cd $(dirname $0); pwd)
echo $workdir

sh $workdir/version_update.sh 4.1.592 R1P7 
sh $workdir/build.sh -j8
