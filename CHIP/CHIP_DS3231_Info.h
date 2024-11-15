/**
 * @file CHIP_DS3231_Info.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-11-02
 * @last modified 2024-11-02
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#ifndef CHIP_DS3231_INFO_H
#define CHIP_DS3231_INFO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

#define DS3231_I2C_ADDR                 0xD0        /**< iic device address */ 

#define DS3231_REG_SECOND               0x00        /**< second register */
#define DS3231_REG_MINUTE               0x01        /**< minute register */
#define DS3231_REG_HOUR                 0x02        /**< hour register */
#define DS3231_REG_WEEK                 0x03        /**< week register */
#define DS3231_REG_DATE                 0x04        /**< date register */
#define DS3231_REG_MONTH                0x05        /**< month register */
#define DS3231_REG_YEAR                 0x06        /**< year register */
#define DS3231_REG_ALARM1_SECOND        0x07        /**< alarm1 second register */
#define DS3231_REG_ALARM1_MINUTE        0x08        /**< alarm1 minute register */
#define DS3231_REG_ALARM1_HOUR          0x09        /**< alarm1 hour register */
#define DS3231_REG_ALARM1_WEEK          0x0A        /**< alarm1 week register */
#define DS3231_REG_ALARM2_MINUTE        0x0B        /**< alarm2 minute register */
#define DS3231_REG_ALARM2_HOUR          0x0C        /**< alarm2 hour register */
#define DS3231_REG_ALARM2_WEEK          0x0D        /**< alarm2 week register */
#define DS3231_REG_CONTROL              0x0E        /**< control register */
#define DS3231_REG_STATUS               0x0F        /**< status register */
#define DS3231_REG_XTAL                 0x10        /**< xtal register */
#define DS3231_REG_TEMPERATUREH         0x11        /**< temperature high register */
#define DS3231_REG_TEMPERATUREL         0x12        /**< temperature low register */
#define DS3231_BASE_YEAR                2000
#define DS3231_Century_SetBitC          0x80

typedef enum
{
    DS3231_ALARM_1 = 0x00,        /**< alarm 1 */
    DS3231_ALARM_2 = 0x01,        /**< alarm 2 */
} DS3231_Alarm_t;

typedef enum
{
    DS3231_AM = 0x00,        /**< am */
    DS3231_PM = 0x01,        /**< pm */
} DS3231_AmPm_t;

typedef enum
{
    DS3231_PIN_SQUARE_WAVE = 0x00,        /**< square wave pin */
    DS3231_PIN_INTERRUPT   = 0x01,        /**< interrupt pin */
} DS3231_PinType_t;

typedef enum
{
    DS3231_FORMAT_12H = 0x01,        /**< 12h format */
    DS3231_FORMAT_24H = 0x00,        /**< 24h format */
} DS3231_HourFormat_t;

typedef enum
{
    DS3231_STATUS_ALARM_2 = (1 << 1),        /**< alarm 2 status */
    DS3231_STATUS_ALARM_1 = (1 << 0),        /**< alarm 1 status */
} DS3231_AlarmStatus_t;

typedef enum
{
    DS3231_ALARM1_MODE_ONCE_A_SECOND                 = 0x0F,        /**< interrupt once a second */
    DS3231_ALARM1_MODE_SECOND_MATCH                  = 0x0E,        /**< interrupt second match */
    DS3231_ALARM1_MODE_MINUTE_SECOND_MATCH           = 0x0C,        /**< interrupt minute second match */
    DS3231_ALARM1_MODE_HOUR_MINUTE_SECOND_MATCH      = 0x08,        /**< interrupt hour minute second match */
    DS3231_ALARM1_MODE_DATE_HOUR_MINUTE_SECOND_MATCH = 0x00,        /**< interrupt date hour minute second match */
    DS3231_ALARM1_MODE_WEEK_HOUR_MINUTE_SECOND_MATCH = 0x10,        /**< interrupt week hour minute second match */
} DS3231_Alarm1Mode_t;

typedef enum
{
    DS3231_ALARM2_MODE_ONCE_A_MINUTE                = 0x07,        /**< interrupt once a minute */
    DS3231_ALARM2_MODE_MINUTE_MATCH                 = 0x06,        /**< interrupt minute match */
    DS3231_ALARM2_MODE_HOUR_MINUTE_MATCH            = 0x04,        /**< interrupt hour minute match */
    DS3231_ALARM2_MODE_DATE_HOUR_MINUTE_MATCH       = 0x00,        /**< interrupt data hour minute match */
    DS3231_ALARM2_MODE_WEEK_HOUR_MINUTE_MATCH       = 0x10,        /**< interrupt week hour minute match */
} DS3231_Alarm2Mode_t;

#ifdef __cplusplus
}
#endif
#endif //! CHIP_DS3231_INFO_H
