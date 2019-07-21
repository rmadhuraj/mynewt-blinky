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
int j = 0;
struct os_sem g_test_sem;
void timer_ev_cb(struct os_event *ev)
{
    os_sem_release(&g_test_sem);
    os_callout_reset(&blinky, OS_TICKS_PER_SEC/4);
}

/**
 * main
 *
 * The main task for the project. This function initializes packages,
 * and then blinks the BSP LED in a loop.
 *
 * @return int NOTE: this function should never return!
 */

#define TASK1_PRIO (1)
#define TASK1_STACK_SIZE    OS_STACK_ALIGN(1024)
struct os_task task1;

/* Global test semaphore */
void
task_handler(void *arg)
{

    while(1)
    {
//        os_time_delay(128);
        os_sem_pend(&g_test_sem, OS_TIMEOUT_NEVER);
        printf("released\n");
    }
}
void init_tasks(void)
{
    os_stack_t *pstack;
    os_sem_init(&g_test_sem, 0);
    pstack = malloc(sizeof(os_stack_t)*TASK1_STACK_SIZE);
    assert(pstack);

    os_task_init(&task1, "spim", task_handler, NULL,
            TASK1_PRIO, OS_WAIT_FOREVER, pstack, TASK1_STACK_SIZE);
    os_callout_init(&blinky, os_eventq_dflt_get(), timer_ev_cb, NULL);
    os_callout_reset(&blinky, OS_TICKS_PER_SEC);

}

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
    //os_time_delay(OS_TICKS_PER_SEC/100);
    g_led_pin = LED_BLINK_PIN;
    hal_gpio_init_out(g_led_pin, 1);
    hal_gpio_toggle(g_led_pin);
    init_tasks();

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);

    return rc;
}

