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
#include "BFL_VCB.h"

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

#define SYS_MODE_AUTO 0
#define SYS_MODE_MANUAL 1

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

        // B1_SysModeGet
        uint16_t SYS_MODE; // 系统模式

        // B1_VCBStatusGet 实时采样滤波得到的反馈状态，
        // 用于执行指令是的初始条件判断
        uint16_t QF_FB;  // QF反馈信号
        uint16_t QS1_FB; // QS1反馈信号
        uint16_t QS2_FB; // QS2反馈信号
        uint16_t KM1_FB; // KM1反馈信号

        float V_SYS_STOP;            // 系统停运电压
        float V_SYS_UNDER;           // 系统投入电压下限（欠压值）
        float V_SYS_THH;             // 系统投入电压上限
        float V_SYS_OV;              // 系统切除电压（过压保护电压)
        uint16_t T_SYS_UNDER_CANCLE; // 系统欠压取消时间

        // 只能在指令执行过程中检查VBC状态
        uint16_t QF_Fault;
        uint16_t QS1_Fault;
        uint16_t QS2_Fault;
        uint16_t KM1_Fault;

        uint16_t VT1_A_Fault;
        uint16_t TV2_A_Fault;
        uint16_t VT1_B_Fault;
        uint16_t TV2_B_Fault;
        uint16_t VT1_C_Fault;
        uint16_t TV2_C_Fault;

        float I_TA_low_thl;  // 线路轻载电流触发阈值
        float I_TA_low_thh;  // 线路轻载电流恢复阈值
        uint16_t T_I_TA_Thh; // 轻载电流恢复延时
        float I_TA_oc;       // 线路过载电流阈值
        uint16_t T_I_TA_oc;  // 线路过载触发延时
        float V_TVx_ov;      // 电容器过压阈值
        uint16_t T_V_TVx_ov; // 电容器过压自动恢复时间
        float Tc_ot;         // 电容器过温阈值
        uint16_t T_Tc_ot;    // 电容器过温恢复延时
        uint16_t T1;         // 闸刀状态反馈信号生效时间
        uint16_t T2;         // 晶闸管触发信号脉宽
        uint16_t T3;         // 晶闸管状态反馈信号生效时间

        uint16_t __crc16;
    } SysInfo_t;

    typedef struct tagAppMainInfo_t
    {
        SysInfo_t sysInfo;
        byte_t buffer[256 + 1];

        // B1_CapacitanceTemperatureMeasure
        uint32_t capTempSensorLastRevMsgTime_Ms; // 电容温度传感器最后接收消息时间
        uint32_t capTempSensorRequestStage;      // 请求时间

        // B1_SysModeGet
        bool modeBtnPressed;           // 按键是否按下
        uint32_t modeBtnFilterTimeCnt; // 按键状态续时间

        // B1_VCBStatusGet
        uint16_t VCB_StateFilterTimeCnt[VCB_SW_NUM];
        BFL_VCB_STATE_t VCB_StateLast[VCB_SW_NUM];
    } AppMainInfo_t;

    extern AppMainInfo_t g_AppMainInfo;
    extern SysInfo_t *g_pSysInfo;

    void APP_Main_Init();
    void APP_Main_Poll();

    void B0_DeltaPoll();

    void B1_ModbusRTUSlaver_Init();
    void B1_ModbusRTUSlaver_Poll();

    void B1_CapacitanceTemperatureMeasure_Init();
    void B1_CapacitanceTemperatureMeasure_Poll();

    void B1_Measure_Init();
    void B1_Measure_Poll();

    void B1_SysModeGet_Init();
    void B1_SysModeGet_DeltaPoll(uint32_t poll_delta);

    void B1_VCBStatusGet_Init();
    void B1_VCBStatusGet_DeltaPoll(uint32_t poll_delta);
#ifdef __cplusplus
}
#endif
#endif //! APP_MAIN_H
