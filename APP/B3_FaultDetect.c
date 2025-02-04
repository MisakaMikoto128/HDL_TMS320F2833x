/**
 * @file B3_FaultDetect.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-20
 * @last modified 2024-05-20
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "APP_Main.h"
#include "mtime.h"
#include "math.h"
#include "BFL_Measure.h"
#include "B2_EventRecord.h"
void B3_Check_SCR_Serious_Fault(uint32_t poll_delta)
{
    // if (CheckConditionDurationMet(
    //         &g_AppMainInfo.satifySCRA_SeriousFaultTimeCnt,
    //         poll_delta,
    //         SECOND_TO_MS(g_pSysInfo->T_V_SCR_ABNORMAL_DIFF_SEC),
    //         (
    //             The_Capacitors_Are_Working() &&
    //             g_pSysInfo->V_TV1A < g_pSysInfo->V_SCR_NORMAL_DIFF_kV &&
    //             g_pSysInfo->I_TA1A > g_pSysInfo->I_SCR_NORMAL_DIFF_A)))
    // {
    //     g_pSysInfo->Serious_Fault = true;
    //     g_pSysInfo->VTx_A_Breakdown_Fault = SCR_FAULT_BRANCH_BREAKDOWN;
    // }

    // if (CheckConditionDurationMet(
    //         &g_AppMainInfo.satifySCRB_SeriousFaultTimeCnt,
    //         poll_delta,
    //         SECOND_TO_MS(g_pSysInfo->T_V_SCR_ABNORMAL_DIFF_SEC),
    //         (
    //             The_Capacitors_Are_Working() &&
    //             g_pSysInfo->V_TV1B < g_pSysInfo->V_SCR_NORMAL_DIFF_kV &&
    //             g_pSysInfo->I_TA1B > g_pSysInfo->I_SCR_NORMAL_DIFF_A)))
    // {
    //     g_pSysInfo->Serious_Fault = true;
    //     g_pSysInfo->VTx_B_Breakdown_Fault = SCR_FAULT_BRANCH_BREAKDOWN;
    // }

    // if (CheckConditionDurationMet(
    //         &g_AppMainInfo.satifySCRC_SeriousFaultTimeCnt,
    //         poll_delta,
    //         SECOND_TO_MS(g_pSysInfo->T_V_SCR_ABNORMAL_DIFF_SEC),
    //         (
    //             The_Capacitors_Are_Working() &&
    //             g_pSysInfo->V_TV1C < g_pSysInfo->V_SCR_NORMAL_DIFF_kV &&
    //             g_pSysInfo->I_TA1C > g_pSysInfo->I_SCR_NORMAL_DIFF_A)))
    // {
    //     g_pSysInfo->Serious_Fault = true;
    //     g_pSysInfo->VTx_C_Breakdown_Fault = SCR_FAULT_BRANCH_BREAKDOWN;
    // }

    float I_TA1_MAX = getI_TA1_MAX();
    if (CheckConditionDurationMet(
            &g_AppMainInfo.satifyT_I_TA_quick_oc_MS,
            poll_delta,
            MS(g_pSysInfo->T_I_TA_quick_oc_MS),
            (I_TA1_MAX > g_pSysInfo->I_TA_quick_oc_A)))
    {
        // 线路过速断电流直接触发
        if ((!EXIST_SERIOUS_FAULT(g_pSysInfo->Serious_Fault2,SERIOUS_FAULT_I_TA_QUICK_OC)) || g_pSysInfo->Serious_Fault == false)
        {
            g_pSysInfo->Serious_Fault = true;
            SET_SERIOUS_FAULT(g_pSysInfo->Serious_Fault2,SERIOUS_FAULT_I_TA_QUICK_OC);
            // TODO:I_TA_quick_oc_Fault Event
            B2_EventRecord_Write(EVENT_SERIOUS_FAULT_SYSTEM_OVERSPEED_BREAKER_CURRENT);
        }
    }

    float V_TV1x_MAX = getV_TV1x_MAX();
    if (CheckConditionDurationMet(
            &g_AppMainInfo.satifyT_V_TVx_ov,
            poll_delta,
            SECOND_TO_MS(g_pSysInfo->T_V_TVx_ov_SEC),
            (V_TV1x_MAX > g_pSysInfo->V_TVx_ov_kV)))
    {
        // 电容器过压故障触发
        if ((!EXIST_SERIOUS_FAULT(g_pSysInfo->Serious_Fault2, SERIOUS_FAULT_CAPACITOR_OV))  || g_pSysInfo->Serious_Fault == false)
        {
            g_pSysInfo->Serious_Fault = true;
            SET_SERIOUS_FAULT(g_pSysInfo->Serious_Fault2, SERIOUS_FAULT_CAPACITOR_OV);
            // TODO:SERIOUS_FAULT_CAPACITOR_OV Event
            B2_EventRecord_Write(EVENT_MINOR_FAULT_CAPACITOR_OVERVOLTAGE_TRIGGER);
        }
    }

    return;
}

/**
 * @brief
 *
 * @return true 存在轻微故障。
 * @return false 不存在轻微故障。
 */
