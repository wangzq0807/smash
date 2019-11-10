#! /bin/sh
make_vdisk() {
    echo "创建虚拟磁盘: hd.img"
    # 创建一个虚拟磁盘hd.img
    dd if=/dev/zero of=$runDir/hd.img bs=1M count=10 > /dev/zero 2>&1 || return 1;
    # 对hd.img进行分区
    fdisk $runDir/hd.img > /dev/zero << EOF
n
p
1
2048
20479
a
w
EOF
    # 创建minix文件系统
    sudo losetup /dev/loop0 $runDir/hd.img > /dev/zero || return 1;
    sudo mkfs.minix -2 /dev/loop0p1 > /dev/zero || return 1;
    # 挂载
    sudo mkdir $mntDir > /dev/zero || return 1;
    sudo mount /dev/loop0p1 $mntDir > /dev/zero || return 1;
    echo "虚拟磁盘创建成功!"
}

# 卸载
umount_vdisk() {
    sudo umount $mntDir || true
    sudo rm -rf $mntDir || true
    sudo losetup -d /dev/loop0 || true
}
