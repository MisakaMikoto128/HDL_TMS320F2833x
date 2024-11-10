/**
* @file HDL_IIC.c
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-11-10
* @last modified 2024-11-10
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#include "HDL_IIC.h"
#include "CPU_Define.h"

#define SDA_PIN 75 // 定义SDA引脚
#define SCL_PIN 74 // 定义SCL引脚

// 设置SDA为输出
#define SDA_OUT()                           \
    do                                      \
    {                                       \
        EALLOW;                             \
        GpioCtrlRegs.GPCDIR.bit.GPIO75 = 1; \
        EDIS;                               \
    } while (0)

// 设置SDA为输入
#define SDA_IN()                            \
    do                                      \
    {                                       \
        EALLOW;                             \
        GpioCtrlRegs.GPCDIR.bit.GPIO75 = 0; \
        EDIS;                               \
    } while (0)

// 设置SCL为输出
#define SCL_OUT()                           \
    do                                      \
    {                                       \
        EALLOW;                             \
        GpioCtrlRegs.GPCDIR.bit.GPIO74 = 1; \
        EDIS;                               \
    } while (0)

// 设置SDA高
#define SDA_HIGH() (GpioDataRegs.GPCSET.bit.GPIO75 = 1)
// 设置SDA低
#define SDA_LOW() (GpioDataRegs.GPCCLEAR.bit.GPIO75 = 1)
// 读取SDA状态
#define SDA_READ() ((int)(GpioDataRegs.GPCDAT.bit.GPIO75 == 1))

// 设置SCL高
#define SCL_HIGH() (GpioDataRegs.GPCSET.bit.GPIO74 = 1)
// 设置SCL低
#define SCL_LOW() (GpioDataRegs.GPCCLEAR.bit.GPIO74 = 1)

// I2C延时，根据你的时钟频率调整
void I2C_Delay(void)
{
    // 可以调整为合适的延时，确保符合I2C时序
    DELAY_US(5L);
}

void I2C_Start(void)
{
    SDA_OUT();  // 设置SDA为输出
    SDA_HIGH(); // SDA高电平
    SCL_HIGH(); // SCL高电平
    I2C_Delay();
    SDA_LOW(); // SDA拉低，开始信号
    I2C_Delay();
    SCL_LOW(); // SCL拉低，准备数据传输
}

void I2C_Stop(void)
{
    SDA_OUT(); // 设置SDA为输出
    SCL_LOW(); // SCL拉低
    SDA_LOW(); // SDA拉低
    I2C_Delay();
    SCL_HIGH(); // SCL拉高
    I2C_Delay();
    SDA_HIGH(); // SDA拉高，停止信号
    I2C_Delay();
}

void I2C_SendAck(void)
{
    SDA_OUT(); // 设置SDA为输出
    SDA_LOW(); // SDA拉低，发送ACK
    I2C_Delay();
    SCL_HIGH(); // 产生一个SCL时钟脉冲
    I2C_Delay();
    SCL_LOW();
}

void I2C_SendNack(void)
{
    SDA_OUT();  // 设置SDA为输出
    SDA_HIGH(); // SDA拉高，发送NACK
    I2C_Delay();
    SCL_HIGH(); // 产生一个SCL时钟脉冲
    I2C_Delay();
    SCL_LOW();
}

byte_t I2C_WaitAck(void)
{
    byte_t ack;

    SDA_IN(); // 设置SDA为输入，等待ACK
    I2C_Delay();
    SCL_HIGH(); // 拉高SCL，等待ACK信号
    I2C_Delay();
    ack = SDA_READ(); // 读取SDA状态，低电平表示ACK
    SCL_LOW();        // SCL拉低

    return (ack == 0) ? 1 : 0; // 返回1表示ACK，0表示NACK
}

void I2C_SendByte(byte_t byte)
{
    SDA_OUT(); // 设置SDA为输出
    for (int i = 0; i < 8; i++)
    {
        if (byte & 0x80) // 发送最高位（MSB）
            SDA_HIGH();
        else
            SDA_LOW();

        byte <<= 1; // 左移，准备发送下一个位
        I2C_Delay();
        SCL_HIGH(); // 拉高SCL，发送位
        I2C_Delay();
        SCL_LOW(); // 拉低SCL，准备下一位
    }
}

byte_t I2C_ReceiveByte(void)
{
    byte_t byte = 0;

    SDA_IN(); // 设置SDA为输入
    for (int i = 0; i < 8; i++)
    {
        byte <<= 1;
        SCL_HIGH(); // 拉高SCL，读取位
        I2C_Delay();
        if (SDA_READ()) // 读取SDA状态
            byte |= 0x01;
        SCL_LOW(); // 拉低SCL，准备下一位
        I2C_Delay();
    }

    return byte;
}

void i2c_mem_write(byte_t device_addr, byte_t reg_addr, byte_t *data, byte_t len)
{
    I2C_Start();                    // 发送起始条件
    I2C_SendByte(device_addr << 1); // 发送设备地址（写操作）
    I2C_WaitAck();                  // 等待ACK
    I2C_SendByte(reg_addr);         // 发送寄存器地址
    I2C_WaitAck();                  // 等待ACK

    for (byte_t i = 0; i < len; i++)
    {
        I2C_SendByte(data[i]); // 发送数据
        I2C_WaitAck();         // 等待ACK
    }

    I2C_Stop(); // 发送停止条件
}

void i2c_mem_read(byte_t device_addr, byte_t reg_addr, byte_t *data, byte_t len)
{
    I2C_Start();                    // 发送起始条件
    I2C_SendByte(device_addr << 1); // 发送设备地址（写操作）
    I2C_WaitAck();                  // 等待ACK
    I2C_SendByte(reg_addr);         // 发送寄存器地址
    I2C_WaitAck();                  // 等待ACK

    I2C_Start();                          // 重新生成Start条件
    I2C_SendByte((device_addr << 1) | 1); // 发送设备地址（读操作）
    I2C_WaitAck();                        // 等待ACK

    for (byte_t i = 0; i < len; i++)
    {
        data[i] = I2C_ReceiveByte(); // 接收数据
        if (i == len - 1)
            I2C_SendNack(); // 最后一个字节发送NACK
        else
            I2C_SendAck(); // 否则发送ACK
    }

    I2C_Stop(); // 发送停止条件
}

static void I2C_GPIO_Init(void)
{
    EALLOW; // 允许访问保护寄存器

    // 配置GPIO74 (SCL) 和 GPIO75 (SDA) 作为通用GPIO引脚
    GpioCtrlRegs.GPCMUX1.bit.GPIO74 = 0; // 将GPIO74配置为通用GPIO
    GpioCtrlRegs.GPCMUX1.bit.GPIO75 = 0; // 将GPIO75配置为通用GPIO

    // 设置GPIO74 (SCL) 和 GPIO75 (SDA) 为输出
    GpioCtrlRegs.GPCDIR.bit.GPIO74 = 1; // 将GPIO74（SCL）配置为输出
    GpioCtrlRegs.GPCDIR.bit.GPIO75 = 1; // 将GPIO75（SDA）配置为输出

    // 初始化SCL和SDA为高电平（I2C空闲状态）
    GpioDataRegs.GPCSET.bit.GPIO74 = 1; // 将GPIO74（SCL）设为高电平
    GpioDataRegs.GPCSET.bit.GPIO75 = 1; // 将GPIO75（SDA）设为高电平

    // Enable the internal pullup on the specified pin.
    GpioCtrlRegs.GPCPUD.bit.GPIO74 = 0;
    // Enable the internal pullup on the specified pin.
    GpioCtrlRegs.GPCPUD.bit.GPIO75 = 0;

    EDIS; // 禁止访问保护寄存器
}

void HDL_IIC_Init(IIC_ID_t iicID, uint32_t speed, byte_t addrLength,uint16_t ownAddr)
{
    UNUSED(iicID);
    UNUSED(speed);
    UNUSED(addrLength);
    UNUSED(ownAddr);

    I2C_GPIO_Init();
}

size_t HDL_IIC_Mem_Write(IIC_ID_t iicID, uint16_t addr, uint16_t memAddr,byte_t memAddrLength, const byte_t *data, size_t size)
{
    UNUSED(iicID);
    UNUSED(memAddrLength);
    i2c_mem_write(addr, memAddr, (byte_t *)data, size);
    return size;
}

size_t HDL_IIC_Mem_Read(IIC_ID_t iicID, uint16_t addr, uint16_t memAddr,byte_t memAddrLength, byte_t *buf, size_t size)
{
    UNUSED(iicID);
    UNUSED(memAddrLength);
    i2c_mem_read(addr, memAddr, buf, size);
    return size;
}
