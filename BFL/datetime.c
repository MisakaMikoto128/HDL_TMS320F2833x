/**
 * @file datetime.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-04-11
 * @last modified 2024-04-11
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "datetime.h"
#include <stddef.h>
#include "HDL_RTC.h"

// RTC上跑的是UTC+0时区，所以需要设置时区
static int16_t timezone = 8;

void datetime_init(void)
{
    HDL_RTC_Init();
}

void datetime_set_timezone(int16_t _timezone)
{
    timezone = _timezone;
}
int16_t datetime_get_timezone()
{
    return timezone;
}
void datetime_set_localtime(mtime_t *mtime)
{
    uint64_t local_timestamp = mtime_2_unix_sec(mtime);
    uint64_t utc0_timestamp  = local_timestamp - timezone * 3600;
    HDL_RTC_SetTimeTick(utc0_timestamp);
}
void datetime_get_localtime(mtime_t *mtime)
{
    uint64_t utc0_timestamp  = HDL_RTC_GetTimeTick(NULL);
    uint64_t local_timestamp = utc0_timestamp + timezone * 3600;
    mtime_unix_sec_2_time(local_timestamp, mtime);
}
void datetime_set_local_timestamp(uint64_t timestamp)
{
    uint64_t utc0_timestamp = timestamp - timezone * 3600;
    HDL_RTC_SetTimeTick(utc0_timestamp);
}
uint64_t datetime_get_local_timestamp(void)
{
    uint64_t utc0_timestamp  = HDL_RTC_GetTimeTick(NULL);
    uint64_t local_timestamp = utc0_timestamp + timezone * 3600;
    return local_timestamp;
}
void datetime_set_unix_timestamp(uint64_t timestamp)
{
    HDL_RTC_SetTimeTick(timestamp);
}
uint64_t datetime_get_unix_timestamp(void)
{
    return HDL_RTC_GetTimeTick(NULL);
}

uint64_t datetime_get_unix_ms_timestamp(void)
{
    uint16_t sub = 0;
    uint64_t timestamp = HDL_RTC_GetTimeTick(&sub);
    return timestamp * 1000 + sub;
}

bool datetime_has_synced(void)
{
    return HDL_RTC_HasSynced();
}
