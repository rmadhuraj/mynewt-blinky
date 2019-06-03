/**
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

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

static volatile int g_task1_loops;

/* For LED toggling */
int g_led_pin;

/**
 * main
 *
 * The main task for the project. This function initializes packages,
 * and then blinks the BSP LED in a loop.
 *
 * @return int NOTE: this function should never return!
 */
#define dw1000_dwt_usecs_to_usecs(_t) (float)( _t * (0x10000/(128*499.2)))
int main(int argc, char **argv)
{
    int rc;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    sysinit();

    g_led_pin = LED_BLINK_PIN;
    hal_gpio_init_out(g_led_pin, 1);
    printf("madhu, sizeif int %d \n",sizeof(long int ));
    uint32_t cputime1 = 4294967000U;

    uint32_t tdma_period = 0x100000;
    uint32_t cputime = cputime1 - 200;
    uint32_t t = ((uint32_t)(cputime +(uint32_t) (dw1000_dwt_usecs_to_usecs(tdma_period))));
    printf("present=%lu t=%lu diff %lu\n",cputime1,t,t-cputime1);

    if(t == 0xffffffff )
    {
        uint32_t c = ((uint32_t)(dw1000_dwt_usecs_to_usecs(tdma_period)) - 200);
        t =  c - (t - cputime1);
        printf("this is being used t=%lu\n",t);
    }


    while (1) {
        ++g_task1_loops;

        /* Wait one second */
        os_time_delay(OS_TICKS_PER_SEC);
        printf("%lu\n",cputime1 = cputime1 + 10000);
        /* Toggle the LED */
        hal_gpio_toggle(g_led_pin);
    }
    assert(0);

    return rc;
}

