/**
 * @file BFL_SCR.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-14
 * @last modified 2024-05-14
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "BFL_SCR.h"
#include "CPU_Define.h"
#include "DSP2833x_Device.h"
#include <stddef.h>

//
// Typedefs
//
typedef struct
{
    volatile struct EPWM_REGS *EPwmRegHandle;
    volatile Uint16 INT_CNT;
    volatile Uint16 PLUSE_NUM;
    Uint16 T_INT; // 中断周期，单位ms
    Uint16 busy;
} EPWM_INFO;

void InitEPwm1Example(void);
void InitEPwm2Example(void);
void InitEPwm3Example(void);
void InitEPwm4Example(void);
__interrupt void epwm1_isr(void);
__interrupt void epwm2_isr(void);
__interrupt void epwm3_isr(void);
__interrupt void epwm4_isr(void);

//
// Globals
//
EPWM_INFO epwm1_info;
EPWM_INFO epwm2_info;
EPWM_INFO epwm3_info;
EPWM_INFO epwm4_info;

#define SCR_LEVEL_MODE 0
#define SCR_PWM_MODE 1
#define SRC_USING_PWM SCR_LEVEL_MODE
//
// InitEPwm1Gpio - This function initializes GPIO pins to function as ePWM1
//
void mInitEPwm1Gpio(void)
{
#if SRC_USING_PWM == SCR_LEVEL_MODE
    EALLOW;
    //
    // Enable internal pull-up for the selected pins
    // Pull-ups can be enabled or disabled by the user.
    // This will enable the pullups for the specified pins.
    // Comment out other unwanted lines.
    //
    GpioCtrlRegs.GPAPUD.bit.GPIO0 = 0; // Enable pull-up on GPIO0 (EPWM1A)
    GpioCtrlRegs.GPAPUD.bit.GPIO1 = 0; // Enable pull-up on GPIO1 (EPWM1B)

    //
    // Configure General purpose I/O (default)
    //
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;

    // Configures the GPIO pin as an output
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;
#else
    InitEPwm1Gpio();
#endif
}

//
// InitEPwm2Gpio - This function initializes GPIO pins to function as ePWM2
//
void mInitEPwm2Gpio(void)
{
#if SRC_USING_PWM == SCR_LEVEL_MODE
    EALLOW;

    //
    // Enable internal pull-up for the selected pins
    // Pull-ups can be enabled or disabled by the user.
    // This will enable the pullups for the specified pins.
    // Comment out other unwanted lines.
    //
    GpioCtrlRegs.GPAPUD.bit.GPIO2 = 0; // Enable pull-up on GPIO2 (EPWM2A)
    GpioCtrlRegs.GPAPUD.bit.GPIO3 = 0; // Enable pull-up on GPIO3 (EPWM3B)

    //
    // Configure General purpose I/O (default)
    //
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 0;

    // Configures the GPIO pin as an output
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 1;
    EDIS;

#else
    InitEPwm2Gpio();
#endif
}

//
// InitEPwm3Gpio - This function initializes GPIO pins to function as ePWM3
//
void mInitEPwm3Gpio(void)
{
#if SRC_USING_PWM == SCR_LEVEL_MODE
    EALLOW;

    //
    // Enable internal pull-up for the selected pins
    // Pull-ups can be enabled or disabled by the user.
    // This will enable the pullups for the specified pins.
    // Comment out other unwanted lines.
    //
    GpioCtrlRegs.GPAPUD.bit.GPIO4 = 0; // Enable pull-up on GPIO4 (EPWM3A)
    GpioCtrlRegs.GPAPUD.bit.GPIO5 = 0; // Enable pull-up on GPIO5 (EPWM3B)

    //
    // Configure General purpose I/O (default)
    //
    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 0;

    // Configures the GPIO pin as an output
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO5 = 1;
    EDIS;
#else
    InitEPwm3Gpio();
#endif
}

//
// Defines that keep track of which way the compare value is moving
//
#define EPWM_CMP_UP 1
#define EPWM_CMP_DOWN 0

#define SCRR1A_IsSet() (GpioDataRegs.GPADAT.bit.GPIO8)
#define SCRR1B_IsSet() (GpioDataRegs.GPADAT.bit.GPIO9)
#define SCRR2A_IsSet() (GpioDataRegs.GPADAT.bit.GPIO10)
#define SCRR2B_IsSet() (GpioDataRegs.GPADAT.bit.GPIO11)
#define SCRR3A_IsSet() (GpioDataRegs.GPADAT.bit.GPIO12)
#define SCRR3B_IsSet() (GpioDataRegs.GPADAT.bit.GPIO13)
#define SCRA_ALL_Read() ((GpioDataRegs.GPADAT.all & 0x00003F00) >> 8)

// GPIO0,1,2,3,4,5
#define SCRT_ALL_Set() (GpioDataRegs.GPASET.all |= 0x0000003F)
#define SCRT_ALL_Clear() (GpioDataRegs.GPACLEAR.all |= 0x0000003F)
#define SCRT_ALL_Toggle() (GpioDataRegs.GPATOGGLE.all |= 0x0000003F)
// GPIO0,1
#define SCRT1_Set() (GpioDataRegs.GPASET.all |= 0x00000003)
#define SCRT1_Clear() (GpioDataRegs.GPACLEAR.all |= 0x00000003)
#define SCRT1_Toggle() (GpioDataRegs.GPATOGGLE.all |= 0x00000003)
// GPIO2,3
#define SCRT2_Set() (GpioDataRegs.GPASET.all |= 0x0000000C)
#define SCRT2_Clear() (GpioDataRegs.GPACLEAR.all |= 0x0000000C)
#define SCRT2_Toggle() (GpioDataRegs.GPATOGGLE.all |= 0x0000000C)
// GPIO4,5
#define SCRT3_Set() (GpioDataRegs.GPASET.all |= 0x00000030)
#define SCRT3_Clear() (GpioDataRegs.GPACLEAR.all |= 0x00000030)
#define SCRT3_Toggle() (GpioDataRegs.GPATOGGLE.all |= 0x00000030)

/**
 * @brief SCR输入输出通道初始化。
 *
 */
