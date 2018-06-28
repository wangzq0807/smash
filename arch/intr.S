// NOTE:中断处理前要保存寄存器的值
.text
.global on_timer_intr
on_timer_intr:
    pushal
    call on_timer_handler
    popal
    iret

.global on_disk_intr
on_disk_intr:
    pushal
    call on_disk_handler
    popal
    iret

.global on_ignore_intr
on_ignore_intr:
    pushal
    call on_ignore_handler
    popal
    iret