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
#include "dw1000_tdma.h"
#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

static volatile int g_task1_loops;

/* For LED toggling */
int g_led_pin;
int g_led_pin1;
extern uint32_t time32_incr;

#if 1 


/* The timer callout */
static struct os_callout blinky_callout;
/*
 * Event callback function for timer events. It toggles the led pin.
 */
static void
timer_ev_cb(struct os_event *ev)
{
    assert(ev != NULL);
    printf("rtc %lu cpu= %lu \n",time32_incr,os_cputime_ticks_to_usecs(os_cputime_get32())/1000000);
    printf("   cpu ticks == %lu \n",os_cputime_get32());
    hal_gpio_toggle(g_led_pin);
    hal_gpio_toggle(g_led_pin1);
    os_callout_reset(&blinky_callout, OS_TICKS_PER_SEC);
}
static void
init_timer(void)
{
    /*
     * Initialize the callout for a timer event.
     */
    os_callout_init(&blinky_callout, os_eventq_dflt_get(),
                    timer_ev_cb, NULL);

    os_callout_reset(&blinky_callout, OS_TICKS_PER_SEC);
}
#endif
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


    printf("start\n");
    init_rtc2();
    printf("init_over\n");
    printf("OS_CPU_FREQUENCY == %d\n",MYNEWT_VAL(OS_CPUTIME_FREQ));

    g_led_pin = LED_BLINK_PIN;
    g_led_pin1 = LED_2;

    hal_gpio_init_out(g_led_pin, 1);
    hal_gpio_init_out(g_led_pin1, 1);
    hal_gpio_toggle(g_led_pin);
    init_timer();
    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);
    return rc;
}
