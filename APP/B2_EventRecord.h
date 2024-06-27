/**
 * @file B2_EventRecord.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief 用于记录事件相关的工作。
 * @version 0.1
 * @date 2024-06-26
 * @last modified 2024-06-26
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#ifndef B2_EVENTRECORD_H
#define B2_EVENTRECORD_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "APP_Main.h"

    /*

    事件记录格式V0.0.1
                缩放	注释
    记录数据结构
    记录编号	4字节	uint32_t		从0开始
    事件发送时间戳	8字节	uint64_t
    事件编号	1字节	uint8_t
    事件发生时的QF、QS1、QS2、KM1状态，系统模式、线路状态	1字节	uint8_t		bit7 QF、bit6 QS1、bit5 QS2、bit4 KM1、bit3-2 SYS_MODE、bit2 Line_State、bit0 reserve
    电容器最高温度	1字节	int8_t
    电容器最低温度	1字节	int8_t
    A相电容器电压	4字节	int32_t	10000
    B相电容器电压	4字节	int32_t	10000
    C相电容器电压	4字节	int32_t	10000
    AB进线电压	4字节	int32_t	10000
    AB出线电压	4字节	int32_t	10000
    A相进线电流	4字节	int32_t	10000
    B相进线电流	4字节	int32_t	10000
    C相进线电流	4字节	int32_t	10000


        4*12=48

    事件编号
    0	系统上电开机
    1	系统掉电关机
    2	线路运行
    3	线路停运
    4	电容器切除
    5	电容器旁路
    6	电容器投入
    7	严重故障晶闸管击穿
    8	严重故障晶闸管导通失败
    9	严重故障系统过速断电流
    10	轻微故障线路轻载触发
    11	轻微故障线路轻载取消
    12	轻微故障线路1段过流触发
    13	轻微故障线路1段过流取消
    14	轻微故障电容器过压故障触发
    15	轻微故障电容器过压故障取消
    16	轻微故障电容器过温故障触发
    17	轻微故障电容器过温故障取消
    18	轻微故障系统欠压触发
    19	轻微故障系统欠压取消
    20	轻微故障系统过压触发
    21	轻微故障系统过压取消
    22	严重故障清除事件
     */

    typedef enum
    {
        EVENT_SYSTEM_POWER_ON = 0,
        EVENT_SYSTEM_POWER_OFF,
        EVENT_LINE_RUN,
        EVENT_LINE_STOP,
        EVENT_CAPACITOR_REMOVAL,
        EVENT_CAPACITOR_BYPASS,
        EVENT_CAPACITOR_INPUT,
        EVENT_SERIOUS_FAULT_THRYRISTOR_BREAKDOWN,
        EVENT_SERIOUS_FAULT_THRYRISTOR_CONDUCTION_FAILURE,
        EVENT_SERIOUS_FAULT_SYSTEM_OVERSPEED_BREAKER_CURRENT,
        EVENT_MINOR_FAULT_LINE_LIGHT_LOAD_TRIGGER,
        EVENT_MINOR_FAULT_LINE_LIGHT_LOAD_CANCEL,
        EVENT_MINOR_FAULT_LINE_STAGE_1_OVERCURRENT_TRIGGER,
        EVENT_MINOR_FAULT_LINE_STAGE_1_OVERCURRENT_CANCEL,
        EVENT_MINOR_FAULT_CAPACITOR_OVERVOLTAGE_TRIGGER,
        EVENT_MINOR_FAULT_CAPACITOR_OVERVOLTAGE_CANCEL,
        EVENT_MINOR_FAULT_CAPACITOR_OVERTEMPERATURE_TRIGGER,
        EVENT_MINOR_FAULT_CAPACITOR_OVERTEMPERATURE_CANCEL,
        EVENT_MINOR_FAULT_SYSTEM_UNDERVOLTAGE_TRIGGER,
        EVENT_MINOR_FAULT_SYSTEM_UNDERVOLTAGE_CANCEL,
        EVENT_MINOR_FAULT_SYSTEM_OVERVOLTAGE_TRIGGER,
        EVENT_MINOR_FAULT_SYSTEM_OVERVOLTAGE_CANCEL,
        EVENT_SERIOUS_FAULT_CLEAR_EVENT,
        B2_EVENT_RECORD_EVENT_NUM,
    } B2_EventCode_t;

    typedef struct
    {
        uint32_t eventID;         // 记录编号
        uint64_t eventTimestamp;  // 事件发送时间戳
        B2_EventCode_t eventCode; // 事件编号
        int16_t eventStatus;      // QF, QS1, QS2, KM1状态，系统模式，线路状态
        int16_t maxTemperature;   // 电容器最高温度
        int16_t minTemperature;   // 电容器最低温度
        int32_t V_TV1A;           // A相电容器电压
        int32_t V_TV1B;           // B相电容器电压
        int32_t V_TV1C;           // C相电容器电压
        int32_t V_UIAB;           // AB进线电压
        int32_t V_UOAB;           // AB出线电压
        int32_t I_TA1A;           // A相进线电流
        int32_t I_TA1B;           // B相进线电流
        int32_t I_TA1C;           // C相进线电流
    } B2_EventRecord_t;

    void B2_EventRecord_Init();
    bool B2_EventRecord_Write(B2_EventCode_t eventCode);
    bool B2_EventRecord_Read(B2_EventRecord_t *eventRecord, uint32_t eventID);
    void B2_EventRecord_Poll();

#ifdef __cplusplus
}
#endif
#endif //! B2_EVENTRECORD_H
