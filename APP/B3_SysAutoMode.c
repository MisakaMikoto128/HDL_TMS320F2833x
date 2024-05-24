/**
 * @file B3_SysAutoMode.c
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
#include "mtime.h"
#include "math.h"

void B3_SysAutoMode_DeltaPoll(uint32_t poll_delta)
{
    if (CheckConditionDurationMet(
            &g_AppMainInfo.satifyLineStateRunningTimeCnt,
            poll_delta, SECOND_TO_MS(5),
            (g_pSysInfo->V_UIAB > g_pSysInfo->V_SYS_STOP_kV)))
    {
        g_pSysInfo->Line_State = LINE_STATE_RUNNING;
    }
    else if (CheckConditionDurationMet(
                 &g_AppMainInfo.satifyLineStateRunningTimeCnt,
                 poll_delta, SECOND_TO_MS(5),
                 (g_pSysInfo->V_UIAB < g_pSysInfo->V_SYS_STOP_kV)))
    {
        g_pSysInfo->Line_State = LINE_STATE_STOP;
    }

    if (g_pSysInfo->Line_State == LINE_STATE_RUNNING)
    {
        if (Have_Serious_Fault())
        {
            B2_CmdCutOffCapacitors_Exec_Solution();
        }
        else
        {
            if (Have_Minor_Fault())
            {
                B2_CmdBypassCapacitors_Exec_Solution();
            }
            else
            {
                if (CheckConditionDurationMet(
                        &g_AppMainInfo.satifyCapacitorsWorkTimeCnt,
                        poll_delta, SECOND_TO_MS(g_pSysInfo->T_SYS_SATIFY_CAPACITORS_WAORK_SEC),
                        (g_pSysInfo->V_UIAB > g_pSysInfo->V_SYS_UNDER_kV && g_pSysInfo->V_UIAB < g_pSysInfo->V_SYS_THH_kV)))
                {
                    B2_CmdMakeCapacitorsWork_Exec_Solution();
                }
                else if (fmaxf(g_pSysInfo->V_UIAB, g_pSysInfo->V_UOAB) > g_pSysInfo->V_SYS_OV_kV)
                {
                    // 系统过压，直接切除
                    B2_CmdCutOffCapacitors_Exec_Solution();
                }
                else
                {
                    // pass
                }
            }
        }
    }
    else
    {
        B2_CmdCutOffCapacitors_Exec_Solution();
    }

    B3_Check_Minor_Fault_Exist(poll_delta);
    B3_Check_SCR_Serious_Fault(poll_delta);
}