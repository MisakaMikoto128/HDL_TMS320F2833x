/**
 * @file B1_ModbusRTUSlaver.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 * @last modified 2024-05-16
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "mb.h"
#include "mbcb.h"
#include "ccommon.h"
#include "mtime.h"
#include "period_query.h"
#include "BFL_RS485.h"
#include "HDL_CPU_Time.h"
#include "HDL_Uart.h"
#include "APP_Main.h"
#include "BFL_VCB.h"
#include "BFL_Measure.h"
#include "CPU_Define.h"

void SyncSysinfoToModbusReg();
void SyncModbusRegToSysinfo();
void MBCmdHandler(uint16_t cmdReg);

void B1_ModbusRTUSlaver_Init()
{
    SyncSysinfoToModbusReg();
    /*
     * RTU模式 从机地址：0x01 串口：这里不起作用，随便写 波特率：115200 无奇偶校验位
     */
    eMBInit(MB_RTU, 0x01, 0, 115200, MB_PAR_NONE);
    eMBEnable();
}

void B1_ModbusRTUSlaver_Poll()
{
    static PeriodREC_t s_tPollTime = 0;
    if_period_query_user_us(&s_tPollTime, MS_TO_US(1))
    {
        eMBPoll();
    }

    static PeriodREC_t s_tPollTime1 = 0;
    if_period_query_user_us(&s_tPollTime1, MS_TO_US(20))
    {
        if (eMBRegHoldingChanged())
        {
            eMBRegHoldingClearChanged();
            APP_Main_NotifyHaveParamNeedToSave();
        }
        SyncSysinfoToModbusReg();
    }
}

#include "mbcb.h"

// 输入寄存器内容
extern uint16_t usRegInputBuf[REG_INPUT_NREGS];
// 输入寄存器起始地址
extern uint16_t usRegInputStart;

// 保持寄存器内容
extern uint16_t usRegHoldingBuf[REG_HOLDING_NREGS];
// 保持寄存器起始地址
extern uint16_t usRegHoldingStart;

// 线圈状态
extern byte_t ucRegCoilsBuf[REG_COILS_SIZE / 8];
// 开关输入状态
extern byte_t ucRegDiscreteBuf[REG_DISCRETE_SIZE / 8];

#define FLOAT_TO_UINT32_SCALE(x, scale) ((uint32_t)((x) * (scale)))
#define FLOAT_TO_UINT16_SCALE(x, scale) ((uint16_t)((x) * (scale)))
#define HIGH_16_BITS(x) ((uint16_t)(((x) >> 16) & 0xFFFF))
#define LOW_16_BITS(x) ((uint16_t)((x) & 0xFFFF))
#define USING_TWO_REG_TO_FLOAT(regBuf, idx, scale) \
    ((float)(((uint32_t)((regBuf)[idx]) << 16) | (uint32_t)((regBuf)[idx + 1])) * (float)(scale))

/**
 * @brief 将系统信息同步到Modbus寄存器中。
 *
 */
