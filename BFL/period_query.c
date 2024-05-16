/**
 * @file period_query.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 * @last modified 2024-05-16
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "period_query.h"
#include "HDL_CPU_Time.h"
static PeriodREC_t period_last_exe_tick_table[MAX_PERIOD_ID + 1] = {0};

/**
 * @brief 查询是否到了需要的周期。这个函数中高速查询，如果判断周期到了，就会
 * 返回true，否则返回false,并且当周期到了之后会更新last_exe_tick，保证每周期只会判
 * 断结果为真一次。用于在主循环中方便的构建周期性执行的代码段。
 *
 * 内置一个Period_t的最后一次执行时间的时间戳表，period_id标识。
 * @param period_id 周期id，全局唯一。
 * @param period 周期,单位tick。
 * @return true 周期到了
 * @return false 周期未到。
 */
bool period_query(uint16_t period_id, PeriodREC_t period)
{
    bool ret = false;

    // 这里一定是>=，如果是 > ，那么在1 cpu tick间隔的时候时间上是2cpu tick执行一次。
    // 这里不允许period为0，不然就会失去调度作用。
    if (((PeriodREC_t)HDL_CPU_Time_GetTick() - period_last_exe_tick_table[period_id]) >= period)
    {
        period_last_exe_tick_table[period_id] = HDL_CPU_Time_GetTick();
        ret = true;
    }
    return ret;
}

/**
 * @brief 同period_query_user，只是时间记录再一个uint32_t*指向的变量中。
 *
 * @param period_recorder 记录运行时间的变量的指针。
 * @param period 周期,单位tick。
 * @return true 周期到了
 * @return false 周期未到。
 */
bool period_query_user(PeriodREC_t *period_recorder, PeriodREC_t period)
{
    bool ret = false;
    // 这里一定是>=，如果是 > ，那么在1 cpu tick间隔的时候时间上是2cpu tick执行一次。
    // 这里不允许period为0，不然就会失去调度作用。
    if ((HDL_CPU_Time_GetTick() - *period_recorder) >= period)
    {
        *period_recorder = HDL_CPU_Time_GetTick();
        ret = true;
    }
    return ret;
}
