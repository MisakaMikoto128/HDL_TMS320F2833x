/**
 * @file B1_Measure.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 * @last modified 2024-05-16
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "ccommon.h"
#include "period_query.h"
#include "BFL_Measure.h"
#include "HDL_CPU_Time.h"
#include "HDL_Uart.h"
#include "APP_Main.h"

void B1_Measure_Init()
{
    BFL_Measure_Init();

    g_pSysInfo->VL1rms_TV1A = 0;
    g_pSysInfo->VL1rms_TV1B = 0;
    g_pSysInfo->VL1rms_TV1C = 0;
    g_pSysInfo->VL1rms_UIAB = 0;
    g_pSysInfo->VL1rms_UOAB = 0;
    g_pSysInfo->VL1rms_TA1A = 0;
    g_pSysInfo->VL1rms_TA1B = 0;
    g_pSysInfo->VL1rms_TA1C = 0;
    g_pSysInfo->V_TV1A = 0;
    g_pSysInfo->V_TV1B = 0;
    g_pSysInfo->V_TV1C = 0;
    g_pSysInfo->V_UIAB = 0;
    g_pSysInfo->V_UOAB = 0;
    g_pSysInfo->I_TA1A = 0;
    g_pSysInfo->I_TA1B = 0;
    g_pSysInfo->I_TA1C = 0;
}

void B1_Measure_Poll()
{
    if (BFL_Measure_ReadReady())
    {
        BFL_Measure_Read(&g_pSysInfo->measure);
        BFL_Measure_t *pMeasure = &g_pSysInfo->measure;
        g_pSysInfo->VL1rms_TV1A = g_pSysInfo->TV1A_ScaleL1 * pMeasure->AdcVoltRMS_Filted_Chunk[V_TV1A_ADC_IDX];
        g_pSysInfo->VL1rms_TV1B = g_pSysInfo->TV1B_ScaleL1 * pMeasure->AdcVoltRMS_Filted_Chunk[V_TV1B_ADC_IDX];
        g_pSysInfo->VL1rms_TV1C = g_pSysInfo->TV1C_ScaleL1 * pMeasure->AdcVoltRMS_Filted_Chunk[V_TV1C_ADC_IDX];
        g_pSysInfo->VL1rms_UIAB = g_pSysInfo->UIAB_ScaleL1 * pMeasure->AdcVoltRMS_Filted_Chunk[V_UIAB_ADC_IDX];
        g_pSysInfo->VL1rms_UOAB = g_pSysInfo->UOAB_ScaleL1 * pMeasure->AdcVoltRMS_Filted_Chunk[V_UOAB_ADC_IDX];
        g_pSysInfo->VL1rms_TA1A = g_pSysInfo->TA1A_ScaleL1 * pMeasure->AdcVoltRMS_Filted_Chunk[I_TA1A_ADC_IDX];
        g_pSysInfo->VL1rms_TA1B = g_pSysInfo->TA1B_ScaleL1 * pMeasure->AdcVoltRMS_Filted_Chunk[I_TA1B_ADC_IDX];
        g_pSysInfo->VL1rms_TA1C = g_pSysInfo->TA1C_ScaleL1 * pMeasure->AdcVoltRMS_Filted_Chunk[I_TA1C_ADC_IDX];

        g_pSysInfo->V_TV1A = g_pSysInfo->VL1rms_TV1A * g_pSysInfo->TV1A_ScaleL2;
        g_pSysInfo->V_TV1B = g_pSysInfo->VL1rms_TV1B * g_pSysInfo->TV1B_ScaleL2;
        g_pSysInfo->V_TV1C = g_pSysInfo->VL1rms_TV1C * g_pSysInfo->TV1C_ScaleL2;
        g_pSysInfo->V_UIAB = g_pSysInfo->VL1rms_UIAB * g_pSysInfo->UIAB_ScaleL2;
        g_pSysInfo->V_UOAB = g_pSysInfo->VL1rms_UOAB * g_pSysInfo->UOAB_ScaleL2;
        g_pSysInfo->I_TA1A = g_pSysInfo->VL1rms_TA1A * g_pSysInfo->TA1A_ScaleL2;
        g_pSysInfo->I_TA1B = g_pSysInfo->VL1rms_TA1B * g_pSysInfo->TA1B_ScaleL2;
        g_pSysInfo->I_TA1C = g_pSysInfo->VL1rms_TA1C * g_pSysInfo->TA1C_ScaleL2;
    }
}