void BFL_SCR_Init()
{
    EALLOW;
    // Specifies the sampling period for pins GPIO8 to GPIO15 with Sampling Period
    // = TSYSCLKOUT
    GpioCtrlRegs.GPACTRL.bit.QUALPRD1 = 0x00;

    // IO输入口电路上被下拉3kΩ
    // General purpose I/O
    GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 0x00;
    // Configures the GPIO pin as an input
    GpioCtrlRegs.GPADIR.bit.GPIO8 = 0;
    // Disable the internal pullup on the specified pin.
    GpioCtrlRegs.GPAPUD.bit.GPIO8 = 1;
    // // Enable the internal pullup on the specified pin.
    // GpioCtrlRegs.GPAPUD.bit.GPIO8 = 0;
    // Qualification using 3 samples
    GpioCtrlRegs.GPAQSEL1.bit.GPIO8 = 0x01;

    GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0x00;
    GpioCtrlRegs.GPADIR.bit.GPIO9 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO9 = 1;
    GpioCtrlRegs.GPAQSEL1.bit.GPIO9 = 0x01;

    GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 0x00;
    GpioCtrlRegs.GPADIR.bit.GPIO10 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO10 = 1;
    GpioCtrlRegs.GPAQSEL1.bit.GPIO10 = 0x01;

    GpioCtrlRegs.GPAMUX1.bit.GPIO11 = 0x00;
    GpioCtrlRegs.GPADIR.bit.GPIO11 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO11 = 1;
    GpioCtrlRegs.GPAQSEL1.bit.GPIO11 = 0x01;

    GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 0x00;
    GpioCtrlRegs.GPADIR.bit.GPIO12 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO12 = 1;
    GpioCtrlRegs.GPAQSEL1.bit.GPIO12 = 0x01;

    GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 0x00;
    GpioCtrlRegs.GPADIR.bit.GPIO13 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO13 = 1;
    GpioCtrlRegs.GPAQSEL1.bit.GPIO13 = 0x01;

    EDIS;

    //
    // For this case just init GPIO pins for ePWM1, ePWM2, ePWM3
    // These functions are in the DSP2833x_EPwm.c file
    //
    mInitEPwm1Gpio();
    mInitEPwm2Gpio();
    mInitEPwm3Gpio();

    //
    // Interrupts that are used in this example are re-mapped to
    // ISR functions found within this file.
    //
    EALLOW; // This is needed to write to EALLOW protected registers
    PieVectTable.EPWM1_INT = &epwm1_isr;
    PieVectTable.EPWM2_INT = &epwm2_isr;
    PieVectTable.EPWM3_INT = &epwm3_isr;
    PieVectTable.EPWM4_INT = &epwm4_isr;
    EDIS; // This is needed to disable write to EALLOW protected registers

    //
    // For this example, only initialize the ePWM
    //
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    InitEPwm1Example();
    InitEPwm2Example();
    InitEPwm3Example();

    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    //
    // Step 5. User specific code, enable interrupts
    //

    //
    // Enable CPU INT3 which is connected to EPWM1-3 INT
    //
    IER |= M_INT3;

    //
    // Enable EPWM INTn in the PIE: Group 3 interrupt 1-3
    //
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;
    PieCtrlRegs.PIEIER3.bit.INTx2 = 1;
    PieCtrlRegs.PIEIER3.bit.INTx3 = 1;
}

