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
#include "BFL_Button.h"
#include "BFL_Buzz.h"
#include "BFL_DebugPin.h"
#include "BFL_Measure.h"
#include "BFL_RS485.h"
#include "BFL_SCR.h"
#include "BFL_VCB.h"
#include "CHIP_W25Q128.h"
#include "CPU_Define.h"
#include "DSP2833x_Device.h"
#include "HDL_CPU_Time.h"
#include "HDL_Uart.h"
#include "ccommon.h"
#include "log.h"
#include "crc.h"
#include "period_query.h"
#include "mtime.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// TODO:利用全局变量初始化为0的特性,但是浮点数还是要手动初始化
AppMainInfo_t g_AppMainInfo = {0};
SysInfo_t *g_pSysInfo = &g_AppMainInfo.sysInfo;

void Config_Default_Parameter()
{
  memset(&g_AppMainInfo, 0, sizeof(g_AppMainInfo));
  g_AppMainInfo.sysInfoSize = sizeof(SysInfo_t);

  g_pSysInfo->TV1A_ScaleL1 = 1.0f;
  g_pSysInfo->TV1B_ScaleL1 = 1.0f;
  g_pSysInfo->TV1C_ScaleL1 = 1.0f;
  g_pSysInfo->UIAB_ScaleL1 = 1.0f;
  g_pSysInfo->UOAB_ScaleL1 = 1.0f;
  g_pSysInfo->TA1A_ScaleL1 = 1.0f;
  g_pSysInfo->TA1B_ScaleL1 = 1.0f;
  g_pSysInfo->TA1C_ScaleL1 = 1.0f;

  g_pSysInfo->TV1A_ScaleL2 = 1.0f;
  g_pSysInfo->TV1B_ScaleL2 = 1.0f;
  g_pSysInfo->TV1C_ScaleL2 = 1.0f;
  g_pSysInfo->UIAB_ScaleL2 = 1.0f;
  g_pSysInfo->UOAB_ScaleL2 = 1.0f;
  g_pSysInfo->TA1A_ScaleL2 = 1.0f;
  g_pSysInfo->TA1B_ScaleL2 = 1.0f;
  g_pSysInfo->TA1C_ScaleL2 = 1.0f;

  g_pSysInfo->I_TA_low_thl = 50;
  g_pSysInfo->I_TA_low_thh = 55;
  g_pSysInfo->T_I_TA_Thh = SEC(10) * 10;
  g_pSysInfo->I_TA_oc = 500;
  g_pSysInfo->T_I_TA_oc = SEC(10) * 10;
  g_pSysInfo->V_TVx_ov = 2.0f;
  g_pSysInfo->T_V_TVx_ov = MINUTE(60);
  g_pSysInfo->Tc_ot = 80;
  g_pSysInfo->T_Tc_ot = SEC(10);
  g_pSysInfo->T1 = MS(1000);
  g_pSysInfo->T2 = US(2000);
  g_pSysInfo->T3 = MS(5);
  g_pSysInfo->T4 = MS(5);

  g_pSysInfo->SYS_MODE = SYS_MODE_AUTO;
  g_pSysInfo->V_SYS_STOP = 4.0f;
  g_pSysInfo->V_SYS_UNDER = 8.0f;
  g_pSysInfo->V_SYS_THH = 10.5f;
  g_pSysInfo->V_SYS_OV = 12.0f;
  g_pSysInfo->T_SYS_UNDER_CANCLE = SEC(10);
}

#define BFL_ARGUMENT_MAX_SIZE (W25Q128_SECTOR_SIZE)
byte_t serializedBuffer[sizeof(SysInfo_t) * 2] = {0};

bool APP_Main_Save_SysInfo()
{
  byte_t *pSysInfo = (byte_t *)&g_AppMainInfo.sysInfo;
  // C2000的CPU没有真正的uint8_t类型，不能直接取对象地址字节化

  for (size_t i = 0; i < sizeof(SysInfo_t); i++)
  {
    serializedBuffer[i * 2 + 0] = pSysInfo[i] & 0xFF;
    serializedBuffer[i * 2 + 1] = (pSysInfo[i] >> 8) & 0xFF;
  }
  uint32_t crc_len = (sizeof(SysInfo_t) - sizeof(g_AppMainInfo.sysInfo.__crc16)) * 2;
  uint16_t crc_get = CRC16_Modbus(serializedBuffer, crc_len);
  g_AppMainInfo.sysInfo.__crc16 = crc_get;

  for (size_t i = 0; i < sizeof(SysInfo_t); i++)
  {
    serializedBuffer[i * 2 + 0] = pSysInfo[i] & 0xFF;
    serializedBuffer[i * 2 + 1] = (pSysInfo[i] >> 8) & 0xFF;
  }
  CHIP_W25Q128_Write(0, serializedBuffer, sizeof(serializedBuffer));
  return true;
}

bool Load_Parameter_From_Flash()
{
  bool ret = false;
  _Static_assert(sizeof(SysInfo_t) <= BFL_ARGUMENT_MAX_SIZE,
                 "sizeof(SysInfo_t) need to be smaller than BFL_ARGUMENT_MAX_SIZE");

  SysInfo_t sysInfo;
  byte_t *pSysInfo = (byte_t *)&sysInfo;
  memset(serializedBuffer, 0, sizeof(serializedBuffer));
  CHIP_W25Q128_Read(0, (byte_t *)serializedBuffer, sizeof(serializedBuffer));
  uint32_t crc_len = (sizeof(SysInfo_t) - sizeof(g_AppMainInfo.sysInfo.__crc16)) * 2;
  uint16_t crc_get = CRC16_Modbus(serializedBuffer, crc_len);
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
  ret = true;
  return ret;
}

void APP_Main_Init()
{
  HDL_CPU_Time_Init();
  BFL_DebugPin_Init();

  // MAX232
  Uart_Init(COM2, 115200, UART_WORD_LEN_8, UART_STOP_BIT_1, UART_PARITY_NONE);
  BFL_Buzz_Init();
  CHIP_W25Q128_Init();
  BFL_VCB_Seurity_Init();
  BFL_SCR_Init();

  Config_Default_Parameter();
  Load_Parameter_From_Flash();

  B1_Measure_Init();
  B1_CapacitanceTemperatureMeasure_Init();
  B1_ModbusRTUSlaver_Init();
  B1_SysModeGet_Init();
  B1_VCBStatusGet_Init();
}

void BackGroundTask()
{
  B0_DeltaPoll();
  B1_CapacitanceTemperatureMeasure_Poll();
  B1_ModbusRTUSlaver_Poll();
  B1_Measure_Poll();
}

void ForeGroundTask()
{
  if (g_pSysInfo->SYS_MODE == SYS_MODE_AUTO)
  {
    B3_SysAutoMode_Poll();
  }
  else if (g_pSysInfo->SYS_MODE == SYS_MODE_MANUAL)
  {
    B3_SysManualMode_Poll();
  }
}

void APP_Main_Poll()
{
  BFL_DebugPin_Set(DEBUG_PIN_2);
  BackGroundTask();
  ForeGroundTask();
  BFL_DebugPin_Reset(DEBUG_PIN_2);
}
