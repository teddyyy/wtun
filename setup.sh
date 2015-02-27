#!/bin/bash

if [ "X$2" = "X" ]; then
	echo "Usage: $0 dst_ip tunnle_ip"
	exit -1
fi

sudo insmod wtun.ko dst_addr=$1

sudo iwconfig wlan0 mode ad-hoc
sudo iwconfig wlan0 essid test
sudo ifconfig wlan0 $2/24
