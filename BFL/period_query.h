/**
 * @file period_query.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 * @last modified 2024-05-16
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#ifndef PERIOD_QUERY_H
#define PERIOD_QUERY_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

    typedef uint32_t PeriodREC_t;
#define MAX_PERIOD_ID 10 // 最大的周期ID号，从0开始计数。

    /**
     * @brief 查询是否到了需要的周期。这个函数中高速查询，如果判断周期到了，就会
     * 返回true，否则返回false,并且当周期到了之后会更新last_exe_tick，保证每周期只会判
     * 断结果为真一次。用于在主循环中方便的构建周期性执行的代码段。
     *
     * 内置一个PeriodREC_t的最后一次执行时间的时间戳表，period_id标识。
     * @param period_id 周期id，全局唯一。
     * @param period 周期。
     * @return true 周期到了
     * @return false 周期未到。
     */
    bool period_query(uint16_t period_id, PeriodREC_t period);
    /**
     * @brief 同period_query_user，只是时间记录再一个uint32_t*指向的变量中。
     *
     * @param period_recorder 记录运行时间的变量的指针。
     * @param period 周期。
     * @return true 周期到了
     * @return false 周期未到。
     */
    bool period_query_user(PeriodREC_t *period_recorder, PeriodREC_t period);
    bool period_query_user_us(PeriodREC_t *period_recorder, PeriodREC_t period);

    typedef struct
    {
        PeriodREC_t start; // 记录时间
        bool isFinished;   // 是否完成
        bool isStarted;    // 是否开始
    } DelayREC_t;

    /**
     * @brief 从系统开始运行，是否经过了delay时间。
     *
     * @param delay_recorder
     * @param delay
     * @return true 延时条件满足
     * @return false
     */
    bool delay_one_times(DelayREC_t *delay_rec, uint32_t delay);


#define if_period_query(period_id, period) if(period_query(period_id, period))
#define if_period_query_user(period_recorder, period) if(period_query_user(period_recorder, period))
#define if_period_query_user_us(period_recorder, period) if(period_query_user_us(period_recorder, period))
#define if_delay_one_times(delay_rec, delay) if(delay_one_times(delay_rec, delay))
#ifdef __cplusplus
}
#endif
#endif //! PERIOD_QUERY_H
