#ifndef BOARD_H
#define BOARD_H


#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>


/* Vector table size (=sizeof(vector_table) */
#define CONFIG_VTOR_SIZE		0x150

/* GPIO level transient time, usec */
#define CONFIG_GPIO_STAB_DELAY		10

/* LED on board */
#define LED_RCC             RCC_GPIOC
#define LED_PORT            GPIOC
#define LED_PIN             GPIO13

/* USB */
#define USB_RCC             RCC_USB
#define USB_GPIO_RCC        RCC_GPIOA
#define USB_PORT            GPIOA
#define USB_DP_PIN          GPIO12
#define USB_DN_PIN          GPIO11


/* SPI Flash NOR flash */
#define SPIFLASH_RCC        RCC_SPI1
#define SPIFLASH_GPIO_RCC   RCC_GPIOA
#define SPIFLASH_PORT       GPIOA
#define SPIFLASH_NSS_PIN    GPIO4
#define SPIFLASH_SCK_PIN    GPIO5
#define SPIFLASH_MOSI_PIN   GPIO7
#define SPIFLASH_MISO_PIN   GPIO6

/* Temperature sensor */
#define DS18B20_GPIO_RCC	RCC_GPIOB
#define DS18B20_GPIO_PORT	GPIOB
#define DS18B20_GPIO_PIN	GPIO10

/* General purpose timer */
#define SWTIMER_TIM_RCC		RCC_TIM2
#define SWTIMER_TIM_BASE	TIM2
#define SWTIMER_TIM_IRQ		NVIC_TIM2_IRQ
#define SWTIMER_TIM_RST		RST_TIM2
#define SWTIMER_TIM_ARR_VAL	5000-1 // 19999
#define SWTIMER_TIM_PSC_VAL	36-1    //5


int board_init(void);

#endif /* BOARD_H */
