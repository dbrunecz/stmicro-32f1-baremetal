
TCPATH = /home/db/gcc-arm-none-eabi-5_4-2016q3/bin

TC      = $(TCPATH)/arm-none-eabi
CC      = $(TC)-gcc
LD      = $(TC)-ld -v
CP      = $(TC)-objcopy
OD      = $(TC)-objdump
 
CFLAGS  = -Wall -Werror
CFLAGS  += -I./ -c -fno-common -O0 -g -mcpu=cortex-m3 -mthumb 
CFLAGS  += -fno-builtin
CFLAGS  += -nostartfiles
#CFLAGS  += -nodefaultlibs
LFLAGS  = -Tstm32.ld
CPFLAGS = -Obinary

SRCS := $(wildcard *.c)
OBJS := $(SRCS:%.c=%.o)

all: test

clean:
	@rm -f main.lst $(OBJS) main.elf main.bin main.S 2&>/dev/null

test: main.elf
	@ echo "...copying"
	$(CP) $(CPFLAGS) main.elf main.bin
	$(OD) -S main.elf > main.lst
	$(OD) -D main.elf > main.S

main.elf: $(OBJS)
	@ echo "..linking"
	$(LD) $(LFLAGS) -o main.elf $(OBJS)

%.o: %.c
	@ echo ".compiling" $<
	#$(TC)-cpp $<
	$(CC) $(CFLAGS) $< -o $@

