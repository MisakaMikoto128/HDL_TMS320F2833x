/**
* @file CHIP_DS3231_Port.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-11-02
* @last modified 2024-11-02
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef CHIP_DS3231_PORT_H
#define CHIP_DS3231_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "ccommon.h"
void CHIP_DS3231_Write_Bytes(byte_t regAddr, byte_t *pBuff, byte_t num);
void CHIP_DS3231_Read_Bytes(byte_t regAddr, byte_t *pBuff, byte_t num);
void CHIP_DS3231_IIC_Init();
#ifdef __cplusplus
}
#endif
#endif //!CHIP_DS3231_PORT_H
