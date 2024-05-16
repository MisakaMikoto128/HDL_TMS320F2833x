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

#include "ccommon.h"
#include "period_query.h"
#include "BFL_Button.h"
#include "BFL_Buzz.h"
#include "BFL_Measure.h"
#include "BFL_SCR.h"
#include "BFL_VCB.h"
#include "CHIP_W25Q128.h"
#include "CPU_Define.h"
#include "DSP2833x_Device.h"
#include "HDL_CPU_TIme.h"
#include "HDL_Uart.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#define RS485C_Take_Bus() (GpioDataRegs.GPBSET.bit.GPIO61 = 1)
#define RS485C_Release_Bus() (GpioDataRegs.GPBCLEAR.bit.GPIO61 = 1)

#define RS485A_Take_Bus() (GpioDataRegs.GPASET.bit.GPIO28 = 1)
#define RS485A_Release_Bus() (GpioDataRegs.GPACLEAR.bit.GPIO28 = 1)

typedef enum
{
  RS485_1 = 0, // SCI-A,隔离 MAX3485
  RS485_2 = 1, // SCI-C,MAX3485
  RS485_NUM,
} BFL_RS485_t;

void BFL_RS485_Take_Bus(BFL_RS485_t rs485)
{
  if (rs485 == RS485_1)
  {
    RS485A_Take_Bus();
  }
  else if (rs485 == RS485_2)
  {
    RS485C_Take_Bus();
  }
}

void BFL_RS485_Release_Bus(BFL_RS485_t rs485)
{
  if (rs485 == RS485_1)
  {
    RS485A_Release_Bus();
  }
  else if (rs485 == RS485_2)
  {
    RS485C_Release_Bus();
  }
}

void BFL_RS485_Take_Bus_RS4851()
{
  RS485A_Take_Bus();
}

void BFL_RS485_Release_Bus_RS4851(void *arg)
{
  UNUSED(arg);
  RS485A_Release_Bus();
}

void BFL_RS485_Take_Bus_RS4852()
{
  RS485C_Take_Bus();
}

void BFL_RS485_Release_Bus_RS4852(void *arg)
{
  UNUSED(arg);
  RS485C_Release_Bus();
}

void BFL_RS485_Init(BFL_RS485_t rs485, uint32_t baud, uint32_t wordLen, uint32_t stopBit, uint32_t parity)
{
  if (rs485 == RS485_1)
  {

    EALLOW;
    // EN485A GPIO,无上下拉电阻
    // General purpose I/O
    GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 0x00;
    // Configures the GPIO pin as an output
    GpioCtrlRegs.GPADIR.bit.GPIO28 = 1;
    // Enable the internal pullup on the specified pin.
    GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;
    RS485A_Release_Bus();
    EDIS;
    Uart_Init(COM1, baud, wordLen, stopBit, parity);
    Uart_SetWriteOverCallback(COM1, BFL_RS485_Release_Bus_RS4851, NULL);
  }
  else if (rs485 == RS485_2)
  {

    EALLOW;
    // EN485C GPIO,无上下拉电阻
    // General purpose I/O
    GpioCtrlRegs.GPBMUX2.bit.GPIO61 = 0x00;
    // Configures the GPIO pin as an output
    GpioCtrlRegs.GPBDIR.bit.GPIO61 = 1;
    // Enable the internal pullup on the specified pin.
    GpioCtrlRegs.GPBPUD.bit.GPIO61 = 0;
    RS485C_Release_Bus();
    EDIS;
    Uart_Init(COM3, baud, wordLen, stopBit, parity);
    Uart_SetWriteOverCallback(COM3, BFL_RS485_Release_Bus_RS4852, NULL);
  }
}

uint32_t BFL_RS485_Write(BFL_RS485_t rs485, const uint16_t *writeBuf, uint32_t uLen)
{
  if (rs485 == RS485_1)
  {
    BFL_RS485_Take_Bus(rs485);
    return Uart_Write(COM1, writeBuf, uLen);
  }
  else if (rs485 == RS485_2)
  {
    BFL_RS485_Take_Bus(rs485);
    return Uart_Write(COM3, writeBuf, uLen);
  }
  return 0;
}

