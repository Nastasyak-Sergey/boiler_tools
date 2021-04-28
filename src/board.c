#include <board.h>
#include <common.h>



enum pull_mode {
    PULL_NO = 0,
    PULL_UP,
    PULL_DOWN,
};

struct pin_mode {
    uint32_t port;
    uint16_t pins;
    uint8_t mode;
    uint8_t conf;
    enum pull_mode pull;
};

static struct pin_mode pins[] = {
    {
        .port = LED_PORT,
        .pins = LED_PIN,
        .mode = GPIO_MODE_OUTPUT_2_MHZ,
        .conf = GPIO_CNF_OUTPUT_PUSHPULL,
    },
    {
        .port = DS18B20_GPIO_PORT,
        .pins = DS18B20_GPIO_PIN,
        .mode = GPIO_MODE_OUTPUT_10_MHZ,
        .conf = GPIO_CNF_OUTPUT_OPENDRAIN,
    },
    {
        .port = USB_PORT,
        .pins = USB_DP_PIN | USB_DN_PIN,
        .mode = GPIO_MODE_OUTPUT_10_MHZ,
        .conf = GPIO_CNF_OUTPUT_PUSHPULL,
    },
    {
        .port = SPIFLASH_PORT,
        .pins = SPIFLASH_NSS_PIN | SPIFLASH_SCK_PIN | SPIFLASH_MOSI_PIN,
        .mode = GPIO_MODE_OUTPUT_50_MHZ,
        .conf = GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
    },
    {
        .port = SPIFLASH_PORT,
        .pins = SPIFLASH_MISO_PIN,
        .mode = GPIO_MODE_INPUT,
        .conf = GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, 
     },
};

enum rcc_periph_clken clocks[] = {
    LED_RCC,
    SPIFLASH_GPIO_RCC,
    SPIFLASH_RCC,
    USB_GPIO_RCC,
    USB_RCC,
    DS18B20_GPIO_RCC,
	SWTIMER_TIM_RCC,
};


/**
 * Configure which internal pull resistor to use if it's set (up or down).
 * @param mode Pins to configure pull resistors
 */
static void board_config_pull(const struct pin_mode *mode)
{
	unsigned long flags;
	uint16_t port;

	if (!mode->pull)
		return;

	enter_critical(flags);
	port = gpio_port_read(mode->port);
	switch (mode->pull) {
	case PULL_UP:
		port |= mode->pins;
        break;
	case PULL_DOWN:
		port &= ~mode->pins;
		break;
	default:
		break;
	}
	gpio_port_write(mode->port, port);
	exit_critical(flags);
}

static void board_pinmux_init(void)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(pins); ++i) {
		gpio_set_mode(pins[i].port, pins[i].mode, pins[i].conf,
			      pins[i].pins);
		board_config_pull(&pins[i]);
	}
}

static void board_clock_init(void)
{
	size_t i;

	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	for (i = 0; i < ARRAY_SIZE(clocks); ++i)
		rcc_periph_clock_enable(clocks[i]);
}

int board_init(void)
{
	board_clock_init();
	board_pinmux_init();
	return 0;
}


