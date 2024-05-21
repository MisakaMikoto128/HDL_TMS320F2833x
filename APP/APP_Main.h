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
#include "state_duration_timer.h"

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

#define CAPACITORS_STATE_CUT_OFF 0
#define CAPACITORS_STATE_WORKING 1
#define CAPACITORS_STATE_BYPASS 2

#define LINE_STATE_STOP 0
#define LINE_STATE_RUNNING 1

#define SCR_NO_FAULT 0
#define SCR_FAULT_CAN_NOT_TRIGGER 1
#define SCR_FAULT_BRANCH_BREAKDOWN 2

// 无故障
#define MINOR_FAULT_NONE 0x0000
// 线路轻载
#define MINOR_FAULT_LINE_LIGHT_LOAD_MASK 0x0001
#define MINOR_FAULT_LINE_LIGHT_LOAD 0x0001
// 线路过载
#define MINOR_FAULT_LINE_OVERLOAD_MASK 0x0002
#define MINOR_FAULT_LINE_OVERLOAD 0x0002
// 线路谐振
#define MINOR_FAULT_LINE_RESONANCE_MASK 0x0004
#define MINOR_FAULT_LINE_RESONANCE 0x0004
// 电容器过压
#define MINOR_FAULT_CAPACITOR_OV_MASK 0x0008
#define MINOR_FAULT_CAPACITOR_OV 0x0008
// 电容器过温
#define MINOR_FAULT_CAPACITOR_OT_MASK 0x0010
#define MINOR_FAULT_CAPACITOR_OT 0x0010
// 线路欠压
#define MINOR_FAULT_LINE_UNDERVOLTAGE_MASK 0x0020
#define MINOR_FAULT_LINE_UNDERVOLTAGE 0x0020

#define SET_MINOR_FAULT(value, mask) (value |= mask)
#define CLEAR_MINOR_FAULT(value, mask) (value &= ~mask)
#define EXIST_MINOR_FAULT(value, mask) (value & mask)

#define Have_Serious_Fault() (g_pSysInfo->Serious_Fault != 0)
#define Have_Minor_Fault() (g_pSysInfo->Minor_Fault != 0)
#define The_System_Line_State_Is_Running() (g_pSysInfo->Line_State == LINE_STATE_RUNNING)
#define The_Capacitors_Are_Working() (g_pSysInfo->Capacitors_State == CAPACITORS_STATE_WORKING)
#define The_Capacitors_Are_Cut_Off() (g_pSysInfo->Capacitors_State == CAPACITORS_STATE_CUT_OFF)
#define The_Capacitors_Are_Bypass() (g_pSysInfo->Capacitors_State == CAPACITORS_STATE_BYPASS)

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

        float V_SYS_STOP_kV;                        // 系统停运电压
        float V_SYS_UNDER_kV;                       // 系统投入电压下限（欠压值）
        float V_SYS_THH_kV;                         // 系统投入电压上限
        float V_SYS_OV_kV;                          // 系统切除电压（过压保护电压)
        uint16_t T_SYS_SATIFY_CAPACITORS_WAORK_SEC; // 电容器满足投入条件有效持续时间

        // 只能在指令执行过程中检查VBC状态
        uint16_t QF_Fault;
        uint16_t QS1_Fault;
        uint16_t QS2_Fault;
        uint16_t KM1_Fault;
        /*
            晶闸管触发故障：
            mask为0x3F。
            bit0对应SCRR3B，bit1对应SCRR3A，bit2对应SCRR2B，
            bit3对应SCRR2A，bit4对应SCRR1B，bit5对应SCRR1A。
            0:正常 1:触发故障
         */
        uint16_t SCRT_Fault;

        uint16_t VTx_A_Breakdown_Fault; // A相晶闸管击穿故障，0:正常 2:击穿
        uint16_t VTx_B_Breakdown_Fault; // B相晶闸管击穿故障，0:正常 2:击穿
        uint16_t VTx_C_Breakdown_Fault; // C相晶闸管击穿故障，0:正常 2:击穿

        float I_TA_low_thl_A;               // 线路轻载电流触发阈值
        float I_TA_low_thh_A;               // 线路轻载电流恢复阈值
        uint16_t T_I_TA_Thh_SEC;            // 轻载电流恢复延时
        float I_TA_oc_A;                    // 线路过载电流阈值
        uint16_t T_I_TA_oc_SEC;             // 线路过载触发延时
        float V_TVx_ov_kV;                  // 电容器过压阈值
        uint16_t T_V_TVx_ov_min;            // 电容器过压自动恢复时间
        float Tc_ot;                        // 电容器过温阈值
        uint16_t T_Tc_ot_SEC;               // 电容器过温恢复延时
        uint16_t T1_MS;                     // 闸刀状态反馈信号生效时间，单位ms
        uint16_t T2_US;                     // 晶闸管触发信号脉宽
        uint16_t T3_MS;                     // 晶闸管状态反馈信号生效时间
        uint16_t T4_MS;                     // 晶闸管导通到闭合VBC延时
        float V_SCR_NORMAL_DIFF_kV;         // 晶闸管在电容器投入工作期间的正常压差
        uint16_t T_V_SCR_ABNORMAL_DIFF_SEC; // 晶闸管击穿条件判断生效时间，必须在电容器投入工作期间判断
        float I_SCR_NORMAL_DIFF_A;          // 晶闸管击穿条件判断有效电流条件
        uint16_t UxAB_OV_Fault;             // 进线或者出线电压是否过压 0:正常 1:过压
        uint16_t T_SYS_UNDER_CANCLE_SEC;    // 系统欠压取消时间

        uint16_t Capacitors_State;      // 电容器状态 0:未投入（默认） 1:已经投入 2:旁路
        uint16_t Capacitors_Exec_State; // 电容器执行的状态 0:未投入（默认） 1:已经投入 2:旁路
        uint16_t Minor_Fault;           // 轻微故障 0:无故障 1:有故障
        uint16_t Serious_Fault;         // 严重故障 0:无故障 1:有故障
        uint16_t Line_State;            // 线路状态 0:停运 1:运行

        uint32_t devId;  //设备编号
        uint16_t devType; //设备型号
        uint32_t __crc16;
    } SysInfo_t;

