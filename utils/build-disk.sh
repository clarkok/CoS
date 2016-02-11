#/bin/sh
sudo chmod 777 /dev/sdb
sudo chmod 777 /dev/sdb1
mkfs.ext2 -b 1024 /dev/sdb1
