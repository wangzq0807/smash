#! /bin/sh
set -e
binDir=$1
runDir=$2
mntDir=/mnt/smash

curDir=$(cd $(dirname $0); pwd)
. $curDir/vdisk.sh

make_vdisk || {
    echo "虚拟磁盘创建失败!"
    umount_vdisk
    exit 1
}
. $curDir/sysfile.sh
create_sysfile
#umount_vdisk

objcopy -j .text -O binary $binDir/mbr/mbr $binDir/mbr.img
objcopy -O binary $binDir/loader/loader $binDir/loader.img
objdump -S $binDir/loader/loader > $runDir/loader.sym
# 将mbr和loader写入hd.img避免擦除分区表
dd if=$binDir/mbr.img of=$runDir/hd.img bs=432 count=1 conv=notrunc > /dev/zero 2>&1
dd if=$binDir/loader.img of=$runDir/hd.img bs=512 count=120 seek=1 conv=notrunc > /dev/zero 2>&1
