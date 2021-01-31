// TO DO list
// - usb msc
// - spi winbond flash read signature ID
// - populate flash size by usb msc volume
//

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

/* Include files necesary for USB & MSC */
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/msc.h>
 
#include "winbond.h"
#include "usb_conf.h"
#include "debug.h"
#include "backup.h"

#include "ramdisk.h"

#include "libprintf/printf.h"

// USE_SEMIHOSTING is defined via our Makefile
#if defined(USE_SEMIHOSTING) && USE_SEMIHOSTING
// Usually we try not to use standard C library. But in this case we need printf
//#include <stdio.h>

// this is our magic from librdimon
// should be called in main before first output
//extern void initialise_monitor_handles(void);
#else
// this is how semihosting may be used conditionally
#error "This example requires SEMIHOSTING=1"
#endif


static void spi_setup(void) {

    rcc_periph_clock_enable(RCC_SPI1);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                  GPIO4|GPIO5|GPIO7 );       // NSS=PA4,SCK=PA5,MOSI=PA7
    
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO6); // MISO=PA6

    spi_reset(SPI1);
    spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_256, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);
    
    spi_disable_software_slave_management(SPI1); // R 10k already soldered on flash board
    spi_enable_ss_output(SPI1);
 
    #ifdef SPI_MANUAL    
    w25_spi_setup( SPI1,		// SPI1 or SPI2
        true,		// True for 8-bits else 16-bits
        true,	    // True if MSB first else LSB first
        true,		// True if mode 0 else mode 3
  	    SPI_CR1_BAUDRATE_FPCLK_DIV_64); // E.g. SPI_CR1_BAUDRATE_FPCLK_DIV_256
    #endif

}

int main(void) {

    enable_log();        //  Uncomment to allow display of debug messages in development devices
	//debug_print_hex(254);
	//debug_print_int(-567);
    
//  initialise_monitor_handles();
	printf("Start main()\n");

	rcc_clock_setup_in_hse_8mhz_out_72mhz();	// Use this for stm32f103

	/* Enable GPIOC clock. */
	rcc_periph_clock_enable(RCC_GPIOC);

	/* Set GPIO8 (in GPIO port C) to 'output push-pull'. */
	gpio_set_mode(GPIOC,GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,GPIO13);

	/*	USB configuration section	*/
	// PA11=USB_DM, PA12=USB_DP
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_USB);
	/* Drive the USB DP pin to override the pull-up */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ,GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
 
    rcc_periph_reset_pulse(RST_USB); // TO DO check function
    
	/* Override hard-wired USB pullup to disconnect and reconnect */
    gpio_clear(GPIOA, GPIO12);
 
	/*	Set LED on Black Pill*/
    gpio_set(GPIOC,GPIO13);    // PC13 = on
    
    spi_setup();

    char serial[USB_SERIAL_NUM_LENGTH+1];
    serial[0] = '\0';
    target_get_serial_number(serial, USB_SERIAL_NUM_LENGTH);
    usb_set_serial_number(serial);


	usbd_device* usbd_dev = usb_setup();

	while (1) {
 

        usbd_poll(usbd_dev);
   
    }
    return 0;

}

