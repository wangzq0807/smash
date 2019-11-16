#! /bin/sh
set -e
binDir=$1
runDir=$2
mntDir=/mnt/smash
bootDir=$binDir/boot

curDir=$(cd $(dirname $0); pwd)
. $curDir/vdisk.sh

make_vdisk || {
    echo "虚拟磁盘创建失败!"
    umount_vdisk
    exit 1
}

objcopy -j .text -O binary $bootDir/mbr/mbr $bootDir/mbr.img
objcopy -O binary $bootDir/loader/loader $bootDir/loader.img
objdump -S $bootDir/loader/loader > $runDir/loader.sym
objdump -S $binDir/kernel/smash > $runDir/smash.sym
# 将mbr和loader写入hd.img避免擦除分区表
dd if=$bootDir/mbr.img of=$runDir/hd.img bs=432 count=1 conv=notrunc > /dev/zero 2>&1
dd if=$bootDir/loader.img of=$runDir/hd.img bs=512 count=120 seek=1 conv=notrunc > /dev/zero 2>&1

. $curDir/sysfile.sh
create_sysfile
umount_vdisk