void SyncSysinfoToModbusReg()
{
    // 保持寄存器同步
    SysInfo_t *pSysinfo = g_pSysInfo;

    usRegHoldingBuf[0] = FLOAT_TO_UINT16_SCALE(pSysinfo->TV1A_ScaleL1, 10);
    usRegHoldingBuf[1] = FLOAT_TO_UINT16_SCALE(pSysinfo->TV1B_ScaleL1, 10);
    usRegHoldingBuf[2] = FLOAT_TO_UINT16_SCALE(pSysinfo->TV1C_ScaleL1, 10);
    usRegHoldingBuf[3] = FLOAT_TO_UINT16_SCALE(pSysinfo->UIAB_ScaleL1, 10);
    usRegHoldingBuf[4] = FLOAT_TO_UINT16_SCALE(pSysinfo->UOAB_ScaleL1, 10);
    usRegHoldingBuf[5] = FLOAT_TO_UINT16_SCALE(pSysinfo->TA1A_ScaleL1, 1000);
    usRegHoldingBuf[6] = FLOAT_TO_UINT16_SCALE(pSysinfo->TA1B_ScaleL1, 1000);
    usRegHoldingBuf[7] = FLOAT_TO_UINT16_SCALE(pSysinfo->TA1C_ScaleL1, 1000);
    usRegHoldingBuf[8] = FLOAT_TO_UINT16_SCALE(pSysinfo->TV1A_ScaleL2, 1000);
    usRegHoldingBuf[9] = FLOAT_TO_UINT16_SCALE(pSysinfo->TV1B_ScaleL2, 1000);
    usRegHoldingBuf[10] = FLOAT_TO_UINT16_SCALE(pSysinfo->TV1C_ScaleL2, 1000);
    usRegHoldingBuf[11] = FLOAT_TO_UINT16_SCALE(pSysinfo->UIAB_ScaleL2, 1000);
    usRegHoldingBuf[12] = FLOAT_TO_UINT16_SCALE(pSysinfo->UOAB_ScaleL2, 1000);
    usRegHoldingBuf[13] = FLOAT_TO_UINT16_SCALE(pSysinfo->TA1A_ScaleL2, 1000);
    usRegHoldingBuf[14] = FLOAT_TO_UINT16_SCALE(pSysinfo->TA1B_ScaleL2, 1000);
    usRegHoldingBuf[15] = FLOAT_TO_UINT16_SCALE(pSysinfo->TA1C_ScaleL2, 1000);

    usRegHoldingBuf[16] = HIGH_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->V_SYS_STOP_kV, 1000));
    usRegHoldingBuf[17] = LOW_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->V_SYS_STOP_kV, 1000));
    usRegHoldingBuf[18] = HIGH_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->V_SYS_UNDER_kV, 1000));
    usRegHoldingBuf[19] = LOW_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->V_SYS_UNDER_kV, 1000));
    usRegHoldingBuf[20] = HIGH_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->V_SYS_THH_kV, 1000));
    usRegHoldingBuf[21] = LOW_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->V_SYS_THH_kV, 1000));
    usRegHoldingBuf[22] = HIGH_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->V_SYS_OV_kV, 1000));
    usRegHoldingBuf[23] = LOW_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->V_SYS_OV_kV, 1000));
    usRegHoldingBuf[24] = pSysinfo->T_SYS_UNDER_CANCLE_SEC;
    usRegHoldingBuf[25] = HIGH_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->I_TA_low_thl_A, 1000));
    usRegHoldingBuf[26] = LOW_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->I_TA_low_thl_A, 1000));
    usRegHoldingBuf[27] = HIGH_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->I_TA_low_thh_A, 1000));
    usRegHoldingBuf[28] = LOW_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->I_TA_low_thh_A, 1000));
    usRegHoldingBuf[29] = pSysinfo->T_I_TA_Thh_SEC;
    usRegHoldingBuf[30] = HIGH_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->I_TA_oc_A, 1000));
    usRegHoldingBuf[31] = LOW_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->I_TA_oc_A, 1000));
    usRegHoldingBuf[32] = pSysinfo->T_I_TA_oc_SEC;
    usRegHoldingBuf[33] = FLOAT_TO_UINT16_SCALE(pSysinfo->V_TVx_ov_kV, 1000);
    usRegHoldingBuf[34] = pSysinfo->T_V_TVx_ov_min;
    usRegHoldingBuf[35] = pSysinfo->Tc_ot;
    usRegHoldingBuf[36] = pSysinfo->T_Tc_ot_SEC;
    usRegHoldingBuf[37] = pSysinfo->T1_MS;
    usRegHoldingBuf[38] = pSysinfo->T2_US;
    usRegHoldingBuf[39] = pSysinfo->T3_MS;
    usRegHoldingBuf[40] = pSysinfo->T4_MS;
    usRegHoldingBuf[41] = FLOAT_TO_UINT16_SCALE(pSysinfo->V_SCR_NORMAL_DIFF_kV, 1000);
    usRegHoldingBuf[42] = pSysinfo->T_V_SCR_ABNORMAL_DIFF_SEC;
    usRegHoldingBuf[43] = FLOAT_TO_UINT16_SCALE(pSysinfo->I_SCR_NORMAL_DIFF_A, 1000);
    usRegHoldingBuf[44] = HIGH_16_BITS(pSysinfo->devId);
    usRegHoldingBuf[45] = LOW_16_BITS(pSysinfo->devId);
    usRegHoldingBuf[46] = pSysinfo->devType;
    usRegHoldingBuf[47] = 'L';
    usRegHoldingBuf[48] = 'Y';
    usRegHoldingBuf[49] = 'L';
    usRegHoldingBuf[52] = pSysinfo->T_SYS_SATIFY_CAPACITORS_WAORK_SEC;

    usRegInputBuf[0] = FLOAT_TO_UINT16_SCALE(pSysinfo->V_TV1A, 1000);
    usRegInputBuf[1] = FLOAT_TO_UINT16_SCALE(pSysinfo->V_TV1B, 1000);
    usRegInputBuf[2] = FLOAT_TO_UINT16_SCALE(pSysinfo->V_TV1C, 1000);
    usRegInputBuf[3] = HIGH_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->V_UIAB, 1000));
    usRegInputBuf[4] = LOW_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->V_UIAB, 1000));
    usRegInputBuf[5] = HIGH_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->V_UOAB, 1000));
    usRegInputBuf[6] = LOW_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->V_UOAB, 1000));
    usRegInputBuf[7] = HIGH_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->I_TA1A, 1000));
    usRegInputBuf[8] = LOW_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->I_TA1A, 1000));
    usRegInputBuf[9] = HIGH_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->I_TA1B, 1000));
    usRegInputBuf[10] = LOW_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->I_TA1B, 1000));
    usRegInputBuf[11] = HIGH_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->I_TA1C, 1000));
    usRegInputBuf[12] = LOW_16_BITS(FLOAT_TO_UINT32_SCALE(pSysinfo->I_TA1C, 1000));

    usRegInputBuf[17] = pSysinfo->capTemp[Tc_A_IDX];
    usRegInputBuf[18] = pSysinfo->capTemp[Tc_B_IDX];
    usRegInputBuf[19] = pSysinfo->capTemp[Tc_C_IDX];
    uint16_t reg = 0;
    reg |= ((pSysinfo->SYS_MODE) & 0x01) << 0;
    reg |= ((pSysinfo->Line_State) & 0x01) << 1;
    reg |= ((pSysinfo->Capacitors_State) & 0x03) << 3;
    reg |= ((pSysinfo->Capacitors_Exec_State) & 0x03) << 5;
    usRegInputBuf[20] = reg;
    usRegInputBuf[21] = pSysinfo->Minor_Fault;
    usRegInputBuf[22] = pSysinfo->Serious_Fault;
    usRegInputBuf[23] = pSysinfo->SCRT_Fault;
    usRegInputBuf[24] = (pSysinfo->VTx_A_Breakdown_Fault);
    usRegInputBuf[25] = (pSysinfo->VTx_B_Breakdown_Fault);
    usRegInputBuf[26] = (pSysinfo->VTx_C_Breakdown_Fault);
    reg = 0;
    reg |= ((pSysinfo->KM1_Fault) & 0x0F) << 0;
    reg |= ((pSysinfo->QF_Fault) & 0x0F) << 4;
    usRegInputBuf[27] = reg;
    reg = 0;
    reg |= ((pSysinfo->QF_FB) & 0x03) << 0;
    reg |= ((pSysinfo->QS1_FB) & 0x03) << 2;
    reg |= ((pSysinfo->QS2_FB) & 0x03) << 4;
    reg |= ((pSysinfo->KM1_FB) & 0x03) << 6;
    usRegInputBuf[28] = reg;
    reg = 0;
    BFL_VCB_STATE_t state = BFL_VCB_Unknow;
    state = BFL_VCB_Get_Setting_State(QF_SW);
    reg |= ((state) & 0x03) << 0;
    state = BFL_VCB_Get_Setting_State(QS1_SW);
    reg |= ((state) & 0x03) << 2;
    state = BFL_VCB_Get_Setting_State(QS2_SW);
    reg |= ((state) & 0x03) << 4;
    state = BFL_VCB_Get_Setting_State(KM1_SW);
    usRegInputBuf[29] = reg;
    usRegInputBuf[30] = pSysinfo->UxAB_OV_Fault;
    usRegInputBuf[31] = pSysinfo->powerOnTimes;
    reg = 0;
    reg |= ((pSysinfo->capTempSensorTransmitConnect) & 0x01) << 0;
    reg |= ((pSysinfo->capTempSensorFault[Tc_A_IDX]) & 0x01) << 8;
    reg |= ((pSysinfo->capTempSensorFault[Tc_B_IDX]) & 0x01) << 9;
    reg |= ((pSysinfo->capTempSensorFault[Tc_C_IDX]) & 0x01) << 10;
    usRegInputBuf[32] = reg;
    reg = 0;
    reg = XIN1_8_Read();
    usRegInputBuf[33] = reg;
    reg = 0;
    reg = XOUT1_4_Read();
    usRegInputBuf[34] = reg;
    usRegInputBuf[35] = FLOAT_TO_UINT16_SCALE(pSysinfo->VL1rms_TV1A, 100);
    usRegInputBuf[36] = FLOAT_TO_UINT16_SCALE(pSysinfo->VL1rms_TV1B, 100);
    usRegInputBuf[37] = FLOAT_TO_UINT16_SCALE(pSysinfo->VL1rms_TV1C, 100);
    usRegInputBuf[38] = FLOAT_TO_UINT16_SCALE(pSysinfo->VL1rms_UIAB, 100);
    usRegInputBuf[39] = FLOAT_TO_UINT16_SCALE(pSysinfo->VL1rms_UOAB, 100);
    usRegInputBuf[40] = FLOAT_TO_UINT16_SCALE(pSysinfo->VL1rms_TA1A, 100);
    usRegInputBuf[41] = FLOAT_TO_UINT16_SCALE(pSysinfo->VL1rms_TA1B, 100);
    usRegInputBuf[42] = FLOAT_TO_UINT16_SCALE(pSysinfo->VL1rms_TA1C, 100);
    usRegInputBuf[43] = 0;
    usRegInputBuf[44] = 0;

    float *AdcVoltRMS_Filted = &g_pSysInfo->measure.AdcVoltRMS_Filted[0];
    usRegInputBuf[45] = FLOAT_TO_UINT16_SCALE(AdcVoltRMS_Filted[V_TV1A_ADC_IDX], 10000);
    usRegInputBuf[46] = FLOAT_TO_UINT16_SCALE(AdcVoltRMS_Filted[V_TV1B_ADC_IDX], 10000);
    usRegInputBuf[47] = FLOAT_TO_UINT16_SCALE(AdcVoltRMS_Filted[V_TV1C_ADC_IDX], 10000);
    usRegInputBuf[48] = FLOAT_TO_UINT16_SCALE(AdcVoltRMS_Filted[V_UIAB_ADC_IDX], 10000);
    usRegInputBuf[49] = FLOAT_TO_UINT16_SCALE(AdcVoltRMS_Filted[V_UOAB_ADC_IDX], 10000);
    usRegInputBuf[50] = FLOAT_TO_UINT16_SCALE(AdcVoltRMS_Filted[I_TA1A_ADC_IDX], 10000);
    usRegInputBuf[51] = FLOAT_TO_UINT16_SCALE(AdcVoltRMS_Filted[I_TA1B_ADC_IDX], 10000);
    usRegInputBuf[52] = FLOAT_TO_UINT16_SCALE(AdcVoltRMS_Filted[I_TA1C_ADC_IDX], 10000);
    usRegInputBuf[53] = 0;
    usRegInputBuf[54] = 0;
}

