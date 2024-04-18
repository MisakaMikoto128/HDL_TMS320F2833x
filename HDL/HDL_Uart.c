/**
 * @file HDL_Uart.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-04-18
 * @last modified 2024-04-18
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "HDL_Uart.h"
#include "CPU_Define.h"
//
// Defines
//
#define CPU_FREQ (Uint32)150E6
#define LSPCLK_FREQ (Uint32)(CPU_FREQ / 4)
#define SCI_FREQ (Uint32)100E3
#define SCI_PRD (Uint16)((LSPCLK_FREQ / (SCI_FREQ * 8)) - 1)

/*
The 28x SCI features autobaud detection and transmit/receive FIFO.
16 TX Fifo and 16 Rx Fifo.
*/

//
// Function Prototypes
//
__interrupt void sciaTxFifoIsr(void);
__interrupt void sciaRxFifoIsr(void);
__interrupt void scibTxFifoIsr(void);
__interrupt void scibRxFifoIsr(void);
__interrupt void scicTxFifoIsr(void);
__interrupt void scicRxFifoIsr(void);

void error(void);

//
// Globals
//
Uint16 sdataA[8];    // Send data for SCI-A
Uint16 sdataB[8];    // Send data for SCI-B
Uint16 sdataC[8];    // Send data for SCI-C
Uint16 rdataA[8];    // Received data for SCI-A
Uint16 rdataB[8];    // Received data for SCI-B
Uint16 rdataC[8];    // Received data for SCI-C
Uint16 rdata_pointA; // Used for checking the received data
Uint16 rdata_pointB;
Uint16 rdata_pointC;