/**
 * @brief 是否由光信号输入。
 * 有光信号输入时IO电平为低电平，没有光信号输入时默认拉高。
 *
 * @param scrr 输入通道。
 * @return > 0 有光信号输入进来。为SCRR_ALL时有效位mask为0x3F。
 * bit0对应SCRR3B，bit1对应SCRR3A，bit2对应SCRR2B，
 * bit3对应SCRR2A，bit4对应SCRR1B，bit5对应SCRR1A。
 * @return 0 没有光信号输入进来。
 */
uint32_t BFL_SCRR_Have_Signal(BFL_SCRR_t scrr)
{
    switch (scrr)
    {
    case SCRR1A:
        return SCRR1A_IsSet() == 0;
    case SCRR1B:
        return SCRR1B_IsSet() == 0;
    case SCRR2A:
        return SCRR2A_IsSet() == 0;
    case SCRR2B:
        return SCRR2B_IsSet() == 0;
    case SCRR3A:
        return SCRR3A_IsSet() == 0;
    case SCRR3B:
        return SCRR3B_IsSet() == 0;
    case SCRR_ALL:
    {
        uint32_t _uiRet = 0;
        _uiRet = SCRA_ALL_Read();
        _uiRet = (~_uiRet) & BFL_SCR_SIGNAL_MASK;
        return _uiRet;
    }
    }
    return false;
}

//
// Defines that configure the period for each timer
//
#define EPWM1_PWM_PERIOD 5 // 5ms Period
#define EPWM1_TIMER_TBPRD \
    (9375000ULL / (1000 / EPWM1_PWM_PERIOD) - 1) // 5ms Period，max 6
#define EPWM1_TIMER_CMPA_MIN 50
#define EPWM123_ENABLE_SYNC 0
#define EPWM456_ENABLE_SYNC 0
//
// InitEPwm1Example -
//

void EPwmx_ActionInit(volatile struct EPWM_REGS *EPwmxRegsHandle)
{
    // //
    // // Set actions
    // //
    // EPwmxRegsHandle->AQCTLA.bit.ZRO = AQ_CLEAR; // Set PWM1A on Zero
    // EPwmxRegsHandle->AQCTLA.bit.CAU = AQ_SET;   // Clear PWM1A on event A, up count
    // EPwmxRegsHandle->AQCTLA.bit.CBU = AQ_CLEAR; // Clear PWM1A on event B, up count

    // EPwmxRegsHandle->AQCTLB.bit.ZRO = AQ_CLEAR; // Set PWM1B on Zero
    // EPwmxRegsHandle->AQCTLB.bit.CAU = AQ_SET;   // Clear PWM1B on event A, up count
    // EPwmxRegsHandle->AQCTLB.bit.CBU = AQ_CLEAR; // Clear PWM1B on event B, up count
    // Set actions
    //
    EPwmxRegsHandle->AQCTLA.bit.ZRO = AQ_SET;   // Set PWM1A on Zero
    EPwmxRegsHandle->AQCTLA.bit.CAU = AQ_CLEAR; // Clear PWM1A on event A, up count
    EPwmxRegsHandle->AQCTLA.bit.CBU = AQ_SET;   // Clear PWM1A on event B, up count

    EPwmxRegsHandle->AQCTLB.bit.ZRO = AQ_SET;   // Set PWM1B on Zero
    EPwmxRegsHandle->AQCTLB.bit.CAU = AQ_CLEAR; // Clear PWM1B on event A, up count
    EPwmxRegsHandle->AQCTLB.bit.CBU = AQ_SET;   // Clear PWM1B on event B, up count
}

