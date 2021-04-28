
#include "usb_it.h"

#include "irq.h"
#include "common.h"
#include "board.h"
#include "backup.h"
#include "libprintf/printf.h"

#define USB_IRQS 3


static irqreturn_t rtc_alarm_isr_handler(int irq, void *data)
{
	UNUSED(irq);
    struct rtc_device *obj = (struct rtc_device *)(data);
    
    if ( rtc_check_flag(RTC_ALR) ) {
	    rtc_clear_flag(RTC_ALR);
        printf("ALARM at %d\n", rtc_get_counter_val());
    }

    obj->cb();
    
    return IRQ_HANDLED;
}



/* Store irq objects */
static struct irq_action usb_irq_act[USB_IRQS] = {
	{
		.handler = usb_hp_isr_handler,
		.irq = NVIC_USB_HP_CAN_TX_IRQ,
		.name = "usb_hp_isr",
	},
	{
		.handler = usb_lp_isr_handler,
		.irq = NVIC_USB_LP_CAN_RX0_IRQ
		.name = "usb_lp_isr,",
	},
	{
		.handler = usb_wakeup_isr_handler,
		.irq = NVIC_USB_WAKEUP_IRQ,
		.name = "usb_wakeup_isr,",
	}

};


int rtc_init(struct rtc_device *obj)
{
    int ret;
	unsigned long flags;
    uint32_t reg32;

    //    32768 = 7FFFh
    //    rtc_auto_awake (RCC_LSE, 32768);	/* rtc_auto_awake (RCC_LSE, 32767); does next tasks */

    reg32 = rcc_rtc_clock_enabled_flag();
	if (reg32 != 0) {
		rtc_awake_from_standby();
	} else {
		rtc_awake_from_off(obj->clock_source);
		rtc_set_prescale_val(obj->prescale_val);
	}

    /* Register interrupt handlers */
	int i;
    for (i = 0; i < RTC_IRQS; i++) {
		rtc_irq_act[i].data = (void *)obj;
		ret = irq_request(&rtc_irq_act[i]);
		if (ret < 0) {
            printf("Unable request RTC IRQ\n");
			return ret;
        }
	}

	//printf("BKP reg [0]: %x \n", BKP_DR1);

	if( BKP_DR1 == CONFIGURATION_RESET) {

        BKP_DR1 = CONFIGURATION_DONE;
		printf("Switchen ON first time\n");
	}

	rtc_interrupt_disable(RTC_SEC);
	rtc_interrupt_disable(RTC_ALR);
	//rtc_interrupt_disable(RTC_OW);

    nvic_enable_irq(NVIC_RTC_ALARM_IRQ);
    nvic_set_priority(NVIC_RTC_ALARM_IRQ, 2);

	exti_set_trigger(EXTI17, EXTI_TRIGGER_RISING);
	exti_enable_request(EXTI17);

	//nvic_enable_irq(NVIC_RTC_IRQ);
	//nvic_set_priority(NVIC_RTC_IRQ, 2);

	enter_critical(flags);
    //cm_disable_interrupts();

    rtc_clear_flag(RTC_SEC);
	rtc_clear_flag(RTC_ALR);
    // rtc_clear_flag(RTC_OW);
	// rtc_interrupt_enable(RTC_SEC);
	rtc_interrupt_enable(RTC_ALR);
	//rtc_interrupt_enable(RTC_OW);

    //cm_enable_interrupts();
	exit_critical(flags);
}
