/**
 * @file mtime.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2023-09-14
 * @last modified 2023-09-14
 *
 * @copyright Copyright (c) 2023 Liu Yuanlin Personal.
 *
 */
#ifndef MTIME_H
#define MTIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "ccommon.h"

/* 自定义的时间结构体 */
typedef struct
{
    uint16_t nYear;
    byte_t nMonth;
    byte_t nDay;
    byte_t nHour;
    byte_t nMin;
    byte_t nSec;
    byte_t nWeek; /* 0 = Sunday */
    uint16_t wSub; // 亚秒
} mtime_t;

/**
 * @brief 根据给定的日期得到对应的星期
 *
 * @param year
 * @param month
 * @param day
 * @return byte_t 0 = Sunday
 */
byte_t mtime_get_week(uint16_t year, byte_t month, byte_t day);
void mtime_unix_sec_2_time(uint32_t utc_sec, mtime_t *result); // 根据UTC时间戳得到对应的日期
uint32_t mtime_2_unix_sec(mtime_t *currTime);                  // 根据时间计算出UTC时间戳
char *mtime_format(uint32_t utc_sec, char *pBuf);              // 根据UTC时间戳得到对应的日期字符串

void mtime_add_hours(mtime_t *currTime, uint32_t hours);
void mtime_sub_hours(mtime_t *currTime, uint32_t hours);
byte_t mtime_is_equal(mtime_t *currTime, mtime_t *targetTime);


#define MINUTE(x) ((x) * 1ull)
#define SEC(x) ((x) * 1ull)
#define MS(x) ((x) * 1ull)
#define US(x) ((x) * 1ull)

#define SEC_TO_US(x) ((x) * 1000000ull)
#define MS_TO_US(x) ((x) * 1000ull)

#define SECOND_TO_MS(x) ((x) * 1000ull)
#define MINUTE_TO_MS(x) ((x) * 60ull * 1000ull)
#define HOUR_TO_MS(x) ((x) * 60ull * 60ull * 1000ull)
#define DAY_TO_MS(x) ((x) * 24ull * 60ull * 60ull * 1000ull)

#define MINUTE_TO_SEC(x) ((x) * 60ull)

#ifdef __cplusplus
}
#endif
#endif //! MTIME_H