void InitEPwm1Example()
{
    volatile struct EPWM_REGS *EPwmxRegsHandle = &EPwm1Regs;
    EPWM_INFO *pEpwmx_info = &epwm1_info;
    //
    // Setup TBCLK
    //
    EPwmxRegsHandle->TBCTL.bit.CTRMODE = TB_FREEZE; // Count up
    // PWM period = (TBPRD + 1 ) × TTBCLK Up-Count mode
    EPwmxRegsHandle->TBPRD = EPWM1_TIMER_TBPRD; // Set timer period

#if EPWM123_ENABLE_SYNC == 1
    EPwmxRegsHandle->TBCTL.bit.PHSEN =
        TB_DISABLE;                                    // Disable phase loading Master module
    EPwmxRegsHandle->TBPHS.half.TBPHS = 0x0000;        // Phase is 0
    EPwmxRegsHandle->TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // Sync down-stream module
    EPwmxRegsHandle->TBCTL.bit.PHSDIR = TB_UP;         // Phase Direction Down
    EPwmxRegsHandle->TBCTL.bit.PRDLD = TB_SHADOW;
#else
    EPwmxRegsHandle->TBCTL.bit.PHSEN =
        TB_DISABLE;                             // Disable phase loading Master module
    EPwmxRegsHandle->TBPHS.half.TBPHS = 0x0000; // Phase is 0
#endif // EPWM123_ENABLE_SYNC

    EPwmxRegsHandle->TBCTR = 0x0000; // Clear counter
    // TBCLK=SYSCLKOUT/(HSPCLKDIV*CLKDIV):150/(4*4)
    EPwmxRegsHandle->TBCTL.bit.HSPCLKDIV = TB_DIV4; // Clock ratio to SYSCLKOUT
    EPwmxRegsHandle->TBCTL.bit.CLKDIV = TB_DIV4;

    //
    // Setup shadow register load on ZERO
    //
    EPwmxRegsHandle->CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwmxRegsHandle->CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwmxRegsHandle->CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwmxRegsHandle->CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    //
    // Set Compare values
    //
    EPwmxRegsHandle->CMPA.half.CMPA =
        EPWM1_TIMER_TBPRD >> 1;                     // Set compare A value
    EPwmxRegsHandle->CMPB = EPWM1_TIMER_TBPRD >> 1; // Set Compare B value

    EPwmx_ActionInit(EPwmxRegsHandle);

    //
    // Interrupt where we will change the Compare Values
    //
    EPwmxRegsHandle->ETSEL.bit.INTSEL = ET_CTR_PRD; // Select INT on Zero event
    EPwmxRegsHandle->ETPS.bit.INTPRD = ET_1ST;      // Generate INT on 3rd event
    //   EPwm1RegsHandle->ETSEL.bit.INTEN = 1;           // Enable INT
    EPwmxRegsHandle->ETSEL.bit.INTEN = 0; // Disable INT

    pEpwmx_info->INT_CNT = 0;
    pEpwmx_info->PLUSE_NUM = 0;
    pEpwmx_info->T_INT = EPWM1_PWM_PERIOD;
    pEpwmx_info->busy = 0;
    pEpwmx_info->EPwmRegHandle = EPwmxRegsHandle;
}

//
// InitEPwm2Example -
//
void InitEPwm2Example()
{
    volatile struct EPWM_REGS *EPwmxRegsHandle = &EPwm2Regs;
    EPWM_INFO *pEpwmx_info = &epwm2_info;
    //
    // Setup TBCLK
    //
    EPwmxRegsHandle->TBCTL.bit.CTRMODE = TB_FREEZE; // Count up
    // PWM period = (TBPRD + 1 ) × TTBCLK Up-Count mode
    EPwmxRegsHandle->TBPRD = EPWM1_TIMER_TBPRD; // Set timer period

#if EPWM123_ENABLE_SYNC == 1
    EPwmxRegsHandle->TBCTL.bit.SYNCOSEL = TB_SYNC_IN;            // Sync down-stream module
    EPwmxRegsHandle->TBCTL.bit.PHSDIR = TB_UP;                   // Phase Direction Down
    EPwmxRegsHandle->TBCTL.bit.PHSEN = TB_ENABLE;                // Disable phase loading
    EPwmxRegsHandle->TBPHS.half.TBPHS = (EPWM1_TIMER_TBPRD / 3); // Phase is 0
    EPwmxRegsHandle->TBCTL.bit.PRDLD = TB_SHADOW;
#else
    EPwmxRegsHandle->TBCTL.bit.PHSEN =
        TB_DISABLE;                             // Disable phase loading Master module
    EPwmxRegsHandle->TBPHS.half.TBPHS = 0x0000; // Phase is 0
#endif // EPWM123_ENABLE_SYNC

    EPwmxRegsHandle->TBCTR = 0x0000; // Clear counter
    // TBCLK=SYSCLKOUT/(HSPCLKDIV*CLKDIV):150/(4*4)
    EPwmxRegsHandle->TBCTL.bit.HSPCLKDIV = TB_DIV4; // Clock ratio to SYSCLKOUT
    EPwmxRegsHandle->TBCTL.bit.CLKDIV = TB_DIV4;

    //
    // Setup shadow register load on ZERO
    //
    EPwmxRegsHandle->CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwmxRegsHandle->CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwmxRegsHandle->CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwmxRegsHandle->CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    //
    // Set Compare values
    //
    EPwmxRegsHandle->CMPA.half.CMPA =
        EPWM1_TIMER_TBPRD >> 1;                     // Set compare A value
    EPwmxRegsHandle->CMPB = EPWM1_TIMER_TBPRD >> 1; // Set Compare B value

    EPwmx_ActionInit(EPwmxRegsHandle);

    //
    // Interrupt where we will change the Compare Values
    //
    EPwmxRegsHandle->ETSEL.bit.INTSEL = ET_CTR_PRD; // Select INT on Zero event
    EPwmxRegsHandle->ETPS.bit.INTPRD = ET_1ST;      // Generate INT on 3rd event
    //   EPwm1RegsHandle->ETSEL.bit.INTEN = 1;           // Enable INT
    EPwmxRegsHandle->ETSEL.bit.INTEN = 0; // Disable INT

    pEpwmx_info->INT_CNT = 0;
    pEpwmx_info->PLUSE_NUM = 0;
    pEpwmx_info->T_INT = EPWM1_PWM_PERIOD;
    pEpwmx_info->busy = 0;
    pEpwmx_info->EPwmRegHandle = EPwmxRegsHandle;
}

