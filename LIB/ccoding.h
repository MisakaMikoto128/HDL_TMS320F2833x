/**
* @file ccode.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-11-10
* @last modified 2024-11-10
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef CCODE_H
#define CCODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "ccommon.h"

/**
 * @brief 将十六进制数转换为BCD格式
 * 
 * @param hex 输入的十六进制数
 * @return byte_t 转换后的BCD数
 */
byte_t Hex2Bcd(byte_t hex);

/**
 * @brief 将BCD格式数转换为十六进制数
 * 
 * @param bcd 输入的BCD数
 * @return byte_t 转换后的十六进制数
 */
byte_t Bcd2Hex(byte_t bcd);
#ifdef __cplusplus
}
#endif
#endif //!CCODE_H
