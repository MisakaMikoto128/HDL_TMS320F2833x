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
#include "DSP2833x_Sci.h"
#include "cqueue.h"
#include <stddef.h>

//
// Defines
//
#define CPU_FREQ (Uint32)150E6
#define LSPCLK_FREQ (Uint32)(CPU_FREQ / 4)
#define SCI_FREQ (Uint32)115200

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

void InitSciaGpio(void);
void InitScibGpio(void);
void InitScicGpio(void);

typedef struct tagCOM_Dev_t
{
    COMID_t comId;
    // 主要是给Modbus使用，因为使用FIFO，发送完成时机不等于释放总线的时机。
    // 其时机是连续发送一串，然后直到硬件FIFO中的数据完全发送出去了产生回调，回调属于中断函数。
    // 这里实现依赖于串口的TC中断。
    // 这意味着如果两个Uart_Write对同一个串口写入的时机过于接近，那么就无法使得一次Uart_Write
    // 产生一个回调。
    UartWriteOverCallback_t write_over_callback;
    void *write_over_callback_args;
    UartReceiveCharCallback_t receive_char_callback;
    UartReceiveCharReadyCallback_t receive_char_ready_callback;
    uint32_t baud;
    uint32_t wordLen;
    uint32_t stopBit;
    uint32_t parity;

    CQueue_t txQueue;
    CQueue_t rxQueue;
    byte_t *txBuf;
    byte_t *rxBuf;
    // 串口是否初始化
    bool inited;
    volatile struct SCI_REGS *ScixRegs;
} COM_Dev_t;

//
// Globals
//
COM_Dev_t _gCOMList[COM_NUM] = {
    [COM1] = {.inited = false},
    [COM2] = {.inited = false},
    [COM3] = {.inited = false},
};

// 串口1相关变量
#define COM1_RX_BUF_SIZE 80
static byte_t m_Com1RxBuf[COM1_RX_BUF_SIZE] = {0};
#define COM1_TX_BUF_SIZE 32
static byte_t m_Com1TxBuf[COM1_TX_BUF_SIZE] = {0};

// 串口2相关变量
#define COM2_RX_BUF_SIZE 80
static byte_t m_Com2RxBuf[COM2_RX_BUF_SIZE] = {0};
#define COM2_TX_BUF_SIZE 256
static byte_t m_Com2TxBuf[COM2_TX_BUF_SIZE] = {0};

// 串口3相关变量
#define COM3_RX_BUF_SIZE 80
static byte_t m_Com3RxBuf[COM3_RX_BUF_SIZE] = {0};
#define COM3_TX_BUF_SIZE 32
static byte_t m_Com3TxBuf[COM3_TX_BUF_SIZE] = {0};