//
// InitEPwm3Example -
//
void InitEPwm3Example(void)
{
    volatile struct EPWM_REGS *EPwmxRegsHandle = &EPwm3Regs;
    EPWM_INFO *pEpwmx_info = &epwm3_info;
    //
    // Setup TBCLK
    //
    EPwmxRegsHandle->TBCTL.bit.CTRMODE = TB_FREEZE; // Count up
    // PWM period = (TBPRD + 1 ) × TTBCLK Up-Count mode
    EPwmxRegsHandle->TBPRD = EPWM1_TIMER_TBPRD; // Set timer period

#if EPWM123_ENABLE_SYNC == 1
    EPwmxRegsHandle->TBCTL.bit.SYNCOSEL = TB_SYNC_IN;                // Sync down-stream module
    EPwmxRegsHandle->TBCTL.bit.PHSDIR = TB_UP;                       // Phase Direction Down
    EPwmxRegsHandle->TBCTL.bit.PHSEN = TB_ENABLE;                    // Disable phase loading
    EPwmxRegsHandle->TBPHS.half.TBPHS = (EPWM1_TIMER_TBPRD * 2 / 3); // Phase is 0
    EPwmxRegsHandle->TBCTL.bit.PRDLD = TB_SHADOW;
#else
    EPwmxRegsHandle->TBCTL.bit.PHSEN =
        TB_DISABLE;                             // Disable phase loading Master module
    EPwmxRegsHandle->TBPHS.half.TBPHS = 0x0000; // Phase is 0
#endif // EPWM123_ENABLE_SYNC

    EPwmxRegsHandle->TBCTR = 0x0000; // Clear counter
    // TBCLK=SYSCLKOUT/(HSPCLKDIV*CLKDIV):150/(4*4)
    EPwmxRegsHandle->TBCTL.bit.HSPCLKDIV = TB_DIV4; // Clock ratio to SYSCLKOUT
    EPwmxRegsHandle->TBCTL.bit.CLKDIV = TB_DIV4;

    //
    // Setup shadow register load on ZERO
    //
    EPwmxRegsHandle->CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwmxRegsHandle->CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwmxRegsHandle->CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwmxRegsHandle->CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    //
    // Set Compare values
    //
    EPwmxRegsHandle->CMPA.half.CMPA =
        EPWM1_TIMER_TBPRD >> 1;                     // Set compare A value
    EPwmxRegsHandle->CMPB = EPWM1_TIMER_TBPRD >> 1; // Set Compare B value

    EPwmx_ActionInit(EPwmxRegsHandle);

    //
    // Interrupt where we will change the Compare Values
    //
    EPwmxRegsHandle->ETSEL.bit.INTSEL = ET_CTR_PRD; // Select INT on Zero event
    EPwmxRegsHandle->ETPS.bit.INTPRD = ET_1ST;      // Generate INT on 3rd event
    //   EPwm1RegsHandle->ETSEL.bit.INTEN = 1;           // Enable INT
    EPwmxRegsHandle->ETSEL.bit.INTEN = 0; // Disable INT

    pEpwmx_info->INT_CNT = 0;
    pEpwmx_info->PLUSE_NUM = 0;
    pEpwmx_info->T_INT = EPWM1_PWM_PERIOD;
    pEpwmx_info->busy = 0;
    pEpwmx_info->EPwmRegHandle = EPwmxRegsHandle;
}

