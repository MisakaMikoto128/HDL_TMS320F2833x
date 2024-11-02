/**
 * @file CHIP_PCF8563.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-06-28
 * @last modified 2024-06-28
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "CHIP_PCF8563.h"
#include "CHIP_PCF8563_Info.h"
#include "CHIP_PCF8563_Port.h"
/*******************************************************************************
 * 文件名：static byte_t RTC_Bcd2ToBin(byte_t BCDValue)
 * 描  述: 将BCD转换为BIN
 * 功  能：
 * 作  者：大核桃
 * 版本号：1.0.1(2017.03.03)
 *******************************************************************************/
static byte_t RTC_Bcd2ToBin(byte_t BCDValue)
{
    byte_t tmp = 0;

    tmp = ((byte_t)(BCDValue & (byte_t)0xF0) >> (byte_t)0x04) * 10;
    return (tmp + (BCDValue & (byte_t)0x0F));
}

/*******************************************************************************
 * 文件名：void CHIP_PCF8563_Set(sTime* pTime)
 * 描  述: PCF8563设置时间信息
 * 功  能：
 * 作  者：大核桃
 * 版本号：1.0.1(2017.03.03)
 *******************************************************************************/
void CHIP_PCF8563_Set(mtime_t *pTime)
{
    byte_t buffer[7] = {0};
    // 拷贝数据
    pTime->nMonth &= ~PCF_Century_SetBitC;

    buffer[0] = pTime->nSec;
    buffer[1] = pTime->nMin;
    buffer[2] = pTime->nHour;
    buffer[3] = pTime->nDay;
    buffer[4] = pTime->nWeek;
    buffer[5] = pTime->nMonth;
    buffer[6] = pTime->nYear;
    // 写入数据到寄存器
    //
    CHIP_PCF8563_Write_Bytes(PCF8563_Address_Seconds, 7, buffer);
}

/*******************************************************************************
 * 文件名：void CHIP_PCF8563_Get(sTime* pTime)
 * 描  述: PCF8563读取时间信息
 * 功  能：
 * 作  者：大核桃
 * 版本号：1.0.1(2017.03.03)
 *******************************************************************************/
void CHIP_PCF8563_Get(mtime_t *pTime)
{
    byte_t buffer[7] = {0};
    CHIP_PCF8563_Read_Bytes(PCF8563_Address_Seconds, 7, buffer); // 读取寄存器数值
    buffer[0] &= PCF8563_Shield_Seconds;                         // 屏蔽无效位
    buffer[1] &= PCF8563_Shield_Minutes;
    buffer[2] &= PCF8563_Shield_Hours;
    buffer[3] &= PCF8563_Shield_Days;
    buffer[4] &= PCF8563_Shield_WeekDays;
    buffer[5] &= PCF8563_Shield_Months_Century;
    buffer[6] &= PCF8563_Shield_Years;

    pTime->nYear = buffer[6];
    pTime->nMonth = buffer[5];
    pTime->nWeek = buffer[4];
    pTime->nDay = buffer[3];
    pTime->nHour = buffer[2];
    pTime->nMin = buffer[1];
    pTime->nSec = buffer[0];
}

int CHIP_PCF8563_Init()
{
    // 初始化I2C引脚
    CHIP_PCF8563_IIC_Init();
    CHIP_PCF8563_Write_Byte(PCF8563_Address_Control_Status_1, 0x00);
    CHIP_PCF8563_Write_Byte(PCF8563_Address_Control_Status_2, 0x00);
    return 0;
}
