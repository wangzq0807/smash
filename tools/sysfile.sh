set -e

usrDir=$binDir/usr

create_sysfile() {
    # 创建dev和bin目录
    sudo mkdir $mntDir/dev $mntDir/bin $mntDir/boot
    # 拷贝内核
    sudo cp $binDir/kernel/smash $mntDir/boot/
    # 拷贝应用程序
    sudo cp $usrDir/bash $mntDir/bin/
    sudo cp $usrDir/cat $mntDir/bin/
    sudo cp $usrDir/echo $mntDir/bin/
    sudo cp $usrDir/helloworld $mntDir/bin/
    sudo cp $usrDir/ln $mntDir/bin/
    sudo cp $usrDir/ls $mntDir/bin/
    sudo cp $usrDir/pipecho $mntDir/bin/
    sudo cp $usrDir/rm $mntDir/bin/
    
    # 创建tty设备节点
    sudo mknod $mntDir/dev/tty c 1 1
}