static void Scix_Init(volatile struct SCI_REGS *ScixRegs, uint32_t baud,
                      uint32_t wordLen, uint32_t stopBit, uint32_t parity)
{
    // setting the word length
    if (wordLen < UART_WORD_LEN_5 || wordLen > UART_WORD_LEN_8)
    {
        wordLen = UART_WORD_LEN_8;
    }
    ScixRegs->SCICCR.bit.SCICHAR = wordLen - 1;
    // setting the polarity
    switch (parity)
    {
    case UART_PARITY_NONE:
        ScixRegs->SCICCR.bit.PARITYENA = 0;
        break;
    case UART_PARITY_ODD:
        ScixRegs->SCICCR.bit.PARITYENA = 1;
        ScixRegs->SCICCR.bit.PARITY = 0;
        break;
    case UART_PARITY_EVEN:
        ScixRegs->SCICCR.bit.PARITYENA = 1;
        ScixRegs->SCICCR.bit.PARITY = 1;
        break;
    default:
        break;
    }

    switch (stopBit)
    {
    case UART_STOP_BIT_1:
        ScixRegs->SCICCR.bit.STOPBITS = 0;
        break;
    case UART_STOP_BIT_2:
        ScixRegs->SCICCR.bit.STOPBITS = 1;
        break;
    default:
        break;
    }

    //
    // enable TX, RX, internal SCICLK,
    // Disable RX ERR, SLEEP, TXWAKE
    //
    ScixRegs->SCICTL1.all = 0x0003;
    ScixRegs->SCICTL2.bit.TXINTENA = 0; // 1h (R/W) = Enable TXRDY interrupt
    ScixRegs->SCICTL2.bit.RXBKINTENA =
        0; // 1h (R/W) = Enable RXRDY/BRKDT interrupt
    uint16_t SCI_PRD = 0;
    SCI_PRD = (Uint16)((LSPCLK_FREQ / (baud * 8)) - 1);
    ScixRegs->SCIHBAUD = SCI_PRD >> 8;
    ScixRegs->SCILBAUD = SCI_PRD;
    ScixRegs->SCICCR.bit.LOOPBKENA = 0; // Enable loop back

    // 寄存器 SCIFFCT 的 ABD 位和 CDC 位控制自动波特率逻辑，使能 SCIRST
    // 位使自动波特率逻辑工作 1 SCI FIFO can resume transmit or receive. SCIRST
    // should be 1 even for Autobaud logic to work. 0 Write 0 to reset the SCI
    // transmit and receive channels. SCI FIFO register configuration bits will be
    // left as is.
    ScixRegs->SCIFFTX.bit.SCIRST = 1; // SCI Reset

    /*
    SCI FIFO enable Reset type: SYSRSn 0h (R/W) = SCI FIFO enhancements are
    disabled 1h (R/W) = SCI FIFO enhancements are enabled
    */
    ScixRegs->SCIFFTX.bit.SCIFFENA = 1;
    ScibRegs.SCIFFTX.bit.TXFFIENA = 0; // 发送FIFO中断失能
    ScibRegs.SCIFFTX.bit.TXFFIL = 0;
    ScixRegs->SCIFFRX.bit.RXFFIENA = 1; // 接收FIFO中断使能
    ScixRegs->SCIFFRX.bit.RXFFIL = 1;   // 接收1个字节产生一次发送中断，如果使能了

    ScixRegs->SCIFFCT.all = 0x00;
    ScixRegs->SCICTL1.all = 0x0023; // Relinquish SCI from Reset
    ScixRegs->SCIFFTX.bit.TXFIFOXRESET = 1;
    ScixRegs->SCIFFRX.bit.RXFIFORESET = 1;
}

