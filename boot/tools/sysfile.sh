set -e

create_sysfile() {
    # 创建dev和bin目录
    sudo mkdir $mntDir/dev $mntDir/bin $mntDir/boot
    # 创建tty设备节点
    sudo mknod $mntDir/dev/tty c 1 1
}
