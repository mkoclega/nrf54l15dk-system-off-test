// System Power Management

#ifndef SYSTEM_PM_H
#define SYSTEM_PM_H

#include <stdbool.h>
#include <stdint.h>


int system_pm_get_reset_cause(uint32_t *p_reset_cause, bool *p_wakeup, bool *p_unexpected);
int system_pm_system_off(uint32_t time_ms);


#endif // SYSTEM_PM_H
