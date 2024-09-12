/**
 * @file APP_Main.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-04-18
 * @last modified 2024-04-18
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "APP_Main.h"
#include "B0_DeltaPoll.h"
#include "B2_EventRecord.h"
#include "BFL_Button.h"
#include "BFL_Buzz.h"
#include "BFL_DebugPin.h"
#include "BFL_Measure.h"
#include "BFL_RS485.h"
#include "BFL_SCR.h"
#include "BFL_VCB.h"
#include "datetime.h"
#include "CHIP_W25Q128.h"
#include "CPU_Define.h"
#include "DSP2833x_Device.h"
#include "HDL_CPU_Time.h"
#include "HDL_IWDG.h"
#include "HDL_Uart.h"
#include "ccommon.h"
#include "crc.h"
#include "log.h"
#include "mtime.h"
#include "period_query.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// TODO:利用全局变量初始化为0的特性,但是浮点数还是要手动初始化
AppMainInfo_t g_AppMainInfo = {0};
SysInfo_t *g_pSysInfo = &g_AppMainInfo.sysInfo;
#define __DEBUG_MODE 1
void AppMainInfo_Init() { memset(&g_AppMainInfo, 0, sizeof(g_AppMainInfo)); }

void Config_Default_Parameter()
{
  g_AppMainInfo.sysInfoSize = sizeof(SysInfo_t);

  g_pSysInfo->TV1A_ScaleL1 = 1411.800048828125f;
  g_pSysInfo->TV1B_ScaleL1 = 1411.800048828125f;
  g_pSysInfo->TV1C_ScaleL1 = 1411.800048828125f;
  g_pSysInfo->UIAB_ScaleL1 = 1411.800048828125f;
  g_pSysInfo->UOAB_ScaleL1 = 1411.800048828125f;
  g_pSysInfo->TA1A_ScaleL1 = 6.2300004959106445f;
  g_pSysInfo->TA1B_ScaleL1 = 6.2300004959106445f;
  g_pSysInfo->TA1C_ScaleL1 = 6.2300004959106445f;

  g_pSysInfo->TV1A_ScaleL2 = 55.0f;
  g_pSysInfo->TV1B_ScaleL2 = 55.0f;
  g_pSysInfo->TV1C_ScaleL2 = 55.0f;
  g_pSysInfo->UIAB_ScaleL2 = 500.0f;
  g_pSysInfo->UOAB_ScaleL2 = 500.0f;
  g_pSysInfo->TA1A_ScaleL2 = 300.0f;
  g_pSysInfo->TA1B_ScaleL2 = 300.0f;
  g_pSysInfo->TA1C_ScaleL2 = 300.0f;

  g_pSysInfo->I_TA_low_thl_A = 50;
  g_pSysInfo->I_TA_low_thh_A = 55;
  g_pSysInfo->T_I_TA_Thh_SEC = SEC(10);
  g_pSysInfo->I_TA_oc_A = 200;
  g_pSysInfo->T_I_TA_oc_SEC = SEC(4);
  g_pSysInfo->V_TVx_ov_kV = 2.0f;
  g_pSysInfo->T_V_TVx_ov_SEC = SEC(10);
  g_pSysInfo->Tc_ot = 80;
  g_pSysInfo->T_Tc_ot_SEC = SEC(10);
  g_pSysInfo->T1_MS = MS(1500);
  g_pSysInfo->T2_US = US(4000);
  g_pSysInfo->T3_MS = MS(5);
  g_pSysInfo->T4_MS = MS(20);
  g_pSysInfo->V_SCR_NORMAL_DIFF_kV = 0.1f;
  g_pSysInfo->T_V_SCR_ABNORMAL_DIFF_SEC = SEC(3);
  g_pSysInfo->I_SCR_NORMAL_DIFF_A = 1.0f;
  g_pSysInfo->I_TA_quick_oc_A = 500;
  g_pSysInfo->T_I_TA_quick_oc_MS = MS(200);
  g_pSysInfo->T_V_SYS_OV_SEC = SEC(2);

  g_pSysInfo->V_SYS_STOP_kV = 4.0f;
  g_pSysInfo->V_SYS_UNDER_kV = 8.0f;
  g_pSysInfo->V_SYS_THH_kV = 10.5f;
  g_pSysInfo->V_SYS_OV_kV = 12.0f;
  g_pSysInfo->T_SYS_UNDER_CANCLE_SEC = SEC(10);
  g_pSysInfo->T_SYS_SATIFY_CAPACITORS_WAORK_SEC = SEC(5);

  g_pSysInfo->devId = 0x0807060504030201ULL;

  g_AppMainInfo.VBCDebugMode = 0;
}

#define BFL_ARGUMENT_MAX_SIZE                         \
  (sizeof(g_AppMainInfo.buffer) > W25Q128_SECTOR_SIZE \
       ? W25Q128_SECTOR_SIZE                          \
       : sizeof(g_AppMainInfo.buffer))
bool APP_Main_Save_SysInfo()
{
  byte_t *pSysInfo = (byte_t *)&g_AppMainInfo.sysInfo;
  byte_t *serializedBuffer = (byte_t *)&g_AppMainInfo.buffer;
  // C2000的CPU没有真正的uint8_t类型，不能直接取对象地址字节化

  for (size_t i = 0; i < sizeof(SysInfo_t); i++)
  {
    serializedBuffer[i * 2 + 0] = pSysInfo[i] & 0xFF;
    serializedBuffer[i * 2 + 1] = (pSysInfo[i] >> 8) & 0xFF;
  }
  uint32_t crc_len =
      (sizeof(SysInfo_t) - sizeof(g_AppMainInfo.sysInfo.__crc16)) * 2;
  uint32_t serialized_len = sizeof(SysInfo_t) * 2;
  uint16_t crc_get = CRC16_Modbus(serializedBuffer, crc_len);
  g_AppMainInfo.sysInfo.__crc16 = crc_get;

  for (size_t i = 0; i < sizeof(SysInfo_t); i++)
  {
    serializedBuffer[i * 2 + 0] = pSysInfo[i] & 0xFF;
    serializedBuffer[i * 2 + 1] = (pSysInfo[i] >> 8) & 0xFF;
  }
  CHIP_W25Q128_Write(0, serializedBuffer, serialized_len);
  return true;
}

bool Load_Parameter_From_Flash()
{
  bool ret = false;
  _Static_assert(
      sizeof(SysInfo_t) * 2 <= BFL_ARGUMENT_MAX_SIZE,
      "sizeof(SysInfo_t) need to be smaller than BFL_ARGUMENT_MAX_SIZE");
  byte_t *serializedBuffer = (byte_t *)&g_AppMainInfo.buffer;
  uint32_t serialized_len = sizeof(SysInfo_t) * 2;
  memset(serializedBuffer, 0, serialized_len);

  CHIP_W25Q128_Read(0, (byte_t *)serializedBuffer, serialized_len);
  uint32_t crc_len =
      (sizeof(SysInfo_t) - sizeof(g_AppMainInfo.sysInfo.__crc16)) * 2;
  uint16_t crc_get = CRC16_Modbus(serializedBuffer, crc_len);

  SysInfo_t sysInfo;
  byte_t *pSysInfo = (byte_t *)&sysInfo;
  for (size_t i = 0; i < sizeof(SysInfo_t); i++)
  {
    byte_t low = serializedBuffer[i * 2 + 0] & 0xFF;
    byte_t high = serializedBuffer[i * 2 + 1] & 0xFF;
    pSysInfo[i] = low | (high << 8);
  }

  if (sysInfo.__crc16 != crc_get)
  {
    APP_Main_Save_SysInfo();
    ret = false;
    return ret;
  }

  // 读取成功
  g_AppMainInfo.sysInfo = sysInfo;
  g_AppMainInfo.sysInfo.powerOnTimes++;
  APP_Main_Save_SysInfo();
  ret = true;
  return ret;
}

void APP_Main_EraseFlashParamData()
{
  Config_Default_Parameter();
  CHIP_W25Q128_Erase_One_Sector(0);
}

void APP_Main_Switch_VCB_Trigger_Mode()
{
  g_AppMainInfo.VBCDebugMode = !g_AppMainInfo.VBCDebugMode;
}

void Config_PowerOn_Parameter()
{
  g_pSysInfo->SYS_MODE = SYS_MODE_AUTO;
  g_pSysInfo->Capacitors_State = CAPACITORS_STATE_CUT_OFF;
  g_pSysInfo->Capacitors_Exec_State = CAPACITORS_STATE_CUT_OFF;
  g_pSysInfo->Minor_Fault = 0;
  g_pSysInfo->Line_State = LINE_STATE_STOP;
}

void APP_Main_Clear_All_Fault()
{
  g_pSysInfo->Serious_Fault = 0;
  g_pSysInfo->KM1_Fault = BFL_VBC_NO_FAULT;
  g_pSysInfo->QF_Fault = BFL_VBC_NO_FAULT;
  g_pSysInfo->SCRT_Fault = 0;
  g_pSysInfo->VTx_A_Breakdown_Fault = SCR_NO_FAULT;
  g_pSysInfo->VTx_B_Breakdown_Fault = SCR_NO_FAULT;
  g_pSysInfo->VTx_C_Breakdown_Fault = SCR_NO_FAULT;
  g_pSysInfo->I_TA_quick_oc_Fault = 0;
  APP_Main_NotifyHaveParamNeedToSave();
}

void APP_Main_Init()
{
  HDL_CPU_Time_Init();
  B1_VCBStatusGet_Init();
  BFL_VCB_Seurity_Init();
  BFL_SCR_Init();
  B1_SysModeGet_Init();
  BFL_DebugPin_Init();
  HDL_IWDG_Init(SECOND_TO_MS(1));
  datetime_init();
  BFL_Buzz_Init();
  CHIP_W25Q128_Init();

  AppMainInfo_Init();
  Config_Default_Parameter();
  Load_Parameter_From_Flash();
  Config_PowerOn_Parameter();

#if __DEBUG_MODE == 1
//  APP_Main_Clear_All_Fault();
#endif

  B1_Measure_Init();
  B1_CapacitanceTemperatureMeasure_Init();
  B1_ModbusRTUSlaver_Init();
  B3_RTUPush_Init();

  B2_EventRecord_Init();
  B2_EventRecord_Write(EVENT_SYSTEM_POWER_ON);
}

uint32_t g_backGroundTaskMaxRuningTimeUS = 0;
uint32_t g_backGroundTaskRuningTimeUS = 0;
uint32_t g_A;
uint32_t g_B;
void BackGroundTask()
{
  // Test Begin
  BFL_DebugPin_Set(DEBUG_PIN_2);
  g_A = HDL_CPU_Time_GetUsTick();
  // Test Begin End

  HDL_IWDG_Feed();
  B1_CapacitanceTemperatureMeasure_Poll();
  B1_ModbusRTUSlaver_Poll();
  B1_Measure_Poll();
  B0_DeltaPoll(poll_delta, B1_SysModeGet_DeltaPoll(poll_delta);
               B1_VCBStatusGet_DeltaPoll(poll_delta););
  APP_Main_SysinfoSavePoll();
  B3_RTUPush_Poll();
  // B2_CmdBypassCapacitors_Test();

  // Test End
  g_B = HDL_CPU_Time_GetUsTick();
  g_backGroundTaskRuningTimeUS = g_B - g_A;
  g_backGroundTaskMaxRuningTimeUS =
      g_backGroundTaskMaxRuningTimeUS > g_backGroundTaskRuningTimeUS
          ? g_backGroundTaskMaxRuningTimeUS
          : (g_B - g_A);

  BFL_DebugPin_Reset(DEBUG_PIN_2);
  // Test End End
}

void BackGroundTask_WhenInSRCPoll()
{
  HDL_IWDG_Feed();
  B1_CapacitanceTemperatureMeasure_Poll();
  B1_ModbusRTUSlaver_Poll();
  B1_Measure_Poll();
  B0_DeltaPoll(poll_delta, B1_SysModeGet_DeltaPoll(poll_delta);
               B1_VCBStatusGet_DeltaPoll(poll_delta););
}

// 正常运行
void ForeGroundTask()
{
  B0_DeltaPoll(poll_delta, {
    if (g_pSysInfo->SYS_MODE == SYS_MODE_AUTO)
    {
      B3_SysAutoMode_DeltaPoll(poll_delta);
    }
    else if (g_pSysInfo->SYS_MODE == SYS_MODE_MANUAL)
    {
      B3_SysManualMode_DeltaPoll(poll_delta);
    }
  });

  B2_EventRecord_Poll();
}

void APP_Main_Poll()
{
  BackGroundTask();
  ForeGroundTask();
}

void APP_Main_SysinfoSavePoll()
{
  static PeriodREC_t s_tPollTime1 = 0;
  if_period_query_user_us(&s_tPollTime1, SEC_TO_US(1))
  {
    // 耗时操作，放在前台执行
    if (APP_Main_HaveParamNeedToSave())
    {
      APP_Main_Save_SysInfo();
      APP_Main_ClearParamNeedToSave();
    }
  }
}

float getMaxCapTemp()
{
  float Tc_MAX = -273.15f;
  int capTempSize = sizeof(g_pSysInfo->capTemp) / sizeof(g_pSysInfo->capTemp[0]);
  for (int i = 0; i < capTempSize; i++)
  {
    if (g_pSysInfo->capTemp[i] > Tc_MAX)
    {
      Tc_MAX = g_pSysInfo->capTemp[i];
    }
  }
  return Tc_MAX;
}

float getMinCapTemp()
{
  float Tc_MIN = 1000.0f;
  int capTempSize = sizeof(g_pSysInfo->capTemp) / sizeof(g_pSysInfo->capTemp[0]);
  for (int i = 0; i < capTempSize; i++)
  {
    if (g_pSysInfo->capTemp[i] < Tc_MIN)
    {
      Tc_MIN = g_pSysInfo->capTemp[i];
    }
  }
  return Tc_MIN;
}