bool B3_Check_Minor_Fault_Exist(uint32_t poll_delta)
{
    float I_TA1_MAX = getI_TA1_MAX();
    float Tc_MAX = getMaxCapTemp();

    if (CheckConditionDurationMet(
            &g_AppMainInfo.satifyT_I_TA_Thl,
            poll_delta,
            SECOND_TO_MS(g_pSysInfo->T_I_TA_Thh_SEC),
            (I_TA1_MAX < g_pSysInfo->I_TA_low_thl_A)))
    {
        // 线路轻载触发
        if (!EXIST_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_LINE_LIGHT_LOAD))
        {
            SET_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_LINE_LIGHT_LOAD);
            // TODO:MINOR_FAULT_LINE_LIGHT_LOAD Event
            B2_EventRecord_Write(EVENT_MINOR_FAULT_LINE_LIGHT_LOAD_TRIGGER);
        }
    }
    else if (CheckConditionDurationMet(
                 &g_AppMainInfo.satifyT_I_TA_Thl_cancle,
                 poll_delta,
                 SECOND_TO_MS(g_pSysInfo->T_I_TA_Thh_SEC),
                 (I_TA1_MAX > g_pSysInfo->I_TA_low_thl_A)))
    {
        // 线路轻载取消
        CLEAR_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_LINE_LIGHT_LOAD);
    }

    if (CheckConditionDurationMet(
            &g_AppMainInfo.satifyT_I_TA_oc,
            poll_delta,
            SECOND_TO_MS(g_pSysInfo->T_I_TA_oc_SEC),
            (I_TA1_MAX > g_pSysInfo->I_TA_oc_A)))
    {
        // 线路1段过流触发
        if (!EXIST_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_LINE_OVERLOAD))
        {
            SET_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_LINE_OVERLOAD);
            // TODO:MINOR_FAULT_LINE_OVERLOAD Event
            B2_EventRecord_Write(EVENT_MINOR_FAULT_LINE_STAGE_1_OVERCURRENT_TRIGGER);
        }
    }
    else if (CheckConditionDurationMet(
                 &g_AppMainInfo.satifyT_I_TA_oc_cancle,
                 poll_delta,
                 SECOND_TO_MS(g_pSysInfo->T_I_TA_oc_SEC),
                 (I_TA1_MAX < g_pSysInfo->I_TA_oc_A)))
    {
        // 线路1段过流取消
        CLEAR_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_LINE_OVERLOAD);
    }

    if (CheckConditionDurationMet(
            &g_AppMainInfo.satifyT_Tc_ot,
            poll_delta,
            SECOND_TO_MS(1),
            (Tc_MAX > g_pSysInfo->Tc_ot)))
    {
        // 电容器过温故障触发
        if (!EXIST_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_CAPACITOR_OT))
        {
            SET_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_CAPACITOR_OT);
        }
    }
    else if (CheckConditionDurationMet(
                 &g_AppMainInfo.satifyT_Tc_ot_cancle,
                 poll_delta,
                 SECOND_TO_MS(g_pSysInfo->T_Tc_ot_SEC),
                 (Tc_MAX < g_pSysInfo->Tc_ot)))
    {
        // 电容器过温故障取消
        CLEAR_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_CAPACITOR_OT);
    }

    if (CheckConditionDurationMet(
            &g_AppMainInfo.satifyT_SYS_UNDER_SEC,
            poll_delta,
            SECOND_TO_MS(3),
            (g_pSysInfo->V_UIAB < g_pSysInfo->V_SYS_UNDER_kV)))
    {
        // 系统欠压触发
        if (!EXIST_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_LINE_UNDERVOLTAGE))
        {
            SET_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_LINE_UNDERVOLTAGE);
            B2_EventRecord_Write(EVENT_MINOR_FAULT_SYSTEM_UNDERVOLTAGE_TRIGGER);
        }
    }
    else if (CheckConditionDurationMet(
                 &g_AppMainInfo.satifyT_SYS_UNDER_CANCLE_SEC,
                 poll_delta,
                 SECOND_TO_MS(g_pSysInfo->T_SYS_UNDER_CANCLE_SEC),
                 (g_pSysInfo->V_UIAB > g_pSysInfo->V_SYS_UNDER_kV)))
    {
        // 系统欠压取消
        CLEAR_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_LINE_UNDERVOLTAGE);
    }

    // 系统过压
    if (CheckConditionDurationMet(
            &g_AppMainInfo.satifyT_V_ov_SEC,
            poll_delta,
            SECOND_TO_MS(g_pSysInfo->T_V_SYS_OV_SEC),
            (fmaxf(g_pSysInfo->V_UIAB, g_pSysInfo->V_UOAB) > g_pSysInfo->V_SYS_OV_kV)))
    {
        // 系统过压触发
        if (!EXIST_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_LINE_OV))
        {
            // TODO:MINOR_FAULT_LINE_OV Event
            B2_EventRecord_Write(EVENT_MINOR_FAULT_SYSTEM_OVERVOLTAGE_TRIGGER);
            SET_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_LINE_OV);
        }
    }
    else if (CheckConditionDurationMet(
                 &g_AppMainInfo.satifyT_V_ov_cancle_SEC,
                 poll_delta,
                 SECOND_TO_MS(g_pSysInfo->T_V_SYS_OV_SEC),
                 (fmaxf(g_pSysInfo->V_UIAB, g_pSysInfo->V_UOAB) < g_pSysInfo->V_SYS_OV_kV)))
    {
        // 系统过压取消
        CLEAR_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_LINE_OV);
    }

    return Have_Minor_Fault();
}
