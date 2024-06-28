/**
 * @file B2_EventRecord.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-06-26
 * @last modified 2024-06-26
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "B2_EventRecord.h"
#include "ccommon.h"
#include "cqueue.h"
#include "datetime.h"
#include "CHIP_W25Q128.h"

static inline bool B2_EventRecord_WriteQueuePush(B2_EventRecord_t *eventRecord);
static inline bool B2_EventRecord_WriteQueuePop(B2_EventRecord_t *eventRecord);
static inline bool B2_EventRecord_WriteQueuePeek(B2_EventRecord_t *eventRecord);
static inline bool B2_EventRecord_WriteQueueIsEmpyt();

static uint32_t g_B2_EventRecord_WriteIdx = 0;
static CQueue_t g_B2_EventRecord_WriteQueue;
#define B2_EVENTRECORD_WRITE_QUEUE_CAPACITY 10
static B2_EventRecord_t g_B2_EventRecord_WriteQueueBuffer[B2_EVENTRECORD_WRITE_QUEUE_CAPACITY];
static bool g_B2_EventRecord_WriteQueueEventDisbale[B2_EVENT_RECORD_EVENT_NUM];

#define B2_EVENTRECORD_SECTOR_START 20 // Start Sector ID
#define B2_EVENTRECORD_SECTOR_END (W25Q128_SECTOR_COUNT)

#define B2_EVENTRECORD_ENCODE_SIZE 64 // 这里必须64字节对齐，不然一个扇区存不了整数个记录
static byte_t g_B2_EventRecord_EncodeBuffer[B2_EVENTRECORD_ENCODE_SIZE];
static byte_t g_B2_EventRecord_DecodeBuffer[B2_EVENTRECORD_ENCODE_SIZE];
#define B2_EVENTRECORD_MAX_EVENT_NUM ((B2_EVENTRECORD_SECTOR_END - B2_EVENTRECORD_SECTOR_START) * W25Q128_SECTOR_SIZE / B2_EVENTRECORD_ENCODE_SIZE)

uint32_t g_recordedEventsReadIdx = 0;

void B2_EventRecord_Init()
{
    cqueue_create(&g_B2_EventRecord_WriteQueue, g_B2_EventRecord_WriteQueueBuffer, B2_EVENTRECORD_WRITE_QUEUE_CAPACITY, sizeof(B2_EventRecord_t));
    g_B2_EventRecord_WriteIdx = g_pSysInfo->recordedEventsNum;
    for (uint32_t i = 0; i < B2_EVENT_RECORD_EVENT_NUM; i++)
    {
        g_B2_EventRecord_WriteQueueEventDisbale[i] = true;
    }
}

/**
 * @brief Creates an event record based on the provided system information.
 *
 * @param eventRecord Pointer to an event record to be populated.
 * @param eventCode Event code indicating the type of event.
 * @param sysInfo Pointer to the current system information.
 * @return B2_EventRecord_t* Pointer to the populated event record.
 */
B2_EventRecord_t *B2_EventRecord_Create(B2_EventRecord_t *eventRecord, B2_EventCode_t eventCode, uint32_t eventID, SysInfo_t *sysInfo)
{
    if (eventRecord == NULL || sysInfo == NULL)
    {
        return NULL;
    }

    eventRecord->eventID = eventID;
    eventRecord->eventTimestamp = datetime_get_unix_ms_timestamp();
    eventRecord->eventCode = eventCode;
    eventRecord->eventStatus = ((sysInfo->QF_FB & 0x01) << 7) | ((sysInfo->QS1_FB & 0x01) << 6) |
                               ((sysInfo->QS2_FB & 0x01) << 5) | ((sysInfo->KM1_FB & 0x01) << 4) |
                               ((sysInfo->SYS_MODE & 0x03) << 2) |
                               ((sysInfo->Line_State & 0x01) << 1);

    eventRecord->maxTemperature = (int16_t)getMaxCapTemp();
    eventRecord->minTemperature = (int16_t)getMinCapTemp();

    eventRecord->V_TV1A = (int32_t)(sysInfo->V_TV1A * 10000);
    eventRecord->V_TV1B = (int32_t)(sysInfo->V_TV1B * 10000);
    eventRecord->V_TV1C = (int32_t)(sysInfo->V_TV1C * 10000);
    eventRecord->V_UIAB = (int32_t)(sysInfo->V_UIAB * 10000);
    eventRecord->V_UOAB = (int32_t)(sysInfo->V_UOAB * 10000);
    eventRecord->I_TA1A = (int32_t)(sysInfo->I_TA1A * 10000);
    eventRecord->I_TA1B = (int32_t)(sysInfo->I_TA1B * 10000);
    eventRecord->I_TA1C = (int32_t)(sysInfo->I_TA1C * 10000);

    return eventRecord;
}

