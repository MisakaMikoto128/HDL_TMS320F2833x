/**
 * @file APP_Main_Test.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-11-02
 * @last modified 2024-11-02
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "APP_Main_Test.h"
#include "B0_DeltaPoll.h"
#include "B2_EventRecord.h"
#include "BFL_Button.h"
#include "BFL_Buzz.h"
#include "BFL_DebugPin.h"
#include "BFL_Measure.h"
#include "BFL_RS485.h"
#include "BFL_SCR.h"
#include "BFL_VCB.h"
#include "datetime.h"
#include "CHIP_W25Q128.h"
#include "CPU_Define.h"
#include "DSP2833x_Device.h"
#include "HDL_CPU_Time.h"
#include "HDL_IWDG.h"
#include "HDL_Uart.h"
#include "ccommon.h"
#include "crc.h"
#include "log.h"
#include "mtime.h"
#include "period_query.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "CHIP_PCF8563.h"
void BFL_VCB_Relay_Set_As_Switch_Closed(BFL_VCB_SW_t vcb);
void BFL_VCB_Relay_Set_As_Switch_Opened(BFL_VCB_SW_t vcb);

mtime_t CurTime;

void APP_Main_Test_Poll()
{

    CHIP_PCF8563_Init();
    PeriodREC_t p1 = 0, p2 = 0;
    for (;;)
    {
        if_period_query_user(&p1, 1000)
        {
            CHIP_PCF8563_Get(&CurTime); // 读取时间
            HDL_IWDG_Feed();
        }
    }
}
