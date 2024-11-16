/********************************************************************************************************
 * @file    main.c
 *
 * @brief   This is the source file for main
 *
 * @author  Zigbee Group
 * @date    2021
 *
 * @par     Copyright (c) 2021, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *			All rights reserved.
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/

#include "zb_common.h"
#include "ext_ota.h"
#include "chip_8258/register.h"

extern void user_init(bool isRetention);

/*
 * main:
 * */
#if ZIGBEE_TUYA_OTA
_attribute_ram_code_sec_
#endif
int main(void){
#if ZIGBEE_TUYA_OTA
	if(*(u32 *)(0x08008) == 0x544c4e4b) {
		//clock_init(SYS_CLK_24M_Crystal);
		tuya_zigbee_ota();
	}
#endif
    // g_zb_txPowerSet = RF_TX_POWER_DEF; // RF_POWER_INDEX_P1p99dBm; mac_phy.c
	startup_state_e state = drv_platform_init();

	u8 isRetention = (state == SYSTEM_DEEP_RETENTION) ? 1 : 0;

	os_init(isRetention);

	/* reduce power consumption, disable CLK disable CLK of unused peripherals*/

	reg_clk_en0 = 0 // FLD_CLK0_SPI_EN
#if UART_PRINTF_MODE
			| FLD_CLK0_UART_EN
#endif
			| FLD_CLK0_SWIRE_EN;

	user_init(isRetention);

	printf("main()");

	irq_enable(); // drv_enable_irq();

#if (MODULE_WATCHDOG_ENABLE)
	drv_wd_setInterval(600);
    drv_wd_start();
#endif

#if VOLTAGE_DETECT_ENABLE
    u32 tick = clock_time();
#endif

	while(1){
#if VOLTAGE_DETECT_ENABLE
		if(clock_time_exceed(tick, 200 * 1000)) { // 200 ms
			batery_detect();
			tick = clock_time();
		}
#endif

    	ev_main();

#if (MODULE_WATCHDOG_ENABLE)
		drv_wd_clear();
#endif

		tl_zbTaskProcedure();

#if	(MODULE_WATCHDOG_ENABLE)
		drv_wd_clear();
#endif
	}

	return 0;
}


