/**
 * @file CHIP_PCF8563.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-06-28
 * @last modified 2024-06-28
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#ifndef CHIP_PCF8563_H
#define CHIP_PCF8563_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "HDL_RTC.h"
#include <stdint.h>
#include <stdbool.h>
#include "ccommon.h"

    /*********************************************/
    // 数据类型定义部分
    // 定义结构体变量等
    /********************************************/

    typedef struct
    {
        uint16_t year;
        byte_t mon;
        byte_t day;
        byte_t hour;
        byte_t min;
        byte_t sec;
        byte_t week;
    } sTime;

    /*********************************************/
    // 预编译部分
    /********************************************/

    /*
    此处定义全局变量及函数
    */
    extern sTime CurTime;

    /*********************************************/
    // 子函数引用声明部分
    /********************************************/

#define PCF8563_Check_Data (byte_t)0x55 // 检测用，可用其他数值

#define PCF8563_ADDR (byte_t)0x51  // 7位地址
#define PCF8563_Write (byte_t)0xa2 // 写命令
#define PCF8563_Read (byte_t)0xa3  // 读命令，或者用（PCF8563_Write + 1）

//
// 电源复位功能
//
#define PCF8563_PowerResetEnable (byte_t)0x08
#define PCF8563_PowerResetDisable (byte_t)0x09

//
// 世纪位操作定义
//
#define PCF_Century_SetBitC (byte_t)0x80

#define PCF_Century_19xx (byte_t)0x03
#define PCF_Century_20xx (byte_t)0x04

//
// 数据格式
//
#define PCF_Format_BIN (byte_t)0x01
#define PCF_Format_BCD (byte_t)0x02

//
// 设置PCF8563模式用
//
#define PCF_Mode_Normal (byte_t)0x05

    /******************************************************************************
                                 参数寄存器地址宏定义
    ******************************************************************************/

#define PCF8563_Address_Control_Status_1 (byte_t)0x00 // 控制/状态寄存器1
#define PCF8563_Address_Control_Status_2 (byte_t)0x01 // 控制/状态寄存器2

    /******************************************************************************
                                   参数屏蔽宏定义
    ******************************************************************************/

#define PCF8563_Shield_Control_Status_1 (byte_t)0xa8
#define PCF8563_Shield_Control_Status_2 (byte_t)0x1f

#define PCF8563_Shield_Seconds (byte_t)0x7f
#define PCF8563_Shield_Minutes (byte_t)0x7f
#define PCF8563_Shield_Hours (byte_t)0x3f

#define PCF8563_Shield_Days (byte_t)0x3f
#define PCF8563_Shield_WeekDays (byte_t)0x07
#define PCF8563_Shield_Months_Century (byte_t)0x1f
#define PCF8563_Shield_Years (byte_t)0xff

/******************************************************************************
                                 参数宏定义
******************************************************************************/
#define PCF8563_Address_Years (byte_t)0x08    // 年
#define PCF8563_Address_Months (byte_t)0x07   // 月
#define PCF8563_Address_Days (byte_t)0x05     // 日
#define PCF8563_Address_WeekDays (byte_t)0x06 // 星期
#define PCF8563_Address_Hours (byte_t)0x04    // 小时
#define PCF8563_Address_Minutes (byte_t)0x03  // 分钟
#define PCF8563_Address_Seconds (byte_t)0x02  // 秒
// 控制/状态寄存器1 --> 0x00
//
#define PCF_Control_Status_NormalMode (byte_t)(~(1 << 7)) // 普通模式
#define PCF_Control_Status_EXT_CLKMode (byte_t)(1 << 7)   // EXT_CLK测试模式
#define PCF_Control_ChipRuns (byte_t)(~(1 << 5))          // 芯片运行
#define PCF_Control_ChipStop (byte_t)(1 << 5)             // 芯片停止运行，所有芯片分频器异步置逻辑0
#define PCF_Control_TestcClose (byte_t)(~(1 << 3))        // 电源复位功能失效（普通模式时置逻辑0）
#define PCF_Control_TestcOpen (byte_t)(1 << 3)            // 电源复位功能有效
// 秒寄存器 --> 0x02
//
#define PCF_Accuracy_ClockNo (byte_t)(1 << 7)     // 不保证准确的时钟/日历数据
#define PCF_Accuracy_ClockYes (byte_t)(~(1 << 7)) // 保证准确的时钟/日历数据

    /******************************************************************************
                                      外部功能函数
    ******************************************************************************/

    void PCF8563_Write_Byte(byte_t REG_ADD, byte_t dat);                  // PCF8563某寄存器写入一个字节数据
    byte_t PCF8563_Read_Byte(byte_t REG_ADD);                             // PCF8563某寄存器读取一个字节数据
    void PCF8563_Write_nByte(byte_t REG_ADD, byte_t num, byte_t *pBuff); // PCF8563写入多组数据
    void PCF8563_Read_nByte(byte_t REG_ADD, byte_t num, byte_t *pBuff);  // PCF8563读取多组数据

    void SetRealTime(sTime *time); // PCF8563写入时间信息
    void GetRealTime(sTime *time); // PCF8563读取时间信息
    void PCF8563Init(void);

#ifdef __cplusplus
}
#endif
#endif //! CHIP_PCF8563_H