void Uart_Init(COMID_t comId, uint32_t baud, uint32_t wordLen, uint32_t stopBit,
               uint32_t parity)
{

    if (comId >= COM_NUM)
    {
        return;
    }

    COM_Dev_t *pDev = &_gCOMList[comId];
    pDev->comId = comId;
    pDev->baud = baud;
    pDev->wordLen = wordLen;
    pDev->stopBit = stopBit;
    pDev->parity = parity;

    switch (comId)
    {
    case COM1:
        if (pDev->inited == false)
        {
            pDev->txBuf = (byte_t *)m_Com1TxBuf;
            pDev->rxBuf = (byte_t *)m_Com1RxBuf;
            cqueue_create(&pDev->txQueue, pDev->txBuf, COM1_TX_BUF_SIZE,
                          sizeof(byte_t));
            cqueue_create(&pDev->rxQueue, pDev->rxBuf, COM1_RX_BUF_SIZE,
                          sizeof(byte_t));
            pDev->ScixRegs = &SciaRegs;
        }
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

        Scix_Init(&SciaRegs, baud, wordLen, stopBit, parity);

        //
        // Step 5. User specific code, enable interrupts:
        //

        PieCtrlRegs.PIEIER9.bit.INTx1 = 1; // PIE Group 9, int1, SCIRXINTA
        PieCtrlRegs.PIEIER9.bit.INTx2 = 1; // PIE Group 9, INT2, SCITXINTA

        // Enable CPU int8 and int9 which are connected to SCIs
        IER |= M_INT9;

        break;
    case COM2:
        if (pDev->inited == false)
        {
            pDev->txBuf = (byte_t *)m_Com2TxBuf;
            pDev->rxBuf = (byte_t *)m_Com2RxBuf;
            cqueue_create(&pDev->txQueue, pDev->txBuf, COM2_TX_BUF_SIZE,
                          sizeof(byte_t));
            cqueue_create(&pDev->rxQueue, pDev->rxBuf, COM2_RX_BUF_SIZE,
                          sizeof(byte_t));
            pDev->ScixRegs = &ScibRegs;
        }

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
        Scix_Init(&ScibRegs, baud, wordLen, stopBit, parity);

        PieCtrlRegs.PIEIER9.bit.INTx3 = 1; // PIE Group 9, INT3, SCIRXINTB
        PieCtrlRegs.PIEIER9.bit.INTx4 = 1; // PIE Group 9, INT4, SCITXINTB

        IER |= M_INT9;
        break;
    case COM3:
        if (pDev->inited == false)
        {
            pDev->txBuf = (byte_t *)m_Com3TxBuf;
            pDev->rxBuf = (byte_t *)m_Com3RxBuf;
            cqueue_create(&pDev->txQueue, pDev->txBuf, COM3_TX_BUF_SIZE,
                          sizeof(byte_t));
            cqueue_create(&pDev->rxQueue, pDev->rxBuf, COM3_RX_BUF_SIZE,
                          sizeof(byte_t));
            pDev->ScixRegs = &ScicRegs;
        }
        // Init COM3
        InitScicGpio();

        // Interrupts that are used in this example are re-mapped to
        // ISR functions found within this file.
        //
        EALLOW; // This is needed to write to EALLOW protected registers
        PieVectTable.SCIRXINTC = &scicRxFifoIsr;
        PieVectTable.SCITXINTC = &scicTxFifoIsr;
        EDIS; // This is needed to disable write to EALLOW protected registers

        Scix_Init(&ScicRegs, baud, wordLen, stopBit, parity);

        PieCtrlRegs.PIEIER8.bit.INTx5 = 1; // PIE Group 8, SCIRXINTC
        PieCtrlRegs.PIEIER8.bit.INTx6 = 1; // PIE Group 8, SCITXINTC
        IER |= M_INT8;
        break;
    default:
        break;
    }

    pDev->inited = true;
    pDev->write_over_callback = NULL;
    pDev->write_over_callback_args = NULL;
    pDev->receive_char_callback = NULL;
    pDev->receive_char_ready_callback = NULL;
}

uint32_t Uart_Write(COMID_t comId, const byte_t *writeBuf, uint32_t uLen)
{
    // TODO:这里如果cnt = 1,在调试模式好像会占用发送一个字节的时间，暂时无法解决
    uint32_t uiBytesWritten = 0;

    if (comId >= COM_NUM || writeBuf == NULL || uLen == 0)
    {
        return uiBytesWritten;
    }
    int res = 0;
    COM_Dev_t *pDev = &_gCOMList[comId];
    if (pDev->inited == false)
    {
        uiBytesWritten = 0;
        return uiBytesWritten;
    }

    for (int i = 0; i < uLen; i++)
    {
        while (true)
        {
            /* 将新数据填入发送缓冲区 */
            DISABLE_INT();
            res = cqueue_enqueue(&_gCOMList[comId].txQueue, (const CObject_t)&writeBuf[i]);
            ENABLE_INT();
            if (res > 0)
            {
                break;
            }
            else
            {
                /* 数据已填满缓冲区 */
                /* 如果发送缓冲区已经满了，则等待缓冲区空 */
                //  TX FIFO Interrupt Enable
                pDev->ScixRegs->SCIFFTX.bit.TXFFIENA = 1;
            }
        }
    }

    //  TX FIFO Interrupt Enable
    pDev->ScixRegs->SCIFFTX.bit.TXFFIENA = 1;
    return uiBytesWritten;
}