#define APP_Main_HaveParamNeedToSave() (g_AppMainInfo.haveParamNeedToSave)
#define APP_Main_ClearParamNeedToSave() (g_AppMainInfo.haveParamNeedToSave = false)
#define APP_Main_NotifyHaveParamNeedToSave() (g_AppMainInfo.haveParamNeedToSave = true)

    typedef struct tagAppMainInfo_t
    {
        SysInfo_t sysInfo;
        uint16_t sysInfoSize;
        byte_t buffer[512 + 1];

        // B1_CapacitanceTemperatureMeasure
        uint32_t capTempSensorLastRevMsgTime_Ms; // 电容温度传感器最后接收消息时间
        uint32_t capTempSensorRequestStage;      // 请求时间

        // B1_SysModeGet
        bool modeBtnPressed;           // 按键是否按下
        uint32_t modeBtnFilterTimeCnt; // 按键状态续时间

        // B1_VCBStatusGet
        uint16_t VCB_StateFilterTimeCnt[VCB_SW_NUM];
        BFL_VCB_STATE_t VCB_StateLast[VCB_SW_NUM];

        // B2_Cmdxxx
        bool haveParamNeedToSave;

        // B3_SysAutoMode
        StateDurationCnt_t satifyLineStateRunningTimeCnt;
        StateDurationCnt_t satifyLineStateStopTimeCnt;
        StateDurationCnt_t satifyCapacitorsWorkTimeCnt;

        // B3_Check_Minor_Fault_Exist
        StateDurationCnt_t satifyT_I_TA_Thl;
        StateDurationCnt_t satifyT_I_TA_Thl_cancle;
        StateDurationCnt_t satifyT_I_TA_oc;
        StateDurationCnt_t satifyT_I_TA_oc_cancle;
        StateDurationCnt_t satifyT_V_TVx_ov;
        StateDurationCnt_t satifyT_V_TVx_ov_cancle;
        StateDurationCnt_t satifyT_Tc_ot;
        StateDurationCnt_t satifyT_Tc_ot_cancle;
        StateDurationCnt_t satifyT_SYS_UNDER_CANCLE_SEC;

        // B3_Check_SCR_Serious_Fault
        StateDurationCnt_t satifySCRA_SeriousFaultTimeCnt;
        StateDurationCnt_t satifySCRB_SeriousFaultTimeCnt;
        StateDurationCnt_t satifySCRC_SeriousFaultTimeCnt;
    } AppMainInfo_t;

    extern AppMainInfo_t g_AppMainInfo;
    extern SysInfo_t *g_pSysInfo;

    void APP_Main_Init();
    void APP_Main_Poll();

    void BackGroundTask();

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
    void B1_VCBStatus_Update();

    typedef struct
    {
        uint32_t code;
        uint16_t QF_Fault;
        uint16_t KM1_Fault;
        uint16_t SCRT_Fault;
    } B2_CmdMakeCapacitorsWork_Result_t;
    B2_CmdMakeCapacitorsWork_Result_t B2_CmdMakeCapacitorsWork_Exec();
    void B2_CmdMakeCapacitorsWork_Exec_Solution();

    typedef struct
    {
        uint32_t code;
        uint16_t QF_Fault;
        uint16_t KM1_Fault;
        uint16_t SCRT_Fault;
    } B2_CmdCutOffCapacitors_Result_t;
    B2_CmdCutOffCapacitors_Result_t B2_CmdCutOffCapacitors_Exec();
    void B2_CmdCutOffCapacitors_Exec_Solution();

    typedef struct
    {
        uint32_t code;
        uint16_t QF_Fault;
        uint16_t KM1_Fault;
        uint16_t SCRT_Fault;
    } B2_CmdBypassCapacitors_Result_t;
    B2_CmdBypassCapacitors_Result_t B2_CmdBypassCapacitors_Exec();
    void B2_CmdBypassCapacitors_Exec_Solution();

    void B3_SysAutoMode_DeltaPoll(uint32_t poll_delta);
    void B3_SysManualMode_DeltaPoll(uint32_t poll_delta);

    void B3_Check_SCR_Serious_Fault(uint32_t poll_delta);
    bool B3_Check_Minor_Fault_Exist(uint32_t poll_delta);
#ifdef __cplusplus
}
#endif
#endif //! APP_MAIN_H