void Uart_Init(COMID_t comId, uint32_t baud, uint32_t wordLen, uint32_t stopBit,
               uint32_t parity)
{
    switch (comId)
    {
    case COM1:
        // Init COM1
        //
        InitSciaGpio();
        // Interrupts that are used in this example are re-mapped to
        // ISR functions found within this file.
        //
        EALLOW; // This is needed to write to EALLOW protected registers
        PieVectTable.SCIRXINTA = &sciaRxFifoIsr;
        PieVectTable.SCITXINTA = &sciaTxFifoIsr;
        EDIS; // This is needed to disable write to EALLOW protected registers

        //
        // 1 stop bit,  No loopback, No parity,8 char bits, async mode,
        // idle-line protocol
        //
        SciaRegs.SCICCR.all = 0x0007;

        //
        // enable TX, RX, internal SCICLK, Disable RX ERR, SLEEP, TXWAKE
        //
        SciaRegs.SCICTL1.all = 0x0003;
        SciaRegs.SCICTL2.bit.TXINTENA = 1;
        SciaRegs.SCICTL2.bit.RXBKINTENA = 1;
        SciaRegs.SCIHBAUD = SCI_PRD >> 8;
        SciaRegs.SCILBAUD = SCI_PRD;
        SciaRegs.SCICCR.bit.LOOPBKENA = 1; // Enable loop back
        SciaRegs.SCIFFTX.all = 0xC028;
        SciaRegs.SCIFFRX.all = 0x0028;
        SciaRegs.SCIFFCT.all = 0x00;

        SciaRegs.SCICTL1.all = 0x0023; // Relinquish SCI from Reset
        SciaRegs.SCIFFTX.bit.TXFIFOXRESET = 1;
        SciaRegs.SCIFFRX.bit.RXFIFORESET = 1;
        //
        // Step 5. User specific code, enable interrupts:
        //

        //
        // Init send data.  After each transmission this data will be updated for
        // the next transmission
        //
        for (int i = 0; i < 8; i++)
        {
            sdataA[i] = i;
        }

        rdata_pointA = sdataA[0];

        PieCtrlRegs.PIEIER9.bit.INTx1 = 1; // PIE Group 9, int1, SCIRXINTA
        PieCtrlRegs.PIEIER9.bit.INTx2 = 1; // PIE Group 9, INT2, SCITXINTA

        break;
    case COM2:
        // Init COM2
        //
        InitScibGpio();
        // Interrupts that are used in this example are re-mapped to
        // ISR functions found within this file.
        //
        EALLOW; // This is needed to write to EALLOW protected registers
        PieVectTable.SCIRXINTB = &scibRxFifoIsr;
        PieVectTable.SCITXINTB = &scibTxFifoIsr;
        EDIS; // This is needed to disable write to EALLOW protected registers
        //
        // 1 stop bit,  No loopback, No parity,8 char bits,
        // async mode, idle-line protocol
        //
        ScibRegs.SCICCR.all = 0x0007;

        //
        // enable TX, RX, internal SCICLK,
        // Disable RX ERR, SLEEP, TXWAKE
        //
        ScibRegs.SCICTL1.all = 0x0003;
        ScibRegs.SCICTL2.bit.TXINTENA = 1;   // The sending interrupt function was enabled
        ScibRegs.SCICTL2.bit.RXBKINTENA = 1; // The receiving interrupt function was enabled
        ScibRegs.SCIHBAUD = SCI_PRD >> 8;
        ScibRegs.SCILBAUD = SCI_PRD;
        ScibRegs.SCICCR.bit.LOOPBKENA = 1; // Enable loop back
        ScibRegs.SCIFFTX.all = 0xC028;
        ScibRegs.SCIFFRX.all = 0x0028;
        ScibRegs.SCIFFCT.all = 0x00;

        ScibRegs.SCICTL1.all = 0x0023; // Relinquish SCI from Reset
        ScibRegs.SCIFFTX.bit.TXFIFOXRESET = 1;
        ScibRegs.SCIFFRX.bit.RXFIFORESET = 1;

        //
        // Init send data.  After each transmission this data will be updated for
        // the next transmission
        //
        for (int i = 0; i < 8; i++)
        {
            sdataB[i] = i;
        }

        rdata_pointB = sdataB[0];

        PieCtrlRegs.PIEIER9.bit.INTx3 = 1; // PIE Group 9, INT3, SCIRXINTB
        PieCtrlRegs.PIEIER9.bit.INTx4 = 1; // PIE Group 9, INT4, SCITXINTB

        break;
    case COM3:
        // Init COM3
        InitScicGpio();

        // Interrupts that are used in this example are re-mapped to
        // ISR functions found within this file.
        //
        EALLOW; // This is needed to write to EALLOW protected registers
        PieVectTable.SCIRXINTC = &scicRxFifoIsr;
        PieVectTable.SCITXINTC = &scicTxFifoIsr;
        EDIS; // This is needed to disable write to EALLOW protected registers
        //
        // 1 stop bit,  No loopback, No parity,8 char bits,
        // async mode, idle-line protocol
        //
        ScicRegs.SCICCR.all = 0x0007;

        //
        // enable TX, RX, internal SCICLK,
        // Disable RX ERR, SLEEP, TXWAKE
        //
        ScicRegs.SCICTL1.all = 0x0003;
        ScicRegs.SCICTL2.bit.TXINTENA = 1;   // The sending interrupt function was enabled
        ScicRegs.SCICTL2.bit.RXBKINTENA = 1; // The receiving interrupt function was enabled
        ScicRegs.SCIHBAUD = SCI_PRD >> 8;
        ScicRegs.SCILBAUD = SCI_PRD;
        ScicRegs.SCICCR.bit.LOOPBKENA = 1; // Enable loop back
        ScicRegs.SCIFFTX.all = 0xC028;
        ScicRegs.SCIFFRX.all = 0x0028;
        ScicRegs.SCIFFCT.all = 0x00;

        ScicRegs.SCICTL1.all = 0x0023; // Relinquish SCI from Reset
        ScicRegs.SCIFFTX.bit.TXFIFOXRESET = 1;
        ScicRegs.SCIFFRX.bit.RXFIFORESET = 1;

        //
        // Init send data.  After each transmission this data will be updated for
        // the next transmission
        //
        for (int i = 0; i < 8; i++)
        {
            sdataC[i] = i;
        }

        rdata_pointC = sdataC[0];

        PieCtrlRegs.PIEIER8.bit.INTx5 = 1; // PIE Group 8, SCIRXINTC
        PieCtrlRegs.PIEIER8.bit.INTx6 = 1; // PIE Group 8, SCITXINTC
        break;
    default:
        break;
    }
}

uint32_t Uart_Write(COMID_t comId, const byte_t *writeBuf, uint32_t uLen);
uint32_t Uart_Read(COMID_t comId, byte_t *pBuf, uint32_t uiLen);
uint32_t Uart_AvailableBytes(COMID_t comId);
uint32_t Uart_EmptyReadBuffer(COMID_t comId);
byte_t Uart_SetWriteOverCallback(COMID_t comId,
                                 UartWriteOverCallback_t callback, void *args);
byte_t Uart_RegisterReceiveCharCallback(COMID_t comId,
                                        UartReceiveCharCallback_t callback);
byte_t Uart_UnregisterReceiveCharCallback(COMID_t comId);

