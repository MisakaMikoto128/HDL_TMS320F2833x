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
#include "HDL_CPU_TIme.h"
#include "HDL_Uart.h"
#include "ccommon.h"
#include "log.h"
#include "period_query.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

char buffer[256 + 1];

void InitSpiaGpio();
void spi_xmit(Uint16 a);

struct APP_Main_Stack_t
{
  bool modeBtnPressed;
};

struct APP_Main_Stack_t g_app_main_stack;

// TODO:利用全局变量初始化为0的特性,但是浮点数还是要手动初始化
AppMainInfo_t g_AppMainInfo = {0};
SysInfo_t *g_pSysInfo = &g_AppMainInfo.sysInfo;

void APP_Main_Init()
{

  HDL_CPU_Time_Init();
  BFL_DebugPin_Init();

  // MAX232
  Uart_Init(COM2, 115200, UART_WORD_LEN_8, UART_STOP_BIT_1,
            UART_PARITY_NONE);
  //   BFL_Buzz_Init();
  //   CHIP_W25Q128_Init();
  //   BFL_VCB_Seurity_Init();
  //   BFL_Button_Init();
  //   BFL_SCR_Init();

  memset(&g_AppMainInfo, 0, sizeof(g_AppMainInfo));

  B1_Measure_Init();
  B1_CapacitanceTemperatureMeasure_Init();
  B1_ModbusRTUSlaver_Init();

  g_app_main_stack.modeBtnPressed = false;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void timer_callback() { Debug_Printf("timer_callback\n"); }
float t = 0;
int cnt = 1;
Uint16 sdata; // send data
Uint16 rdata; // received data
uint32_t signal;
void APP_Main_Poll()
{
  BFL_DebugPin_Set(DEBUG_PIN_2);

  B1_CapacitanceTemperatureMeasure_Poll();
  B1_ModbusRTUSlaver_Poll();
  B1_Measure_Poll();
  //   HDL_CPU_Time_DelayMs(1000);
  BFL_DebugPin_Reset(DEBUG_PIN_2);
}
