/*
 * Copyright (c) 2025, Fusion Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-08-27     Wyj          Architecture merge
 * 2026-03-04     Alex.min     Add CPU load detection function.
 */

#include "tc387_def.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"
#include "board.h"
#include "schedule.h"
#include "Bsp.h"

#if (USE_DEBUG_UART)
#include "log.h"
#include "ASCLIN_UART.h"
#endif
#if (USE_CAN)
#include "can_device.h"
#include "mcmcan_Interface.h"
#endif
#if (USE_LIN)
#include "asclin.h"
#endif
#if (USE_OTA)
#include "ota.h"
#endif

void core0_main(void)
{
    /* Enable the global interrupts of this CPU */
    IfxCpu_enableInterrupts();

    /* !!WATCHDOG0 AND SAFETY WATCHDOG ARE DISABLED HERE!!
     * Enable the watchdogs and service them periodically if it is required
     */
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());

    /* Initialize the Shell Interface and the UART communication */
#if (USE_DEBUG_UART)
    early_console_init();
#endif

#if (USE_CPU0_LOAD)
    init_cpu0_load_timer();
#endif

#if (USE_CAN)
    {
        device_t device;
        hw_can_init();
        for (uint8_t num = 0; num < 4; num++) {
            device = channel_find_hwcan(num);
            device_init(device);
            can_default_cfg(device);
        }
    }
#endif
#if (USE_LIN)
    {
        device_t device_lin;
        hw_ascLin_init();
        for (uint8_t num = 0; num < LIN_CHANNEL_NUM; num++) {
            device_lin = channel_find_hwlin(num);
            device_init(device_lin);
        }
    }
#endif
#if (USE_OTA)
    {
        device_t device_ota;
        hw_ota_init();
        device_ota = hw_ota_find(0);
        device_init(device_ota);
        device_open(device_ota, DEVICE_OFLAG_RDWR);
    }
#endif

    set_core0_status();

#if (USE_CAN)
    init_CAN_PHY();
#endif

    main_schedule();/*OTA FLASH_EARSE USE*/
}