//
// InitSciaGpio - This function initializes GPIO pins to function as SCI-A pins
//
void InitSciaGpio()
{
    EALLOW;

    //
    // Enable internal pull-up for the selected pins
    // Pull-ups can be enabled or disabled disabled by the user.
    // This will enable the pullups for the specified pins.
    //
    GpioCtrlRegs.GPBPUD.bit.GPIO36 = 0; // Enable pull-up for GPIO36 (SCIRXDA)
    GpioCtrlRegs.GPBPUD.bit.GPIO35 = 0; // Enable pull-up for GPIO35 (SCITXDA)

    //
    // Set qualification for selected pins to asynch only
    // Inputs are synchronized to SYSCLKOUT by default.
    // This will select asynch (no qualification) for the selected pins.
    //
    GpioCtrlRegs.GPBQSEL1.bit.GPIO36 = 3; // Asynch input GPIO36 (SCIRXDA)

    //
    // Configure SCI-A pins using GPIO regs
    // This specifies which of the possible GPIO pins will be SCI functional
    // pins.
    //
    GpioCtrlRegs.GPBMUX1.bit.GPIO36 = 1; // Configure GPIO36 to SCIRXDA
    GpioCtrlRegs.GPBMUX1.bit.GPIO35 = 1; // Configure GPIO35 to SCITXDA

    EDIS;
}

//
// InitScibGpio - This function initializes GPIO pins to function as SCI-B pins
//
void InitScibGpio()
{
    EALLOW;

    //
    // Enable internal pull-up for the selected pins
    // Pull-ups can be enabled or disabled disabled by the user.
    // This will enable the pullups for the specified pins.
    // Comment out other unwanted lines.
    //
    // GpioCtrlRegs.GPAPUD.bit.GPIO9 = 0;  //Enable pull-up for GPIO9  (SCITXDB)
    GpioCtrlRegs.GPAPUD.bit.GPIO14 = 0; // Enable pull-up for GPIO14 (SCITXDB)
    // GpioCtrlRegs.GPAPUD.bit.GPIO18 = 0;  //Enable pull-up for GPIO18 (SCITXDB)
    // GpioCtrlRegs.GPAPUD.bit.GPIO22 = 0; //Enable pull-up for GPIO22 (SCITXDB)

    // GpioCtrlRegs.GPAPUD.bit.GPIO11 = 0; //Enable pull-up for GPIO11 (SCIRXDB)
    GpioCtrlRegs.GPAPUD.bit.GPIO15 = 0; // Enable pull-up for GPIO15 (SCIRXDB)
    // GpioCtrlRegs.GPAPUD.bit.GPIO19 = 0;  //Enable pull-up for GPIO19 (SCIRXDB)
    // GpioCtrlRegs.GPAPUD.bit.GPIO23 = 0; //Enable pull-up for GPIO23 (SCIRXDB)

    //
    // Set qualification for selected pins to asynch only
    // This will select asynch (no qualification) for the selected pins.
    // Comment out other unwanted lines.
    //
    // GpioCtrlRegs.GPAQSEL1.bit.GPIO11 = 3;  // Asynch input GPIO11 (SCIRXDB)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 3; // Asynch input GPIO15 (SCIRXDB)
    // GpioCtrlRegs.GPAQSEL2.bit.GPIO19 = 3;  // Asynch input GPIO19 (SCIRXDB)
    // GpioCtrlRegs.GPAQSEL2.bit.GPIO23 = 3;  // Asynch input GPIO23 (SCIRXDB)

    //
    // Configure SCI-B pins using GPIO regs
    // This specifies which of the possible GPIO pins will be SCI functional
    // pins.
    // Comment out other unwanted lines.
    //
    // GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 2;  //Configure GPIO9 to SCITXDB
    GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 2; // Configure GPIO14 to SCITXDB
    // GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 2;  //Configure GPIO18 to SCITXDB
    // GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 3; //Configure GPIO22 to SCITXDB

    // GpioCtrlRegs.GPAMUX1.bit.GPIO11 = 2;  //Configure GPIO11 for SCIRXDB
    GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 2; // Configure GPIO15 for SCIRXDB
    // GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 2;   //Configure GPIO19 for SCIRXDB
    // GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 3;  //Configure GPIO23 for SCIRXDB

    EDIS;
}

//
// InitScicGpio - This function initializes GPIO pins to function as SCI-C pins
//
void InitScicGpio()
{
    EALLOW;

    //
    // Enable internal pull-up for the selected pins
    // Pull-ups can be enabled or disabled disabled by the user.
    // This will enable the pullups for the specified pins.
    //
    GpioCtrlRegs.GPBPUD.bit.GPIO62 = 0; // Enable pull-up for GPIO62 (SCIRXDC)
    GpioCtrlRegs.GPBPUD.bit.GPIO63 = 0; // Enable pull-up for GPIO63 (SCITXDC)

    //
    // Set qualification for selected pins to asynch only
    // Inputs are synchronized to SYSCLKOUT by default.
    // This will select asynch (no qualification) for the selected pins.
    //
    GpioCtrlRegs.GPBQSEL2.bit.GPIO62 = 3; // Asynch input GPIO62 (SCIRXDC)

    //
    // Configure SCI-C pins using GPIO regs
    // This specifies which of the possible GPIO pins will be SCI functional
    // pins.
    //
    GpioCtrlRegs.GPBMUX2.bit.GPIO62 = 1; // Configure GPIO62 to SCIRXDC
    GpioCtrlRegs.GPBMUX2.bit.GPIO63 = 1; // Configure GPIO63 to SCITXDC

    EDIS;
}

