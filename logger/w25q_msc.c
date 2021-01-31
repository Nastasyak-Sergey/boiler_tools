#include <string.h>
#include <stdbool.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include "debug.h"
#include "winbond.h"
#include "w25q_msc.h"


/*
static void padded_memcpy(char *dst, const char *src, int len) {
    for (int i = 0; i < len; ++i) {
        if (*src)
           *dst = *src++;
        else
            *dst = ' ';
        dst++;
    }
}
*/

int read_block(uint32_t block_no, uint8_t *data) {
	
    uint32_t read_addr __attribute__((unused)) = 0;
 
    read_addr = w25_read_data(SPI1, block_no * 512, data, 256);
    read_addr = w25_read_data(SPI1, (block_no * 512 ) + 256, data, 256);
    
    return 0;
}




int write_block(uint32_t lba, const uint8_t *copy_from)
{
    uint32_t write_addr = 0;

	w25_write_en(SPI1,true); // More to write

	if (lba == 0 ) {
	
        w25_erase_block(SPI1, 0, W25_CMD_ERA_SECTOR);
        debug_println("Attempt to erase from "); debug_print_int(lba); debug_write("\n"); debug_flush();

 	} else if (!(lba % 8)) { // erase Sector of 4K when required (lba 512 * 8 = 4K )
        w25_erase_block(SPI1, lba * 512, W25_CMD_ERA_SECTOR);
        debug_println("Attempt to erase from "); debug_print_int(lba*512); debug_write("\n"); debug_flush();
    }
    
	w25_write_en(SPI1,true); // More to write
    write_addr = w25_write_data(SPI1, lba * 512, (uint8_t *)copy_from, 256);
	
    if (write_addr == 0xFFFFFFFF) {
        debug_println("SPI flash is write protected"); debug_write("\n"); debug_flush();
	}

	w25_write_en(SPI1,true); // More to write
    write_addr =  w25_write_data(SPI1, lba * 512 + 256, (uint8_t *)copy_from, 256);

	if (write_addr == 0xFFFFFFFF) {
        debug_println("SPI flash is write protected"); debug_write("\n"); debug_flush();
    }

    return 0;
}
