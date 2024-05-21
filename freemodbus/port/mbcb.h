/**
* @file mbcb.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-05-21
* @last modified 2024-05-21
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef MBCB_H
#define MBCB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

// 输入寄存器起始地址
#define REG_INPUT_START 0x0000
// 输入寄存器数量
#define REG_INPUT_NREGS 60
// 保持寄存器起始地址
#define REG_HOLDING_START 0x0000
// 保持寄存器数量
#define REG_HOLDING_NREGS 60

// 线圈起始地址
#define REG_COILS_START 0x0000
// 线圈数量
#define REG_COILS_SIZE 16

// 开关寄存器起始地址
#define REG_DISCRETE_START 0x0000
// 开关寄存器数量
#define REG_DISCRETE_SIZE 16

bool eMBRegHoldingChanged(void);
void eMBRegHoldingClearChanged(void);
#ifdef __cplusplus
}
#endif
#endif //!MBCB_H
