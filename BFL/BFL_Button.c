/**
 * @file BFL_Button.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-14
 * @last modified 2024-05-14
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "BFL_Button.h"
#include "CPU_Define.h"

#define XIN5_IsSet() (GpioDataRegs.GPBDAT.bit.GPIO44)
#define XIN6_IsSet() (GpioDataRegs.GPBDAT.bit.GPIO45)
#define XIN7_IsSet() (GpioDataRegs.GPBDAT.bit.GPIO46)
#define XIN8_IsSet() (GpioDataRegs.GPBDAT.bit.GPIO47)

void BFL_Button_Init()
{
    EALLOW;
    // Specifies the sampling period for pins GPIO40 to GPIO47 with Sampling Period = TSYSCLKOUT
    GpioCtrlRegs.GPBCTRL.bit.QUALPRD1 = 0x00;

    // IO输入口电路上被下拉
    // General purpose I/O
    GpioCtrlRegs.GPBMUX1.bit.GPIO44 = 0x00;
    // Configures the GPIO pin as an input
    GpioCtrlRegs.GPBDIR.bit.GPIO44 = 0;
    // Disable the internal pullup on the specified pin.
    GpioCtrlRegs.GPBPUD.bit.GPIO44 = 1;
    // Qualification using 3 samples
    GpioCtrlRegs.GPBQSEL1.bit.GPIO44 = 0x01;

    GpioCtrlRegs.GPBMUX1.bit.GPIO45 = 0x00;
    GpioCtrlRegs.GPBDIR.bit.GPIO45 = 0;
    GpioCtrlRegs.GPBPUD.bit.GPIO45 = 1;
    GpioCtrlRegs.GPBQSEL1.bit.GPIO45 = 0x01;

    GpioCtrlRegs.GPBMUX1.bit.GPIO46 = 0x00;
    GpioCtrlRegs.GPBDIR.bit.GPIO46 = 0;
    GpioCtrlRegs.GPBPUD.bit.GPIO46 = 1;
    GpioCtrlRegs.GPBQSEL1.bit.GPIO46 = 0x01;

    GpioCtrlRegs.GPBMUX1.bit.GPIO47 = 0x00;
    GpioCtrlRegs.GPBDIR.bit.GPIO47 = 0;
    GpioCtrlRegs.GPBPUD.bit.GPIO47 = 1;
    GpioCtrlRegs.GPBQSEL1.bit.GPIO47 = 0x01;
    EDIS;
}

bool BFL_Button_IsPressed(BFL_Button_t btn)
{
    // TODO: 认为按键按下时，对应的GPIO口电平为高
    switch (btn)
    {
    case MODE_BTN:
        return XIN6_IsSet();
    }
    return false;
}
