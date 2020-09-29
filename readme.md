# STMicro 32F1xx toy cooperative multitasking OS and some peripheral drivers
# includes systick interrupt for extension to pre-emptive multitasking
# also includes tiny libc implementation (mainly printf, strcpy, etc.)

stlink: may need to set pc correctly before continuing execution
gdb> set $pc = 0x0800...
