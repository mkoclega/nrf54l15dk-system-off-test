// System Power Management

#include <zephyr/devicetree.h>
#include <zephyr/pm/device.h>
#include <zephyr/sys/poweroff.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/drivers/timer/nrf_grtc_timer.h>
#include <zephyr/logging/log.h>
#include <helpers/nrfx_reset_reason.h>

#include "system_pm.h"

LOG_MODULE_REGISTER(system_pm);


#if defined(CONFIG_SERIAL) && DT_NODE_EXISTS(DT_CHOSEN(zephyr_console))

static const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

int uart_console_suspend()
{
    int err = pm_device_action_run(uart_dev, PM_DEVICE_ACTION_SUSPEND);
    if (err == 0)
    {
        enum pm_device_state state;
        do
        {
            err = pm_device_state_get(uart_dev, &state);
		} while (err == 0 && state != PM_DEVICE_STATE_SUSPENDED);
    }
    else
    {
        LOG_ERR("Could not suspend Console (err = %d).", err);
    }

    return err;
}

int uart_console_resume()
{
    int err = pm_device_action_run(uart_dev, PM_DEVICE_ACTION_RESUME);
    if (err == 0)
    {
        enum pm_device_state state;
        do
        {
            err = pm_device_state_get(uart_dev, &state);
		} while (err == 0 && state != PM_DEVICE_STATE_ACTIVE);
    }
    else
    {
        LOG_ERR("Could not resume Console (err = %d).", err);
    }

    return err;
}

#else // defined(CONFIG_SERIAL) && DT_NODE_EXISTS(DT_CHOSEN(zephyr_console))

int uart_console_suspend()
{
    return -ENOTSUP;
}

int uart_console_resume()
{
    return -ENOTSUP;
}

#endif // defined(CONFIG_SERIAL) && DT_NODE_EXISTS(DT_CHOSEN(zephyr_console))

int system_pm_get_reset_cause(uint32_t *p_reset_cause, bool *p_wakeup, bool *p_unexpected)
{
    bool wakeup = false;
    bool unexpected = false;
    uint32_t reset_cause = 0;
    int err = hwinfo_get_reset_cause(&reset_cause);
    if (err != 0)
    {
        k_msleep(2000);     //TODO: remove me
        LOG_ERR("Could not retrieve reset cause (err = %d).", err);
        return err;
    }

	if (reset_cause & RESET_POR)
    {
		LOG_INF("Power-on reset.");
	}
    else if (reset_cause & RESET_PIN)
    {
		LOG_INF("External pin reset.");
	}
    else if (reset_cause & RESET_WATCHDOG)
    {
        unexpected = true;
		LOG_WRN("Watchdog reset.");
	}
    else if (reset_cause & RESET_SOFTWARE)
    {
		LOG_INF("Software reset.");
	}
    else if (reset_cause & RESET_CPU_LOCKUP)
    {
        unexpected = true;
		LOG_WRN("CPU lock-up reset.");
	}
    else if (reset_cause & RESET_DEBUG)
    {
		LOG_INF("Debugger reset.");
	}
    else if (reset_cause & RESET_SECURITY)
    {
        unexpected = true;
		LOG_WRN("Tamper detect reset.");
	}
    else if (reset_cause & RESET_CLOCK)
    {
        wakeup = true;
		LOG_INF("Wake-up from System OFF by GRTC.");
	}
    else if (reset_cause & RESET_LOW_POWER_WAKE)
    {
        wakeup = true;
		LOG_INF("Wake-up from System OFF by DETECT/LPCOMP/NFC.");
	}
    else
    {
        uint32_t reset_cause_nrfx = nrfx_reset_reason_get();
        k_msleep(2000);     //TODO: remove me
        LOG_WRN("nrfx reset cause 0x%08X.", reset_cause_nrfx);

        unexpected = true;
		LOG_WRN("Other reset cause 0x%08X.", reset_cause);
	}

    if (p_reset_cause   != NULL) *p_reset_cause = reset_cause;
    if (p_wakeup        != NULL) *p_wakeup      = wakeup;
    if (p_unexpected    != NULL) *p_unexpected  = unexpected;

	return err;
}

int system_pm_system_off(uint32_t time_ms)
{
    int err = z_nrf_grtc_wakeup_prepare((uint64_t)time_ms * USEC_PER_MSEC);
    if (err == 0)
    {
        LOG_INF("Entering System OFF, wait %u ms for wake-up.", time_ms);
	}
    else
    {
		LOG_ERR("Unable to prepare GRTC as a wake-up source (err = %d).", err);
		return err;
	}

    uart_console_suspend();

    hwinfo_clear_reset_cause();
	sys_poweroff();

	// in fact, this function never returns
    return 0;
}
