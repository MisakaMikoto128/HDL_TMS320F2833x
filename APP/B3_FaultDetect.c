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

void B3_Check_SCR_Serious_Fault(uint32_t poll_delta)
{
    if (CheckConditionDurationMet(
            &g_AppMainInfo.satifySCRA_SeriousFaultTimeCnt,
            poll_delta,
            SECOND_TO_MS(g_pSysInfo->T_V_SCR_ABNORMAL_DIFF_SEC),
            (
                The_Capacitors_Are_Working() &&
                g_pSysInfo->V_TV1A < g_pSysInfo->V_SCR_NORMAL_DIFF_kV &&
                g_pSysInfo->I_TA1A > g_pSysInfo->I_SCR_NORMAL_DIFF_A)))
    {
        g_pSysInfo->Serious_Fault = true;
        g_pSysInfo->VTx_A_Breakdown_Fault = SCR_FAULT_BRANCH_BREAKDOWN;
    }

    if (CheckConditionDurationMet(
            &g_AppMainInfo.satifySCRB_SeriousFaultTimeCnt,
            poll_delta,
            SECOND_TO_MS(g_pSysInfo->T_V_SCR_ABNORMAL_DIFF_SEC),
            (
                The_Capacitors_Are_Working() &&
                g_pSysInfo->V_TV1B < g_pSysInfo->V_SCR_NORMAL_DIFF_kV &&
                g_pSysInfo->I_TA1B > g_pSysInfo->I_SCR_NORMAL_DIFF_A)))
    {
        g_pSysInfo->Serious_Fault = true;
        g_pSysInfo->VTx_B_Breakdown_Fault = SCR_FAULT_BRANCH_BREAKDOWN;
    }

    if (CheckConditionDurationMet(
            &g_AppMainInfo.satifySCRC_SeriousFaultTimeCnt,
            poll_delta,
            SECOND_TO_MS(g_pSysInfo->T_V_SCR_ABNORMAL_DIFF_SEC),
            (
                The_Capacitors_Are_Working() &&
                g_pSysInfo->V_TV1C < g_pSysInfo->V_SCR_NORMAL_DIFF_kV &&
                g_pSysInfo->I_TA1C > g_pSysInfo->I_SCR_NORMAL_DIFF_A)))
    {
        g_pSysInfo->Serious_Fault = true;
        g_pSysInfo->VTx_C_Breakdown_Fault = SCR_FAULT_BRANCH_BREAKDOWN;
    }
}

/**
 * @brief
 *
 * @return true 存在轻微故障。
 * @return false 不存在轻微故障。
 */
bool B3_Check_Minor_Fault_Exist(uint32_t poll_delta)
{
    float I_TA1_MAX = 0;
    float V_TV1x_MAX = 0;
    float Tc_MAX = 0;
    I_TA1_MAX = fmaxf(g_pSysInfo->I_TA1A, g_pSysInfo->I_TA1B);
    I_TA1_MAX = fmaxf(I_TA1_MAX, g_pSysInfo->I_TA1C);
    V_TV1x_MAX = fmaxf(g_pSysInfo->V_TV1A, g_pSysInfo->V_TV1B);
    V_TV1x_MAX = fmaxf(V_TV1x_MAX, g_pSysInfo->V_TV1C);
    Tc_MAX = fmaxf(g_pSysInfo->capTemp[Tc_A_IDX], g_pSysInfo->capTemp[Tc_B_IDX]);
    Tc_MAX = fmaxf(Tc_MAX, g_pSysInfo->capTemp[Tc_C_IDX]);

    if (CheckConditionDurationMet(
            &g_AppMainInfo.satifyT_I_TA_Thl,
            poll_delta,
            SECOND_TO_MS(g_pSysInfo->T_I_TA_Thh_SEC),
            (I_TA1_MAX < g_pSysInfo->I_TA_low_thl_A)))
    {
        // 线路轻载触发
        SET_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_LINE_LIGHT_LOAD);
    }
    else if (CheckConditionDurationMet(
                 &g_AppMainInfo.satifyT_I_TA_Thl_cancle,
                 poll_delta,
                 SECOND_TO_MS(g_pSysInfo->T_I_TA_Thh_SEC),
                 (I_TA1_MAX > g_pSysInfo->I_TA_low_thh_A)))
    {
        // 线路轻载取消
        CLEAR_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_LINE_LIGHT_LOAD);
    }

    if (I_TA1_MAX > g_pSysInfo->I_TA_oc_A * 2)
    {
        // 线路过载直接触发
        SET_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_LINE_OVERLOAD);
    }
    else if (CheckConditionDurationMet(
                 &g_AppMainInfo.satifyT_I_TA_oc,
                 poll_delta,
                 SECOND_TO_MS(g_pSysInfo->T_I_TA_oc_SEC),
                 (I_TA1_MAX > g_pSysInfo->I_TA_oc_A)))
    {
        // 线路过载触发
        SET_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_LINE_OVERLOAD);
    }
    else if (CheckConditionDurationMet(
                 &g_AppMainInfo.satifyT_I_TA_oc_cancle,
                 poll_delta,
                 SECOND_TO_MS(g_pSysInfo->T_I_TA_oc_SEC),
                 (I_TA1_MAX < g_pSysInfo->I_TA_oc_A)))
    {
        // 线路过载取消
        CLEAR_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_LINE_OVERLOAD);
    }

    if (CheckConditionDurationMet(
            &g_AppMainInfo.satifyT_V_TVx_ov,
            poll_delta,
            MS(60),
            (V_TV1x_MAX > g_pSysInfo->V_TVx_ov_kV)))
    {
        // 电容器过压故障触发
        SET_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_CAPACITOR_OV);
    }
    else if (CheckConditionDurationMet(
                 &g_AppMainInfo.satifyT_V_TVx_ov_cancle,
                 poll_delta,
                 MINUTE_TO_MS(g_pSysInfo->T_V_TVx_ov_min),
                 (V_TV1x_MAX < g_pSysInfo->V_TVx_ov_kV)))
    {
        // 电容器过压故障取消
        CLEAR_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_CAPACITOR_OV);
    }

    if (CheckConditionDurationMet(
            &g_AppMainInfo.satifyT_Tc_ot,
            poll_delta,
            SECOND_TO_MS(1),
            (Tc_MAX > g_pSysInfo->Tc_ot)))
    {
        // 电容器过温故障触发
        SET_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_CAPACITOR_OT);
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

    if (g_pSysInfo->V_UIAB < g_pSysInfo->V_SYS_UNDER_kV)
    {
        // 直接触发系统欠压
        SET_MINOR_FAULT(g_pSysInfo->Minor_Fault, MINOR_FAULT_LINE_UNDERVOLTAGE);
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

    if (fmaxf(g_pSysInfo->V_UIAB, g_pSysInfo->V_UOAB) > g_pSysInfo->V_SYS_OV_kV)
    {
        // 检测到系统过压
        g_pSysInfo->UxAB_OV_Fault = 1;
    }
    else
    {
        g_pSysInfo->UxAB_OV_Fault = 0;
    }

    return Have_Minor_Fault();
}