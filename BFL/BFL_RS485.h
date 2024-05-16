/**
* @file BFL_RS485.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-05-16
* @last modified 2024-05-16
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef BFL_RS485_H
#define BFL_RS485_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    RS485_1 = 0, // SCI-A,隔离 MAX3485
    RS485_2 = 1, // SCI-C,MAX3485
    RS485_NUM,
} BFL_RS485_t;

void BFL_RS485_Init(BFL_RS485_t rs485, uint32_t baud, uint32_t wordLen, uint32_t stopBit, uint32_t parity);
uint32_t BFL_RS485_Write(BFL_RS485_t rs485, const uint16_t *writeBuf, uint32_t uLen);
uint32_t BFL_RS485_Read(BFL_RS485_t rs485, uint16_t *pBuf, uint32_t uiLen);

#ifdef __cplusplus
}
#endif
#endif //!BFL_RS485_H
