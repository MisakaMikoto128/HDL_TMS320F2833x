/**
 * @file HDL_SPI.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2023-01-19
 *
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 *
 */
#include "HDL_SPI.h"
#include "HDL_CPU_Time.h"
#include "CPU_Define.h"
#include "ccommon.h"
#include <stddef.h>


//
// InitSpiGpio - This function initializes GPIO pins to function as SPI pins
//
// Each GPIO pin can be configured as a GPIO pin or up to 3 different
// peripheral functional pins. By default all pins come up as GPIO
// inputs after reset.
//
// Caution:
// For each SPI peripheral
// Only one GPIO pin should be enabled for SPISOMO operation.
// Only one GPIO pin should be enabled for SPISOMI operation.
// Only one GPIO pin should be enabled for SPICLKA operation.
// Only one GPIO pin should be enabled for SPISTEA operation.
// Comment out other unwanted lines.
//
void InitSpiGpio()
{
    InitSpiaGpio();
}

//
// InitSpiaGpio - This function initializes GPIO poins to function as SPI pins
//
void InitSpiaGpio()
{
    EALLOW;

    //
    // Enable internal pull-up for the selected pins
    // Pull-ups can be enabled or disabled by the user.
    // This will enable the pullups for the specified pins.
    // Comment out other unwanted lines.
    //
    GpioCtrlRegs.GPAPUD.bit.GPIO16 = 0; // Enable pull-up on GPIO16 (SPISIMOA)
    GpioCtrlRegs.GPAPUD.bit.GPIO17 = 0; // Enable pull-up on GPIO17 (SPISOMIA)
    GpioCtrlRegs.GPAPUD.bit.GPIO18 = 0; // Enable pull-up on GPIO18 (SPICLKA)
    GpioCtrlRegs.GPAPUD.bit.GPIO19 = 0; // Enable pull-up on GPIO19 (SPISTEA)

    // GpioCtrlRegs.GPBPUD.bit.GPIO54 = 0; //Enable pull-up on GPIO54 (SPISIMOA)
    // GpioCtrlRegs.GPBPUD.bit.GPIO55 = 0; //Enable pull-up on GPIO55 (SPISOMIA)
    // GpioCtrlRegs.GPBPUD.bit.GPIO56 = 0; //Enable pull-up on GPIO56 (SPICLKA)
    // GpioCtrlRegs.GPBPUD.bit.GPIO57 = 0; //Enable pull-up on GPIO57 (SPISTEA)

    //
    // Set qualification for selected pins to asynch only
    // This will select asynch (no qualification) for the selected pins.
    // Comment out other unwanted lines.
    //
    GpioCtrlRegs.GPAQSEL2.bit.GPIO16 = 3; // Asynch input GPIO16 (SPISIMOA)
    GpioCtrlRegs.GPAQSEL2.bit.GPIO17 = 3; // Asynch input GPIO17 (SPISOMIA)
    GpioCtrlRegs.GPAQSEL2.bit.GPIO18 = 3; // Asynch input GPIO18 (SPICLKA)
    GpioCtrlRegs.GPAQSEL2.bit.GPIO19 = 3; // Asynch input GPIO19 (SPISTEA)

    // GpioCtrlRegs.GPBQSEL2.bit.GPIO54 = 3; // Asynch input GPIO16 (SPISIMOA)
    // GpioCtrlRegs.GPBQSEL2.bit.GPIO55 = 3; // Asynch input GPIO17 (SPISOMIA)
    // GpioCtrlRegs.GPBQSEL2.bit.GPIO56 = 3; // Asynch input GPIO18 (SPICLKA)
    // GpioCtrlRegs.GPBQSEL2.bit.GPIO57 = 3; // Asynch input GPIO19 (SPISTEA)

    //
    // Configure SPI-A pins using GPIO regs
    // This specifies which of the possible GPIO pins will be SPI
    // functional pins.
    // Comment out other unwanted lines.
    //
    GpioCtrlRegs.GPAMUX2.bit.GPIO16 = 1; // Configure GPIO16 as SPISIMOA
    GpioCtrlRegs.GPAMUX2.bit.GPIO17 = 1; // Configure GPIO17 as SPISOMIA
    GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 1; // Configure GPIO18 as SPICLKA
    GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 0; // Configure GPIO19 as SPISTEA

    // GpioCtrlRegs.GPBMUX2.bit.GPIO54 = 1; // Configure GPIO54 as SPISIMOA
    // GpioCtrlRegs.GPBMUX2.bit.GPIO55 = 1; // Configure GPIO55 as SPISOMIA
    // GpioCtrlRegs.GPBMUX2.bit.GPIO56 = 1; // Configure GPIO56 as SPICLKA
    // GpioCtrlRegs.GPBMUX2.bit.GPIO57 = 1; // Configure GPIO57 as SPISTEA

    GpioCtrlRegs.GPADIR.bit.GPIO19 = 1;
    EDIS;
}

