#!/bin/bash

export PATH=$PATH:/home/db/Downloads/gcc-arm-none-eabi-5_4-2016q3/bin

make clean && make && arm-none-eabi-gdb -s cfg.gdb main.elf

#sudo ~/stlink/build/Release/src/gdbserver/st-util

#echo "target extended-remote nb.local:4242" > cmd-file.txt 
#gdb -x cmd-file.txt

#tar extended-remote :4242
