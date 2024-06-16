/**
 * @file B3_RTUPush.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-06-10
 * @last modified 2024-06-10
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include <stdint.h>
#include "ccommon.h"
#include "mbcb.h"
#include "BFL_RS485.h"
#include "period_query.h"
#include <stdlib.h>
#include "APP_Main.h"
#include "crc.h"
#include "mtime.h"
// 输入寄存器内容
extern uint16_t usRegInputBuf[REG_INPUT_NREGS];
// 输入寄存器起始地址
extern uint16_t usRegInputStart;

// 保持寄存器内容
extern uint16_t usRegHoldingBuf[REG_HOLDING_NREGS];
// 保持寄存器起始地址
extern uint16_t usRegHoldingStart;

// 线圈状态
extern byte_t ucRegCoilsBuf[REG_COILS_SIZE / 8];
// 开关输入状态
extern byte_t ucRegDiscreteBuf[REG_DISCRETE_SIZE / 8];

#define RTU_RS485 RS485_1

void B3_RTUPush_Init()
{
    BFL_RS485_Init(RTU_RS485, 115200, UART_WORD_LEN_8, UART_STOP_BIT_1, UART_PARITY_NONE); // 非隔离 MAX3485
    BFL_RS485_Take_Bus(RTU_RS485);
}

void B3_RTUPush_Poll()
{
    static PeriodREC_t s_tPollTime = 0;
    if (!period_query_user_us(&s_tPollTime, MS_TO_US(500)))
    {
        return;
    }
    static const byte_t funcCodeStart = 0x1A;
    static const byte_t sendNum = 12;
    static byte_t sendId = 0;

    byte_t rtuDataBufSend[16 + 4 + 2 + 16] = {0};
    rtuDataBufSend[0] = 0x55;
    rtuDataBufSend[1] = 0xAA;
    rtuDataBufSend[2] = sizeof(rtuDataBufSend) - 4;;
    rtuDataBufSend[3] = 0x00;

    byte_t *rtuHeaderDataBuf = rtuDataBufSend + 4;
    uint64_t devId = 0x0807060504030201ULL;
    rtuHeaderDataBuf[0] = 0x01;
    rtuHeaderDataBuf[1] = 0;
    rtuHeaderDataBuf[2] = 0;
    rtuHeaderDataBuf[3] = 0;
    rtuHeaderDataBuf[4] = 0;
    rtuHeaderDataBuf[5] = 0;
    rtuHeaderDataBuf[6] = 0;
    rtuHeaderDataBuf[7] = (devId >> (0*8) ) & 0xFF;
    rtuHeaderDataBuf[8] = (devId >> (1*8) ) & 0xFF;
    rtuHeaderDataBuf[9] = (devId >> (2*8) ) & 0xFF;
    rtuHeaderDataBuf[10]= (devId >> (3*8) ) & 0xFF;
    rtuHeaderDataBuf[11]= (devId >> (4*8) ) & 0xFF;
    rtuHeaderDataBuf[12]= (devId >> (5*8) ) & 0xFF;
    rtuHeaderDataBuf[13]= (devId >> (6*8) ) & 0xFF;
    rtuHeaderDataBuf[14]= (devId >> (7*8) ) & 0xFF;
    rtuHeaderDataBuf[15]= 0;

    byte_t *rtuDataBuf = rtuDataBufSend + 4 + 16;

    rtuDataBuf[0] = funcCodeStart + sendId;
    rtuDataBuf[1] = 0;
    rtuDataBuf[2] = 0;
    rtuDataBuf[3] = 0;
    rtuDataBuf[4] = 0;
    rtuDataBuf[5] = 0;
    rtuDataBuf[6] = 0;
    rtuDataBuf[15] = 0;
    byte_t *pRtuDataBufData = rtuDataBuf + 7;
    switch (sendId)
    {
    case 0:
        pRtuDataBufData[0] = usRegHoldingBuf[17] & 0xFF;
        pRtuDataBufData[1] = (usRegHoldingBuf[17] >> 8) & 0xFF;
        pRtuDataBufData[2] = usRegHoldingBuf[16] & 0xFF;
        pRtuDataBufData[3] = (usRegHoldingBuf[16] >> 8) & 0xFF;

        pRtuDataBufData[4] = usRegHoldingBuf[19] & 0xFF;
        pRtuDataBufData[5] = (usRegHoldingBuf[19] >> 8) & 0xFF;
        pRtuDataBufData[6] = usRegHoldingBuf[18] & 0xFF;
        pRtuDataBufData[7] = (usRegHoldingBuf[18] >> 8) & 0xFF;
        break;
    case 1:
        pRtuDataBufData[0] = usRegHoldingBuf[21] & 0xFF;
        pRtuDataBufData[1] = (usRegHoldingBuf[21] >> 8) & 0xFF;
        pRtuDataBufData[2] = usRegHoldingBuf[20] & 0xFF;
        pRtuDataBufData[3] = (usRegHoldingBuf[20] >> 8) & 0xFF;

        pRtuDataBufData[4] = usRegHoldingBuf[23] & 0xFF;
        pRtuDataBufData[5] = (usRegHoldingBuf[23] >> 8) & 0xFF;
        pRtuDataBufData[6] = usRegHoldingBuf[22] & 0xFF;
        pRtuDataBufData[7] = (usRegHoldingBuf[22] >> 8) & 0xFF;
        break;
    case 2:
        pRtuDataBufData[0] = usRegHoldingBuf[24] & 0xFF;
        pRtuDataBufData[1] = (usRegHoldingBuf[24] >> 8) & 0xFF;

        pRtuDataBufData[2] = usRegHoldingBuf[26] & 0xFF;
        pRtuDataBufData[3] = (usRegHoldingBuf[26] >> 8) & 0xFF;
        pRtuDataBufData[4] = usRegHoldingBuf[25] & 0xFF;
        pRtuDataBufData[5] = (usRegHoldingBuf[25] >> 8) & 0xFF;

        pRtuDataBufData[6] = usRegHoldingBuf[34] & 0xFF;
        pRtuDataBufData[7] = (usRegHoldingBuf[34] >> 8) & 0xFF;
        break;
    case 3:
        pRtuDataBufData[0] = usRegHoldingBuf[28] & 0xFF;
        pRtuDataBufData[1] = (usRegHoldingBuf[28] >> 8) & 0xFF;
        pRtuDataBufData[2] = usRegHoldingBuf[27] & 0xFF;
        pRtuDataBufData[3] = (usRegHoldingBuf[27] >> 8) & 0xFF;

        pRtuDataBufData[4] = usRegHoldingBuf[29] & 0xFF;
        pRtuDataBufData[5] = (usRegHoldingBuf[29] >> 8) & 0xFF;

        pRtuDataBufData[6] = usRegHoldingBuf[35] & 0xFF;
        pRtuDataBufData[7] = (usRegHoldingBuf[35] >> 8) & 0xFF;
        break;
    case 4:
        pRtuDataBufData[0] = usRegHoldingBuf[31] & 0xFF;
        pRtuDataBufData[1] = (usRegHoldingBuf[31] >> 8) & 0xFF;
        pRtuDataBufData[2] = usRegHoldingBuf[30] & 0xFF;
        pRtuDataBufData[3] = (usRegHoldingBuf[30] >> 8) & 0xFF;

        pRtuDataBufData[4] = usRegHoldingBuf[32] & 0xFF;
        pRtuDataBufData[5] = (usRegHoldingBuf[32] >> 8) & 0xFF;

        pRtuDataBufData[6] = usRegHoldingBuf[33] & 0xFF;
        pRtuDataBufData[7] = (usRegHoldingBuf[33] >> 8) & 0xFF;
        break;
    case 5:
        pRtuDataBufData[0] = usRegHoldingBuf[36] & 0xFF;
        pRtuDataBufData[1] = (usRegHoldingBuf[36] >> 8) & 0xFF;

        pRtuDataBufData[2] = usRegHoldingBuf[37] & 0xFF;
        pRtuDataBufData[3] = (usRegHoldingBuf[37] >> 8) & 0xFF;

        pRtuDataBufData[4] = usRegHoldingBuf[38] & 0xFF;
        pRtuDataBufData[5] = (usRegHoldingBuf[38] >> 8) & 0xFF;

        pRtuDataBufData[6] = usRegHoldingBuf[39] & 0xFF;
        pRtuDataBufData[7] = (usRegHoldingBuf[39] >> 8) & 0xFF;
        break;
    case 6:
        pRtuDataBufData[0] = usRegHoldingBuf[40] & 0xFF;
        pRtuDataBufData[1] = (usRegHoldingBuf[40] >> 8) & 0xFF;

        pRtuDataBufData[2] = usRegHoldingBuf[41] & 0xFF;
        pRtuDataBufData[3] = (usRegHoldingBuf[41] >> 8) & 0xFF;

        pRtuDataBufData[4] = usRegHoldingBuf[42] & 0xFF;
        pRtuDataBufData[5] = (usRegHoldingBuf[42] >> 8) & 0xFF;

        pRtuDataBufData[6] = usRegHoldingBuf[43] & 0xFF;
        pRtuDataBufData[7] = (usRegHoldingBuf[43] >> 8) & 0xFF;
        break;
    case 7:
        pRtuDataBufData[0] = usRegInputBuf[0] & 0xFF;
        pRtuDataBufData[1] = (usRegInputBuf[0] >> 8) & 0xFF;

        pRtuDataBufData[2] = usRegInputBuf[1] & 0xFF;
        pRtuDataBufData[3] = (usRegInputBuf[1] >> 8) & 0xFF;

        pRtuDataBufData[4] = usRegInputBuf[2] & 0xFF;
        pRtuDataBufData[5] = (usRegInputBuf[2] >> 8) & 0xFF;

        pRtuDataBufData[6] = usRegInputBuf[21] & 0xFF;
        pRtuDataBufData[7] = (usRegInputBuf[22] >> 8) & 0xFF;
        break;
    case 8:
        pRtuDataBufData[0] = usRegInputBuf[4] & 0xFF;
        pRtuDataBufData[1] = (usRegInputBuf[4] >> 8) & 0xFF;
        pRtuDataBufData[2] = usRegInputBuf[3] & 0xFF;
        pRtuDataBufData[3] = (usRegInputBuf[3] >> 8) & 0xFF;

        pRtuDataBufData[4] = usRegInputBuf[6] & 0xFF;
        pRtuDataBufData[5] = (usRegInputBuf[6] >> 8) & 0xFF;
        pRtuDataBufData[6] = usRegInputBuf[5] & 0xFF;
        pRtuDataBufData[7] = (usRegInputBuf[5] >> 8) & 0xFF;
        break;
    case 9:
        pRtuDataBufData[0] = usRegInputBuf[8] & 0xFF;
        pRtuDataBufData[1] = (usRegInputBuf[8] >> 8) & 0xFF;
        pRtuDataBufData[2] = usRegInputBuf[7] & 0xFF;
        pRtuDataBufData[3] = (usRegInputBuf[7] >> 8) & 0xFF;

        pRtuDataBufData[4] = usRegInputBuf[10] & 0xFF;
        pRtuDataBufData[5] = (usRegInputBuf[10] >> 8) & 0xFF;
        pRtuDataBufData[6] = usRegInputBuf[9] & 0xFF;
        pRtuDataBufData[7] = (usRegInputBuf[9] >> 8) & 0xFF;
        break;
    case 10:
        pRtuDataBufData[0] = usRegInputBuf[12] & 0xFF;
        pRtuDataBufData[1] = (usRegInputBuf[12] >> 8) & 0xFF;
        pRtuDataBufData[2] = usRegInputBuf[11] & 0xFF;
        pRtuDataBufData[3] = (usRegInputBuf[11] >> 8) & 0xFF;

        pRtuDataBufData[4] = usRegInputBuf[17] & 0xFF;
        pRtuDataBufData[5] = usRegInputBuf[18] & 0xFF;
        pRtuDataBufData[6] = usRegInputBuf[19] & 0xFF;
        pRtuDataBufData[7] = usRegInputBuf[20] & 0xFF;
        break;
    case 11:
        pRtuDataBufData[0] = usRegInputBuf[27] & 0xFF;
        pRtuDataBufData[1] = usRegInputBuf[28] & 0xFF;
        pRtuDataBufData[2] = usRegInputBuf[29] & 0xFF;
        pRtuDataBufData[3] = usRegInputBuf[30] & 0xFF;
        {
            uint16_t SCRT_Fault = g_pSysInfo->SCRT_Fault;
            if (g_pSysInfo->VTx_A_Breakdown_Fault != 0)
            {
                pRtuDataBufData[4] = g_pSysInfo->VTx_A_Breakdown_Fault;
            }

            if (g_pSysInfo->VTx_B_Breakdown_Fault != 0)
            {
                pRtuDataBufData[5] = g_pSysInfo->VTx_B_Breakdown_Fault;
            }

            if (g_pSysInfo->VTx_C_Breakdown_Fault != 0)
            {
                pRtuDataBufData[6] = g_pSysInfo->VTx_C_Breakdown_Fault;
            }

#define BFL_SCRR3B 0x01
#define BFL_SCRR3A 0x02
#define BFL_SCRR2B 0x04
#define BFL_SCRR2A 0x08
#define BFL_SCRR1B 0x10
#define BFL_SCRR1A 0x20

            if ((SCRT_Fault & BFL_SCRR1B) || (SCRT_Fault & BFL_SCRR1A))
            {
                pRtuDataBufData[4] = 1;
            }

            if ((SCRT_Fault & BFL_SCRR2B) || (SCRT_Fault & BFL_SCRR2A))
            {
                pRtuDataBufData[5] = 1;
            }

            if ((SCRT_Fault & BFL_SCRR3B) || (SCRT_Fault & BFL_SCRR3A))
            {
                pRtuDataBufData[6] = 1;
            }
        }
        break;
    }
    sendId++;
    if (sendId >= sendNum)
    {
        sendId = 0;
    }

    uint16_t crc = 0xFFFF;
    crc = CRC16_Modbus(rtuDataBufSend, sizeof(rtuDataBufSend) - 2);
    rtuDataBufSend[sizeof(rtuDataBufSend) - 2] = (crc >> 8) & 0xFF;
    rtuDataBufSend[sizeof(rtuDataBufSend) - 1] = crc & 0xFF;
    BFL_RS485_Write(RTU_RS485, rtuDataBufSend, sizeof(rtuDataBufSend));
} // B3_RTUPush_Poll
