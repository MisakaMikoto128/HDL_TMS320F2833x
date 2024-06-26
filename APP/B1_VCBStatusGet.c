/**
 * @file B1_VCBStatusGet.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-17
 * @last modified 2024-05-17
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */

#include "crc.h"
#include "ccommon.h"
#include "period_query.h"
#include "BFL_VCB.h"
#include "HDL_CPU_Time.h"
#include "APP_Main.h"
#include <stdlib.h>

void B1_VCBStatusGet_Init()
{
    BFL_VCB_Seurity_Init();
    BFL_VCB_Set_As_Switch_Closed(QF_SW);
    BFL_VCB_Set_As_Switch_Closed(KM1_SW);
}

void B1_VCBStatus_Update()
{
    g_pSysInfo->QF_FB = (uint16_t)BFL_VCB_Get_Actual_State(QF_SW);
    g_pSysInfo->QS1_FB = (uint16_t)BFL_VCB_Get_Actual_State(QS1_SW);
    g_pSysInfo->QS2_FB = (uint16_t)BFL_VCB_Get_Actual_State(QS2_SW);
    g_pSysInfo->KM1_FB = (uint16_t)BFL_VCB_Get_Actual_State(KM1_SW);

    if (g_pSysInfo->QF_FB == BFL_VCB_Closed)
    {
        g_pSysInfo->Capacitors_State = CAPACITORS_STATE_CUT_OFF;
    }
    else if (g_pSysInfo->QF_FB == BFL_VCB_Opened && g_pSysInfo->KM1_FB == BFL_VCB_Opened)
    {
        g_pSysInfo->Capacitors_State = CAPACITORS_STATE_WORKING;
    }
    else if (g_pSysInfo->QF_FB == BFL_VCB_Opened && g_pSysInfo->KM1_FB == BFL_VCB_Closed)
    {
        g_pSysInfo->Capacitors_State = CAPACITORS_STATE_BYPASS;
    }
}

void B1_VCBStatusGet_DeltaPoll(uint32_t poll_delta)
{
    for_Each_VCB_SW_t(vcb)
    {
        BFL_VCB_STATE_t state = BFL_VCB_Get_Actual_State(vcb);
        if (g_AppMainInfo.VCB_StateLast[vcb] != state)
        {
            g_AppMainInfo.VCB_StateFilterTimeCnt[vcb] += poll_delta;
            if (g_AppMainInfo.VCB_StateFilterTimeCnt[vcb] > 80)
            {
                g_AppMainInfo.VCB_StateLast[vcb] = state;
                g_AppMainInfo.VCB_StateFilterTimeCnt[vcb] = 0;
                B1_VCBStatus_Update();
            }
        }
        else
        {
            g_AppMainInfo.VCB_StateFilterTimeCnt[vcb] = 0;
        }
    }
}