//
// errror -
//
void error(void)
{
    __asm("     ESTOP0"); // Test failed!! Stop!
    for (;;)
        ;
}

//
// sciaTxFifoIsr -
//
__interrupt void
sciaTxFifoIsr(void)
{
    Uint16 i;
    for (i = 0; i < 8; i++)
    {
        SciaRegs.SCITXBUF = sdataA[i]; // Send data
    }

    for (i = 0; i < 8; i++) // Increment send data for next cycle
    {
        sdataA[i] = (sdataA[i] + 1) & 0x00FF;
    }

    SciaRegs.SCIFFTX.bit.TXFFINTCLR = 1; // Clear SCI Interrupt flag
    PieCtrlRegs.PIEACK.all |= 0x100;     // Issue PIE ACK
}

//
// sciaRxFifoIsr -
//
__interrupt void
sciaRxFifoIsr(void)
{
    Uint16 i;
    for (i = 0; i < 8; i++)
    {
        rdataA[i] = SciaRegs.SCIRXBUF.all; // Read data
    }

    for (i = 0; i < 8; i++) // Check received data
    {
        if (rdataA[i] != ((rdata_pointA + i) & 0x00FF))
        {
            error();
        }
    }

    rdata_pointA = (rdata_pointA + 1) & 0x00FF;

    SciaRegs.SCIFFRX.bit.RXFFOVRCLR = 1; // Clear Overflow flag
    SciaRegs.SCIFFRX.bit.RXFFINTCLR = 1; // Clear Interrupt flag

    PieCtrlRegs.PIEACK.all |= 0x100; // Issue PIE ack
}

//
// scibTxFifoIsr -
//
__interrupt void
scibTxFifoIsr(void)
{
    Uint16 i;
    for (i = 0; i < 8; i++)
    {
        ScibRegs.SCITXBUF = sdataB[i]; // Send data
    }

    for (i = 0; i < 8; i++) // Increment send data for next cycle
    {
        sdataB[i] = (sdataB[i] + 1) & 0x00FF;
    }

    ScibRegs.SCIFFTX.bit.TXFFINTCLR = 1; // Clear Interrupt flag
    PieCtrlRegs.PIEACK.all |= 0x100;     // Issue PIE ACK
}

//
// scibRxFifoIsr -
//
__interrupt void
scibRxFifoIsr(void)
{
    Uint16 i;
    for (i = 0; i < 8; i++)
    {
        rdataB[i] = ScibRegs.SCIRXBUF.all; // Read data
    }
    for (i = 0; i < 8; i++) // Check received data
    {
        if (rdataB[i] != ((rdata_pointB + i) & 0x00FF))
        {
            error();
        }
    }
    rdata_pointB = (rdata_pointB + 1) & 0x00FF;

    ScibRegs.SCIFFRX.bit.RXFFOVRCLR = 1; // Clear Overflow flag
    ScibRegs.SCIFFRX.bit.RXFFINTCLR = 1; // Clear Interrupt flag
    PieCtrlRegs.PIEACK.all |= 0x100;     // Issue PIE ack
}

//
// scicTxFifoIsr -
//
__interrupt void
scicTxFifoIsr(void)
{
    Uint16 i;
    for (i = 0; i < 8; i++)
    {
        ScicRegs.SCITXBUF = sdataC[i]; // Send data
    }

    for (i = 0; i < 8; i++) // Increment send data for next cycle
    {
        sdataC[i] = (sdataC[i] + 1) & 0x00FF;
    }

    ScicRegs.SCIFFTX.bit.TXFFINTCLR = 1; // Clear Interrupt flag
    PieCtrlRegs.PIEACK.all |= 0x100;     // Issue PIE ACK
}

//
// scicRxFifoIsr -
//
__interrupt void
scicRxFifoIsr(void)
{
    Uint16 i;
    for (i = 0; i < 8; i++)
    {
        rdataC[i] = ScicRegs.SCIRXBUF.all; // Read data
    }
    for (i = 0; i < 8; i++) // Check received data
    {
        if (rdataC[i] != ((rdata_pointC + i) & 0x00FF))
        {
            error();
        }
    }
    rdata_pointC = (rdata_pointC + 1) & 0x00FF;

    ScicRegs.SCIFFRX.bit.RXFFOVRCLR = 1; // Clear Overflow flag
    ScicRegs.SCIFFRX.bit.RXFFINTCLR = 1; // Clear Interrupt flag
    PieCtrlRegs.PIEACK.all |= 0x100;     // Issue PIE ack
}
