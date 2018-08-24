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
#include "dw1000_tdma.h"

static struct os_callout blinky_callout;

void init_tdma(dw1000_tdma_config_params_t * config_params)
{
    assert(config_params != NULL);
    os_callout_init(&blinky_callout, os_eventq_dflt_get(), config_params->tdma_cb->timer_ev_cb, NULL);
    os_callout_reset(&blinky_callout, config_params->tick_count);
}

void reset_tdma(uint16_t tick)
{
    os_callout_reset(&blinky_callout, OS_TICKS_PER_SEC/tick);
}

uint32_t time32_incr;

void init_rtc2(void)
{
    NVIC_SetVector(RTC_IRQ,(uint32_t)handler_rtc2);
    NVIC_DisableIRQ(RTC_IRQ);
    NVIC_SetPriority(RTC_IRQ, (1 << __NVIC_PRIO_BITS) - 1);
    /* set prescaler to decide the RTC frequency */
    RTC_DEV->PRESCALER = RTC_PRESCALER;
    /* enable the low-frequency clock */
    NRF_CLOCK->TASKS_LFCLKSTART = 1;
    /* start the RTC to count */
    RTC_DEV->TASKS_START = 1;
    /* set the value to be compared with counter value */
    RTC_DEV->CC[0] = (RTC_TICKS_TO_WAIT & RTC_MAX_VALUE);
    /* Enable event once CC[0] value matches with counter value*/
    RTC_DEV->INTENSET = RTC_INTENSET_COMPARE0_Msk;
    /*Enable Interrupt on the RTC events */
    NVIC_EnableIRQ(RTC_IRQ);

}

int get_port_count(void)
{
    return time32_incr;
}

void handler_rtc2(void)
{
  //os_trace_enter_isr();
    if(RTC_DEV->EVENTS_COMPARE[0] == 1)//check for the match event
     {
         RTC_DEV->EVENTS_COMPARE[0] = 0; //Clear the event flag
         RTC_DEV->INTENCLR = RTC_INTENSET_COMPARE0_Msk; //disable the interrupt
         RTC_DEV->TASKS_CLEAR = 1; //Reset the counter
         RTC_DEV->INTENSET = RTC_INTENSET_COMPARE0_Msk;//enable the interrupt
         ++time32_incr; //Increment the variable
     }
    //os_trace_exit_isr();
}