//
// spi_init -
//
void spi_init()
{
    SpiaRegs.SPICCR.all = 0x000F; // Reset on, rising edge, 16-bit char bits

    //
    // Enable master mode, normal phase, enable talk, and SPI int disabled.
    //
    SpiaRegs.SPICTL.all = 0x0006;
    SpiaRegs.SPIBRR = 0x007F;
    SpiaRegs.SPICCR.all = 0x009F; // Relinquish SPI from Reset
    SpiaRegs.SPIPRI.bit.FREE = 1; // Set so breakpoints don't disturb xmission
}

//
// spi_xmit -
//
void spi_xmit(Uint16 a)
{
    SpiaRegs.SPITXBUF = a;
}

/**
 * @brief SPI初始化。默认为全双工主机,MSB First。通信频率会尽量接近SPI外设允许的最大频率。软件片选。
 *
 * @param spiID SPI设备ID。
 * @param dataSize 数SPI数据读写据位宽8或者16bit。
 * @param CPOL Specifies the serial clock steady state. HDL_SPI_CPOL_LOW: Low level, HDL_SPI_CPOL_HIGH: High level.
 * @param CPHA Specifies the clock active edge for the bit capture. HDL_SPI_CPHA_1EDGE: The first clock transition is the first data capture edge, HDL_SPI_CPHA_2EDGE: The second clock transition is the first data capture edge.
 */
void HDL_SPI_Init(SPI_ID_t spiID, byte_t dataSize, uint32_t CPOL, uint32_t CPHA)
{
    UNUSED(spiID);

    InitSpiaGpio();

    //
    // spi_init -
    //
    //SpiaRegs.SPICCR.all = 0x000F; // Reset on, rising edge, 16-bit char bits
    SpiaRegs.SPICCR.bit.SPICHAR = dataSize;
    SpiaRegs.SPICCR.bit.SPILBK = 0; // Loopback mode disabled
    SpiaRegs.SPICCR.bit.CLKPOLARITY = CPOL;

    //
    // Enable master mode, normal phase, enable talk, and SPI int disabled.
    //
    SpiaRegs.SPICTL.all = 0x0006;
    /*
    0h (R/W) = Normal SPI clocking scheme, depending on the
    CLOCK POLARITY bit (SPICCR.6).
    1h (R/W) = SPICLK signal delayed by one half-cycle. Polarity
    determined by the CLOCK POLARITY bit.
    */
    //Register (LOSPCP) -> LSPCLK -> default
    SpiaRegs.SPICTL.bit.CLK_PHASE = CPHA;
    SpiaRegs.SPIBRR = 0x0000;//波特率=150M(CPU FREQ)/4/(SPIBRR + 1)
    SpiaRegs.SPICCR.bit.SPISWRESET = 1; // Relinquish SPI from Reset
    SpiaRegs.SPIPRI.bit.FREE = 1; // Set so breakpoints don't disturb xmission

    //
    // spi_fifo_init -
    //
    //
    // Initialize SPI FIFO registers
    //
    SpiaRegs.SPIFFTX.all = 0xE040;
    SpiaRegs.SPIFFRX.all = 0x204f;
    SpiaRegs.SPIFFCT.all = 0x0;

    SpiaRegs.SPICCR.bit.SPISWRESET = 1; //SPI功能恢复
}

