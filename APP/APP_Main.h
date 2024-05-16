/**
 * @file APP_Main.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-04-18
 * @last modified 2024-04-18
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#ifndef APP_MAIN_H
#define APP_MAIN_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "ccommon.h"
#include "BFL_Measure.h"



#define Tc_A_IDX 0
#define Tc_B_IDX 1
#define Tc_C_IDX 2

#define V_TV1A_ADC_IDX 0
#define V_TV1B_ADC_IDX 1
#define V_TV1C_ADC_IDX 2
#define V_UIAB_ADC_IDX 3
#define V_UOAB_ADC_IDX 4
#define I_TA1A_ADC_IDX 5
#define I_TA1B_ADC_IDX 6
#define I_TA1C_ADC_IDX 7

    typedef struct tagSysInfo_t
    {
        // B1_CapacitanceTemperatureMeasure
        float capTemp[3];                  // 电容温度
        bool capTempSensorFault[3];        // 电容温度传感器故障 true:故障 false:正常
        bool capTempSensorTransmitConnect; // 电容温度传感器传输连接 true:连接 false:断开

        // B1_Measure
        BFL_Measure_t measure;

        float TV1A_ScaleL1; // 拓展版测量输入 = ADC测量电压 * ScaleL1
        float TV1B_ScaleL1; // 拓展版测量输入 = ADC测量电压 * ScaleL1
        float TV1C_ScaleL1; // 拓展版测量输入 = ADC测量电压 * ScaleL1
        float UIAB_ScaleL1; // 拓展版测量输入 = ADC测量电压 * ScaleL1
        float UOAB_ScaleL1; // 拓展版测量输入 = ADC测量电压 * ScaleL1
        float TA1A_ScaleL1; // 拓展版测量输入 = ADC测量电压 * ScaleL1
        float TA1B_ScaleL1; // 拓展版测量输入 = ADC测量电压 * ScaleL1
        float TA1C_ScaleL1; // 拓展版测量输入 = ADC测量电压 * ScaleL1

        float VL1rms_TV1A; // 电容电压_A相拓展板测量输入
        float VL1rms_TV1B; // 电容电压_B相拓展板测量输入
        float VL1rms_TV1C; // 电容电压_C相拓展板测量输入
        float VL1rms_UIAB; // 进线电压拓展板测量输入
        float VL1rms_UOAB; // 出线电压拓展板测量输入
        float VL1rms_TA1A; // 进线电流_A相拓展板测量输入
        float VL1rms_TA1B; // 进线电流_B相拓展板测量输入
        float VL1rms_TA1C; // 进线电流_C相拓展板测量输入

        float TV1A_ScaleL2; // 实际输入模拟量 = L1电压 * ScaleL2
        float TV1B_ScaleL2; // 实际输入模拟量 = L1电压 * ScaleL2
        float TV1C_ScaleL2; // 实际输入模拟量 = L1电压 * ScaleL2
        float UIAB_ScaleL2; // 实际输入模拟量 = L1电压 * ScaleL2
        float UOAB_ScaleL2; // 实际输入模拟量 = L1电压 * ScaleL2
        float TA1A_ScaleL2; // 实际输入模拟量 = L1电压 * ScaleL2
        float TA1B_ScaleL2; // 实际输入模拟量 = L1电压 * ScaleL2
        float TA1C_ScaleL2; // 实际输入模拟量 = L1电压 * ScaleL2

        float V_TV1A; // 电容电压_A相
        float V_TV1B; // 电容电压_B相
        float V_TV1C; // 电容电压_C相
        float V_UIAB; // 进线电压
        float V_UOAB; // 出线电压
        float I_TA1A; // 进线电流_A相
        float I_TA1B; // 进线电流_B相
        float I_TA1C; // 进线电流_C相



        float I_TA_low_thl;
        float I_TA_low_thh;
        float T_I_TA_Thh;
        float I_TA_oc;
        float T_I_TA_oc;
        float V_TVx_ov;
        float T_V_TVx_ov;
        float Tc_ot;
        float T_Tc_ot;
        float T1;
        float T2;
        float T3;

        uint16_t __crc16;
    } SysInfo_t;

    typedef struct tagAppMainInfo_t
    {
        SysInfo_t sysInfo;
        byte_t buffer[256 + 1];
        
        // B1_CapacitanceTemperatureMeasure
        uint32_t capTempSensorLastRevMsgTime_Ms; // 电容温度传感器最后接收消息时间
        uint32_t capTempSensorRequestStage;                  // 请求时间
    
    } AppMainInfo_t;

    extern AppMainInfo_t g_AppMainInfo;
    extern SysInfo_t *g_pSysInfo;

    void APP_Main_Init();
    void APP_Main_Poll();

    void B1_ModbusRTUSlaver_Init();
    void B1_ModbusRTUSlaver_Poll();

    void B1_CapacitanceTemperatureMeasure_Init();
    void B1_CapacitanceTemperatureMeasure_Poll();

    void B1_Measure_Init();
    void B1_Measure_Poll();
#ifdef __cplusplus
}
#endif
#endif //! APP_MAIN_H
