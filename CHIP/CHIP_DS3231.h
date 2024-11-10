/**
 * @file CHIP_DS3231.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-06-28
 * @last modified 2024-06-28
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#ifndef CHIP_DS3231_H
#define CHIP_DS3231_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "HDL_RTC.h"
#include <stdint.h>
#include <stdbool.h>
#include "ccommon.h"

    void CHIP_DS3231_Set(mtime_t *time); // DS3231写入时间信息
    void CHIP_DS3231_Get(mtime_t *time); // DS3231读取时间信息
    /**
     * @brief 初始化DS3231通信。
     * 
     * @return int 
     */
    int CHIP_DS3231_Init(void);

#ifdef __cplusplus
}
#endif
#endif //! CHIP_DS3231_H
