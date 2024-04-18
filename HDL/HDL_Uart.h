/**
 * @file HDL_Uart.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-04-18
 * @last modified 2024-04-18
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#ifndef HDL_UART_H
#define HDL_UART_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "ccommon.h"

// Serial Port ID definition
typedef enum {
    COM1 = 0, // SCI-A
    COM2 = 1, // SCI-B
    COM3 = 2, // SCI-C
    COM_NUM,
} COMID_t;


#define UART_WORD_LEN_5 5
#define UART_WORD_LEN_6 6
#define UART_WORD_LEN_7 7
#define UART_WORD_LEN_8 8

#define UART_STOP_BIT_1 0
#define UART_STOP_BIT_1_5 1
#define UART_STOP_BIT_2 2

#define UART_PARITY_NONE 0
#define UART_PARITY_ODD 1
#define UART_PARITY_EVEN 2


typedef void (*UartWriteOverCallback_t)(void *args);
typedef void (*UartReceiveCharCallback_t)(byte_t ch);

void Uart_Init(COMID_t comId, uint32_t baud, uint32_t wordLen, uint32_t stopBit, uint32_t parity);
uint32_t Uart_Write(COMID_t comId, const byte_t *writeBuf, uint32_t uLen);
uint32_t Uart_Read(COMID_t comId, byte_t *pBuf, uint32_t uiLen);
uint32_t Uart_AvailableBytes(COMID_t comId);
uint32_t Uart_EmptyReadBuffer(COMID_t comId);
byte_t Uart_SetWriteOverCallback(COMID_t comId, UartWriteOverCallback_t callback, void *args);
byte_t Uart_RegisterReceiveCharCallback(COMID_t comId, UartReceiveCharCallback_t callback);
byte_t Uart_UnregisterReceiveCharCallback(COMID_t comId);

#ifdef __cplusplus
}
#endif
#endif //! HDL_UART_H
