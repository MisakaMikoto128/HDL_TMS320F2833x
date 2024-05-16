/**
 * @file crc.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-08
 * 
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 * 
 */
#ifndef CRC_H
#define CRC_H
#include <stdint.h>
#include "ccommon.h"
uint16_t CRC16_Modbus(const byte_t *_pBuf, uint16_t _usLen);
uint16_t CRC16_Modbus_start();
uint16_t CRC16_Modbus_update(uint16_t crc, const byte_t *_pBuf, uint16_t _usLen);
uint32_t CRC32(const byte_t *_pBuf, uint32_t _ulLen);
uint32_t CRC32_With(const byte_t *_pBuf, uint32_t _ulLen, uint32_t _ulCRC);
#endif // !CRC_H
