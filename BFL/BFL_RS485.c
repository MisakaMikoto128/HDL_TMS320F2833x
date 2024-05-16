/**
 * @file BFL_RS485.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 * @last modified 2024-05-16
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "BFL_RS485.h"
#include "HDL_Uart.h"
#include "CPU_Define.h"
#include <stddef.h>

#define RS485C_Take_Bus() (GpioDataRegs.GPBSET.bit.GPIO61 = 1)
#define RS485C_Release_Bus() (GpioDataRegs.GPBCLEAR.bit.GPIO61 = 1)

#define RS485A_Take_Bus() (GpioDataRegs.GPASET.bit.GPIO28 = 1)
#define RS485A_Release_Bus() (GpioDataRegs.GPACLEAR.bit.GPIO28 = 1)

void BFL_RS485_Take_Bus(BFL_RS485_t rs485)
{
    if (rs485 == RS485_1)
    {
        RS485A_Take_Bus();
    }
    else if (rs485 == RS485_2)
    {
        RS485C_Take_Bus();
    }
}

void BFL_RS485_Release_Bus(BFL_RS485_t rs485)
{
    if (rs485 == RS485_1)
    {
        RS485A_Release_Bus();
    }
    else if (rs485 == RS485_2)
    {
        RS485C_Release_Bus();
    }
}

void BFL_RS485_Take_Bus_RS4851()
{
    RS485A_Take_Bus();
}

void BFL_RS485_Release_Bus_RS4851(void *arg)
{
    UNUSED(arg);
    RS485A_Release_Bus();
}

void BFL_RS485_Take_Bus_RS4852()
{
    RS485C_Take_Bus();
}

void BFL_RS485_Release_Bus_RS4852(void *arg)
{
    UNUSED(arg);
    RS485C_Release_Bus();
}

void BFL_RS485_Init(BFL_RS485_t rs485, uint32_t baud, uint32_t wordLen, uint32_t stopBit, uint32_t parity)
{
    if (rs485 == RS485_1)
    {
        EALLOW;
        // EN485A GPIO,无上下拉电阻
        // General purpose I/O
        GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 0x00;
        // Configures the GPIO pin as an output
        GpioCtrlRegs.GPADIR.bit.GPIO28 = 1;
        // Enable the internal pullup on the specified pin.
        GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;
        RS485A_Release_Bus();
        EDIS;
        Uart_Init(COM1, baud, wordLen, stopBit, parity);
        Uart_SetWriteOverCallback(COM1, BFL_RS485_Release_Bus_RS4851, NULL);
    }
    else if (rs485 == RS485_2)
    {
        EALLOW;
        // EN485C GPIO,无上下拉电阻
        // General purpose I/O
        GpioCtrlRegs.GPBMUX2.bit.GPIO61 = 0x00;
        // Configures the GPIO pin as an output
        GpioCtrlRegs.GPBDIR.bit.GPIO61 = 1;
        // Enable the internal pullup on the specified pin.
        GpioCtrlRegs.GPBPUD.bit.GPIO61 = 0;
        RS485C_Release_Bus();
        EDIS;
        Uart_Init(COM3, baud, wordLen, stopBit, parity);
        Uart_SetWriteOverCallback(COM3, BFL_RS485_Release_Bus_RS4852, NULL);
    }
}

uint32_t BFL_RS485_Write(BFL_RS485_t rs485, const uint16_t *writeBuf, uint32_t uLen)
{
    if (rs485 == RS485_1)
    {
        BFL_RS485_Take_Bus(rs485);
        return Uart_Write(COM1, writeBuf, uLen);
    }
    else if (rs485 == RS485_2)
    {
        BFL_RS485_Take_Bus(rs485);
        return Uart_Write(COM3, writeBuf, uLen);
    }
    return 0;
}

uint32_t BFL_RS485_Read(BFL_RS485_t rs485, uint16_t *pBuf, uint32_t uiLen)
{
    if (rs485 == RS485_1)
    {
        return Uart_Read(COM1, pBuf, uiLen);
    }
    else if (rs485 == RS485_2)
    {
        return Uart_Read(COM3, pBuf, uiLen);
    }
    return 0;
}
