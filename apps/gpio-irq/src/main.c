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

#if MYNEWT_VAL(INT_SOURCE)
#define INT_SOURCE
#endif

/* GPIO Interrupt Pin LED toggling */
int gpio_int_pin = 26;
/* For LED toggling */
int g_led_pin;

#ifdef INT_SOURCE
struct os_callout gpio_callout;
void timer_ev_cb(struct os_event *ev)
{
    hal_gpio_write(gpio_int_pin, 1);
    hal_gpio_write(gpio_int_pin, 0);
    hal_gpio_toggle(g_led_pin);
    os_callout_reset(&gpio_callout, OS_TICKS_PER_SEC/4);
}
#else
void gpio_irq_handler(void *arg)
{
    hal_gpio_irq_disable(gpio_int_pin);
    hal_gpio_toggle(g_led_pin);
    hal_gpio_irq_enable(gpio_int_pin);
}
#endif

void init_tasks(void)
{
#ifdef INT_SOURCE
    hal_gpio_init_out(gpio_int_pin, 1);
    os_callout_init(&gpio_callout, os_eventq_dflt_get(), timer_ev_cb, NULL);
    os_callout_reset(&gpio_callout, OS_TICKS_PER_SEC);
#else
    hal_gpio_irq_init(gpio_int_pin, gpio_irq_handler, NULL, HAL_GPIO_TRIG_RISING, HAL_GPIO_PULL_DOWN);
    hal_gpio_irq_enable(gpio_int_pin);
#endif
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
    init_tasks();
    g_led_pin = LED_BLINK_PIN;
    hal_gpio_init_out(g_led_pin, 1);

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);

    return rc;
}
