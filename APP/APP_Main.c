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
#include "BFL_Measure.h"
#include "BFL_SCR.h"
#include "BFL_VCB.h"
#include "CHIP_W25Q128.h"
#include "CPU_Define.h"
#include "HDL_CPU_TIme.h"
#include "HDL_Uart.h"

#include <stdarg.h>
#include <stdio.h>

// 调试输出数据包最大长度
#define MAXDEBUGSEND 256
static char buffer[MAXDEBUGSEND + 1];
#define DEBUG_COM COM2

/**
 * @brief 使用串口调试的格式化输出方法
 *
 * @param format
 * @param ...
 */
void Debug_Printf(const void *format, ...)
{
  uint32_t uLen;
  va_list vArgs;
  va_start(vArgs, format);
  uLen = vsnprintf(buffer, MAXDEBUGSEND, (char const *)format, vArgs);
  va_end(vArgs);
  if (uLen > MAXDEBUGSEND)
    uLen = MAXDEBUGSEND;
  Uart_Write(DEBUG_COM, (uint16_t *)buffer, uLen);
}

void InitSpiaGpio();
void spi_xmit(Uint16 a);

struct APP_Main_Stack_t
{
  bool modeBtnPressed;
};

struct APP_Main_Stack_t g_app_main_stack;
void APP_Main_Init()
{
  HDL_CPU_Time_Init();
  // MAX232
  Uart_Init(COM2, 115200, UART_WORD_LEN_8, UART_STOP_BIT_1, UART_PARITY_NONE);
  Uart_Init(COM3, 115200, UART_WORD_LEN_8, UART_STOP_BIT_1, UART_PARITY_NONE);
  BFL_Buzz_Init();
  CHIP_W25Q128_Init();
  BFL_Measure_Init();
  BFL_VCB_Seurity_Init();
  BFL_Button_Init();
  BFL_SCR_Init();

  g_app_main_stack.modeBtnPressed = false;

  // // General purpose I/O
  // GpioCtrlRegs.GPBMUX2.bit.GPIO49 = 0x00;
  // // Configures the GPIO pin as an output
  // GpioCtrlRegs.GPBDIR.bit.GPIO49 = 1;
  // // Enable the internal pullup on the specified pin.
  // GpioCtrlRegs.GPBPUD.bit.GPIO49 = 0;

  // GpioDataRegs.GPBSET.bit.GPIO49 = 1;

  // GpioDataRegs.GPBCLEAR.bit.GPIO49 = 1;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void timer_callback() { Debug_Printf("timer_callback\n"); }

Uint16 sdata; // send data
Uint16 rdata; // received data
void APP_Main_Poll()
{
  // BFL_Buzz_Toggle();

  // for_Each_VCB_SW_t(vcb)
  // {
  //   if (BFL_VCB_Get_Setting_State(vcb) == BFL_VCB_Opened)
  //   {
  //     BFL_VCB_Set_As_Switch_Closed(vcb);
  //   }
  //   else if (BFL_VCB_Get_Setting_State(vcb) == BFL_VCB_Closed)
  //   {
  //     BFL_VCB_Set_As_Switch_Opened(vcb);
  //   }
  //   else
  //   {
  //     BFL_VCB_Set_As_Switch_Closed(vcb);
  //   }
  // }

  //   {

  //     HDL_CPU_Time_DelayMs(1000);

  //     for_Each_VCB_SW_t(vcb)
  //     {
  //       BFL_VCB_STATE_t state = BFL_VCB_Get_Actual_State(vcb);
  //       BFL_VCB_STATE_t settingState = BFL_VCB_Get_Setting_State(vcb);
  //       Debug_Printf("VCB:%s, fb: %s, set:%s\n", BFL_VCB_SW_To_String(vcb),
  //                    BFL_VCB_STATE_To_String(state),
  //                    BFL_VCB_STATE_To_String(settingState));
  //     }

  //     if (BFL_Button_IsPressed(MODE_BTN))
  //     {
  //       if (g_app_main_stack.modeBtnPressed == false)
  //       {
  //         g_app_main_stack.modeBtnPressed = true;
  //         Debug_Printf("MODE_BTN is pressed\n");
  //       }
  //     }
  //     else
  //     {
  //       if (g_app_main_stack.modeBtnPressed == true)
  //       {
  //         g_app_main_stack.modeBtnPressed = false;
  //         Debug_Printf("MODE_BTN is released\n");
  //       }
  //     }

  //     Debug_Printf("Hello World\n");
  //     HDL_CPU_Time_StartHardTimer(1, 1500000U, timer_callback);

  //     HDL_CPU_Time_DelayMs(1000);
  //     Debug_Printf("bb Hello World\n");
  //   }

  {
    HDL_CPU_Time_DelayMs(100);
    BFL_SCRT_Pluse_Transmit(SCRTA, 4, 2000);
  }
}
