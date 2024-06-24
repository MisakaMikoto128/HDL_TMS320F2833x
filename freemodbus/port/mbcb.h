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
#define REG_INPUT_END (REG_INPUT_START + REG_INPUT_NREGS)
// 保持寄存器起始地址
#define REG_HOLDING_START 0x0000
// 保持寄存器数量
#define REG_HOLDING_NREGS 70
#define REG_HOLDING_END (REG_HOLDING_START + REG_HOLDING_NREGS)

// 线圈起始地址
#define REG_COILS_START 0x0000
// 线圈数量
#define REG_COILS_SIZE 16
#define REG_COILS_END (REG_COILS_START + REG_COILS_SIZE)

// 开关寄存器起始地址
#define REG_DISCRETE_START 0x0000
// 开关寄存器数量
#define REG_DISCRETE_SIZE 16
#define REG_DISCRETE_END (REG_DISCRETE_START + REG_DISCRETE_SIZE)

bool eMBRegHoldingChanged(void);
void eMBRegHoldingClearChanged(void);
#ifdef __cplusplus
}
#endif
#endif //!MBCB_H
