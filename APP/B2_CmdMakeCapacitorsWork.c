/**
 * @file B2_CmdMakeCapacitorsWork.c
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

static void async_delay_callback(void *arg)
{
  UNUSED(arg);
  BackGroundTask();
}

B2_CmdMakeCapacitorsWork_Result_t B2_CmdMakeCapacitorsWork_Exec()
{
  B2_CmdMakeCapacitorsWork_Result_t result;
  result.code = CMD_CODE_NO_FAULT;
  result.QF_Fault = BFL_VBC_NO_FAULT;
  result.KM1_Fault = BFL_VBC_NO_FAULT;
  result.SCRT_Fault = SCR_NO_FAULT;
  if (Have_Serious_Fault())
  {
    result.code = CMD_CODE_SYS_IS_IN_SERIOUS_FAULT;
    return result;
  }

  if (!The_System_Line_State_Is_Running())
  {
    result.code = CMD_CODE_SYS_LINE_STATE_IS_NOT_RUNNING;
    return result;
  }

  if (The_Capacitors_Are_Working())
  {
    result.code = CMD_CODE_SYS_CAPACITORS_ARE_WORKING;
    return result;
  }

  uint16_t QF_Fault = BFL_VBC_NO_FAULT;
  uint16_t KM1_Fault = BFL_VBC_NO_FAULT;
  for (int tryCnt = 0; tryCnt < 3; tryCnt++)
  {
    BFL_VCB_Set_As_Switch_Opened(QF_SW);
    BFL_VCB_Set_As_Switch_Opened(KM1_SW);
    async_delay(MS(g_pSysInfo->T1_MS), async_delay_callback, NULL);

    // 执行持续10ms的检测
    QF_Fault = BFL_VBC_NO_FAULT;
    KM1_Fault = BFL_VBC_NO_FAULT;
    for (int i = 0; i < 10; i++)
    {
      BFL_VCB_STATE_t QF_State = BFL_VCB_Get_Actual_State(QF_SW);
      BFL_VCB_STATE_t KM1_State = BFL_VCB_Get_Actual_State(KM1_SW);

      if (QF_State == BFL_VCB_Opened && KM1_State == BFL_VCB_Opened)
      {
        QF_Fault = BFL_VBC_NO_FAULT;
        KM1_Fault = BFL_VBC_NO_FAULT;
        break;
      }

      if (QF_State == BFL_VCB_Closed)
      {
        QF_Fault = BFL_VBC_CANT_OPEN;
      }
      else
      {
        QF_Fault = BFL_VBC_NO_FAULT;
      }

      if (KM1_State == BFL_VCB_Closed)
      {
        KM1_Fault = BFL_VBC_CANT_OPEN;
      }
      else
      {
        KM1_Fault = BFL_VBC_NO_FAULT;
      }

      async_delay(MS(1), async_delay_callback, NULL);
    }

    // 判断持续10ms的检测结果
    if (QF_Fault == BFL_VBC_NO_FAULT && KM1_Fault == BFL_VBC_NO_FAULT)
    {
      break;
    }
  }

  // 判断最终结果
  if (QF_Fault != BFL_VBC_NO_FAULT || KM1_Fault != BFL_VBC_NO_FAULT)
  {
    result.code = CMD_CODE_EXEC_OCCUR_SERIOUS_FAULT;
    result.QF_Fault = QF_Fault;
    result.KM1_Fault = KM1_Fault;
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

void B2_CmdMakeCapacitorsWork_Exec_Solution()
{
  B2_CmdMakeCapacitorsWork_Result_t result = B2_CmdMakeCapacitorsWork_Exec();

  // 更新电容器状态
  B1_VCBStatus_Update();

  // 执行过程中出现严重故障
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

      APP_Main_NotifyHaveParamNeedToSave();
      // TODO:Serious_Fault Event
    }

    // TODO: 处理严重故障
    B2_CmdCutOffCapacitors_Exec();
  }
  else if (result.code == CMD_CODE_EXEC_SUCCESS)
  {
    g_pSysInfo->Capacitors_Exec_State = CAPACITORS_STATE_WORKING;
  }
}