bool B2_EventRecord_Write_Generic(B2_EventRecord_t *pEventRecord)
{
    if (pEventRecord == NULL)
    {
        return false;
    }

    if (pEventRecord->eventID >= B2_EVENTRECORD_MAX_EVENT_NUM)
    {
        return false;
    }

    bool ret = B2_EventRecord_WriteQueuePush(pEventRecord);
    if (ret)
    {
        g_B2_EventRecord_WriteIdx++;
    }

    return ret;
}

bool B2_EventRecord_Write(B2_EventCode_t eventCode)
{
    B2_EventRecord_t eventRecord;

    if (g_B2_EventRecord_WriteQueueEventDisbale[eventCode])
    {
        return false;
    }

    if (g_B2_EventRecord_WriteIdx >= B2_EVENTRECORD_MAX_EVENT_NUM)
    {
        return false;
    }

    return B2_EventRecord_Write_Generic(B2_EventRecord_Create(&eventRecord, eventCode, g_B2_EventRecord_WriteIdx, g_pSysInfo));
}

/**
 * @brief Encode a 32-bit unsigned integer into a buffer in little-endian order.
 *
 * @param buffer The buffer to encode into.
 * @param value The 32-bit unsigned integer to encode.
 */
static void encode_uint32(byte_t *buffer, uint32_t value)
{
    buffer[0] = value & 0xFF;
    buffer[1] = (value >> 8) & 0xFF;
    buffer[2] = (value >> 16) & 0xFF;
    buffer[3] = (value >> 24) & 0xFF;
}

/**
 * @brief Encode a 64-bit unsigned integer into a buffer in little-endian order.
 *
 * @param buffer The buffer to encode into.
 * @param value The 64-bit unsigned integer to encode.
 */
static void encode_uint64(byte_t *buffer, uint64_t value)
{
    buffer[0] = value & 0xFF;
    buffer[1] = (value >> 8) & 0xFF;
    buffer[2] = (value >> 16) & 0xFF;
    buffer[3] = (value >> 24) & 0xFF;
    buffer[4] = (value >> 32) & 0xFF;
    buffer[5] = (value >> 40) & 0xFF;
    buffer[6] = (value >> 48) & 0xFF;
    buffer[7] = (value >> 56) & 0xFF;
}

static byte_t *B2_EventRecord_Encode(B2_EventRecord_t *eventRecor, byte_t encodeBuffer[B2_EVENTRECORD_ENCODE_SIZE], uint32_t bufferSize)
{
    if (eventRecor == NULL || encodeBuffer == NULL || bufferSize < B2_EVENTRECORD_ENCODE_SIZE)
    {
        return NULL;
    }

    encode_uint32(encodeBuffer, eventRecor->eventID);
    encode_uint64(&encodeBuffer[4], eventRecor->eventTimestamp);

    encodeBuffer[12] = eventRecor->eventCode;
    encodeBuffer[13] = eventRecor->eventStatus;
    encodeBuffer[14] = eventRecor->maxTemperature;
    encodeBuffer[15] = eventRecor->minTemperature;

    encode_uint32(&encodeBuffer[16], eventRecor->V_TV1A);
    encode_uint32(&encodeBuffer[20], eventRecor->V_TV1B);
    encode_uint32(&encodeBuffer[24], eventRecor->V_TV1C);
    encode_uint32(&encodeBuffer[28], eventRecor->V_UIAB);
    encode_uint32(&encodeBuffer[32], eventRecor->V_UOAB);
    encode_uint32(&encodeBuffer[36], eventRecor->I_TA1A);
    encode_uint32(&encodeBuffer[40], eventRecor->I_TA1B);
    encode_uint32(&encodeBuffer[44], eventRecor->I_TA1C);

    return encodeBuffer;
}

