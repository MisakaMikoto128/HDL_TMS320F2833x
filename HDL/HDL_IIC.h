/**
* @file HDL_IIC.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-11-10
* @last modified 2024-11-10
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef HDL_IIC_H
#define HDL_IIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ccommon.h"

typedef enum
{
    SOFT_IIC_1 = 0,
    IIC_NUM,
}IIC_ID_t;

#define NONE_IIC_SPEED 0
#define NONE_IIC_ADDR_LEN 0
#define NONE_IIC_OWN_ADDR 0

void HDL_IIC_Init(IIC_ID_t iicID, uint32_t speed, byte_t addrLength,uint16_t ownAddr);

void HDL_IIC_DeInit(IIC_ID_t iicID);

size_t HDL_IIC_Write(IIC_ID_t iicID, uint16_t addr, const byte_t *data, size_t size);

size_t HDL_IIC_Read(IIC_ID_t iicID, uint16_t addr, byte_t *buf, size_t size);

size_t HDL_IIC_Mem_Write(IIC_ID_t iicID, uint16_t addr, uint16_t memAddr,byte_t memAddrLength, const byte_t *data, size_t size);

size_t HDL_IIC_Mem_Read(IIC_ID_t iicID, uint16_t addr, uint16_t memAddr,byte_t memAddrLength, byte_t *buf, size_t size);

#ifdef __cplusplus
}
#endif
#endif //!HDL_IIC_H
