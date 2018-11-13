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

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

#define WORK_STACK_SIZE OS_STACK_ALIGN(256)
#define WORK_TASK_PRIO (0)

struct os_task work_task;
void work_task_handler(void *arg)
{
    struct os_task *t;
    printf("work_task_handler\n");
    while (1) {
        t = os_sched_get_current_task();
        assert(t->t_func == work_task_handler);
        //        hal_gpio_toggle(LED_1);
        int i;
        for(i = 0; i < 1000000; ++i) {
            /* Simulate doing a noticeable amount of work */
            hal_gpio_write(LED_1, 0);
        }
        os_time_delay(3 * OS_TICKS_PER_SEC);
        printf("work_task_handler\n");
            /* Do work... */
    }
}

    int
init_tasks(void)
{
    /* … */
    os_stack_t *work_stack;
    work_stack = malloc(sizeof(os_stack_t)*WORK_STACK_SIZE);

    assert(work_stack);
    os_task_init(&work_task, "work", work_task_handler, NULL,
            WORK_TASK_PRIO, OS_WAIT_FOREVER, work_stack,
            WORK_STACK_SIZE);

    return 0;
}


int
main(int argc, char **argv)
{
    int rc;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    sysinit();

    hal_gpio_init_out(LED_1, 1);
    hal_gpio_init_out(LED_2, 2);
    hal_gpio_toggle(LED_1);
#if 1
    init_tasks();
#else
    (void)init_tasks;
#endif
    os_time_delay(OS_TICKS_PER_SEC/10);
    while(1)
    {
        hal_gpio_toggle(LED_1);
        printf("LED_1\n");
        //os_time_delay(OS_TICKS_PER_SEC/10);
    }

    assert(0);

    return rc;
}