//
// InitEPwm4Example -
//
void InitEPwm4Example(void)
{
    volatile struct EPWM_REGS *EPwmxRegsHandle = &EPwm4Regs;
    EPWM_INFO *pEpwmx_info = &epwm4_info;
    //
    // Setup TBCLK
    //
    EPwmxRegsHandle->TBCTL.bit.CTRMODE = TB_FREEZE; // Count up
    // PWM period = (TBPRD + 1 ) × TTBCLK Up-Count mode
    EPwmxRegsHandle->TBPRD = EPWM1_TIMER_TBPRD; // Set timer period

#if EPWM456_ENABLE_SYNC == 1
    EPwmxRegsHandle->TBCTL.bit.SYNCOSEL = TB_SYNC_IN;                // Sync down-stream module
    EPwmxRegsHandle->TBCTL.bit.PHSDIR = TB_UP;                       // Phase Direction Down
    EPwmxRegsHandle->TBCTL.bit.PHSEN = TB_ENABLE;                    // Disable phase loading
    EPwmxRegsHandle->TBPHS.half.TBPHS = (EPWM1_TIMER_TBPRD * 2 / 3); // Phase is 0
    EPwmxRegsHandle->TBCTL.bit.PRDLD = TB_SHADOW;
#else
    EPwmxRegsHandle->TBCTL.bit.PHSEN =
        TB_DISABLE;                             // Disable phase loading Master module
    EPwmxRegsHandle->TBPHS.half.TBPHS = 0x0000; // Phase is 0
#endif // EPWM456_ENABLE_SYNC

    EPwmxRegsHandle->TBCTR = 0x0000; // Clear counter
    // TBCLK=SYSCLKOUT/(HSPCLKDIV*CLKDIV):150/(4*4)
    EPwmxRegsHandle->TBCTL.bit.HSPCLKDIV = TB_DIV4; // Clock ratio to SYSCLKOUT
    EPwmxRegsHandle->TBCTL.bit.CLKDIV = TB_DIV4;

    //
    // Setup shadow register load on ZERO
    //
    EPwmxRegsHandle->CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwmxRegsHandle->CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwmxRegsHandle->CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwmxRegsHandle->CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    //
    // Set Compare values
    //
    EPwmxRegsHandle->CMPA.half.CMPA =
        EPWM1_TIMER_TBPRD >> 1;                     // Set compare A value
    EPwmxRegsHandle->CMPB = EPWM1_TIMER_TBPRD >> 1; // Set Compare B value

    EPwmx_ActionInit(EPwmxRegsHandle);

    //
    // Interrupt where we will change the Compare Values
    //
    EPwmxRegsHandle->ETSEL.bit.INTSEL = ET_CTR_PRD; // Select INT on Zero event
    EPwmxRegsHandle->ETPS.bit.INTPRD = ET_1ST;      // Generate INT on 3rd event
    //   EPwm1RegsHandle->ETSEL.bit.INTEN = 1;           // Enable INT
    EPwmxRegsHandle->ETSEL.bit.INTEN = 0; // Disable INT

    pEpwmx_info->INT_CNT = 0;
    pEpwmx_info->PLUSE_NUM = 0;
    pEpwmx_info->T_INT = EPWM1_PWM_PERIOD;
    pEpwmx_info->busy = 0;
    pEpwmx_info->EPwmRegHandle = EPwmxRegsHandle;
}

//
// epwm1_isr -
//
__interrupt void epwm1_isr(void)
{

    EPWM_INFO *pEpwmx_info = &epwm1_info;
    volatile struct EPWM_REGS *EPwmRegHandle = pEpwmx_info->EPwmRegHandle;

    //
    // Clear INT flag for this timer
    //
    EPwmRegHandle->ETCLR.bit.INT = 1;

    if (pEpwmx_info->INT_CNT >= pEpwmx_info->PLUSE_NUM)
    {
        EPwmRegHandle->TBCTR = 0x0000;                // Clear counter
        EPwmRegHandle->TBCTL.bit.CTRMODE = TB_FREEZE; // freeze count
        EPwmRegHandle->ETSEL.bit.INTEN = 0;           // Disable INT
        pEpwmx_info->busy = 0;

#if SRC_USING_PWM == SCR_LEVEL_MODE
        SCRT1_Set();
#endif
    }
    pEpwmx_info->INT_CNT++;

    //
    // Acknowledge this interrupt to receive more interrupts from group 3
    //
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP3;
}

//
// epwm2_isr -
//
__interrupt void epwm2_isr(void)
{
    EPWM_INFO *pEpwmx_info = &epwm2_info;

    volatile struct EPWM_REGS *EPwmRegHandle = pEpwmx_info->EPwmRegHandle;
    //
    // Clear INT flag for this timer
    //
    EPwmRegHandle->ETCLR.bit.INT = 1;

    if (pEpwmx_info->INT_CNT >= pEpwmx_info->PLUSE_NUM)
    {
        EPwmRegHandle->TBCTR = 0x0000;                // Clear counter
        EPwmRegHandle->TBCTL.bit.CTRMODE = TB_FREEZE; // freeze count
        EPwmRegHandle->ETSEL.bit.INTEN = 0;           // Disable INT
        pEpwmx_info->busy = 0;
#if SRC_USING_PWM == SCR_LEVEL_MODE
        SCRT2_Set();
#endif
    }
    pEpwmx_info->INT_CNT++;

    //
    // Acknowledge this interrupt to receive more interrupts from group 3
    //
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP3;
}

