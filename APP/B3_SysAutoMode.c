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

volatile int debugVar = 0;

void DebugTest()
{
  if (debugVar == 1)
  {
    debugVar = 0;
    B2_CmdCutOffCapacitors_Exec_Solution();
  }

  if (debugVar == 2)
  {
    debugVar = 0;
    B2_CmdBypassCapacitors_Exec_Solution();
  }

  if (debugVar == 3)
  {
    debugVar = 0;
    B2_CmdMakeCapacitorsWork_Exec_Solution();
  }
}

void B3_SysAutoMode_DeltaPoll(uint32_t poll_delta)
{
  DebugTest();

  if (CheckConditionDurationMet(
          &g_AppMainInfo.satifyLineStateRunningTimeCnt, poll_delta,
          SECOND_TO_MS(5), (g_pSysInfo->V_UIAB > g_pSysInfo->V_SYS_STOP_kV)))
  {
    g_pSysInfo->Line_State = LINE_STATE_RUNNING;
  }
  else if (CheckConditionDurationMet(
               &g_AppMainInfo.dissatifyLineStateRunningTimeCnt, poll_delta,
               SECOND_TO_MS(5),
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
        float I_TA1_MAX = getI_TA1_MAX();

        if (CheckConditionDurationMet(
                &g_AppMainInfo.satifyCapacitorsWorkTimeCnt, poll_delta,
                SECOND_TO_MS(g_pSysInfo->T_SYS_SATIFY_CAPACITORS_WAORK_SEC),
                (g_pSysInfo->V_UIAB > g_pSysInfo->V_SYS_UNDER_kV &&
                 g_pSysInfo->V_UIAB < g_pSysInfo->V_SYS_THH_kV &&
                 I_TA1_MAX < g_pSysInfo->I_TA_oc_A)))
        {
          B2_CmdMakeCapacitorsWork_Exec_Solution();
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
    // BFL_VCB_Set_As_Switch_Closed(QF_SW);
    // BFL_VCB_Set_As_Switch_Closed(KM1_SW);
    // g_pSysInfo->Capacitors_Exec_State = CAPACITORS_STATE_CUT_OFF;
  }

}