/**
 * @brief 读取Modbus寄存器的值，同步到系统信息中。
 *
 */
void SyncModbusRegToSysinfo()
{
    SysInfo_t *pSysinfo = g_pSysInfo;

    pSysinfo->TV1A_ScaleL1 = (float)usRegHoldingBuf[0] * 0.1f;
    pSysinfo->TV1B_ScaleL1 = (float)usRegHoldingBuf[1] * 0.1f;
    pSysinfo->TV1C_ScaleL1 = (float)usRegHoldingBuf[2] * 0.1f;
    pSysinfo->UIAB_ScaleL1 = (float)usRegHoldingBuf[3] * 0.1f;
    pSysinfo->UOAB_ScaleL1 = (float)usRegHoldingBuf[4] * 0.1f;
    pSysinfo->TA1A_ScaleL1 = (float)usRegHoldingBuf[5] * 0.001f;
    pSysinfo->TA1B_ScaleL1 = (float)usRegHoldingBuf[6] * 0.001f;
    pSysinfo->TA1C_ScaleL1 = (float)usRegHoldingBuf[7] * 0.001f;
    pSysinfo->TV1A_ScaleL2 = (float)usRegHoldingBuf[8] * 0.001f;
    pSysinfo->TV1B_ScaleL2 = (float)usRegHoldingBuf[9] * 0.001f;
    pSysinfo->TV1C_ScaleL2 = (float)usRegHoldingBuf[10] * 0.001f;
    pSysinfo->UIAB_ScaleL2 = (float)usRegHoldingBuf[11] * 0.001f;
    pSysinfo->UOAB_ScaleL2 = (float)usRegHoldingBuf[12] * 0.001f;
    pSysinfo->TA1A_ScaleL2 = (float)usRegHoldingBuf[13] * 0.001f;
    pSysinfo->TA1B_ScaleL2 = (float)usRegHoldingBuf[14] * 0.001f;
    pSysinfo->TA1C_ScaleL2 = (float)usRegHoldingBuf[15] * 0.001f;

    pSysinfo->V_SYS_STOP_kV = USING_TWO_REG_TO_FLOAT(usRegHoldingBuf, 16, 0.001f);
    pSysinfo->V_SYS_UNDER_kV = USING_TWO_REG_TO_FLOAT(usRegHoldingBuf, 18, 0.001f);
    pSysinfo->V_SYS_THH_kV = USING_TWO_REG_TO_FLOAT(usRegHoldingBuf, 20, 0.001f);
    pSysinfo->V_SYS_OV_kV = USING_TWO_REG_TO_FLOAT(usRegHoldingBuf, 22, 0.001f);
    pSysinfo->T_SYS_UNDER_CANCLE_SEC = usRegHoldingBuf[24];
    pSysinfo->I_TA_low_thl_A = USING_TWO_REG_TO_FLOAT(usRegHoldingBuf, 25, 0.001f);
    pSysinfo->I_TA_low_thh_A = USING_TWO_REG_TO_FLOAT(usRegHoldingBuf, 27, 0.001f);
    pSysinfo->T_I_TA_Thh_SEC = usRegHoldingBuf[29];
    pSysinfo->I_TA_oc_A = USING_TWO_REG_TO_FLOAT(usRegHoldingBuf, 30, 0.001f);
    pSysinfo->T_I_TA_oc_SEC = usRegHoldingBuf[32];
    pSysinfo->V_TVx_ov_kV = (float)usRegHoldingBuf[33] * 0.001f;
    pSysinfo->T_V_TVx_ov_min = usRegHoldingBuf[34];
    pSysinfo->T_Tc_ot_SEC = usRegHoldingBuf[35];
    pSysinfo->T_Tc_ot_SEC = usRegHoldingBuf[36];
    pSysinfo->T1_MS = usRegHoldingBuf[37];
    pSysinfo->T2_US = usRegHoldingBuf[38];
    pSysinfo->T3_MS = usRegHoldingBuf[39];
    pSysinfo->T4_MS = usRegHoldingBuf[40];
    pSysinfo->V_SCR_NORMAL_DIFF_kV = (float)usRegHoldingBuf[41] * 0.001f;
    pSysinfo->T_V_SCR_ABNORMAL_DIFF_SEC = usRegHoldingBuf[42];
    pSysinfo->I_SCR_NORMAL_DIFF_A = (float)usRegHoldingBuf[43] * 0.001f;
    pSysinfo->devId = (((uint32_t)usRegHoldingBuf[44] << 16) | (uint32_t)usRegHoldingBuf[45]);
    pSysinfo->devType = usRegHoldingBuf[46];

    pSysinfo->T_SYS_SATIFY_CAPACITORS_WAORK_SEC = usRegHoldingBuf[52];

    // 指令解析
    uint16_t cmdReg = usRegHoldingBuf[49];
    usRegHoldingBuf[49] = 0;
    MBCmdHandler(cmdReg);
}

