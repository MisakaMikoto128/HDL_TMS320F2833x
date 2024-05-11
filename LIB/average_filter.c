/**
 * @file average_filter.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-01-23
 * @last modified 2024-01-23
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "average_filter.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

bool average_filter_init(average_filter_t *af, float *pBuf, uint16_t size)
{
    if (af == NULL || pBuf == NULL || size == 0)
    {
        return false;
    }
    af->pBuf = pBuf;
    af->size = size;
    average_filter_reset(af);
    return true;
}

/**
 * @file average_filter.c
 * @brief Implementation of the average filter update function.
 */

/**
 * @brief Updates the average filter with a new value.
 *
 * This function updates the average filter with a new value and returns the updated average.
 * The average filter calculates the average value of a series of input values and keeps track of the maximum and minimum variances.
 *
 * @param af The average filter object.
 * @param value The new value to be added to the filter.
 * @return The updated average value.
 */
float average_filter_update(average_filter_t *af, float value)
{
    // Update the sum by subtracting the oldest value and adding the new value
    af->sum += value - af->pBuf[af->index];

    // Update the buffer with the new value
    af->pBuf[af->index] = value;

    // Update the index
    af->index = (af->index + 1) % af->size;

    // Update the count
    if (af->count < af->size)
    {
        af->count++;
    }

    // Calculate the average
    af->average = af->sum / af->count;

    return af->average;
}

void average_filter_reset(average_filter_t *af)
{
    af->count = 0;
    af->index = 0;
    af->sum = 0;
    for (uint16_t i = 0; i < af->size; i++)
    {
        af->pBuf[i] = 0;
    }
    af->average = 0;
}