// uint32_t Uart_Write(COMID_t comId, const byte_t *writeBuf, uint32_t uLen)
// {
//     uint32_t cnt = 0;
//     switch (comId)
//     {
//     case COM1:
//     {
//         for (cnt = 0; cnt < uLen; cnt++)
//         {
//             SciaRegs.SCITXBUF = writeBuf[cnt];
//             // 在此做判断，如果发送FIFO缓冲中数据  >=
//             // 16字节，要等待下直到FIFO小于16才能再次向FIFO中存数据
//             while (SciaRegs.SCIFFTX.bit.TXFFST >= 16)
//             {
//             }
//         }
//         //  TX FIFO Interrupt Enable
//         SciaRegs.SCIFFTX.bit.TXFFIENA = 1;
//         // TODO:这里如果cnt =
//         1,在调试模式好像会占用发送一个字节的时间，暂时无法解决
//     }

//     break;
//     case COM2:
//         for (cnt = 0; cnt < uLen; cnt++)
//         {
//             ScibRegs.SCITXBUF = writeBuf[cnt];
//             while (ScibRegs.SCIFFTX.bit.TXFFST >= 16)
//             {
//             }
//         }
//         //  TX FIFO Interrupt Enable
//         ScibRegs.SCIFFTX.bit.TXFFIENA = 1;
//         break;
//     case COM3:
//         for (cnt = 0; cnt < uLen; cnt++)
//         {
//             ScicRegs.SCITXBUF = writeBuf[cnt];
//             while (ScicRegs.SCIFFTX.bit.TXFFST >= 16)
//             {
//             }
//         }
//         //  TX FIFO Interrupt Enable
//         ScicRegs.SCIFFTX.bit.TXFFIENA = 1;
//         break;
//     default:
//         break;
//     }
//     return cnt;
// }

/**
 * @brief 串口读操作
 *
 * @param comx 串口号
 * @param pBuf 存放读取数据的缓存区的指针
 * @param uiLen 本次操作最多能读取的字节数
 * @return uint32_t >0-实际读取的字节数，0-没有数据或者串口不可用
 */
uint32_t Uart_Read(COMID_t comId, byte_t *pBuf, uint32_t uiLen)
{
    uint32_t uRtn = 0;
    if (comId < COM_NUM)
    {
        _disable_interrupts();
        uRtn = cqueue_out(&_gCOMList[comId].rxQueue, pBuf, uiLen);
        _enable_interrupts();
    }
    return uRtn;
}

/**
 * @brief 获取当前串口接收缓存中收到字节数。
 *
 * @param comId
 * @return uint32_t 当前串口接收缓存中收到字节数。
 */
uint32_t Uart_AvailableBytes(COMID_t comId)
{
    uint32_t uRtn = 0;
    if (comId < COM_NUM)
    {
        uRtn = cqueue_size(&_gCOMList[comId].rxQueue);
    }
    return uRtn;
}

/**
 * @brief 清空串口接收缓存。
 *
 * @param comId 串口号。
 * @return uint32_t 成功清空的字节数。
 */
uint32_t Uart_EmptyReadBuffer(COMID_t comId)
{
    int uRtn = 0;
    if (comId < COM_NUM)
    {
        uRtn = cqueue_size(&_gCOMList[comId].rxQueue);
        _disable_interrupts();
        cqueue_make_empty(&_gCOMList[comId].rxQueue);
        _enable_interrupts();
    }
    return uRtn;
}

/**
 * @brief
 * 注册Uart_Write写入完成回调函数。需要注意的是两次Uart_Write时间间隔太短可能就不能
 * 对应每个Uart_Write调用产生中断。
 *
 * @param comId 串口号。
 * @param callback 回调函数指针。
 * @param args 回调函数参数。
 * @return byte_t 成功1，失败0.
 */