uint32_t BFL_RS485_Read(BFL_RS485_t rs485, uint16_t *pBuf, uint32_t uiLen)
{
  if (rs485 == RS485_1)
  {
    return Uart_Read(COM1, pBuf, uiLen);
  }
  else if (rs485 == RS485_2)
  {
    return Uart_Read(COM3, pBuf, uiLen);
  }
  return 0;
}

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

void cb1(void *arg) { Debug_Printf("cb1\n"); }
// 01 03 00 00 00 02 C4 0B
// 02 03 00 00 00 02 C4 38
void APP_Main_Init()
{
  HDL_CPU_Time_Init();

  BFL_RS485_Init(RS485_1, 38400, UART_WORD_LEN_8, UART_STOP_BIT_1,
                 UART_PARITY_NONE); // 隔离 MAX3485
  Uart_Init(COM2, 115200, UART_WORD_LEN_8, UART_STOP_BIT_1,
            UART_PARITY_NONE); // MAX232
  BFL_RS485_Init(RS485_2, 115200, UART_WORD_LEN_8, UART_STOP_BIT_1,
                 UART_PARITY_NONE); // MAX3485
  //   BFL_Buzz_Init();
  //   CHIP_W25Q128_Init();
  //   BFL_Measure_Init();
  //   BFL_VCB_Seurity_Init();
  //   BFL_Button_Init();
  //   BFL_SCR_Init();

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
float t = 0;
int cnt = 1;
Uint16 sdata; // send data
Uint16 rdata; // received data
uint32_t signal;

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

  // {
  //   HDL_CPU_Time_DelayMs(1000ULL);
  //   // BFL_SCRT_Pluse_Transmit(SCRT_ALL, 4, 2000);

  //   // signal = BFL_SCRR_Have_Signal(SCRR_ALL);

  //   RS485C_Release_Bus();
  //   RS485A_Release_Bus();

  //   // RS485C_Take_Bus();
  //   // RS485A_Take_Bus();

  //   Uint32 tickA = HDL_CPU_Time_GetUsTick();

  //   Uart_Write(COM1, (const uint16_t *)"123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789",
  //              cnt);
  //   uint32_t readLen = Uart_Read(COM2, (uint16_t *)buffer, sizeof(buffer));
  //   if (readLen > 0)
  //   {
  //     Uart_Write(COM2, (const uint16_t *)buffer, readLen);
  //   }

  //   // for (uint32_t i = 0; i < cnt; i++) {
  //   //   //            cnt);
  //   //   SciaRegs.SCITXBUF = 'a';
  //   //   // 在此做判断，如果发送FIFO缓冲中数据  >=
  //   //   // 16字节，要等待下直到FIFO小于16才能再次向FIFO中存数据
  //   //   while (SciaRegs.SCICTL2.bit.TXRDY == 0) {
  //   //   }
  //   // }
  //   // //  TX FIFO Interrupt Enable
  //   // SciaRegs.SCIFFTX.bit.TXFFIENA = 1;

  //   Uint32 tickB = HDL_CPU_Time_GetUsTick();

  //   t = (tickB - tickA);
  //   // Uart_Write(COM2, (const uint16_t *)"123456789123456789", 18);
  //   // Uart_Write(COM3, (const uint16_t *)"123456789123456789", 18);
  // }

  {
    period_query(1, 100)
    {

      const byte_t request_cmd1[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x0B};
      const byte_t request_cmd2[] = {0x02, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x38};
      BFL_RS485_Write(RS485_1, request_cmd1, sizeof(request_cmd1));
    }

    uint32_t readLen = BFL_RS485_Read(RS485_1, (uint16_t *)buffer, sizeof(buffer));
    if (readLen > 0)
    {
      //    Debug_Printf("Hello World\n");
      Uart_Write(COM2, buffer, readLen);
    }
  }
}
