/**
 * @file HDL_RTC.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-07
 *
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 *
 */
#include "HDL_RTC.h"
#include "mtime.h"
#include "HDL_CPU_Time.h"
#include <stddef.h>

/**
 * @brief 没有时区概念在这个RTC库中，这里RTC的时间时本地时间还是UTC时间有用户自己决定。
 *
 */
/*
BaseYear存在一定的问题，ST的这个RTC没有一个单独的秒寄存器，就是一个DateTime日历，
对于闰年的处理取决于RTC_BASE_YEAR,但是RTC_BASE_YEAR手册中却从来没有提及。更具
论坛讨论的结果为2000年1月1日0时0分为基础时间
*/
#define RTC_BASE_YEAR_TIMESTAMP 946684800 // 2000-1-1 0:0:0的时间戳

struct softRTC_t {
    uint64_t unixMsTimestamp;   // 1970-1-1以来的总毫秒数
    bool calibratedAtLeastOnce; // 校准过至少一次
};

static struct softRTC_t softRTC   = {0};
static struct softRTC_t *pSoftRTC = &softRTC;

static void HDL_RTC_Update()
{
    pSoftRTC->unixMsTimestamp ++;
}

/**
 * @brief RTC外设初始化。启动RTC，但是不会设置RTC时间。默认LSE时钟源。
 * 设置/获取时间默认使用BIN格式的时间。
 *
 */
void HDL_RTC_Init()
{
    pSoftRTC->calibratedAtLeastOnce = false;
    pSoftRTC->unixMsTimestamp       = 0;
    HDL_CPU_Time_SetCPUTickCallback(HDL_RTC_Update);
}

/**
 * @brief 获取本地1970-1-1以来总秒数
 *
 * @param pSub 用于获取亚秒数。不需要可以为NULL。
 * @return uint32_t 秒数
 */
uint64_t HDL_RTC_GetTimeTick(uint16_t *pSub)
{
    uint64_t timestamp = 0;
    timestamp          = pSoftRTC->unixMsTimestamp / 1000;
    if (pSub != NULL) {
        *pSub = pSoftRTC->unixMsTimestamp % 1000;
    }
    return timestamp;
}

/**
 * @brief 获取mtime_t类型的时间。
 *
 * @param myTime
 */
void HDL_RTC_GetStructTime(mtime_t *myTime)
{
    uint32_t timestamp = 0;
    timestamp          = (uint32_t)(pSoftRTC->unixMsTimestamp / 1000);
    mtime_unix_sec_2_time(timestamp, myTime);
}

/**
 * @brief 使用时间戳设置日历时间。
 *
 * @param timestamp 1970-1-1以来总秒数。必须大于rtc_base_year_timestamp。参考RTC_BASE_YEAR。
 *
 */
void HDL_RTC_SetTimeTick(uint64_t timestamp)
{
    if (timestamp > RTC_BASE_YEAR_TIMESTAMP) {
        pSoftRTC->unixMsTimestamp = timestamp * 1000;
        pSoftRTC->calibratedAtLeastOnce = true;
    }
}

/**
 * @brief 使用mtime_t对象来设置时间。被设置的时间必须大于等于RTC_BASE_YEAR
 * 否则没有效果。设置时间的时间精度为秒。
 *
 * @param myTime
 */
void HDL_RTC_SetStructTime(mtime_t *myTime)
{
    uint32_t timestamp = 0;
    timestamp          = mtime_2_unix_sec(myTime);
    if (timestamp > RTC_BASE_YEAR_TIMESTAMP) {
        pSoftRTC->unixMsTimestamp = (uint64_t)timestamp * 1000;
        pSoftRTC->calibratedAtLeastOnce = true;
    }
}

uint64_t HDL_RTC_GetMsTimestamp()
{
    return pSoftRTC->unixMsTimestamp;
}

bool HDL_RTC_HasSynced()
{
    return pSoftRTC->calibratedAtLeastOnce;
}