byte_t Uart_SetWriteOverCallback(COMID_t comId,
                                 UartWriteOverCallback_t callback, void *args)
{
    byte_t ret = 0;

    if (comId < COM_NUM)
    {
        _gCOMList[comId].write_over_callback = callback;
        _gCOMList[comId].write_over_callback_args = args;
        ret = 1;
    }

    return ret;
}

/**
 * @brief
 *
 * @param comId 串口号。
 * @param callback 回调函数指针。
 * @return byte_t 成功1，失败0.
 */
byte_t Uart_RegisterReceiveCharCallback(COMID_t comId,
                                        UartReceiveCharCallback_t callback)
{
    byte_t ret = 0;
    if (comId < COM_NUM)
    {
        _gCOMList[comId].receive_char_callback = callback;
        ret = 1;
    }
    return ret;
}

/**
 * @brief
 * 取消注册中断中接收字符数据流的函数，这样就能且只能使用Uart_Read读取数据了。
 *
 * @param comId 串口号。
 * @return byte_t 成功1，失败0.
 */
byte_t Uart_UnregisterReceiveCharCallback(COMID_t comId)
{
    byte_t ret = 0;
    if (comId < COM_NUM)
    {
        _gCOMList[comId].receive_char_callback = NULL;
        ret = 1;
    }
    return ret;
}

byte_t
Uart_RegisterReceiveReadyCharCallback(COMID_t comId,
                                      UartReceiveCharReadyCallback_t callback)
{
    byte_t ret = 0;
    if (comId < COM_NUM)
    {
        _gCOMList[comId].receive_char_ready_callback = callback;
        ret = 1;
    }
    return ret;
}

//
// InitSciaGpio - This function initializes GPIO pins to function as SCI-A pins
//
void InitSciaGpio(void)
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
void InitScibGpio(void)
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
void InitScicGpio(void)
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

#include "BFL_DebugPin.h"
void USART_Callback(COM_Dev_t *pDev)
{
    /*
      TXFFIENA:
      Transmit FIFO interrrupt enable Reset type: SYSRSn
      0h (R/W) = TX FIFO interrupt is disabled
      1h (R/W) = TX FIFO interrupt is enabled.
      This interrupt is triggered whenever the transmit FIFO status (TXFFST) bits
      match (equal to or less than) the interrupt trigger level bits TXFFIL (bits
      4-0).
      TXFFST : 发送FIFO中的待发送的数据个数
      */

    /*
    TXRDY:
    Transmitter buffer register ready flag. When set, this bit indicates that the
    transmit data buffer register, SCITXBUF, is ready to receive another
    character. Writing data to the SCITXBUF automatically clears this bit. When
    set, this flag asserts a transmitter interrupt request if the interrupt-enable
    bit, TX INT ENA (SCICTL2.0), is also set. TXRDY is set to 1 by enabling the SW
    RESET bit (SCICTL1.5) or by a system reset. Reset type: SYSRSn 0h (R/W) =
    SCITXBUF is full 1h (R/W) = SCITXBUF is ready to receive the next character
    */
    volatile struct SCI_REGS *ScixRegs = pDev->ScixRegs;
    if (ScixRegs->SCICTL2.bit.TXRDY == 1)
    {
        byte_t ch = 0;
        size_t fifo_res = 16 - ScixRegs->SCIFFTX.bit.TXFFST;
        for (size_t i = 0; i < fifo_res; i++)
        {
            if (cqueue_dequeue(&pDev->txQueue, &ch) == 1)
            {
                ScixRegs->SCITXBUF = ch;
            }
            else
            {
                break;
            }
        }
    }

    /*
    TXEMPTY:
    Reset type: SYSRSn 0h (R/W) = Transmitter buffer or shift register or both are
    loaded with data 1h (R/W) = Transmitter buffer and shift registers are both
    empty
    */
    if (ScixRegs->SCICTL2.bit.TXEMPTY == 1)
    {
        if (cqueue_size(&pDev->txQueue) == 0)
        {
            // Disable transmit FIFO interrrupt
            ScixRegs->SCIFFTX.bit.TXFFIENA = 0;
            if (pDev->write_over_callback != NULL)
            {
                pDev->write_over_callback(pDev->write_over_callback_args);
            }
        }
    }
}
//
// sciaTxFifoIsr -
//
__interrupt void sciaTxFifoIsr(void)
{

    USART_Callback(&_gCOMList[COM1]);

    /*
    TXFFINTCLR:
    Transmit FIFO clear Reset type: SYSRSn
    0h (R/W) = Write 0 has no effect on TXFIFINT flag bit, Bit reads back a zero
    1h (R/W) = Write 1 to clear TXFFINT flag in bit 7
    */
    SciaRegs.SCIFFTX.bit.TXFFINTCLR = 1; // Clear SCI Interrupt flag
    PieCtrlRegs.PIEACK.all |= 0x100;     // Issue PIE ACK
}

