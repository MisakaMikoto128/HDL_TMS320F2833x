/**
 * @file B1_CapacitanceTemperatureMeasure.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 * @last modified 2024-05-16
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */

#include "crc.h"
#include "ccommon.h"
#include "period_query.h"
#include "BFL_RS485.h"
#include "HDL_CPU_Time.h"
#include "HDL_Uart.h"
#include "APP_Main.h"
#include <stdlib.h>

void B1_CapacitanceTemperatureMeasure_Init()
{
    BFL_RS485_Init(RS485_1, 38400, UART_WORD_LEN_8, UART_STOP_BIT_1,
                   UART_PARITY_NONE); // 隔离 MAX3485

    for (size_t i = 0; i < (sizeof(g_pSysInfo->capTemp) / sizeof(g_pSysInfo->capTemp[0])); i++)
    {
        g_pSysInfo->capTemp[i] = 0;
    }
}

void B1_CapacitanceTemperatureMeasure_Poll()
{
    static PeriodREC_t s_tPollTime = 0;
    if(!period_query_user(&s_tPollTime, 100))
    {
        return;
    }

    switch (g_AppMainInfo.capTempSensorRequestStage)
    {
    case 0:
        const byte_t request_cmd[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB};
        BFL_RS485_Write(RS485_1, request_cmd, sizeof(request_cmd));
        g_AppMainInfo.capTempSensorRequestStage = 1;
        break;
    case 1:
    {
        g_AppMainInfo.capTempSensorRequestStage = 0;
        
        uint32_t readLen = BFL_RS485_Read(RS485_1, g_AppMainInfo.buffer, sizeof(g_AppMainInfo.buffer));
        if (readLen > 0)
        {
            g_pSysInfo->capTempSensorTransmitConnect = true;
            g_AppMainInfo.capTempSensorLastRevMsgTime_Ms = HDL_CPU_Time_GetTick();
            byte_t *dataBuffer = g_AppMainInfo.buffer;
            // Check CRC
            if (CRC16_Modbus(dataBuffer, readLen) == 0)
            {
                // Parse data eg: 01 03 06 FF FF 01 05 00 00 30 93
                int regNum = dataBuffer[2] / 2;
                byte_t *regStart = &dataBuffer[3];
                uint16_t reg = 0;
                int tempChannelNum = sizeof(g_pSysInfo->capTemp) / sizeof(g_pSysInfo->capTemp[0]);
                regNum = regNum > tempChannelNum ? tempChannelNum : regNum;
                for (int i = 0; i < regNum; i++)
                {
                    reg = regStart[i * 2] << 8 | regStart[i * 2 + 1];
                    if (reg == 0xFFFF)
                    {
                        g_pSysInfo->capTemp[i] = 0;
                        g_pSysInfo->capTempSensorFault[i] = true;
                    }
                    else
                    {
                        g_pSysInfo->capTemp[i] = reg * 0.1f;
                        g_pSysInfo->capTempSensorFault[i] = false;
                    }
                }
            }
        }
        else
        {
            if (g_pSysInfo->capTempSensorTransmitConnect == true)
            {
                if (HDL_CPU_Time_GetTick() - g_AppMainInfo.capTempSensorLastRevMsgTime_Ms > 2000)
                {
                    g_pSysInfo->capTempSensorTransmitConnect = false;
                }
            }
        }
    }
    break;
    default:
    break;
    }
}
