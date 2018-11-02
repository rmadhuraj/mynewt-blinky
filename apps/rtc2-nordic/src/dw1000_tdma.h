/**
 * Copyright (C) 2017-2018, Decawave Limited, All Rights Reserved
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef _DW1000_TDMA_H_
#define _DW1000_TDMA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "sysinit/sysinit.h"
#include "os/os.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include "hal/hal_bsp.h"
#include "nrf52.h"
#include "nrf52_bitfields.h"
//#include "cmsis_nvic.h"

#define RTC_DEV             NRF_RTC2 //NRF_RTC0 // IN mynewt OS NRF_RTC1 is used for the OS_TIME
#define RTC_IRQ             RTC2_IRQn //RTC0_IRQn
#define RTC_MAX_VALUE       (0xffffff)
#define RTC_TICKS_TO_WAIT   31U        /*Generate RTC interrupt for every 1ms, 1000 counts in 1sec */
//#define RTC_TICKS_TO_WAIT   50U          /*Generate RTC interrupt for every 1.584786054ms , 631 in 1sec */
#define RTC_FREQUENCY       (32768U)       /* in Hz */
#define RTC_PRESCALER       (0U)           /* RTC runs at 32.768khz*/



/**
 * @brief struct which defines the necessary call backs for tdma
 */
typedef struct
{
    void (*timer_ev_cb)(struct os_event *ev);    
}dw1000_tdma_callback_t;

/**
 * @brief Struct which defines params required by tdma
 */
typedef struct
{
    uint16_t tick_count; 
    dw1000_tdma_callback_t *tdma_cb;
}dw1000_tdma_config_params_t;

/**
 * @brief This API initializes the time slot.
 *
 * @param[in] config_params Pointer to configuration parameter structure
 *
 */
void init_tdma(dw1000_tdma_config_params_t * config_params);
/**
 * @brief This API adjusts the time slot
 *
 * @param[in] tick Specifies by what factor os_timer_handler should be called
 *
 */
void reset_tdma(uint16_t tick);

/**
 * @brief This API initializes the RTC2 module
 *
*/
void init_rtc2(void);

/**
 * @brief This API handles the RTC2 interrupt
 *
*/
void handler_rtc2(void);
/**
 * @brief This API returns the value of the time32_incr variable
 *
*/
int  get_port_count(void);
#ifdef __cplusplus
}
#endif
#endif