//
// epwm3_isr -
//
__interrupt void epwm3_isr(void)
{
    EPWM_INFO *pEpwmx_info = &epwm3_info;

    volatile struct EPWM_REGS *EPwmRegHandle = pEpwmx_info->EPwmRegHandle;
    //
    // Clear INT flag for this timer
    //
    EPwmRegHandle->ETCLR.bit.INT = 1;

    if (pEpwmx_info->INT_CNT >= pEpwmx_info->PLUSE_NUM)
    {
        EPwmRegHandle->TBCTR = 0x0000;                // Clear counter
        EPwmRegHandle->TBCTL.bit.CTRMODE = TB_FREEZE; // freeze count
        EPwmRegHandle->ETSEL.bit.INTEN = 0;           // Disable INT
        pEpwmx_info->busy = 0;
#if SRC_USING_PWM == SCR_LEVEL_MODE
        SCRT3_Set();
#endif
    }
    pEpwmx_info->INT_CNT++;

    //
    // Acknowledge this interrupt to receive more interrupts from group 3
    //
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP3;
}

//
// epwm4_isr -
//
__interrupt void epwm4_isr(void)
{
    EPWM_INFO *pEpwmx_info = &epwm4_info;

    volatile struct EPWM_REGS *EPwmRegHandle = pEpwmx_info->EPwmRegHandle;
    //
    // Clear INT flag for this timer
    //
    EPwmRegHandle->ETCLR.bit.INT = 1;

    if (pEpwmx_info->INT_CNT >= pEpwmx_info->PLUSE_NUM)
    {
        EPwmRegHandle->TBCTR = 0x0000;                // Clear counter
        EPwmRegHandle->TBCTL.bit.CTRMODE = TB_FREEZE; // freeze count
        EPwmRegHandle->ETSEL.bit.INTEN = 0;           // Disable INT
        pEpwmx_info->busy = 0;
#if SRC_USING_PWM == SCR_LEVEL_MODE
        SCRT_ALL_Set();
#endif
    }
    pEpwmx_info->INT_CNT++;

    //
    // Acknowledge this interrupt to receive more interrupts from group 3
    //
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP3;
}

static void BFL_SCRT_Pluse_Transmit_Config(EPWM_INFO *pEpwmx_info,
                                           uint16_t _uiPluseNum,
                                           uint16_t _uiPluseWidth)
{
    volatile struct EPWM_REGS *EPwmRegHandle = pEpwmx_info->EPwmRegHandle;
    _disable_interrupts();
    pEpwmx_info->INT_CNT = 0;
    pEpwmx_info->PLUSE_NUM = _uiPluseNum - 1;
    pEpwmx_info->busy = 1;
    _enable_interrupts();

    // 最大值为5000 + 1
    // us。为5000us时，输出的脉冲宽度为5000us，会有一个非常小的脉冲，当为5000 +
    // 1时，输出的脉冲宽度为5000us。
    //(_uiPluseWidth / (EPWM1_PWM_PERIOD * 1000)) * EPWM1_TIMER_TBPRD;
    EPwmRegHandle->CMPA.half.CMPA = EPWM1_TIMER_CMPA_MIN;
    EPwmRegHandle->CMPB = ((uint32_t)_uiPluseWidth * EPWM1_TIMER_TBPRD /
                           (EPWM1_PWM_PERIOD * 1000ULL));
    EPwmRegHandle->TBCTR = 0x0000;                  // Clear counter
    EPwmRegHandle->TBCTL.bit.CTRMODE = TB_COUNT_UP; // Up count mode
    EPwmRegHandle->ETSEL.bit.INTEN = 1;             // Enable INT
}

/**
 * @brief 输出脉冲，如果当前通道有脉冲正在输出，则会被覆盖。
 *
 * @param scrt 输出通道。
 * @param _uiPluseNum
 * @param _uiPluseWidth 单位us，范围[4,5000]
 */
