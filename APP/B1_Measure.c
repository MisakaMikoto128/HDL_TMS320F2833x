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
#include "mtime.h"
#include "period_query.h"
#include "BFL_Measure.h"
#include "HDL_CPU_Time.h"
#include "HDL_Uart.h"
#include "APP_Main.h"
#include <math.h>
static UsTimer_t g_b1_measure_init_moment = 0;

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

    g_b1_measure_init_moment = HDL_CPU_Time_GetUsTick();
}

void B1_Measure_Read(B1_Measure_t *pMeasure)
{
    BFL_Measure_t measure;
    BFL_Measure_Read(&measure);
    pMeasure->V_TV1A = g_pSysInfo->TV1A_ScaleL1 * measure.AdcVoltRMS_Filted_Chunk[V_TV1A_ADC_IDX] * g_pSysInfo->TV1A_ScaleL2  * 0.001f;
    pMeasure->V_TV1B = g_pSysInfo->TV1B_ScaleL1 * measure.AdcVoltRMS_Filted_Chunk[V_TV1B_ADC_IDX] * g_pSysInfo->TV1B_ScaleL2  * 0.001f;
    pMeasure->V_TV1C = g_pSysInfo->TV1C_ScaleL1 * measure.AdcVoltRMS_Filted_Chunk[V_TV1C_ADC_IDX] * g_pSysInfo->TV1C_ScaleL2  * 0.001f;
    pMeasure->V_UIAB = g_pSysInfo->UIAB_ScaleL1 * measure.AdcVoltRMS_Filted_Chunk[V_UIAB_ADC_IDX] * g_pSysInfo->UIAB_ScaleL2  * 0.001f;
    pMeasure->V_UOAB = g_pSysInfo->UOAB_ScaleL1 * measure.AdcVoltRMS_Filted_Chunk[V_UOAB_ADC_IDX] * g_pSysInfo->UOAB_ScaleL2  * 0.001f;
    pMeasure->I_TA1A = g_pSysInfo->TA1A_ScaleL1 * measure.AdcVoltRMS_Filted_Chunk[I_TA1A_ADC_IDX] * g_pSysInfo->TA1A_ScaleL2 ;
    pMeasure->I_TA1B = g_pSysInfo->TA1B_ScaleL1 * measure.AdcVoltRMS_Filted_Chunk[I_TA1B_ADC_IDX] * g_pSysInfo->TA1B_ScaleL2 ;
    pMeasure->I_TA1C = g_pSysInfo->TA1C_ScaleL1 * measure.AdcVoltRMS_Filted_Chunk[I_TA1C_ADC_IDX] * g_pSysInfo->TA1C_ScaleL2 ;
}

void B1_Measure_Poll()
{
    if (HDL_CPU_Time_GetUsTick() - g_b1_measure_init_moment > SEC_TO_US(2))
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

            g_pSysInfo->V_TV1A = g_pSysInfo->VL1rms_TV1A * g_pSysInfo->TV1A_ScaleL2 * 0.001f;
            g_pSysInfo->V_TV1B = g_pSysInfo->VL1rms_TV1B * g_pSysInfo->TV1B_ScaleL2 * 0.001f;
            g_pSysInfo->V_TV1C = g_pSysInfo->VL1rms_TV1C * g_pSysInfo->TV1C_ScaleL2 * 0.001f;
            g_pSysInfo->V_UIAB = g_pSysInfo->VL1rms_UIAB * g_pSysInfo->UIAB_ScaleL2 * 0.001f;
            g_pSysInfo->V_UOAB = g_pSysInfo->VL1rms_UOAB * g_pSysInfo->UOAB_ScaleL2 * 0.001f;
            g_pSysInfo->I_TA1A = g_pSysInfo->VL1rms_TA1A * g_pSysInfo->TA1A_ScaleL2;
            g_pSysInfo->I_TA1B = g_pSysInfo->VL1rms_TA1B * g_pSysInfo->TA1B_ScaleL2;
            g_pSysInfo->I_TA1C = g_pSysInfo->VL1rms_TA1C * g_pSysInfo->TA1C_ScaleL2;
        }
    }
}

float getI_TA1_MAX()
{
    float I_TA1_MAX = 0;
    I_TA1_MAX = fmaxf(g_pSysInfo->I_TA1A, g_pSysInfo->I_TA1B);
    I_TA1_MAX = fmaxf(I_TA1_MAX, g_pSysInfo->I_TA1C);
    return I_TA1_MAX;
}

float getV_TV1x_MAX()
{
    float V_TV1x_MAX = 0;
    V_TV1x_MAX = fmaxf(g_pSysInfo->V_TV1A, g_pSysInfo->V_TV1B);
    V_TV1x_MAX = fmaxf(V_TV1x_MAX, g_pSysInfo->V_TV1C);
    return V_TV1x_MAX;
}
