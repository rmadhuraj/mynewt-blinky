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
#include "console/console.h"
#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

static volatile int g_task1_loops;

/* For LED toggling */
int g_led_pin;
struct os_callout blinky;
struct os_eventq my_queue;
struct os_event my_event;

void timer_ev_cb(struct os_event *ev)
{
    hal_gpio_toggle(g_led_pin);
    printf("timer_ev_cb  utime== %lu \n",os_cputime_ticks_to_usecs(os_cputime_get32()));
    os_eventq_put(&my_queue, &my_event);
}

/**
 * main
 *
 * The main task for the project. This function initializes packages,
 * and then blinks the BSP LED in a loop.
 *
 * @return int NOTE: this function should never return!
 */

int
main(int argc, char **argv)
{
    int rc;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    sysinit();
    console_printf("Rajendran Madhu - Author 000000\n");
    printf("blinky\n");
    os_time_delay(OS_TICKS_PER_SEC/100);
    g_led_pin = LED_BLINK_PIN;
    hal_gpio_init_out(g_led_pin, 1);
    hal_gpio_toggle(g_led_pin);
    os_eventq_init(&my_queue);
    my_event.ev_cb = timer_ev_cb;
    os_eventq_put(&my_queue, &my_event);
    os_eventq_run(&my_queue);  // not need if this is while loop down

    while (1) {
        os_eventq_run(&my_queue); // Commenting this line will run the my_event only one time 
    }
    assert(0);

    return rc;
}

