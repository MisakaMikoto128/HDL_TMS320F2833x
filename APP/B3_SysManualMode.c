/**
 * @file B3_SysManualMode.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-05-17
 * @last modified 2024-05-17
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */

#include "APP_Main.h"

void B3_SysManualMode_DeltaPoll(uint32_t poll_delta)
{
    B3_Check_Minor_Fault_Exist(poll_delta);
    B3_Check_SCR_Serious_Fault(poll_delta);
}
