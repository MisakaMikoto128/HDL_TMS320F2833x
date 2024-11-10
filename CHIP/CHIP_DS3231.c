/**
 * @file CHIP_DS3231.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-06-28
 * @last modified 2024-06-28
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "CHIP_DS3231.h"
#include "CHIP_DS3231_Info.h"
#include "CHIP_DS3231_Port.h"
#include "ccoding.h"

/**
 * @brief DS3231设置时间信息
 *
 * @param pTime 指向mtime_t结构体的指针，包含要设置的时间信息
 * @return void
 */
void CHIP_DS3231_Set(mtime_t *pTime)
{
    byte_t buffer[7] = {0};                    /**< 用于存储时间数据的缓冲区 */
    uint16_t year_set = pTime->nYear;          /**< 设置的年份 */
    byte_t month_set = Hex2Bcd(pTime->nMonth); /**< 设置的月份，已转换为BCD格式 */
    // 判断世纪位处理
    if (year_set >= (DS3231_BASE_YEAR + 100))
    {
        year_set -= DS3231_BASE_YEAR + 100;  // 将年份转换为两位数（2000年后的年份）
        month_set |= DS3231_Century_SetBitC; // 设置世纪位，表示2000年后的世代
    }
    else
    {
        year_set -= DS3231_BASE_YEAR;         // 将年份转换为两位数（1900年后的年份）
        month_set &= ~DS3231_Century_SetBitC; // 清除世纪位，表示1900年后的世代
    }

    // 填充缓冲区，转换为BCD格式
    buffer[0] = Hex2Bcd(pTime->nSec);      // 秒
    buffer[1] = Hex2Bcd(pTime->nMin);      // 分
    buffer[2] = Hex2Bcd(pTime->nHour);     // 时，默认24小时制
    buffer[3] = Hex2Bcd(pTime->nWeek + 1); // 星期，加1后存储（DS3231星期范围1-7）
    buffer[4] = Hex2Bcd(pTime->nDay);      // 日
    buffer[5] = month_set;                 // 月，包含世纪位
    buffer[6] = Hex2Bcd(year_set % 100);         // 年

    // 写入数据到DS3231寄存器，从秒开始的7个寄存器
    CHIP_DS3231_Write_Bytes(DS3231_REG_SECOND, buffer, 7);
}

/**
 * @brief DS3231读取时间信息
 *
 * @param pTime 指向mtime_t结构体的指针，用于存储读取的时间信息
 * @return void
 */
void CHIP_DS3231_Get(mtime_t *pTime)
{
    byte_t buffer[7] = {0}; // 用于接收从DS3231读取的时间数据

    // 从DS3231读取时间数据，从秒开始的7个寄存器
    CHIP_DS3231_Read_Bytes(DS3231_REG_SECOND, buffer, 7);

    // 解析秒、分、时
    pTime->nSec = Bcd2Hex(buffer[0]); // 秒
    pTime->nMin = Bcd2Hex(buffer[1]); // 分
    // 处理剩余的时间字段
    DS3231_HourFormat_t time_mode = (DS3231_HourFormat_t)((buffer[2] >> 6) & 0x01); // 12h/24h
    if (time_mode == DS3231_FORMAT_12H)
    {
        byte_t am_pm = ((buffer[2] >> 5) & 0x01);              // am/pm
        pTime->nHour = Bcd2Hex(buffer[2] & 0x1F) + am_pm * 12; // hour
    }
    else
    {
        pTime->nHour = Bcd2Hex(buffer[2] & 0x3F); // hour
    }
    // 解析星期，将DS3231的1-7范围转换为0-6
    pTime->nWeek = Bcd2Hex(buffer[3]) - 1;     // 星期，0 = Sunday
    pTime->nDay = Bcd2Hex(buffer[4]);          // date
    pTime->nMonth = Bcd2Hex(buffer[5] & 0x1F); // 去掉世纪位，得到月份

    // 解析年，考虑世纪位
    pTime->nYear = DS3231_BASE_YEAR + Bcd2Hex(buffer[6]) + ((buffer[5] >> 7) & 0x01) * 100; // 年
}

/**
 * @brief DS3231初始化函数
 * 
 * 该函数初始化DS3231所需的I2C接口引脚。
 * 
 * @return int 返回0表示初始化成功，返回其他值表示初始化失败。
 */
int CHIP_DS3231_Init()
{
    // 初始化I2C引脚
    CHIP_DS3231_IIC_Init();
    return 0;
}
