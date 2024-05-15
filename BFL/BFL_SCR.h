/**
 * @file BFL_SCR.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-14
 * @last modified 2024-05-14
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#ifndef BFL_SCR_H
#define BFL_SCR_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

    /*
    SCR:
    SCRT: 光纤信号发送通道
    SCRR: 光纤信号接收通道
    */
    typedef enum
    {
        SCRTA = 0,
        SCRTB,
        SCRTC,
        SCRT_NUM,
        SCRT_ALL,
    } BFL_SCRT_t;

    typedef enum
    {
        SCRR1A = 0,
        SCRR1B,
        SCRR2A,
        SCRR2B,
        SCRR3A,
        SCRR3B,
        SCRR_NUM,
    } BFL_SCRR_t;

    /**
     * @brief SCR输入输出通道初始化。
     *
     */
    void BFL_SCR_Init();

    /**
     * @brief 输出脉冲，如果当前通道有脉冲正在输出，则会被覆盖。
     *
     * @param scrt 输出通道。
     * @param _uiPluseNum
     * @param _uiPluseWidth 单位us，最大值为5000us。
     */
    void BFL_SCRT_Pluse_Transmit(BFL_SCRT_t scrt, uint16_t _uiPluseNum, uint16_t _uiPluseWidth);

    /**
     * @brief
     *
     * @param scrr 输入通道。
     * @return true 有光信号输入进来。
     * @return false 没有光信号输入进来。
     */
    bool BFL_SCRR_Have_Signal(BFL_SCRR_t scrr);

#ifdef __cplusplus
}
#endif
#endif //! BFL_SCR_H
