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
#include "ccoding.h"

/**
 * @brief 设置 PCF8563 的时间信息
 *
 * @param pTime 指向 mtime_t 结构体的指针，包含要设置的时间信息
 */
void CHIP_PCF8563_Set(mtime_t *pTime)
{
    byte_t buffer[7] = {0};

    // 处理世纪位和年份
    uint16_t year_set = pTime->nYear;
    byte_t month_set = Hex2Bcd(pTime->nMonth); /**< 设置的月份，已转换为BCD格式 */
    // 判断世纪位处理
    if (year_set >= (PCF8563_BASE_YEAR + 100))
    {
        year_set -= PCF8563_BASE_YEAR + 100; // 将年份转换为两位数（2000年后的年份）
        month_set |= PCF_Century_SetBitC;    // 设置世纪位，表示2000年后的世代
    }
    else
    {
        year_set -= PCF8563_BASE_YEAR;     // 将年份转换为两位数（1900年后的年份）
        month_set &= ~PCF_Century_SetBitC; // 清除世纪位，表示1900年后的世代
    }

    // 将时间字段转换为BCD格式
    buffer[0] = Hex2Bcd(pTime->nSec);    // 秒
    buffer[1] = Hex2Bcd(pTime->nMin);    // 分
    buffer[2] = Hex2Bcd(pTime->nHour);   // 时
    buffer[3] = Hex2Bcd(pTime->nDay);    // 日
    buffer[4] = Hex2Bcd(pTime->nWeek);   // 星期
    buffer[5] = month_set;               // 月（含世纪位）
    buffer[6] = Hex2Bcd(year_set % 100); // 年（两位数）

    // 写入数据到寄存器
    CHIP_PCF8563_Write_Bytes(PCF8563_Address_Seconds, buffer, 7);
}

/**
 * @brief 读取 PCF8563 的时间信息
 *
 * @param pTime 指向 mtime_t 结构体的指针，用于存储读取到的时间信息
 */
void CHIP_PCF8563_Get(mtime_t *pTime)
{
    byte_t buffer[7] = {0};
    CHIP_PCF8563_Read_Bytes(PCF8563_Address_Seconds, buffer, 7); // 读取寄存器数值
    buffer[0] &= PCF8563_Shield_Seconds;                         // 屏蔽无效位
    buffer[1] &= PCF8563_Shield_Minutes;
    buffer[2] &= PCF8563_Shield_Hours;
    buffer[3] &= PCF8563_Shield_Days;
    buffer[4] &= PCF8563_Shield_WeekDays;
    buffer[5] &= PCF8563_Shield_Months_Century;
    buffer[6] &= PCF8563_Shield_Years;

    // 处理剩余的时间字段
    pTime->nSec = Bcd2Hex(buffer[0]);
    pTime->nMin = Bcd2Hex(buffer[1]);
    pTime->nHour = Bcd2Hex(buffer[2]);
    pTime->nDay = Bcd2Hex(buffer[3]);
    pTime->nWeek = Bcd2Hex(buffer[4]);                         // 周天数（0～6）
    pTime->nMonth = Bcd2Hex(buffer[5] & ~PCF_Century_SetBitC); // 去掉世纪位，得到月份
    // 处理年份和世纪位
    pTime->nYear = PCF8563_BASE_YEAR + Bcd2Hex(buffer[6]) + ((buffer[5] >> 7) & 0x01) * 100; // 年
}

/**
 * @brief 初始化 PCF8563 RTC 模块
 *
 * @return int 初始化状态，0表示成功，非0表示失败
 */
int CHIP_PCF8563_Init()
{
    // 初始化I2C引脚
    CHIP_PCF8563_IIC_Init();

    // 设置控制和状态寄存器为默认值
    // 根据具体应用需求调整寄存器值
    byte_t reg = 0x00;
    CHIP_PCF8563_Write_Bytes(PCF8563_Address_Control_Status_1, &reg, 1);
    CHIP_PCF8563_Write_Bytes(PCF8563_Address_Control_Status_2, &reg, 1);
    return 0;
}
