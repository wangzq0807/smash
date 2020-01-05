#include "buddy.h"
#include "lib/bitmap.h"
#include "lib/list.h"
#include "lib/log.h"

#define BUDDY_MAX_ORDER     (8)
#define BUDDY_LEVEL         (BUDDY_MAX_ORDER+1)
#define SLICE_SIZE          (4096)
#define BUDDY_GET(val)      ((val)^1UL)

typedef struct _Buddy
{
    BitMap      bl_bitmap;
    // size_t      bl_order;         
} Buddy;

static uint8_t buf[SLICE_SIZE*BUDDY_LEVEL];
static Buddy buddys[BUDDY_LEVEL];

static void
_buddy_clear(size_t size)
{
    pym_t pos = 0;
    for (int level = BUDDY_LEVEL-1; level >= 0; level--) {
        size_t bdsize = 1 << level;
        size_t num = size / bdsize;
        bm_clear_bitrange(&buddys[level].bl_bitmap, pos, num);
        size = size % bdsize;
        if (size == 0)
            break;
        pos = 2*(pos + 1);
    }
}

void
buddy_setup(size_t total)
{
    for (int i = 0; i < SLICE_SIZE*BUDDY_LEVEL; ++i)
        buf[i] = 0xff;
    for (int level = 0; level < BUDDY_LEVEL; ++level) {
        // buddys[level].bl_order = level;
        buddys[level].bl_bitmap.b_nsize = SLICE_SIZE;
        buddys[level].bl_bitmap.b_bitbuf = buf + (level*SLICE_SIZE);
    }
    _buddy_clear(total);
}

static void buddy_remain_give_back(int from, int pos, int remain)
{
    for (int level = from - 1; remain > 0 && level >= 0; level--) {
        size_t cursize = 1 << level;
        if (remain >= cursize) {
            pos = 2*pos;
            bm_clear_bit(&buddys[level].bl_bitmap, pos+1);
            remain -= cursize;
        }
        else {
            pos = 2*pos + 1;
        }
    }
}

pym_t
buddy_alloc(size_t size)
{
    pym_t ret = -1;
    for (int level = 0; level < BUDDY_LEVEL; level++) {
        size_t bdsize = 1 << level;
        if (bdsize < size)
            continue;
        int pos = bm_find_bit(&buddys[level].bl_bitmap);
        if (pos < 0)
            continue;
        // 分配左侧
        bm_set_bit(&buddys[level].bl_bitmap, pos);
        ret = pos << level;
        // 归还右侧
        size_t remain = bdsize - size;
        buddy_remain_give_back(level, pos, remain);
        break;
    }
    return ret;
}

static void
buddy_merge(int from, int pos)
{
    for (int level = from; level < BUDDY_LEVEL - 1; level++) {
        int bdpos = BUDDY_GET(pos);
        int test = bm_test_bit(&buddys[level].bl_bitmap, bdpos);
        if (test != 0)
            break;
        bm_set_bit(&buddys[level].bl_bitmap, pos);
        bm_set_bit(&buddys[level].bl_bitmap, bdpos);
        pos = pos / 2;
        bm_clear_bit(&buddys[level+1].bl_bitmap, pos);
    }
}

void
buddy_free(pym_t addr, size_t size)
{
    for (int level = BUDDY_LEVEL-1; level >= 0; level--) {
        size_t bdsize = 1 << level;
        while (size >= bdsize) {
            int pos = addr >> level;
            bm_clear_bit(&buddys[level].bl_bitmap, pos);
            // 向上合并
            buddy_merge(level, pos);
            size -= bdsize;
            addr += bdsize;
        }
    }
}

unsigned int
buddy_test(int level, int pos)
{
    unsigned int *p = (unsigned int *)(buddys[level].bl_bitmap.b_bitbuf);
    return p[pos];
}
