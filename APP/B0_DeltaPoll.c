/**
 * @file B0_DeltaPoll.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-17
 * @last modified 2024-05-17
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "APP_Main.h"
#include "HDL_CPU_Time.h"

void B0_DeltaPollCallback(uint32_t poll_delta)
{
    B1_SysModeGet_DeltaPoll(poll_delta);
    B1_VCBStatusGet_DeltaPoll(poll_delta);
}

void B0_DeltaPoll()
{
    static int poll_stage = 0;
    static uint32_t last_poll_time = 0;
    if (poll_stage == 0)
    {
        last_poll_time = HDL_CPU_Time_GetTick();
        poll_stage = 1;
    }
    else
    {
        uint32_t now = HDL_CPU_Time_GetTick();
        uint32_t poll_delta = now - last_poll_time;
        if (poll_delta == 0)
        {
            return;
        }
        last_poll_time = now;

        B0_DeltaPollCallback(poll_delta);
    }
}
