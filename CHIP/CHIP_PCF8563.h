/**
 * @file CHIP_PCF8563.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-06-28
 * @last modified 2024-06-28
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#ifndef CHIP_PCF8563_H
#define CHIP_PCF8563_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "HDL_RTC.h"
#include <stdint.h>
#include <stdbool.h>
#include "ccommon.h"

    void CHIP_PCF8563_Set(mtime_t *time); // PCF8563写入时间信息
    void CHIP_PCF8563_Get(mtime_t *time); // PCF8563读取时间信息
    /**
     * @brief 初始化PCF8563通信。
     * 
     * @return int 
     */
    int CHIP_PCF8563_Init(void);

#ifdef __cplusplus
}
#endif
#endif //! CHIP_PCF8563_H
