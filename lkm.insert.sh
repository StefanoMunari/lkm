#!/bin/bash
# example usage: ./lkm.insert.sh event/event_dev.ko
EX_NOINPUT=66
EX_CANTCREAT=73

driver_fpath="$1"

if [ ! -e "$driver_fpath" ]; then
    exit $EX_NOINPUT
fi

driver=$(basename "$driver_fpath")
driver="${driver%.*}"
device_path=/dev/$driver

if [ -e "$device_path" ]; then
    exit $EX_CANTCREAT
fi

# Insert the kernel module
sudo insmod $driver_fpath
major=$(sudo dmesg |grep $driver |tail -n1 |rev |cut -d' ' -f1 |rev)
# Create the device node
sudo mknod $device_path c $major 0
sudo chown $USER:$USER $device_path

echo "inserted:$device_path"