
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


#include "board.h"
#include "common.h"
#include "ds18b20.h"
#include "systick.h"

#include "debug.h"
#include "backup.h"
#include "rtc.h"

//part which is from https://github.com/Mark-271/kitchen-clock-poc
#include "irq.h"
#include "sched.h"
#include "swtimer.h"

#include "libprintf/printf.h"


#define GET_TEMP_DELAY		10000 /* msec */

static struct rtc_device rtc = {
    .time = {0},
    .alarm = 0,
};

static struct ds18b20 ts = {
	.port = DS18B20_GPIO_PORT,
	.pin = DS18B20_GPIO_PIN,
};


static void show_temp(void * param) {
    
    UNUSED(param);

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


static void blink_led(void * param) {
    
    UNUSED(param);
    gpio_toggle(LED_PORT, LED_PIN);
}


static void init (void) {
    
    int err;
   
    const struct swtimer_hw_tim hw_tim = {
    	.base = SWTIMER_TIM_BASE,
    	.irq = SWTIMER_TIM_IRQ,
    	.rst = SWTIMER_TIM_RST,
    	.arr = SWTIMER_TIM_ARR_VAL,
    	.psc = SWTIMER_TIM_PSC_VAL,
    };

	irq_init();

    board_init();

    sched_init();

	err = systick_init();
	if (err) {
		printf("Can't initialize systick\n");
		hang();
	}

	err = swtimer_init(&hw_tim);
	if (err) {
		printf("Can't initialize swtimer\n");
		hang();
	}

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

    /* Register timer for DS18B20 */
    int temp_tim_id;
	
    temp_tim_id = swtimer_tim_register(show_temp, NULL, GET_TEMP_DELAY);
	if (temp_tim_id < 0) {
		printf("Unable to register swtimer for DS18B20\n");
		hang();
	}

    /* Register timer for LED */
    /*
    int led_tim_id;
    led_tim_id = swtimer_tim_register(blink_led, NULL, 125); // 250 ms
	if (led_tim_id < 0) {
		printf("Unable to register swtimer for LED\n");
	    hang();
    }
    */
    
    rtc.alarm = 10;

	set_alarm(rtc.alarm);	                // Set alarm to 10 sec.

    gpio_set(LED_PORT,LED_PIN);     // PC13 = on
}

int main(void) {

    enable_log();        //  Uncomment to allow display of debug messages in development devices
    
    init();

    sched_start();

    return 0;
}

