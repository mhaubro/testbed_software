#!/bin/sh -
#This is a shell-script designed to run as a backgruond service always.
#Works by doing a git pull once a minute, and if there is no new software, it just does nothing, and if there is new software, it makes and restarts the service
cd /home/rpi/testbed-source
i=1
while [ "$i" -ne 0 ]
do
    if git pull | grep -q 'Already up to date'; then
        echo "match"
    else
        echo "building and restarting"
        make clean
        make
        systemctl restart testbed-software.service
    fi

    sleep 45
done
