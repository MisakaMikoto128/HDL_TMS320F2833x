/**
* @file HDL_CAN.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-05-15
* @last modified 2024-05-15
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef HDL_CAN_H
#define HDL_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "ccommon.h"

typedef enum
{
    CANA = 0,
    CANB,
    CAN_NUM,
} HDL_CAN_t;

/**
 * @brief CAN初始化。
 *
 */
void HDL_CAN_Init(HDL_CAN_t can);

/**
 * @brief CAN发送数据。
 *
 * @param can
 * @param _uiID
 * @param _uiData
 * @param _uiDataLen 数据长度，最大为8。
 */
void HDL_CAN_Send(HDL_CAN_t can, uint32_t _uiID, byte_t *_uiData, byte_t _uiDataLen);


#ifdef __cplusplus
}
#endif
#endif //!HDL_CAN_H