bool B2_EventRecord_Write_RawData(B2_EventRecord_t *eventRecord)
{
    if (eventRecord == NULL)
    {
        return false;
    }

    if (eventRecord->eventID >= B2_EVENTRECORD_MAX_EVENT_NUM)
    {
        return false;
    }

    // Step 1. Encode the event record
    byte_t *encodeBuffer = NULL;
    encodeBuffer = B2_EventRecord_Encode(eventRecord, g_B2_EventRecord_EncodeBuffer, B2_EVENTRECORD_ENCODE_SIZE);

    if (encodeBuffer == NULL)
    {
        return false;
    }

    // Step 2. 检查是第一次写入这个扇区，如果是，那么先擦除这个扇区
    uint32_t address = B2_EVENTRECORD_SECTOR_START * W25Q128_SECTOR_SIZE + eventRecord->eventID * B2_EVENTRECORD_ENCODE_SIZE;
    if (address % W25Q128_SECTOR_SIZE == 0)
    {
        if (!CHIP_W25Q128_Is_Sector_Erased(address / W25Q128_SECTOR_SIZE))
        {
            if (CHIP_W25Q128_Erase_Sector(address / W25Q128_SECTOR_SIZE) != 0)
            {
                return false; // Erase failed
            }
        }
    }

    // Step 3. Write the encoded data to the flash and do not need to erase the sector
    int32_t writtenBytesNum = CHIP_W25Q128_Write_No_Erase(address, g_B2_EventRecord_EncodeBuffer, B2_EVENTRECORD_ENCODE_SIZE);

    // Check if the write was successful
    return writtenBytesNum == B2_EVENTRECORD_ENCODE_SIZE;
}

static void B2_EventRecord_WritePoll()
{
    B2_EventRecord_t eventRecord;
    // 一次最多写入10个事件
    for (size_t i = 0; i < 10; i++)
    {
        if (!B2_EventRecord_WriteQueuePeek(&eventRecord))
        {
            break;
        }

        if (B2_EventRecord_Write_RawData(&eventRecord))
        {
            g_pSysInfo->recordedEventsNum++;
            APP_Main_NotifyHaveParamNeedToSave();
        }

        B2_EventRecord_WriteQueuePop(&eventRecord);
    }
}

/**
 * @brief Decode a 32-bit unsigned integer from a buffer in little-endian order.
 *
 * @param buffer The buffer to decode from.
 * @return The decoded 32-bit unsigned integer.
 */
static uint32_t decode_uint32(const byte_t *buffer)
{
    return (uint32_t)buffer[0] |
           ((uint32_t)buffer[1] << 8) |
           ((uint32_t)buffer[2] << 16) |
           ((uint32_t)buffer[3] << 24);
}

/**
 * @brief Decode a 64-bit unsigned integer from a buffer in little-endian order.
 *
 * @param buffer The buffer to decode from.
 * @return The decoded 64-bit unsigned integer.
 */
static uint64_t decode_uint64(const byte_t *buffer)
{
    return (uint64_t)buffer[0] |
           ((uint64_t)buffer[1] << 8) |
           ((uint64_t)buffer[2] << 16) |
           ((uint64_t)buffer[3] << 24) |
           ((uint64_t)buffer[4] << 32) |
           ((uint64_t)buffer[5] << 40) |
           ((uint64_t)buffer[6] << 48) |
           ((uint64_t)buffer[7] << 56);
}

