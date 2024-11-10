/**
* @file ccoding.c
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-11-10
* @last modified 2024-11-10
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#include "ccoding.h"
byte_t Hex2Bcd(byte_t hex)
{
    return (hex % 10) + ((hex / 10) << 4);
}

byte_t Bcd2Hex(byte_t bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}
