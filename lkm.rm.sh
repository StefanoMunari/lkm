#!/bin/bash
# example usage: ./lkm.rm.sh event_dev
driver="$1"
device_path=/dev/$driver

sudo rmmod -f $driver
sudo rm -f $device_path

echo "removed:$device_path"