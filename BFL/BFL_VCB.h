/**
 * @file BFL_VCB.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-13
 * @last modified 2024-05-13
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#ifndef BFL_BFL_VCB_H
#define BFL_BFL_VCB_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
    // VCB: Vacuum Circuit Breaker 真空断路器
    typedef enum
    {
        QF_SW = 0,
        QS1_SW,
        QS2_SW,
        KM1_SW,
        RES_SW,
        VCB_SW_NUM,
        VCB_NONE,
    } BFL_VCB_SW_t;

#define for_Each_VCB_SW_t(vcb) for (BFL_VCB_SW_t vcb = QF_SW; vcb < VCB_SW_NUM; vcb++)

    const char *BFL_VCB_SW_To_String(BFL_VCB_SW_t vcb);

    typedef enum
    {
        BFL_VCB_Opened = 0,
        BFL_VCB_Closed,
        BFL_VCB_Unknow,
    } BFL_VCB_STATE_t;

    const char *BFL_VCB_STATE_To_String(BFL_VCB_STATE_t state);

    typedef enum
    {
        BFL_VCB_No_Ctrl = 0,
        BFL_VCB_Ctrl_Open,
        BFL_VCB_Ctrl_Close,
    } BFL_VCB_CTRL_STATE_t;

    const char *BFL_VCB_CTRL_STATE_To_String(BFL_VCB_CTRL_STATE_t state);

    /**
     * @brief 初始化真空断路控制继电器，使其处于无控制的状态。
     *
     */
    void BFL_VCB_Seurity_Init();

    /**
     * @brief 获取真空断路器的控制继电器的已经执行完成的状态。
     *
     * @param vcb
     * @return BFL_VCB_STATE_t
     */
    BFL_VCB_CTRL_STATE_t BFL_VCB_Relay_Get_Setting_State(BFL_VCB_SW_t vcb);

    /**
     * @brief 设置真空断路器为使真空断路器闭合。
     *
     * @param vcb
     */
    void BFL_VCB_Set_As_Switch_Closed(BFL_VCB_SW_t vcb);

    /**
     * @brief 设置真空断路器为使真空断路器断开。
     *
     * @param vcb
     */
    void BFL_VCB_Set_As_Switch_Opened(BFL_VCB_SW_t vcb);

    /**
     * @brief 设置真空断路器为使真空断路器处于无控制状态。
     *
     * @param vcb
     */
    void BFL_VCB_Set_As_Switch_No_Ctrl(BFL_VCB_SW_t vcb);

    /**
     * @brief 获取真空断路器的已经执行完成的设置状态。
     *
     * @param vcb
     * @return BFL_VCB_STATE_t
     */
    BFL_VCB_STATE_t BFL_VCB_Get_Setting_State(BFL_VCB_SW_t vcb);

    /**
     * @brief 获取真空断路器的实际状态。
     *
     * @param vcb
     * @return BFL_VCB_STATE_t
     */
    BFL_VCB_STATE_t BFL_VCB_Get_Actual_State(BFL_VCB_SW_t vcb);


#define BFL_VBC_NO_FAULT 0
#define BFL_VBC_CANT_OPEN 1
#define BFL_VBC_CANT_CLOSE 2
#ifdef __cplusplus
}
#endif
#endif //! BFL_BFL_VCB_H
