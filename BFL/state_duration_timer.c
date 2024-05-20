/**
 * @file state_duration_timer.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-20
 * @last modified 2024-05-20
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "state_duration_timer.h"
bool CheckConditionDurationMet(StateDurationCnt_t *elapsed_time_cnt_ms, uint32_t elapsed_delta_ms, uint32_t duration_ms, bool condition)
{
    if (condition)
    {
        if (*elapsed_time_cnt_ms >= duration_ms)
        {
            return true;
        }
        else
        {
            *elapsed_time_cnt_ms += elapsed_delta_ms;
        }
    }
    else
    {
        *elapsed_time_cnt_ms = 0;
    }

    return false;
}
