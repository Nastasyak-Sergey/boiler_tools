
#include <string.h>
#include <stdbool.h>

#include <libopencm3/cm3/vector.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/f1/bkp.h>
#include <libopencm3/stm32/spi.h>


#include "winbond.h"
#include "backup.h"



#define FLASH_PAGE_SIZE         256
#define FLASH_SECTOR_SIZE       4096
#define FLASH_NUMBER_OF_PAGES   32768
#define FLASH_DISK_SIZE         FLASH_NUMBER_OF_PAGES * FLASH_PAGE_SIZE


static void spi_setup(void) {

    #ifdef SPI_MANUAL    
    rcc_periph_clock_enable(RCC_SPI1);

    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
       GPIO4|GPIO5|GPIO7 );       // NSS=PA4,SCK=PA5,MOSI=PA7
    
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO6); // MISO=PA6

    spi_reset(SPI1);
    spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_256, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
        SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);
    
    spi_disable_software_slave_management(SPI1); // R 10k already soldered on flash board
    spi_enable_ss_output(SPI1);
 
    #endif
    w25_spi_setup( SPI1,		// SPI1 or SPI2
        true,		// True for 8-bits else 16-bits
        true,	    // True if MSB first else LSB first
        true,		// True if mode 0 else mode 3
  	    SPI_CR1_BAUDRATE_FPCLK_DIV_256); // E.g. SPI_CR1_BAUDRATE_FPCLK_DIV_256

}


	uint32_t info = 0;
	uint8_t w25_buff[512] ={0};
    uint8_t uid_buf[8] = {0};





int main(void) 
{
    
    rcc_clock_setup_in_hse_8mhz_out_72mhz();	// Use this for stm32f103
	rcc_periph_clock_enable(RCC_GPIOC);
	gpio_set_mode(GPIOC,GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,GPIO13);
	rcc_periph_clock_enable(RCC_GPIOA);

    spi_setup();

    if (w25_is_wprotect (SPI1)) {
	    w25_write_en(SPI1, true);
    }

    w25_erase_block	(SPI1, 0, W25_CMD_ERA_SECTOR);


    if (w25_is_wprotect (SPI1)) {
	    w25_write_en(SPI1, true);
    }


	for (uint16_t s = 0; s < FLASH_PAGE_SIZE; ++s) {
        w25_buff[s] = s;
    }
     
    info = w25_write_data(SPI1, 0, w25_buff, FLASH_PAGE_SIZE);

	if (0xffffffff == info) {
		while (1) {};
	}
	
	memset(w25_buff, 0, 255);

    w25_read_data(SPI1, 0, w25_buff, FLASH_PAGE_SIZE);
 
	while (1) {
 
	}

    return 0;

}

