/**
* @file state_duration_timer.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-05-20
* @last modified 2024-05-20
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef STATE_DURATION_TIMER_H
#define STATE_DURATION_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 状态持续时间计数器
 *
 * 该类型指向一个用于跟踪状态条件持续时间的计数器,以毫秒为单位。
 */
typedef uint32_t StateDurationCnt_t;

/**
 * @brief 检查条件是否持续满足了指定的持续时间
 *
 * 该函数用于检查一个条件是否持续满足了指定的持续时间。如果条件成立,则增加计数器的值;
 * 如果计数器达到指定的持续时间阈值,则将计数器重置为0,并返回true表示条件持续满足了指定的持续时间。
 * 如果条件不成立,则不计时，直接将计数器重置为0,并返回false。
 *
 * @param elapsed_time_cnt_ms 状态持续时间计数器的指针,以毫秒为单位
 * @param elapsed_delta_ms 自上次更新以来经过的时间,以毫秒为单位
 * @param duration_ms 状态持续所需的最小持续时间,以毫秒为单位
 * @param condition 状态持续条件
 * @return true 如果条件持续满足了指定的持续时间,则一直返回true，不再计时
 * @return false 如果条件不满足,则返回false，计时清零
 */
bool CheckConditionDurationMet(StateDurationCnt_t *elapsed_time_cnt_ms, uint32_t elapsed_delta_ms, uint32_t duration_ms, bool condition);

#ifdef __cplusplus
}
#endif
#endif //!STATE_DURATION_TIMER_H
