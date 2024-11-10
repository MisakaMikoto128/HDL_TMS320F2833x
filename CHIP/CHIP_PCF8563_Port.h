/**
* @file CHIP_PCF8563_Port.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-11-02
* @last modified 2024-11-02
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef CHIP_PCF8563_PORT_H
#define CHIP_PCF8563_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "ccommon.h"
void CHIP_PCF8563_Write_Bytes(byte_t regAddr, byte_t *pBuff, byte_t num); // PCF8563写入多组数据
void CHIP_PCF8563_Read_Bytes(byte_t regAddr, byte_t *pBuff, byte_t num);  // PCF8563读取多组数据
void CHIP_PCF8563_IIC_Init();
#ifdef __cplusplus
}
#endif
#endif //!CHIP_PCF8563_PORT_H