//
// sciaRxFifoIsr -
//
__interrupt void sciaRxFifoIsr(void)
{

    /*
    RXRDY: R
    SCI receiver-ready flag. When a new character is ready to be read from the
    SCIRXBUF register, the receiver sets this bit, and a receiver interrupt is
    generated if the RX/BK INT ENA bit (SCICTL2.1) is a 1. RXRDY is cleared by a
    reading of the SCIRXBUF register, by an active SW RESET, or by a system reset.
    Reset type: SYSRSn 0h (R/W) = No new character in SCIRXBUF 1h (R/W) =
    Character ready to be read from SCIRXBUF

    BRKDT: R
    SCI break-detect flag. The SCI sets this bit when a break condition occurs. A
    break conditionoccurs when the SCI receiver data line (SCIRXD) remains
    continuously low for at least ten bits, beginning after a missing first stop
    bit. The occurrence of a break causes a receiver interrupt to be generated if
    the RX/BK INT ENA bit is a 1, but it does not cause the receiver buffer to be
    loaded. A BRKDT interrupt can occur even if the receiver SLEEP bit is set
    to 1. BRKDT is cleared by an active SW RESET or by a system reset. It is not
    cleared by receipt of a character after the break is detected. In order to
    receive more characters, the SCI must be reset by toggling the SW RESET bit or
    by a system reset. Reset type: SYSRSn 0h (R/W) = No break condition 1h (R/W) =
    Break condition occurred

    RXWAKE: R
    Receiver wake-up-detect flag Reset type: SYSRSn 0h (R/W) = No detection of a
      receiver wake-up condition 1h (R/W) = A value of 1 in this bit indicates
      detection of a receiver wake-up condition. In the address-bit multiprocessor
      mode (SCICCR.3 = 1), RXWAKE reflects the value of the address bit for the
      character contained in SCIRXBUF. In the idle-line multiprocessor mode,
    RXWAKE is set if the SCIRXD data line is detected as idle. RXWAKE is a
    read-only flag, cleared by one of the following: - The transfer of the first
    byte after the address byte to SCIRXBUF (only in non-FIFO mode) - The reading
    of SCIRXBUF - An active SW RESET - A system reset
    */

    COM_Dev_t *pDev = &_gCOMList[COM1];
    if (pDev->receive_char_callback != NULL)
    {
        byte_t ret = pDev->ScixRegs->SCIRXBUF.all;
        pDev->receive_char_callback(ret);
    }
    else if (pDev->receive_char_ready_callback != NULL)
    {
        pDev->receive_char_ready_callback();
    }
    else
    {
        uint16_t rx_fifo_ele_num = pDev->ScixRegs->SCIFFRX.bit.RXFFST;
        for (uint16_t i = 0; i < rx_fifo_ele_num; i++)
        {
            byte_t ch = pDev->ScixRegs->SCIRXBUF.all; // Read data
            cqueue_enqueue(&pDev->rxQueue, &ch);
        }
    }

    if (SciaRegs.SCIRXST.bit.RXRDY == 1)
    {
    }

    if (SciaRegs.SCIRXST.bit.BRKDT == 1)
    {
    }

    SciaRegs.SCIFFRX.bit.RXFFOVRCLR =
        1; // 1h (R/W) = Write 1 to clear RXFFOVF flag in bit 15
    SciaRegs.SCIFFRX.bit.RXFFINTCLR =
        1; // 1h (R/W) = Write 1 to clear RXFFINT flag in bit 7

    PieCtrlRegs.PIEACK.all |= 0x100; // Issue PIE ack
}

