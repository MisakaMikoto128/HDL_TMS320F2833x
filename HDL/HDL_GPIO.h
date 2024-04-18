/**
 * @file HDL_GPIO.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-04-17
 * @last modified 2024-04-17
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#ifndef HDL_GPIO_H
#define HDL_GPIO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

    enum HDL_GPIO_MODE
    {
        HDL_GPIO_MODE_INPUT = 0,
        HDL_GPIO_MODE_OUTPUT,
        HDL_GPIO_MODE_ANALOG,
        HDL_GPIO_MODE_AF,
    };

    enum HDL_GPIO_PIN
    {
        HDL_GPIO_PIN_0 = 0,
        HDL_GPIO_PIN_1,
        HDL_GPIO_PIN_2,
        HDL_GPIO_PIN_3,
        HDL_GPIO_PIN_4,
        HDL_GPIO_PIN_5,
        HDL_GPIO_PIN_6,
        HDL_GPIO_PIN_7,
        HDL_GPIO_PIN_8,
        HDL_GPIO_PIN_9,
        HDL_GPIO_PIN_10,
        HDL_GPIO_PIN_11,
        HDL_GPIO_PIN_12,
        HDL_GPIO_PIN_13,
        HDL_GPIO_PIN_14,
        HDL_GPIO_PIN_15,
        HDL_GPIO_PIN_16,
        HDL_GPIO_PIN_17,
        HDL_GPIO_PIN_18,
        HDL_GPIO_PIN_19,
        HDL_GPIO_PIN_21,
        HDL_GPIO_PIN_22,
        HDL_GPIO_PIN_23,
        HDL_GPIO_PIN_24,
        HDL_GPIO_PIN_25,
        HDL_GPIO_PIN_26,
        HDL_GPIO_PIN_27,
        HDL_GPIO_PIN_28,
        HDL_GPIO_PIN_29,
        HDL_GPIO_PIN_30,
        HDL_GPIO_PIN_31,
        HDL_GPIO_PIN_NUM,
    };

    enum HDL_GPIO_Port
    {
        HDL_GPIO_A = 0,
        HDL_GPIO_B,
        HDL_GPIO_C,
    };

    typedef enum
    {
        HDL_GPIO_LOW = 0,
        HDL_GPIO_HIGH,
    } HDL_GPIO_PinState_t;

    void HDL_GPIO_SetMode(enum HDL_GPIO_Port port, enum HDL_GPIO_PIN pin, enum HDL_GPIO_MODE mode);
    void HDL_GPIO_Write(enum HDL_GPIO_Port port, enum HDL_GPIO_PIN pin, HDL_GPIO_PinState_t value);
    void HDL_GPIO_Toggle(enum HDL_GPIO_Port port, enum HDL_GPIO_PIN pin);
    HDL_GPIO_PinState_t HDL_GPIO_Read(enum HDL_GPIO_Port port, enum HDL_GPIO_PIN pin);
#ifdef __cplusplus
}
#endif
#endif //! HDL_GPIO_H
