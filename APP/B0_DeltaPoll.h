/**
 * @file B0_DeltaPoll.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-20
 * @last modified 2024-05-20
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#ifndef B0_DELTAPOLL_H
#define B0_DELTAPOLL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "HDL_CPU_Time.h"

#define B0_DeltaPoll(poll_delta, statement)             \
    do                                                  \
    {                                                   \
        static int poll_stage = 0;                      \
        static uint32_t last_poll_time = 0;             \
        if (poll_stage == 0)                            \
        {                                               \
            last_poll_time = HDL_CPU_Time_GetTick();    \
            poll_stage = 1;                             \
        }                                               \
        else                                            \
        {                                               \
            uint32_t now = HDL_CPU_Time_GetTick();      \
            uint32_t poll_delta = now - last_poll_time; \
            if (poll_delta == 0)                        \
            {                                           \
                break;                                 \
            }                                           \
            last_poll_time = now;                       \
            {                                           \
                statement                               \
            }                                           \
        }                                               \
    } while (0)

typedef struct tagB0_DeltaPoll_t
{
    int poll_stage;
    uint32_t last_poll_time;
} B0_DeltaPoll_t;

#define B0_DeltaPoll_User(pDelatPoll, poll_delta, statement) \
    do                                                       \
    {                                                        \
        if ((pDelatPoll)->poll_stage == 0)                   \
        {                                                    \
            (pDelatPoll)->last_poll_time = HDL_CPU_Time_GetTick(); \
            (pDelatPoll)->poll_stage = 1;                    \
        }                                                    \
        else                                                 \
        {                                                    \
            uint32_t now = HDL_CPU_Time_GetTick();           \
            uint32_t poll_delta = now - (pDelatPoll)->last_poll_time; \
            if (poll_delta == 0)                             \
            {                                                \
                break;                                      \
            }                                                \
            (pDelatPoll)->last_poll_time = now;              \
            {                                                \
                statement                                    \
            }                                                \
        }                                                    \
    } while (0)
#ifdef __cplusplus
}
#endif
#endif //! B0_DELTAPOLL_H
