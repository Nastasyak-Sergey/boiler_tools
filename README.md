Project stm32f103c8t6
---------------------
README.md
THIS PROJECT ONLY USES OPEN SOURCE TOOLS
----------------------------------------

PROJECT STRUCTURE:
------------------

The top level defines the environment common to libopencm3 mainly

    boiler_tools/
        |- README.md
        |- logger/
        |- libopencm3/          git submodule
        |- stm32f103c8t6.ld
        |- Makefile             makes all top-level projects
        |- Makefile.include     rules for make file builds
        
PREREQUISITES:
--------------

    0a. It will be assumed here that you have your cross compiler
        toolchain installed. In Makefile.incl you'll see references
        to a PREFIX variable. This defaults to arm-none-eabi so that
        gcc for example will be arm-none-eabi-gcc etc. If your 
        prefix differs, edit Makefile.incl at the top level to
        match, or use the shell: export PREFIX=whatever
    
    0b. It is also assumed that you have the openOCD command installed
        on your system. You may need to download and install it. 
        Google is your friend.

    1.  If you didn't use a --recursive git clone, then you need to make
        sure that libopencm3 is fetched now. From the top level apply
        one of:
    
            $ git submodule update --init --recursive   # First time

            $ git submodule update --recursive          # Subsequent
    
    2.  Go into ./libopencm3 and type "make". This should build the 
        static libraries that will be needed. Any issues there should
        go to the libopencm3 community.

USB CDC implementation from https://github.com/Apress/Beg-STM32-Devel-FreeRTOS-libopencm3-GCC/blob/master/rtos/usbcdcdemo/usbcdc.c
USB MSC patch from https://habr.com/company/thirdpin/blog/304924/
FATFS	from http://elm-chan.org/fsw/ff/00index_e.html
Logger project based on https://github.com/lupyuen/bluepill-bootloader Grate thanks to Lupyuen 

BUILD INSTRUCTION
-----------------

    git clone --recurse-submodules https://github.com/
    cd boiler_tools
	make

TEST EXAMPLES:
--------------

After the prerequisites have been met:

Going into ./logger and:

    $ make
    $ make flash
	$ make connect    -if st-link connected you will see all debug msg

should build and flash your example program. Flashing assumes the
use of the openOCD command.


LICENSE:
--------

All files covered by _this_ repo (_except_ those copied from libopencm3, 
or otherwise marked), are covered by the GNU Lesser
General Public License Version 3. See the file named LICENSE.

Bill of Materials for Book:
---------------------------

1. ST-Link V2 Programmer
1. Winbond W25Q32/W25Q64 flash chip (DIP)
1. STM32F103C8T6 board (Black Pill)

Notes:
======


