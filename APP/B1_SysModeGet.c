/**
 * @file B1_SysModeGet.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 * @last modified 2024-05-16
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "crc.h"
#include "ccommon.h"
#include "period_query.h"
#include "BFL_Button.h"
#include "HDL_CPU_Time.h"
#include "APP_Main.h"
#include <stdlib.h>

void B1_SysModeGet_Init()
{
    BFL_Button_Init();
}

void B1_SysModeGet_DeltaPoll(uint32_t poll_delta)
{
    if (BFL_Button_IsPressed(MODE_BTN))
    {
        if (!g_AppMainInfo.modeBtnPressed)
        {
            g_AppMainInfo.modeBtnFilterTimeCnt += poll_delta;
            if (g_AppMainInfo.modeBtnFilterTimeCnt > 80)
            {
                g_AppMainInfo.modeBtnPressed = true;
                g_pSysInfo->SYS_MODE = SYS_MODE_AUTO;
                g_AppMainInfo.modeBtnFilterTimeCnt = 0;
            }
        }
        else
        {
            g_AppMainInfo.modeBtnFilterTimeCnt = 0;
        }
    }
    else
    {
        if (g_AppMainInfo.modeBtnPressed == true)
        {
            g_AppMainInfo.modeBtnFilterTimeCnt += poll_delta;
            if (g_AppMainInfo.modeBtnFilterTimeCnt > 80)
            {
                g_AppMainInfo.modeBtnPressed = false;
                g_pSysInfo->SYS_MODE = SYS_MODE_MANUAL;
                g_AppMainInfo.modeBtnFilterTimeCnt = 0;
            }
        }
        else
        {
            g_AppMainInfo.modeBtnFilterTimeCnt = 0;
        }
    }
}
