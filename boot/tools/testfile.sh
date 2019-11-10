set -e

create_testfile() {
    sudo touch $mntDir/aaa
    sudo chmod 777 $mntDir/aaa
    echo aaa > $mntDir/aaa

    sudo mkdir $mntDir/bbb
    sudo touch $mntDir/bbb/ccc
    sudo truncate $mntDir/bbb/ccc --size 5M
    sudo chmod 777 $mntDir/bbb/ccc
    echo ccc >> $mntDir/bbb/ccc
}