static B2_EventRecord_t *B2_EventRecord_Decode(B2_EventRecord_t *eventRecord, byte_t decodeBuffer[B2_EVENTRECORD_ENCODE_SIZE], uint32_t bufferSize)
{
    if (eventRecord == NULL || decodeBuffer == NULL || bufferSize < B2_EVENTRECORD_ENCODE_SIZE)
    {
        return NULL;
    }

    eventRecord->eventID = decode_uint32(&decodeBuffer[0]);
    eventRecord->eventTimestamp = decode_uint64(&decodeBuffer[4]);

    eventRecord->eventCode = (B2_EventCode_t)decodeBuffer[12];
    eventRecord->eventStatus = decodeBuffer[13];
    eventRecord->maxTemperature = decodeBuffer[14] & 0xFF;
    eventRecord->minTemperature = decodeBuffer[15] & 0xFF;

    eventRecord->V_TV1A = decode_uint32(&decodeBuffer[16]);
    eventRecord->V_TV1B = decode_uint32(&decodeBuffer[20]);
    eventRecord->V_TV1C = decode_uint32(&decodeBuffer[24]);
    eventRecord->V_UIAB = decode_uint32(&decodeBuffer[28]);
    eventRecord->V_UOAB = decode_uint32(&decodeBuffer[32]);
    eventRecord->I_TA1A = decode_uint32(&decodeBuffer[36]);
    eventRecord->I_TA1B = decode_uint32(&decodeBuffer[40]);
    eventRecord->I_TA1C = decode_uint32(&decodeBuffer[44]);

    return eventRecord;
}

bool B2_EventRecord_Read_RawData(uint32_t eventID, byte_t *decodeBuffer, uint32_t bufferSize)
{
    if (decodeBuffer == NULL || bufferSize < B2_EVENTRECORD_ENCODE_SIZE || eventID >= B2_EVENTRECORD_MAX_EVENT_NUM)
    {
        return false;
    }

    uint32_t address = B2_EVENTRECORD_SECTOR_START * W25Q128_SECTOR_SIZE + eventID * B2_EVENTRECORD_ENCODE_SIZE;
    int32_t readBytesNum = CHIP_W25Q128_Read(address, decodeBuffer, B2_EVENTRECORD_ENCODE_SIZE);

    return readBytesNum == B2_EVENTRECORD_ENCODE_SIZE;
}

bool B2_EventRecord_Read(B2_EventRecord_t *eventRecord, uint32_t eventID)
{
    bool result = false;
    // Step 1. 计算所在地址,读取数据
    result = B2_EventRecord_Read_RawData(eventID, g_B2_EventRecord_DecodeBuffer, B2_EVENTRECORD_ENCODE_SIZE);
    if (!result)
    {
        return false;
    }

    // Step 2. 解码
    B2_EventRecord_t *pEventRecord = B2_EventRecord_Decode(eventRecord, g_B2_EventRecord_DecodeBuffer, B2_EVENTRECORD_ENCODE_SIZE);

    return pEventRecord != NULL;
}

void B2_EventRecord_Poll()
{
    B2_EventRecord_WritePoll();
}

static inline bool B2_EventRecord_WriteQueuePush(B2_EventRecord_t *eventRecord)
{
    if (cqueue_enqueue(&g_B2_EventRecord_WriteQueue, eventRecord))
    {
        g_B2_EventRecord_WriteIdx = (g_B2_EventRecord_WriteIdx + 1) % B2_EVENTRECORD_MAX_EVENT_NUM;
        return true;
    }
    return false;
}

static inline bool B2_EventRecord_WriteQueuePop(B2_EventRecord_t *eventRecord)
{
    return cqueue_dequeue(&g_B2_EventRecord_WriteQueue, eventRecord);
}

static inline bool B2_EventRecord_WriteQueuePeek(B2_EventRecord_t *eventRecord)
{
    return cqueue_peek(&g_B2_EventRecord_WriteQueue, eventRecord);
}

static inline bool B2_EventRecord_WriteQueueIsEmpyt()
{
    return cqueue_is_empty(&g_B2_EventRecord_WriteQueue);
}

uint32_t B2_EventRecord_Get_ReadIdx()
{
    return g_recordedEventsReadIdx;
}

uint32_t B2_EventRecord_Set_ReadIdx(uint32_t recordedEventsReadIdx)
{
    g_recordedEventsReadIdx = recordedEventsReadIdx % B2_EVENTRECORD_MAX_EVENT_NUM;
}

void B2_EventRecord_Inc_ReadIdx()
{
    g_recordedEventsReadIdx = (g_recordedEventsReadIdx + 1) % B2_EVENTRECORD_MAX_EVENT_NUM;
}

