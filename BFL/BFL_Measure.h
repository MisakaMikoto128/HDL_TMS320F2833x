/**
 * @file BFL_Measure.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-04-23
 * @last modified 2024-04-23
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#ifndef BFL_MEASURE_H
#define BFL_MEASURE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

#define BFL_MEASURE_CHANNEL_NUM 8

    typedef struct
    {
        float AdcVoltRMS[BFL_MEASURE_CHANNEL_NUM];
        float AdcVoltAvg[BFL_MEASURE_CHANNEL_NUM];

        float AdcVoltRMS_Filted[BFL_MEASURE_CHANNEL_NUM];
        float AdcVoltRMS_Filted_Chunk[BFL_MEASURE_CHANNEL_NUM];
        float AdcVoltAvg_Filted[BFL_MEASURE_CHANNEL_NUM];
    } BFL_Measure_t;

    void BFL_Measure_Init();

    /**
     * @brief 判断测量模块是否有数据更新。
     * 读取后会自动清除标志位。
     * 
     * @return true 有新数据。
     * @return false 
     */
    bool BFL_Measure_ReadReady();

    /**
     * @brief 读取测量数据。为了避免读写竞争问题，
     * 内部会关闭中断，所以频繁调用。
     * 
     * @param pMeasure 
     */
    void BFL_Measure_Read(BFL_Measure_t *pMeasure);

    float getI_TA1_MAX();

    float getV_TV1x_MAX();
#ifdef __cplusplus
}
#endif
#endif //! BFL_MEASURE_H
