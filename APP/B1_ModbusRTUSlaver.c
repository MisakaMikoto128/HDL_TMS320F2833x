/**
 * @file B1_ModbusRTUSlaver.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 * @last modified 2024-05-16
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "mb.h"
#include "ccommon.h"
#include "period_query.h"
#include "BFL_RS485.h"
#include "HDL_CPU_Time.h"
#include "HDL_Uart.h"
#include "APP_Main.h"
void B1_ModbusRTUSlaver_Init()
{
    /*
     * RTU模式 从机地址：0x01 串口：这里不起作用，随便写 波特率：115200 无奇偶校验位
     */
    eMBInit(MB_RTU, 0x01, 0, 115200, MB_PAR_NONE);
    eMBEnable();
}

void B1_ModbusRTUSlaver_Poll()
{
    static PeriodREC_t s_tPollTime = 0;
    if_period_query_user(&s_tPollTime, 1)
    {
        eMBPoll();
    }
}