//
// scibTxFifoIsr -
//
__interrupt void scibTxFifoIsr(void)
{
    BFL_DebugPin_Set(DEBUG_PIN_2);
    USART_Callback(&_gCOMList[COM2]);
    BFL_DebugPin_Reset(DEBUG_PIN_2);
    ScibRegs.SCIFFTX.bit.TXFFINTCLR = 1; // Clear Interrupt flag
    PieCtrlRegs.PIEACK.all |= 0x100;     // Issue PIE ACK
}

//
// scibRxFifoIsr -
//
__interrupt void scibRxFifoIsr(void)
{
    COM_Dev_t *pDev = &_gCOMList[COM2];
    if (pDev->receive_char_callback != NULL)
    {
        byte_t ret = pDev->ScixRegs->SCIRXBUF.all;
        pDev->receive_char_callback(ret);
    }
    else if (pDev->receive_char_ready_callback != NULL)
    {
        pDev->receive_char_ready_callback();
    }
    else
    {
        uint16_t rx_fifo_ele_num = pDev->ScixRegs->SCIFFRX.bit.RXFFST;
        for (uint16_t i = 0; i < rx_fifo_ele_num; i++)
        {
            byte_t ch = pDev->ScixRegs->SCIRXBUF.all; // Read data
            cqueue_enqueue(&pDev->rxQueue, &ch);
        }
    }

    if (ScibRegs.SCIRXST.bit.RXRDY == 1)
    {
    }

    if (ScibRegs.SCIRXST.bit.BRKDT == 1)
    {
    }

    ScibRegs.SCIFFRX.bit.RXFFOVRCLR = 1; // Clear Overflow flag
    ScibRegs.SCIFFRX.bit.RXFFINTCLR = 1; // Clear Interrupt flag
    PieCtrlRegs.PIEACK.all |= 0x100;     // Issue PIE ack
}

//
// scicTxFifoIsr -
//
__interrupt void scicTxFifoIsr(void)
{

    USART_Callback(&_gCOMList[COM3]);

    ScicRegs.SCIFFTX.bit.TXFFINTCLR = 1; // Clear Interrupt flag
    PieCtrlRegs.PIEACK.all |= 0x100;     // Issue PIE ACK
}

//
// scicRxFifoIsr -
//
__interrupt void scicRxFifoIsr(void)
{

    COM_Dev_t *pDev = &_gCOMList[COM3];
    if (pDev->receive_char_callback != NULL)
    {
        byte_t ret = pDev->ScixRegs->SCIRXBUF.all;
        pDev->receive_char_callback(ret);
    }
    else if (pDev->receive_char_ready_callback != NULL)
    {
        pDev->receive_char_ready_callback();
    }
    else
    {
        uint16_t rx_fifo_ele_num = pDev->ScixRegs->SCIFFRX.bit.RXFFST;
        for (uint16_t i = 0; i < rx_fifo_ele_num; i++)
        {
            byte_t ch = pDev->ScixRegs->SCIRXBUF.all; // Read data
            cqueue_enqueue(&pDev->rxQueue, &ch);
        }
    }

    if (ScicRegs.SCIRXST.bit.RXRDY == 1)
    {
    }

    if (ScicRegs.SCIRXST.bit.BRKDT == 1)
    {
    }

    ScicRegs.SCIFFRX.bit.RXFFOVRCLR = 1; // Clear Overflow flag
    ScicRegs.SCIFFRX.bit.RXFFINTCLR = 1; // Clear Interrupt flag
    PieCtrlRegs.PIEACK.all |= 0x100;     // Issue PIE ack
}
