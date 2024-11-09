/**
 * @file B2_CmdCutOffCapacitors.c
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
#include <stddef.h>
#include "B2_EventRecord.h"

#define FUALT_LEVEL_NO_FAULT 0
#define FUALT_LEVEL_MINOR_FAULT 1
#define FUALT_LEVEL_SERIOUS_FAULT 2

#define CMD_CODE_NO_FAULT 0
#define CMD_CODE_SYS_IS_IN_SERIOUS_FAULT 1
#define CMD_CODE_SYS_LINE_STATE_IS_NOT_RUNNING 2
#define CMD_CODE_SYS_CAPACITORS_ARE_WORKING 3
#define CMD_CODE_EXEC_OCCUR_SERIOUS_FAULT 4
#define CMD_CODE_EXEC_SUCCESS 5

#define CMD_CODE_SYS_CAPACITORS_ARE_CUTOFF 6

static void async_delay_callback1(void *arg)
{
    uint32_t *scrtFb = (uint32_t *)arg;
    uint32_t haveSignal = BFL_SCRR_Have_Signal(SCRR_ALL);
    uint32_t haveFault = (~haveSignal) & BFL_SCR_SIGNAL_MASK;
    *scrtFb |= haveFault;
    // BackGroundTask_WhenInSRCPoll();
}

static void async_delay_callback_vot_check(void *arg)
{
    void **args = (void **)arg;
    uint32_t *scrtFb = (uint32_t *)args[0];
    B1_Measure_t *begin_measure = (B1_Measure_t *)args[1];
    B1_Measure_t current_measure;
    B1_Measure_Read(&current_measure);

    const float th = 0.5f;
    const float abs_th_vot_kV = 0.1f;
    if (g_pSysInfo->V_UIAB > g_pSysInfo->V_SYS_STOP_kV)
    {
        if (current_measure.V_TV1A < begin_measure->V_TV1A * th || current_measure.V_TV1A < abs_th_vot_kV)
        {
            *scrtFb &= ~(BFL_SCRR1A | BFL_SCRR1B);
        }

        if (current_measure.V_TV1B < begin_measure->V_TV1B * th || current_measure.V_TV1A < abs_th_vot_kV)
        {
            *scrtFb &= ~(BFL_SCRR2A | BFL_SCRR2B);
        }

        if (current_measure.V_TV1C < begin_measure->V_TV1C * th || current_measure.V_TV1A < abs_th_vot_kV)
        {
            *scrtFb &= ~(BFL_SCRR3A | BFL_SCRR3B);
        }
    }
    else
    {
        *scrtFb = 0;
    }

    BackGroundTask_WhenInSRCPoll();
}

static void async_delay_callback2(void *arg)
{
    UNUSED(arg);
    BackGroundTask();
}

B2_CmdCutOffCapacitors_Result_t B2_CmdCutOffCapacitors_Exec()
{
    B2_CmdCutOffCapacitors_Result_t result;
    result.code = CMD_CODE_NO_FAULT;
    result.QF_Fault = BFL_VBC_NO_FAULT;
    result.KM1_Fault = BFL_VBC_NO_FAULT;

    // 如果电容未投入并且也没有旁路那么直接返回->即电容器已经切除
    if (The_Capacitors_Are_Cut_Off())
    {
        result.code = CMD_CODE_SYS_CAPACITORS_ARE_CUTOFF;
        return result;
    }

    uint16_t QF_Fault = BFL_VBC_NO_FAULT;
    uint16_t KM1_Fault = BFL_VBC_NO_FAULT;
    uint16_t SCRT_Fault = 0;
    uint32_t scrtFb = 0;
    for (int tryCnt = 0; tryCnt < 3; tryCnt++)
    {
        BFL_VCB_STATE_t QF_State = BFL_VCB_Get_Actual_State(QF_SW);
        BFL_VCB_STATE_t KM1_State = BFL_VCB_Get_Actual_State(KM1_SW);

        if (!(QF_State == BFL_VCB_Closed || KM1_State == BFL_VCB_Closed))
        {
            // 5000us Period x30 times = 600*5=3000ms
            BFL_SCRT_Pluse_Transmit(SCRT_ALL, MS(g_pSysInfo->T2_MS), US(4000));
            B1_Measure_t measures;
            B1_Measure_Read(&measures);
            void *args[] = {&scrtFb, &measures};

            scrtFb = BFL_SCRR1A | BFL_SCRR1B | BFL_SCRR2A | BFL_SCRR2B | BFL_SCRR3A | BFL_SCRR3B;
            async_delay(MS(g_pSysInfo->T4_MS), async_delay_callback_vot_check, args);
            SCRT_Fault = scrtFb;
        }

        BFL_VCB_Set_As_Switch_Closed(QF_SW);
        BFL_VCB_Set_As_Switch_Closed(KM1_SW);
        async_delay(MS(g_pSysInfo->T1_MS), async_delay_callback2, NULL);

        // 执行持续10ms的检测
        QF_Fault = BFL_VBC_NO_FAULT;
        KM1_Fault = BFL_VBC_NO_FAULT;
        for (int i = 0; i < 10; i++)
        {
            BFL_VCB_STATE_t QF_State = BFL_VCB_Get_Actual_State(QF_SW);
            BFL_VCB_STATE_t KM1_State = BFL_VCB_Get_Actual_State(KM1_SW);

            if (QF_State == BFL_VCB_Closed && KM1_State == BFL_VCB_Closed)
            {
                QF_Fault = BFL_VBC_NO_FAULT;
                KM1_Fault = BFL_VBC_NO_FAULT;
                break;
            }

            if (QF_State == BFL_VCB_Opened)
            {
                QF_Fault = BFL_VBC_CANT_CLOSE;
            }
            else
            {
                QF_Fault = BFL_VBC_NO_FAULT;
            }

            if (KM1_State == BFL_VCB_Opened)
            {
                KM1_Fault = BFL_VBC_CANT_CLOSE;
            }
            else
            {
                KM1_Fault = BFL_VBC_NO_FAULT;
            }

            async_delay(MS(1), async_delay_callback2, NULL);
        }

        // 判断持续10ms的检测结果
        if (QF_Fault == BFL_VBC_NO_FAULT && KM1_Fault == BFL_VBC_NO_FAULT)
        {
            break;
        }
    }

    // 判断最终结果
    if (QF_Fault != BFL_VBC_NO_FAULT || KM1_Fault != BFL_VBC_NO_FAULT ||
        SCRT_Fault != 0)
    {
        result.code = CMD_CODE_EXEC_OCCUR_SERIOUS_FAULT;
        result.QF_Fault = QF_Fault;
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
        BFL_VCB_Set_As_Switch_No_Ctrl(QF_SW);
    }

    return result;
}

void B2_CmdCutOffCapacitors_Exec_Solution()
{
    B2_CmdCutOffCapacitors_Result_t result = B2_CmdCutOffCapacitors_Exec();

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

                B2_EventRecord_Write(EVENT_SERIOUS_FAULT_RES1);
            }

            if (result.KM1_Fault != BFL_VBC_NO_FAULT)
            {
                g_pSysInfo->KM1_Fault = result.KM1_Fault;

                B2_EventRecord_Write(EVENT_SERIOUS_FAULT_RES2);
            }

            g_pSysInfo->SCRT_Fault |= result.SCRT_Fault;

            APP_Main_NotifyHaveParamNeedToSave();
            // TODO:Serious_Fault Event
        }
    }
    else if (result.code == CMD_CODE_EXEC_SUCCESS ||
             result.code == CMD_CODE_SYS_CAPACITORS_ARE_CUTOFF)
    {
        g_pSysInfo->Capacitors_Exec_State = CAPACITORS_STATE_CUT_OFF;
    }
}
