#include "types.h"
#include "fs/disk_drv.h"
#include "fs/superblk.h"
#include "fs/file.h"
#include "log.h"

void
start_main()
{
    init_file_system();
}
