/**
 * @file BFL_DebugPin.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 * @last modified 2024-05-16
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "BFL_DebugPin.h"
#include "CPU_Define.h"
void BFL_DebugPin_Init()
{
    BFL_DebugPin_Reset(DEBUG_PIN_1);
    BFL_DebugPin_Reset(DEBUG_PIN_2);
    EALLOW;
    // VCB控制信号外部电路上无上下拉
    //  General purpose I/O
    GpioCtrlRegs.GPCMUX1.bit.GPIO77 = 0x00;
    // Configures the GPIO pin as an output
    GpioCtrlRegs.GPCDIR.bit.GPIO77 = 1;
    // Enable the internal pullup on the specified pin.
    GpioCtrlRegs.GPCPUD.bit.GPIO77 = 0;


    GpioCtrlRegs.GPCMUX1.bit.GPIO76 = 0x00;
    GpioCtrlRegs.GPCDIR.bit.GPIO76 = 1;
    GpioCtrlRegs.GPCPUD.bit.GPIO76 = 0;
    EDIS;
}
void BFL_DebugPin_Set(BFL_DebugPin_t pin)
{
    switch (pin)
    {
    case DEBUG_PIN_1:
        GpioDataRegs.GPCSET.bit.GPIO77 = 1;
        break;
    case DEBUG_PIN_2:
        GpioDataRegs.GPCSET.bit.GPIO76 = 1;
        break;
    default:
        break;
    }
}

void BFL_DebugPin_Reset(BFL_DebugPin_t pin)
{
    switch (pin)
    {
    case DEBUG_PIN_1:
        GpioDataRegs.GPCCLEAR.bit.GPIO77 = 1;
        break;
    case DEBUG_PIN_2:
        GpioDataRegs.GPCCLEAR.bit.GPIO76 = 1;
        break;
    default:
        break;
    }
}

void BFL_DebugPin_Toggle(BFL_DebugPin_t pin)
{
    switch (pin)
    {
    case DEBUG_PIN_1:
        GpioDataRegs.GPCTOGGLE.bit.GPIO77 = 1;
        break;
    case DEBUG_PIN_2:
        GpioDataRegs.GPCTOGGLE.bit.GPIO76 = 1;
        break;
    default:
        break;
    }
}