void BFL_SCRT_Pluse_Transmit(BFL_SCRT_t scrt, uint16_t _uiPluseNum,
                             uint16_t _uiPluseWidth)
{
    if (_uiPluseWidth == 0 ||
        _uiPluseNum == 0)
    {
        return;
    }

    if (_uiPluseWidth > (EPWM1_PWM_PERIOD * 1000ULL))
    {
        _uiPluseWidth = (EPWM1_PWM_PERIOD * 1000ULL);
    }

    if (_uiPluseWidth < EPWM1_TIMER_CMPA_MIN)
    {
        _uiPluseWidth = EPWM1_TIMER_CMPA_MIN;
    }
#if SRC_USING_PWM == SCR_PWM_MODE
    switch (scrt)
    {
    case SCRTA:
        BFL_SCRT_Pluse_Transmit_Config(&epwm1_info, _uiPluseNum, _uiPluseWidth);
        break;
    case SCRTB:
        BFL_SCRT_Pluse_Transmit_Config(&epwm2_info, _uiPluseNum, _uiPluseWidth);
        break;
    case SCRTC:
        BFL_SCRT_Pluse_Transmit_Config(&epwm3_info, _uiPluseNum, _uiPluseWidth);
        break;
    case SCRT_ALL:
    {
        EPWM_INFO *pEpwmx_info = NULL;
        volatile struct EPWM_REGS *EPwmRegHandle = NULL;
        _disable_interrupts();
        pEpwmx_info = &epwm1_info;
        EPwmRegHandle = pEpwmx_info->EPwmRegHandle;
        pEpwmx_info->INT_CNT = 0;
        pEpwmx_info->PLUSE_NUM = _uiPluseNum - 1;
        pEpwmx_info->busy = 1;
        _enable_interrupts();
        // 最大值为5000 + 1
        // us。为5000us时，输出的脉冲宽度为5000us，会有一个非常小的脉冲，当为5000 +
        // 1时，输出的脉冲宽度为5000us。
        //(_uiPluseWidth / (EPWM1_PWM_PERIOD * 1000)) * EPWM1_TIMER_TBPRD;
        EPwmRegHandle->CMPA.half.CMPA = EPWM1_TIMER_CMPA_MIN;
        EPwmRegHandle->CMPB = ((uint32_t)_uiPluseWidth * EPWM1_TIMER_TBPRD /
                               (EPWM1_PWM_PERIOD * 1000ULL));
        EPwmRegHandle->TBCTR = 0x0000; // Clear counter

        _disable_interrupts();
        pEpwmx_info = &epwm2_info;
        EPwmRegHandle = pEpwmx_info->EPwmRegHandle;
        pEpwmx_info->INT_CNT = 0;
        pEpwmx_info->PLUSE_NUM = _uiPluseNum - 1;
        pEpwmx_info->busy = 1;
        _enable_interrupts();
        // 最大值为5000 + 1
        // us。为5000us时，输出的脉冲宽度为5000us，会有一个非常小的脉冲，当为5000 +
        // 1时，输出的脉冲宽度为5000us。
        //(_uiPluseWidth / (EPWM1_PWM_PERIOD * 1000)) * EPWM1_TIMER_TBPRD;
        EPwmRegHandle->CMPA.half.CMPA = EPWM1_TIMER_CMPA_MIN;
        EPwmRegHandle->CMPB = ((uint32_t)_uiPluseWidth * EPWM1_TIMER_TBPRD /
                               (EPWM1_PWM_PERIOD * 1000ULL));
        EPwmRegHandle->TBCTR = 0x0000; // Clear counter

        _disable_interrupts();
        pEpwmx_info = &epwm3_info;
        EPwmRegHandle = pEpwmx_info->EPwmRegHandle;
        pEpwmx_info->INT_CNT = 0;
        pEpwmx_info->PLUSE_NUM = _uiPluseNum - 1;
        pEpwmx_info->busy = 1;
        _enable_interrupts();
        // 最大值为5000 + 1
        // us。为5000us时，输出的脉冲宽度为5000us，会有一个非常小的脉冲，当为5000 +
        // 1时，输出的脉冲宽度为5000us。
        //(_uiPluseWidth / (EPWM1_PWM_PERIOD * 1000)) * EPWM1_TIMER_TBPRD;
        EPwmRegHandle->CMPA.half.CMPA = EPWM1_TIMER_CMPA_MIN;
        EPwmRegHandle->CMPB = ((uint32_t)_uiPluseWidth * EPWM1_TIMER_TBPRD /
                               (EPWM1_PWM_PERIOD * 1000ULL));
        EPwmRegHandle->TBCTR = 0x0000; // Clear counter

        EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; // Up count mode
        EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; // Up count mode
        EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; // Up count mode
        EPwm2Regs.ETSEL.bit.INTEN = 1;             // Enable INT
        EPwm1Regs.ETSEL.bit.INTEN = 1;             // Enable INT
        EPwm3Regs.ETSEL.bit.INTEN = 1;             // Enable INT
        break;
    }
    default:
        break;
    }
#elif SRC_USING_PWM == SCR_LEVEL_MODE
    switch (scrt)
    {
    case SCRTA:
        SCRT1_Clear();
        BFL_SCRT_Pluse_Transmit_Config(&epwm1_info, _uiPluseNum, 2500);
        break;
    case SCRTB:
        SCRT2_Clear();
        BFL_SCRT_Pluse_Transmit_Config(&epwm2_info, _uiPluseNum, 2500);
        break;
    case SCRTC:
        SCRT3_Clear();
        BFL_SCRT_Pluse_Transmit_Config(&epwm3_info, _uiPluseNum, 2500);
        break;
    case SCRT_ALL:
        SCRT_ALL_Clear();
        BFL_SCRT_Pluse_Transmit_Config(&epwm4_info, _uiPluseNum, 2500);
    default:
        break;
    }
#endif
}
/**
 * @brief 输出脉冲。
 *
 * @param scrt 输出通道。
 */
void BFL_SCRT_Pluse_Set(BFL_SCRT_t scrt)
{
    ;
}
