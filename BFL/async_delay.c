/**
 * @file async_delay.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-17
 * @last modified 2024-05-17
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "async_delay.h"
#include "HDL_CPU_Time.h"

void async_delay(uint32_t delayMs, void (*callback)(void *), void *arg)
{
    uint32_t tickstart = HDL_CPU_Time_GetTick();
    uint32_t wait = delayMs;

    while ((HDL_CPU_Time_GetTick() - tickstart) < wait)
    {
        if (callback)
        {
            callback(arg);
        }
    }
}