/**
 * @brief SPI读写。阻塞式读写方法。不能再中断中调用。pTxData和pRxData不能同时为NULL。
 *
 * @param spiID SPI设备ID。
 * @param pTxData 发送数据缓冲区。为NULL时发送0xFF。
 * @param pRxData 接收数据缓冲区。为NULL时不接收数据。
 * @param size 接收数据的长度。
 * @return true 读写成功。
 * @return false 读写失败。
 */
bool HDL_SPI_WriteRead(SPI_ID_t spiID, byte_t *pTxData, byte_t *pRxData, uint16_t size, uint32_t timeout)
{
    UNUSED(spiID);
    if ((pTxData == NULL && pRxData == NULL) || size == 0)
    {
        return false;
    }
    bool ret = true;
    uint32_t startMoment = HDL_CPU_Time_GetTick();

    for (uint32_t i = 0; i < size; i++)
    {
        // 等待发送缓冲区非满
        while ((SpiaRegs.SPISTS.bit.BUFFULL_FLAG == 1))
        {
            if (HDL_CPU_Time_GetTick() - startMoment > timeout)
            {
                ret = false;
                goto __spi_error;
            }
        }

        // 向SPIA发送一个字节的数据
        if(pTxData == NULL)
        {
            SpiaRegs.SPITXBUF = (0x00FFU << 8);
        }
        else
        {
            SpiaRegs.SPITXBUF = pTxData[i] << 8;
        }

        //
        // Wait until data is received
        //
        while (SpiaRegs.SPIFFRX.bit.RXFFST == 0)
        {
            if (HDL_CPU_Time_GetTick() - startMoment > timeout)
            {
                ret = false;
                goto __spi_error;
            }
        }

        if(pRxData != NULL)
        {
            pRxData[i] = SpiaRegs.SPIRXBUF;
        }else
        {
            uint16_t tmp = SpiaRegs.SPIRXBUF;
            tmp = 0;
            UNUSED(tmp);
        }
    }
__spi_error:

    return ret;
}

/**
 * @brief SPI写。
 *
 * @param spiID SPI设备ID。
 * @param pTxData 发送数据缓冲区。
 * @param size 数据长度。
 * @return true 写成功。
 * @return false 写失败。
 */
uint32_t HDL_SPI_Write(SPI_ID_t spiID, byte_t *pTxData, uint32_t size, uint32_t timeout)
{
    UNUSED(spiID);
    if (pTxData == NULL || size == 0)
    {
        return false;
    }
    uint32_t startMoment = 0;
    uint32_t ret = 0;

    startMoment = HDL_CPU_Time_GetTick();
    
    for (uint32_t i = 0; i < size; i++)
    {
        // spi fifo is full
        while ((SpiaRegs.SPISTS.bit.BUFFULL_FLAG == 1))
        {
            if (HDL_CPU_Time_GetTick() - startMoment > timeout)
            {
                goto __spi_error;
            }
        }
        //
        // Transmit data
        //
        spi_xmit(pTxData[i]);
        ret++;
    }

    __spi_error:
        
        return ret;
}

/**
 * @brief SPI读。
 *
 * @param spiID SPI设备ID。
 * @param pRxData 接收数据缓冲区。
 * @param size 要读取的数据长度。
 * @return uint32_t 实际读取的数据长度。
 */
uint32_t HDL_SPI_Read(SPI_ID_t spiID, byte_t *pRxData, uint16_t size, uint32_t timeout)
{
    UNUSED(spiID);
    if (pRxData == NULL || size == 0)
    {
        return false;
    }
    uint32_t startMoment = 0;
    uint32_t ret = 0;
    startMoment = HDL_CPU_Time_GetTick();
    
    for (uint32_t i = 0; i < size; i++)
    {
        //
        // Wait until data is received
        //
        while (SpiaRegs.SPIFFRX.bit.RXFFST == 0)
        {
            if (HDL_CPU_Time_GetTick() - startMoment > timeout)
            {
                goto __spi_error;
            }
        }

        pRxData[i] = SpiaRegs.SPIRXBUF;
        ret++;
    }
    __spi_error:
        
        return ret;
}
