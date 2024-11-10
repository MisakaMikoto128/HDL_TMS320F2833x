/**
* @file CHIP_DS3231_Port.c
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-11-02
* @last modified 2024-11-02
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#include "CHIP_DS3231_Port.h"
#include "CHIP_DS3231_Info.h"
#include "HDL_IIC.h"

/*******************************************************************************
 * 文件名：void CHIP_DS3231_Write_Bytes(byte_t regAddr, byte_t *pBuff, byte_t num)
 * 描  述: DS3231写入多组数据
 * 功  能：REG_ADD：要操作寄存器起始地址          num：    写入数据数量           *WBuff： 写入数据缓存
 * 作  者：刘沅林
 * 版本号：1.0.1(2017.03.03)
 *******************************************************************************/
void CHIP_DS3231_Write_Bytes(byte_t regAddr, byte_t *pBuff, byte_t num)
{
    HDL_IIC_Mem_Write(SOFT_IIC_1, DS3231_I2C_ADDR, regAddr, 8, pBuff, num);
}

/*******************************************************************************
 * 文件名：void CHIP_DS3231_Read_Bytes(byte_t regAddr, byte_t *pBuff, byte_t num)
 * 描  述: DS3231读取多组数据
 * 功  能：REG_ADD：要操作寄存器起始地址          num：    写入数据数量           *WBuff： 读取数据缓存
 * 作  者：刘沅林
 * 版本号：1.0.1(2017.03.03)
 *******************************************************************************/
void CHIP_DS3231_Read_Bytes(byte_t regAddr, byte_t *pBuff, byte_t num)
{
    HDL_IIC_Mem_Read(SOFT_IIC_1, DS3231_I2C_ADDR, regAddr, 8, pBuff, num);
}

void CHIP_DS3231_IIC_Init()
{
    HDL_IIC_Init(SOFT_IIC_1,NONE_IIC_SPEED,NONE_IIC_ADDR_LEN,NONE_IIC_OWN_ADDR);
}
