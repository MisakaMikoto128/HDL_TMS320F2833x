/**
 * @file BFL_Buzz.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-04-18
 * @last modified 2024-04-18
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "BFL_Buzz.h"
#include "DSP2833x_Device.h"

static HDL_GPIO_PinState_t g_buzz_trig_level = HDL_GPIO_HIGH;

void BFL_Buzz_Init() {
  EALLOW;
  // GPIO6/EPWM4A/EPWMSYNCI/EPWMSYNCO
  GpioCtrlRegs.GPACTRL.bit.QUALPRD0 = 0;
  // GPIO6 - General purpose I/O 6 (default)
  GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 0x00;
  // Configures the GPIO pin as an output
  GpioCtrlRegs.GPADIR.bit.GPIO6 = 1;
  // Enable the internal pullup on the specified pin. (default for
  // GPIO12-GPIO31)
  GpioCtrlRegs.GPAPUD.bit.GPIO6 = 0;
  BFL_Buzz_Off();
  EDIS;
}

void BFL_Buzz_Off() {
  if (g_buzz_trig_level == HDL_GPIO_HIGH) {
    GpioDataRegs.GPACLEAR.bit.GPIO6 = 1;
  } else {
    GpioDataRegs.GPASET.bit.GPIO6 = 1;
  }
}

void BFL_Buzz_Toggle() { GpioDataRegs.GPATOGGLE.bit.GPIO6 = 1; }

void BFL_Buzz_On() {
  if (g_buzz_trig_level == HDL_GPIO_HIGH) {
    GpioDataRegs.GPASET.bit.GPIO6 = 1;
  } else {
    GpioDataRegs.GPACLEAR.bit.GPIO6 = 1;
  }
}

bool BFL_Buzz_IsOff() {
  if (g_buzz_trig_level == HDL_GPIO_HIGH) {
    return (GpioDataRegs.GPADAT.bit.GPIO6 == 1);
  }

  return (GpioDataRegs.GPADAT.bit.GPIO6 == 0);
}

void BFL_Buzz_SetTrigLevel(HDL_GPIO_PinState_t level) {
  g_buzz_trig_level = level;
}