#include "BFL_VCB.h"
#include "BFL_SCR.h"
#include "BFL_Buzz.h"
void MBCmdHandler(uint16_t cmdReg)
{
/*
无指令	0
控制电容器投入	1
控制电容器切除	2
控制电容器旁路	3
单独控制QF闭合	4
单独控制KM1闭合	5
单独控制QS1闭合	6
单独控制QS2闭合	7
单此发送晶闸管导通指令	8
清除严重故障	9
打开蜂鸣器	10
关闭蜂鸣器	11
清除Flash保存的参数	12
*/
#define MB_CMD_NONE 0
#define MB_SRC_SCRT_PLUSE_TRANSMIT 8
#define MB_CMD_SERIOUS_FAUIL_CLEAR 9
#define MB_CMD_OPEN_BUZZ 10
#define MB_CMD_CLOSE_BUZZ 11
#define MB_CMD_ERASE_FLASH_PARAM_DATA 12

    switch (cmdReg)
    {
    case MB_SRC_SCRT_PLUSE_TRANSMIT:
        BFL_SCRT_Pluse_Transmit(SCRT_ALL, 50, US(g_pSysInfo->T2_US));
        break;
    case MB_CMD_SERIOUS_FAUIL_CLEAR:
        APP_Main_Clear_All_Fault();
        break;
    case MB_CMD_OPEN_BUZZ:
        BFL_Buzz_On();
        break;
    case MB_CMD_CLOSE_BUZZ:
        BFL_Buzz_Off();
        break;
    case MB_CMD_ERASE_FLASH_PARAM_DATA:
        APP_Main_EraseFlashParamData();
        break;
    default:
        break;
    }
}
