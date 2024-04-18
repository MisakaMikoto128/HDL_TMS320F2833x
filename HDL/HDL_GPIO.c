/**
 * @file HDL_GPIO.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-04-17
 * @last modified 2024-04-17
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "HDL_GPIO.h"
#include "CPU_Define.h"
#include "ccommon.h"
void HDL_GPIO_SetMode(enum HDL_GPIO_Port port, enum HDL_GPIO_PIN pin,
                      enum HDL_GPIO_MODE mode) {
  UNSUSED(port);

  EALLOW;

  if (pin < HDL_GPIO_PIN_16) {
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = mode;
  } else {
    GpioCtrlRegs.GPBMUX1.bit.GPIO32 = mode;
  }
  GpioCtrlRegs.GPCMUX1.bit.GPIO68 = 0;
  GpioCtrlRegs.GPCDIR.bit.GPIO68 = 1;
  GpioCtrlRegs.GPCPUD.bit.GPIO68 = 0;

  GpioCtrlRegs.GPCMUX1.bit.GPIO67 = 0;
  GpioCtrlRegs.GPCDIR.bit.GPIO67 = 1;
  GpioCtrlRegs.GPCPUD.bit.GPIO67 = 0;
  EDIS;
}
