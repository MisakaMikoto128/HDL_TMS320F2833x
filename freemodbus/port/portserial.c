/*
 * FreeModbus Libary: MSP430 Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

/* ----------------------- Platform includes --------------------------------*/
#include "DSP2833x_Device.h"
#include "port.h"
#include "CPU_Define.h"
#include "HDL_Uart.h"
#include "BFL_RS485.h"
#include "ccommon.h"
#include <stddef.h>
 #define RTU_COM COM3
 #define RTU_COM_RS485 RS485_2
 #define SciregUsed ScicRegs

//#define RTU_COM COM2
//#define RTU_COM_RS485 RS485_NONE
//#define SciregUsed ScibRegs
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Defines ------------------------------------------*/
void serialReceiveOneByteISR(void);

void serialSentOneByteISR(void *);

/* ----------------------- Static variables ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/
void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    if (xTxEnable)
    {
        BFL_RS485_Take_Bus(RTU_COM_RS485);
    }
    else
    {
        BFL_RS485_Release_Bus(RTU_COM_RS485);
    }

    if (xTxEnable)
    {
        // UART_EnableIT_TC(USART3); // 使能发送完成中断
        //  TX FIFO Interrupt Enable
        SciregUsed.SCIFFTX.bit.TXFFIENA = 1;
    }
    else
    {
        // UART_DisableIT_TC(USART3); // 禁能发送完成中断
        // Disable transmit FIFO interrrupt
        SciregUsed.SCIFFTX.bit.TXFFIENA = 0;
    }

    if (xRxEnable)
    {
        // UART_EnableIT_RXNE(USART3);
        SciregUsed.SCIFFRX.bit.RXFFIENA = 1; // 接收FIFO中断使能
    }
    else
    {
        // UART_DisableIT_RXNE(USART3);
        SciregUsed.SCIFFRX.bit.RXFFIENA = 0;
    }
}

BOOL xMBPortSerialInit(UCHAR ucPort, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity)
{
    BOOL bInitialized = TRUE;
    /**
     * ucPort 用于区分不同的串口，这里没有使用。
     * ucDataBits 这里再Modbus-RTU下一定是8，所以不使用。这个可以在eMBRTUInit查看。
     * eParity Modbus-RTU要求一帧是11位，所以如果有奇偶校验，那么就是1位停止位，否侧使用2位停止位。
     */
    UNUSED(ucPort);
    UNUSED(ucDataBits);
    uint32_t stopBit = UART_STOP_BIT_1;
    uint32_t parity = UART_PARITY_NONE;

    stopBit = eParity == MB_PAR_NONE ? UART_STOP_BIT_2 : UART_STOP_BIT_1;
    switch (eParity)
    {
    case MB_PAR_NONE:
        parity = UART_PARITY_NONE;
        break;
    case MB_PAR_ODD:
        parity = UART_PARITY_ODD;
        break;
    case MB_PAR_EVEN:
        parity = UART_PARITY_EVEN;
        break;
    default:
        break;
    }

     BFL_RS485_Init(RTU_COM_RS485, ulBaudRate, UART_WORD_LEN_8, stopBit, parity);
//    Uart_Init(RTU_COM, ulBaudRate, UART_WORD_LEN_8, stopBit, parity);

    // 因为我使用的这个库已经自己实现了接收完成中断，所以这里没有单独列出中断函数来,按照文章说明的意思来即可
    Uart_RegisterReceiveReadyCharCallback(RTU_COM, serialReceiveOneByteISR);
    Uart_SetWriteOverCallback(RTU_COM, serialSentOneByteISR, NULL);

    // 如果初始化失败，应当返回FALSE
    return bInitialized;
}

BOOL xMBPortSerialPutByte(CHAR ucByte)
{
    Uart_Write(RTU_COM, (const uint16_t *)&ucByte, 1);
    return TRUE;
}

BOOL xMBPortSerialGetByte(CHAR *pucByte)
{
    *pucByte = SciregUsed.SCIRXBUF.all;
    return TRUE;
}

void serialReceiveOneByteISR(void)
{
    pxMBFrameCBByteReceived();
}

void serialSentOneByteISR(void *arg)
{
    UNUSED(arg);
    pxMBFrameCBTransmitterEmpty();
}

void EnterCriticalSection(void)
{
    // 禁止中断
    _disable_interrupts();
}

void ExitCriticalSection(void)
{
    // 恢复中断
    _enable_interrupts();
}
