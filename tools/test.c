    char f1_name[4]; 
    IndexNode *fnode1 = file_open("/aaa", 0, 0);
    KLOG(DEBUG, "fnode1: %x", fnode1);
    file_read(fnode1, 0, &f1_name, 4);
    KLOG(DEBUG, "f1_name: %s", f1_name);

    char f2_name[4]; 
    IndexNode *fnode2 = file_open("/bbb/ccc", 0, 0);
    KLOG(DEBUG, "fnode2: %x", fnode2);
    file_read(fnode1, fnode2->in_file_size - 4, &f2_name, 4);
    KLOG(DEBUG, "f2_name: %s", f2_name);