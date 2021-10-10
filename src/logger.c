#include <string.h>
#include <stdbool.h>
#include <stdint.h>


#include <libopencm3/cm3/vector.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencmsis/core_cm3.h> //
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/f1/bkp.h>
//#include <libopencm3/stm32/spi.h>

#include "board.h"
#include "common.h"
#include "ds18b20.h"
//#include "systick.h"

#include "debug.h"
#include "backup.h"
#include "rtc.h"

// USB  part
/* Include files necesary for USB & MSC */
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/msc.h>
#include "usb_conf.h"

//part which is from https://github.com/Mark-271/kitchen-clock-poc
#include "irq.h"
//#include "sched.h"
//#include "swtimer.h"

#include "libprintf/printf.h"

#define GET_TEMP_DELAY		9 /* sec */

static void show_temp(void);
static void blink_led(void);

static struct rtc_device rtc = {
    	.clock_source = RCC_LSE,
    	.prescale_val = 32768, // 7FFFh
    	.time = {0},
    	.alarm = GET_TEMP_DELAY,
    	.cb =show_temp,    //blink_led, //show_temp,
	};

static struct ds18b20 ts = {
	.port = DS18B20_GPIO_PORT,
	.pin = DS18B20_GPIO_PIN,
};


static void show_temp(void) {
//static void show_temp(void * param) {
	unsigned long flags;
    char buf[20];

    char *temp1, *temp2;
    uint8_t id1[] = {0x28, 0xc6, 0x19, 0x5a, 0x30, 0x20, 0x01, 0x3e};
    uint8_t id2[] = {0x28, 0x81, 0x8d, 0x4e, 0x30, 0x20, 0x01, 0x50};

    ds18b20_convert_temp(&ts); // Send to all sensors on the bus to start convert temperature

    enter_critical(flags);
	mdelay(750);
	exit_critical(flags);

    // ts.temp = ds18b20_read_temp(&ts);
    ts.temp = ds18b20_read_temp_by_id(&ts, id1);
    while (ts.temp.frac > 9)
		ts.temp.frac /= 10;
	temp1 = ds18b20_temp2str(&ts.temp, buf);
    printf("Temp1 = %s ", temp1);

	memset(buf, 0, 20);

	ts.temp = ds18b20_read_temp_by_id(&ts, id2);
    while (ts.temp.frac > 9)
  	    ts.temp.frac /= 10;
    temp2 = ds18b20_temp2str(&ts.temp, buf);
    printf("Temp2 = %s \n", temp2);
}


__attribute__((unused)) static void blink_led(void) {
//static void blink_led(void * param) {

    //UNUSED(param);
    gpio_toggle(LED_PORT, LED_PIN);
}


static void init (void) {

    int err;

	irq_init();

    board_init();

    err = ds18b20_init(&ts);
    if (err) {
        printf("Can't initialize ds18b20: %d\n", err);
    }

	err = rtc_init(&rtc);                    // Start RTC interrupts
    if (err) {
        printf("Can't initialize RTC: %d\n", err);
    }

   /*
    ds18b20_read_id(&ts);
    size_t i;
    for(i = 0; i < 8; i++)
    {
       printf("id[ %d ] = %x \n", i , ts.id[i]);
    }
    */

	set_alarm(rtc.alarm);	                // Set alarm to 10 sec.

    gpio_clear(LED_PORT,LED_PIN);     // PC13 = on
    //gpio_set(LED_PORT,LED_PIN);     // PC13 = on
}

int main(void) {

    enable_log();        //  Uncomment to allow display of debug messages in development devices

	/* Enable power and backup interface clocks. */
    //rcc_periph_clock_enable(RCC_PWR);
    //rcc_periph_clock_enable(RCC_BKP);

	#ifdef POWER_DOWN

    if ( pwr_get_standby_flag() ){
        pwr_clear_standby_flag();
        printf("Awake from SB \n");
    };

	#endif

	init();

	//    rcc_periph_reset_pulse(RST_USB); // TO DO check function
	/* Override hard-wired USB pullup to disconnect and reconnect */
   	/*gpio_clear(USB_PORT, USB_DP_PIN);
    gpio_clear(GPIOA, GPIO12);
    int i;
    for (i = 0; i < 800000; i++) {
        __asm__("nop");
	}
	*/
	
	char serial[USB_SERIAL_NUM_LENGTH+1];
    serial[0] = '\0';
    target_get_serial_number(serial, USB_SERIAL_NUM_LENGTH);
    usb_set_serial_number(serial);

	usbd_device* usbd_dev = usb_setup();

	#ifdef POWER_DOWN
	// RM0008 5.3.5 Standby mode
    SCB_SCR |= SCB_SCR_SLEEPDEEP; // Set SLEEPDEEP in Cortex ® -M3 System Control register
    pwr_set_standby_mode();       // Set PDDS bit in Power Control register
    pwr_clear_wakeup_flag();     //
	__WFI();
	#endif

	while (1) {

        usbd_poll(usbd_dev);
    }

    return 0;
}

