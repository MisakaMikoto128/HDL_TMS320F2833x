/**
 * @file HDL_IWDG.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-04-20
 * @last modified 2024-04-20
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "HDL_IWDG.h"
#include "CPU_Define.h"
#include "DSP2833x_Device.h"

//
// Function Prototypes
//
__interrupt void wakeint_isr(void);
//
// Globals
//
static Uint32 WakeCount = 0;
static Uint32 IWDGCountTimeOut = 0;
/**
 * @brief
 * default wdg interrupt period is 1.0922 * 0xFF = 278.511ms
 * @param timeout_ms if timeout_ms < 278.511ms, it will be set to 278.511ms
 */
void HDL_IWDG_Init(uint32_t timeout_ms)
{
    WakeCount = 0;
    IWDGCountTimeOut = 0;
    /*
     8-bit Watchdog Counter
    */
    //
    // Interrupts that are used in this example are re-mapped to
    // ISR functions found within this file.
    //
    EALLOW; // This is needed to write to EALLOW protected registers
    PieVectTable.WAKEINT = &wakeint_isr;
    EDIS; // This is needed to disable write to EALLOW protected registers

    //
    // Connect the watchdog to the WAKEINT interrupt of the PIE
    // Write to the whole SCSR register to avoid clearing WDOVERRIDE bit
    //
    EALLOW;
    SysCtrlRegs.SCSR = BIT1;
    // SysCtrlRegs.SCSR &= ~BIT1;  //改为复位
    EDIS;

    //
    // Enable WAKEINT in the PIE: Group 1 interrupt 8
    // Enable INT1 which is connected to WAKEINT:
    //
    PieCtrlRegs.PIEIER1.bit.INTx8 = 1; // Enable PIE Group 1 INT8
    IER |= M_INT1;                     // Enable CPU int1

    //
    // Reset the watchdog counter
    //
    ServiceDog();

    //
    // Enable the watchdog
    //
    EALLOW;
// if WDPS = 0,WDCLK = OSCCLK/512/1 = 30MHz/512/1 else WDCLK = OSCCLK/512/(2^(WDPS - 1))
//  SysCtrlRegs.WDCR = 0x0028;
#define GET_WDCR(WDFLAG, WDDIS, WDCHK, WDPS) ((WDFLAG) << 7) | ((WDDIS) << 6) | ((WDCHK) << 3) | ((WDPS) << 0)
    SysCtrlRegs.WDCR = GET_WDCR(0, 0, 0b101, 0b111); // Enable Watchdog, 1st stage, 58.6kHz WDCLK
    // WDCLK = 30MHz/512/64 = 915.527Hz = 1.0922ms
    EDIS;

    uint16_t timeout = timeout_ms / (0xFF * 1.0922f);
    if (timeout > 0xFF)
    {
        timeout = 0xFF;
    }
    if (timeout < 1)
    {
        timeout = 1;
    }

    IWDGCountTimeOut = timeout;
}

void HDL_IWDG_Feed(void)
{
    // Reload the IWDG counter to prevent reset
    ServiceDog();
}

//
// Step 7. Insert all local Interrupt Service Routines (ISRs) and functions
// here: If local ISRs are used, reassign vector addresses in vector table as
// shown in Step 5
//

//
// wakeint_isr -
//
__interrupt void
wakeint_isr(void)
{
    WakeCount++;
    if (WakeCount > IWDGCountTimeOut)
    {
        // IWDGCountTimeOut = 0;
        // Reset MCU

        //
        // Connect the watchdog to the WAKEINT interrupt of the PIE
        // Write to the whole SCSR register to avoid clearing WDOVERRIDE bit
        //
        EALLOW;
        SysCtrlRegs.SCSR &= ~BIT1;  //改为复位
        EDIS;

        EALLOW;
        SysCtrlRegs.WDCR = GET_WDCR(0, 0, 0b101, 0);
        EDIS;
        for (;;)
            ;
    }

    //
    // Acknowledge this interrupt to get more from group 1
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
