/**
 * @file B2_CmdBypassCapacitors.c
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
#include "BFL_SCR.h"
#include "BFL_VCB.h"
#include "async_delay.h"
#include "mtime.h"
#include "period_query.h"
#include <stddef.h>

#define FUALT_LEVEL_NO_FAULT 0
#define FUALT_LEVEL_MINOR_FAULT 1
#define FUALT_LEVEL_SERIOUS_FAULT 2

#define CMD_CODE_NO_FAULT 0
#define CMD_CODE_SYS_IS_IN_SERIOUS_FAULT 1
#define CMD_CODE_SYS_LINE_STATE_IS_NOT_RUNNING 2
#define CMD_CODE_SYS_CAPACITORS_ARE_WORKING 3
#define CMD_CODE_EXEC_OCCUR_SERIOUS_FAULT 4
#define CMD_CODE_EXEC_SUCCESS 5

#define CMD_CODE_SYS_CAPACITORS_ARE_NOT_WORKING 6
#define CMD_CODE_BYPASS_NOT_REQUIRED 7

static void async_delay_callback1(void *arg)
{
    uint32_t *scrtFb = (uint32_t *)arg;
    uint32_t haveSignal = BFL_SCRR_Have_Signal(SCRR_ALL);
    uint32_t haveFault = (~haveSignal) & BFL_SCR_SIGNAL_MASK;
    *scrtFb |= haveFault;
    // BackGroundTask_WhenInSRCPoll();
}

static void async_delay_callback2(void *arg)
{
    UNUSED(arg);
    BackGroundTask();
}

uint16_t SCRT_Fault1 = 0;
uint32_t scrtFb1 = 0;
PeriodREC_t s_t1;
void B2_CmdBypassCapacitors_Test()
{
    if (period_query_user_us(&s_t1, MS_TO_US(500)))
    {
        SCRT_Fault1 = 0;
        scrtFb1 = 0;
        BFL_SCRT_Pluse_Transmit(SCRT_ALL, 6000, US(g_pSysInfo->T2_US));
        async_delay(MS(g_pSysInfo->T4_MS), async_delay_callback1, &scrtFb1);
        // SCRT_Fault1 = scrtFb1;
    }
}

B2_CmdBypassCapacitors_Result_t B2_CmdBypassCapacitors_Exec()
{
    B2_CmdBypassCapacitors_Result_t result;
    result.code = CMD_CODE_NO_FAULT;
    result.QF_Fault = BFL_VBC_NO_FAULT;
    result.KM1_Fault = BFL_VBC_NO_FAULT;

    if (Have_Serious_Fault())
    {
        result.code = CMD_CODE_SYS_IS_IN_SERIOUS_FAULT;
        return result;
    }

    if (!Have_Minor_Fault())
    {
        result.code = CMD_CODE_BYPASS_NOT_REQUIRED;
        return result;
    }

    if (!The_System_Line_State_Is_Running())
    {
        result.code = CMD_CODE_SYS_LINE_STATE_IS_NOT_RUNNING;
        return result;
    }

    if (!The_Capacitors_Are_Working())
    {
        result.code = CMD_CODE_SYS_CAPACITORS_ARE_NOT_WORKING;
        return result;
    }

    uint16_t KM1_Fault = BFL_VBC_NO_FAULT;
    uint16_t SCRT_Fault = 0;
    uint32_t scrtFb = 0;
    for (int tryCnt = 0; tryCnt < 3; tryCnt++)
    {
        BFL_SCRT_Pluse_Transmit(SCRT_ALL, 6000, US(g_pSysInfo->T2_US));
        async_delay(MS(g_pSysInfo->T4_MS), async_delay_callback1, &scrtFb);
        // SCRT_Fault = scrtFb;

        BFL_VCB_Set_As_Switch_Closed(KM1_SW);
        async_delay(MS(g_pSysInfo->T1_MS), async_delay_callback2, NULL);

        // 执行持续10ms的检测
        KM1_Fault = BFL_VBC_NO_FAULT;
        for (int i = 0; i < 10; i++)
        {
            BFL_VCB_STATE_t KM1_State = BFL_VCB_Get_Actual_State(KM1_SW);

            if (KM1_State == BFL_VCB_Opened)
            {
                KM1_Fault = BFL_VBC_CANT_CLOSE;
            }
            else if (KM1_State == BFL_VCB_Closed)
            {
                KM1_Fault = BFL_VBC_NO_FAULT;
                break;
            }

            async_delay(MS(1), async_delay_callback2, NULL);
        }

        // 判断持续10ms的检测结果
        if (KM1_Fault == BFL_VBC_NO_FAULT)
        {
            break;
        }
    }

    // 判断最终结果
    if (KM1_Fault != BFL_VBC_NO_FAULT || SCRT_Fault != 0)
    {
        result.code = CMD_CODE_EXEC_OCCUR_SERIOUS_FAULT;
        result.KM1_Fault = KM1_Fault;
        result.SCRT_Fault = SCRT_Fault;
    }
    else
    {
        result.code = CMD_CODE_EXEC_SUCCESS;
    }

    // 关闭控制
    if (g_AppMainInfo.VBCDebugMode != 1)
    {
        BFL_VCB_Set_As_Switch_No_Ctrl(KM1_SW);
    }

    return result;
}

void B2_CmdBypassCapacitors_Exec_Solution()
{
    B2_CmdBypassCapacitors_Result_t result = B2_CmdBypassCapacitors_Exec();

    // 更新电容器状态
    B1_VCBStatus_Update();

    if (result.code == CMD_CODE_EXEC_OCCUR_SERIOUS_FAULT)
    {
        // 更新故障状态
        if (g_pSysInfo->Serious_Fault == 0 ||
            g_pSysInfo->QF_Fault != result.QF_Fault ||
            g_pSysInfo->KM1_Fault != result.KM1_Fault ||
            g_pSysInfo->SCRT_Fault != result.SCRT_Fault)
        {
            g_pSysInfo->Serious_Fault = 1;
            if (result.QF_Fault != BFL_VBC_NO_FAULT)
            {
                g_pSysInfo->QF_Fault = result.QF_Fault;
            }

            if (result.KM1_Fault != BFL_VBC_NO_FAULT)
            {
                g_pSysInfo->KM1_Fault = result.KM1_Fault;
            }

            g_pSysInfo->SCRT_Fault |= result.SCRT_Fault;

            APP_Main_NotifyHaveParamNeedToSave();
            // TODO:Serious_Fault Event
        }

        // TODO: 处理严重故障
        B2_CmdCutOffCapacitors_Exec();
    }
    else if (result.code == CMD_CODE_EXEC_SUCCESS)
    {
        g_pSysInfo->Capacitors_Exec_State = CAPACITORS_STATE_BYPASS;
    }
}
