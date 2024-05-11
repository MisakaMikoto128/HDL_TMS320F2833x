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
#include "BFL_Buzz.h"
#include "BFL_Measure.h"
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
// void Debug_Printf(const void *format, ...) {
//
//   uint32_t uLen;
//   va_list vArgs;
//   va_start(vArgs, format);
//   uLen = vsnprintf(buffer, MAXDEBUGSEND, (char const *)format, vArgs);
//   va_end(vArgs);
//   if (uLen > MAXDEBUGSEND)
//     uLen = MAXDEBUGSEND;
//   Uart_Write(DEBUG_COM, (uint16_t *)buffer, uLen);
// }

void InitSpiaGpio();
void spi_xmit(Uint16 a);

void APP_Main_Init() {
  HDL_CPU_Time_Init();
  // MAX232
  Uart_Init(COM2, 115200, UART_WORD_LEN_8, UART_STOP_BIT_1, UART_PARITY_NONE);
  // Uart_Init(COM3,115200,UART_WORD_LEN_8,UART_STOP_BIT_1,UART_PARITY_NONE);
  BFL_Buzz_Init();
  //  CHIP_W25Q128_Init();
  BFL_Measure_Init();
  EALLOW;
  // General purpose I/O
  GpioCtrlRegs.GPCMUX1.bit.GPIO64 = 0x00;
  // Configures the GPIO pin as an output
  GpioCtrlRegs.GPCDIR.bit.GPIO64 = 1;
  // Enable the internal pullup on the specified pin.
  GpioCtrlRegs.GPCPUD.bit.GPIO64 = 0;

  GpioDataRegs.GPCCLEAR.bit.GPIO64 = 1;

  // General purpose I/O
  GpioCtrlRegs.GPCMUX1.bit.GPIO65 = 0x00;
  // Configures the GPIO pin as an output
  GpioCtrlRegs.GPCDIR.bit.GPIO65 = 1;
  // Enable the internal pullup on the specified pin.
  GpioCtrlRegs.GPCPUD.bit.GPIO65 = 0;

  GpioDataRegs.GPCSET.bit.GPIO65 = 1;

  // General purpose I/O
  GpioCtrlRegs.GPBMUX2.bit.GPIO49 = 0x00;
  // Configures the GPIO pin as an output
  GpioCtrlRegs.GPBDIR.bit.GPIO49 = 1;
  // Enable the internal pullup on the specified pin.
  GpioCtrlRegs.GPBPUD.bit.GPIO49 = 0;

  GpioDataRegs.GPBSET.bit.GPIO49 = 1;
  EDIS;

  InitSpiaGpio();
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Uint16 sdata; // send data
Uint16 rdata; // received data
void APP_Main_Poll() {
  // BFL_Buzz_Toggle();
  HDL_CPU_Time_DelayMs(1000);

  //   GpioDataRegs.GPCTOGGLE.bit.GPIO64 = 1;
  //   GpioDataRegs.GPCTOGGLE.bit.GPIO65 = 1;
}

/*
byte_t flash_sector_buf[W25Q128_SECTOR_SIZE] = {0};
void CHIP_W25Q128_Test() {
  // Debug_Printf("111\r\n");
  uint16_t id = CHIP_W25Q128_Read_ID();
  Debug_Printf("CHIP_W25Q128_Read_ID:%x\r\n", id);

  for (int i = 0; i < W25Q128_SECTOR_SIZE; i++) {
    flash_sector_buf[i] = i & 0xFF;
  }

  CHIP_W25q128_Write_One_Sector(1, flash_sector_buf);

  for (int i = 0; i < W25Q128_SECTOR_SIZE; i++) {
    flash_sector_buf[i] = 0;
  }

  CHIP_W25Q128_Read(W25Q128_SECTOR_SIZE * 1, flash_sector_buf,
                    W25Q128_SECTOR_SIZE);

  int cnt = 0;
  for (int i = 0; i < W25Q128_SECTOR_SIZE; i++) {
    if (flash_sector_buf[i] != (i & 0xFF)) {
      cnt++;
    }
  }

  if (cnt == 0) {
    Debug_Printf("CHIP_W25q128_Write_One_Sector success\r\n");
  } else {
    Debug_Printf("CHIP_W25q128_Write_One_Sector fail, cnt:%d\r\n", cnt);
  }

  cnt = 0;
}
*/
