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
#include "HDL_RTC.h"

#include "APP_Main.h"
#include "BFL_SCR.h"
#include "BFL_VCB.h"
#include "async_delay.h"
#include "mtime.h"
#include <stddef.h>
#include "B2_EventRecord.h"

void BFL_VCB_Relay_Set_As_Switch_Closed(BFL_VCB_SW_t vcb);
void BFL_VCB_Relay_Set_As_Switch_Opened(BFL_VCB_SW_t vcb);
mtime_t settingTime;
uint64_t ts_device_utc_ms = 0;
mtime_t CurTime;
mtime_t pTime;

static void async_delay_callback_vot_check(void *arg)
{
    void **args = (void **)arg;
    uint32_t *scrtFb = (uint32_t *)args[0];
    B1_Measure_t *begin_measure = (B1_Measure_t *)args[1];
    B1_Measure_t current_measure;
    B1_Measure_Read(&current_measure);

    const float th = 0.5f;
    if (g_pSysInfo->V_UIAB > g_pSysInfo->V_SYS_STOP_kV)
    {
        if (current_measure.V_TV1A < begin_measure->V_TV1A * th)
        {
            *scrtFb &= ~(BFL_SCRR1A | BFL_SCRR1B);
        }

        if (current_measure.V_TV1B < begin_measure->V_TV1B * th)
        {
            *scrtFb &= ~(BFL_SCRR2A | BFL_SCRR2B);
        }

        if (current_measure.V_TV1C < begin_measure->V_TV1C * th)
        {
            *scrtFb &= ~(BFL_SCRR3A | BFL_SCRR3B);
        }
    }
    else
    {
        *scrtFb = 0;
    }

    BackGroundTask_WhenInSRCPoll();
}

uint16_t SCRT_Fault_res = 0;
uint16_t SCRT_Fault_cnt = 0;

extern void AppMainInfo_Init();
extern void Config_Default_Parameter();
extern void Config_PowerOn_Parameter();
extern bool Load_Parameter_From_Flash();
void APP_Main_Test_Poll()
{
    HDL_CPU_Time_Init();
//    B1_VCBStatusGet_Init();
//    BFL_VCB_Seurity_Init();
//    BFL_SCR_Init();
//    B1_SysModeGet_Init();
//    BFL_DebugPin_Init();
    datetime_init();
//    BFL_Buzz_Init();
//    CHIP_W25Q128_Init();
//
//    AppMainInfo_Init();
//    Config_Default_Parameter();
//    Load_Parameter_From_Flash();
//    Config_PowerOn_Parameter();


    PeriodREC_t p1 = 0, p2 = 0;

    pTime.nSec = 11;
    pTime.nMin = 11;
    pTime.nHour = 11;
    pTime.nDay = 11;
    pTime.nWeek = 0;
    pTime.nMonth = 11; // 包含了世纪位信息
    pTime.nYear = 2024;
//     HDL_RTC_SetStructTime(&pTime);
//    CHIP_PCF8563_Set(&pTime);
    for (;;)
    {
        if_period_query_user(&p1, 1000)
        {
            CHIP_PCF8563_Get(&CurTime); // 读取时间
            ts_device_utc_ms = datetime_get_unix_ms_timestamp();
            mtime_unix_sec_2_time((uint32_t)(ts_device_utc_ms / 1000), &settingTime);
        }
//        HDL_IWDG_Feed();

//        if_period_query_user(&p2, 5000)
//        {
//            uint16_t SCRT_Fault = 0;
//            uint32_t scrtFb = 0;
//            BFL_SCRT_Pluse_Transmit(SCRT_ALL, MS(g_pSysInfo->T2_MS), US(4000));
//            B1_Measure_t measures;
//            B1_Measure_Read(&measures);
//            void *args[] = {&scrtFb, &measures};
//
//            SCRT_Fault = BFL_SCRR1A | BFL_SCRR1B | BFL_SCRR2A | BFL_SCRR2B | BFL_SCRR3A | BFL_SCRR3B;
//            async_delay(MS(g_pSysInfo->T4_MS), async_delay_callback_vot_check, args);
//            SCRT_Fault = scrtFb;
//
//            if (SCRT_Fault_res != SCRT_Fault)
//            {
//                SCRT_Fault_res = SCRT_Fault;
//            }
//
//            if (SCRT_Fault)
//            {
//                SCRT_Fault_cnt++;
//            }
//        }
    }
}
