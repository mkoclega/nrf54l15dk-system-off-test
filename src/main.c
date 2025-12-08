#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/hwinfo.h>

#include "system_pm.h"

LOG_MODULE_REGISTER(main);


#define SLEEP_TIME_MS 5000

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios);


static void flash_led(uint32_t reset_cause, bool wakeup)
{
	if (wakeup)
		gpio_pin_set_dt(&led0, 1);
	if (reset_cause & RESET_POR)
		gpio_pin_set_dt(&led1, 1);
	if (reset_cause & RESET_PIN)
		gpio_pin_set_dt(&led2, 1);
	if (reset_cause & RESET_DEBUG)
		gpio_pin_set_dt(&led3, 1);

	k_msleep(200);

    if (wakeup)
		gpio_pin_set_dt(&led0, 0);
	if (reset_cause & RESET_POR)
		gpio_pin_set_dt(&led1, 0);
	if (reset_cause & RESET_PIN)
		gpio_pin_set_dt(&led2, 0);
	if (reset_cause & RESET_DEBUG)
		gpio_pin_set_dt(&led3, 0);
}

static int config_hw()
{
	if (!gpio_is_ready_dt(&led0) ||
		!gpio_is_ready_dt(&led1) ||
		!gpio_is_ready_dt(&led2) ||
		!gpio_is_ready_dt(&led3))
	{
		return -1;
	}

	int err;
	err = gpio_pin_configure_dt(&led0, GPIO_OUTPUT | GPIO_ACTIVE_HIGH);
	if (err != 0) return err;
	err = gpio_pin_configure_dt(&led1, GPIO_OUTPUT | GPIO_ACTIVE_HIGH);
	if (err != 0) return err;
	err = gpio_pin_configure_dt(&led2, GPIO_OUTPUT | GPIO_ACTIVE_HIGH);
	if (err != 0) return err;
	err = gpio_pin_configure_dt(&led3, GPIO_OUTPUT | GPIO_ACTIVE_HIGH);
	return err;
}

int main(void)
{
	// retrieve reset cause
	uint32_t reset_cause = 0;
	bool wakeup = false;
	system_pm_get_reset_cause(&reset_cause, &wakeup, NULL);

	if (config_hw() == 0)
	{
		flash_led(reset_cause, wakeup);
	}

	// System OFF
	return system_pm_system_off(SLEEP_TIME_MS);
}
