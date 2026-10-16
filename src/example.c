/**
 * @file        example.c
 * @brief       A small library made to try esp32-installer on: a period that never blocks.
 * @version     1.0.0
 *
 * @author      Nima Askari (NimaLTD)
 * @email       nima.askari@gmail.com
 * @github      https://www.github.com/nimaltd
 * @linkedin    https://www.linkedin.com/in/nimaltd
 * @youtube     https://www.youtube.com/@nimaltd
 * @instagram   https://instagram.com/github.nimaltd
 *
 * @copyright   (c) 2026 Nima Askari (NimaLTD)
 *              SPDX-License-Identifier: Apache-2.0
 *              See LICENSE.md in the project root for the full license text.
 */

/*
 * ****************************************************************************************************
 * Includes
 * ****************************************************************************************************
*/

#include "example.h"
#include <assert.h>
#include <stdio.h>
#include "esp_timer.h"
#include "sdkconfig.h"

/*
 * ****************************************************************************************************
 * Private function prototypes
 * ****************************************************************************************************
*/

static uint32_t example_now_ms(void);

/*
 * ****************************************************************************************************
 * Public function implementations
 * ****************************************************************************************************
*/

/*****************************************************************************************************/
/**
 * @brief Start a period of CONFIG_EXAMPLE_PERIOD_MS from now.
 *
 * @param[out] handle  The period to start.
 */
void example_init(example_t *handle)
{
    assert(handle != NULL);

    handle->start = example_now_ms();
    handle->count = 0U;
}

/*****************************************************************************************************/
/**
 * @brief Whether the period has passed. Returns at once, and starts the next one when it has.
 *
 * @param[in,out] handle  Period from example_init().
 * @return true once every CONFIG_EXAMPLE_PERIOD_MS, false in between.
 */
bool example_due(example_t *handle)
{
    bool due = false;

    assert(handle != NULL);

    /* Subtracted, never added, so the answer stays right when the 32 bit
       uptime wraps round after 49 days. */
    if ((example_now_ms() - handle->start) >= (uint32_t)CONFIG_EXAMPLE_PERIOD_MS)
    {
        /* The next period starts where this one should have ended, not now,
           so a loop that comes back late does not make the rhythm drift. */
        handle->start += (uint32_t)CONFIG_EXAMPLE_PERIOD_MS;
        handle->count++;
        due = true;
    }

    return due;
}

/*****************************************************************************************************/
/**
 * @brief How many times example_due() has said yes.
 *
 * @param[in] handle  Period from example_init().
 * @return The count since example_init().
 */
uint32_t example_count(const example_t *handle)
{
    assert(handle != NULL);

    return handle->count;
}

/*****************************************************************************************************/
/**
 * @brief Print CONFIG_EXAMPLE_GREETING, and how many periods have passed.
 *
 * @param[in] handle  Period from example_init().
 */
void example_hello(const example_t *handle)
{
    assert(handle != NULL);

    printf("%s %u\n", CONFIG_EXAMPLE_GREETING, (unsigned int)handle->count);
}

/*
 * ****************************************************************************************************
 * Private function implementations
 * ****************************************************************************************************
*/

/*****************************************************************************************************/
/**
 * @brief Uptime in milliseconds, from esp_timer's microseconds.
 *
 * @return Milliseconds since start up, wrapping round after 49 days.
 */
static uint32_t example_now_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}
