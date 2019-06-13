/*
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
#include "os/mynewt.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include "hal/hal_spi.h"
#include "stats/stats.h"
#include "config/config.h"
#include <console/console.h>
#ifdef ARCH_sim
#include <mcu/mcu_sim.h>
#endif

struct sblinky_spi_cb_arg
{
    int transfers;
    int txlen;
    uint32_t tx_rx_bytes;
};
struct sblinky_spi_cb_arg spi_cb_obj;
void *spi_cb_arg;
int g_rx_len ;

/* Task 1 */
#define TASK1_PRIO (1)
#define TASK1_STACK_SIZE    OS_STACK_ALIGN(1024)
struct os_task task1;

/* Global test semaphore */
struct os_sem g_test_sem;

/* For LED toggling */
int g_led_pin;

#if MYNEWT_VAL(SPI_0_MASTER) || MYNEWT_VAL(SPI_1_MASTER) || MYNEWT_VAL(SPI_2_MASTER)
#define SPI_MASTER 1
#define SPI_SS_PIN  (MYNEWT_VAL(SPITEST_SS_PIN))
#if SPI_SS_PIN < 0
#error "SPITEST_SS_PIN must be set in the target config."
#endif
#define SPI_M_NUM  (MYNEWT_VAL(SPITEST_M_NUM))
#endif

#if MYNEWT_VAL(SPI_0_SLAVE) || MYNEWT_VAL(SPI_1_SLAVE) || MYNEWT_VAL(SPI_2_SLAVE)
#define SPI_SLAVE 1
#define SPI_S_NUM  (MYNEWT_VAL(SPITEST_S_NUM))
#endif

#if !defined(SPI_MASTER) && !defined(SPI_SLAVE)
#error "Select the Device as SPI_MASTER or SPI_SLAVE, check app/syscfg.yml"
#endif
#if defined(SPI_MASTER) && defined(SPI_SLAVE)
#if SPI_M_NUM == SPI_S_NUM
#error "SPI_M_NUM and SPI_S_NUM cannot be the same."
#endif
#endif

static void sblinky_spi_cfg(int spi_num, hal_spi_txrx_cb txrx_cb, void *arg)
{
    struct hal_spi_settings my_spi;
    my_spi.data_order = HAL_SPI_MSB_FIRST;
    my_spi.data_mode = HAL_SPI_MODE0;
    my_spi.baudrate = MYNEWT_VAL(SPI_BAUDRATE);
    my_spi.word_size = HAL_SPI_WORD_SIZE_8BIT;
    assert(hal_spi_config(spi_num, &my_spi) == 0);
    assert(hal_spi_set_txrx_cb(spi_num, txrx_cb, arg) == 0);
}

#ifdef SPI_MASTER
uint8_t g_spi_tx_buf[32];
uint8_t g_spi_rx_buf[32];

static void
sblinky_spi_irqm_handler(void *arg, int len)
{
    struct sblinky_spi_cb_arg *cb;
    hal_gpio_write(SPI_SS_PIN, 1);
    assert(arg == spi_cb_arg);
    if (spi_cb_arg) {
        cb = (struct sblinky_spi_cb_arg *)arg;
        assert(len == cb->txlen);
        ++cb->transfers;
    }
    os_sem_release(&g_test_sem);
}

#endif

#ifdef SPI_SLAVE
uint8_t g_spi_tx_buf[32];
uint8_t g_spi_rx_buf[32];

void
sblinky_spi_irqs_handler(void *arg, int len)
{
    assert(arg == spi_cb_arg);
    g_rx_len = len;
    /* Post semaphore to task waiting for SPI slave */
    os_sem_release(&g_test_sem);
}

#endif

#ifdef SPI_MASTER
void
spim_task_handler(void *arg)
{
    int i =0, j=0;
    int rc;

    /* Set the led pin */
    g_led_pin = LED_BLINK_PIN;
    hal_gpio_init_out(g_led_pin, 1);
    /* Use SS pin for testing */
    hal_gpio_init_out(SPI_SS_PIN, 1);
    memset(g_spi_tx_buf, 0, 32);
    spi_cb_arg = &spi_cb_obj;
    spi_cb_obj.txlen = 32;
    sblinky_spi_cfg(SPI_M_NUM, sblinky_spi_irqm_handler, spi_cb_arg);
    hal_spi_enable(SPI_M_NUM);

    while (1) {
        /* Wait one second */
        os_time_delay(OS_TICKS_PER_SEC);

        /* Toggle the LED */
        hal_gpio_toggle(g_led_pin);

        assert(hal_gpio_read(SPI_SS_PIN) == 1);
            hal_gpio_write(SPI_SS_PIN, 0);

        /* Send non-blocking */
        rc = hal_spi_txrx_noblock(SPI_M_NUM, g_spi_tx_buf, g_spi_rx_buf,
                spi_cb_obj.txlen);
        assert(!rc);
        os_sem_pend(&g_test_sem, OS_TIMEOUT_NEVER);
        console_printf("transmitted: ");
        for (i = 0; i < spi_cb_obj.txlen; i++) {
            console_printf("%2x ", g_spi_tx_buf[i]);
        }
        console_printf("\n");
        console_printf("received   : ");
        for (i = 0; i < spi_cb_obj.txlen; i++) {
            console_printf("%2x ", g_spi_rx_buf[i]);
        }
        console_printf("\n");
        /* Get random length to send */
        spi_cb_obj.txlen = (rand() & 0x1F) + 1;
        console_printf("ranlen= %d \r\n",spi_cb_obj.txlen);
        memset(g_spi_tx_buf, ++j, spi_cb_obj.txlen);
    }
}
#endif

#ifdef SPI_SLAVE
void
spis_task_handler(void *arg)
{
    int rc,i;

    g_led_pin = LED_BLINK_PIN;
    hal_gpio_init_out(g_led_pin, 1);

    spi_cb_arg = &spi_cb_obj;
    sblinky_spi_cfg(SPI_S_NUM, sblinky_spi_irqs_handler, spi_cb_arg);
    hal_spi_enable(SPI_S_NUM);

    /* Make the default character 0x77 */
    hal_spi_slave_set_def_tx_val(SPI_S_NUM, 0x77);

    /*  Fill buffer with 0x77 for first transfer. */
    memset(g_spi_tx_buf, 0x77, 32);
    spi_cb_obj.txlen = 32;
    rc = hal_spi_txrx_noblock(SPI_S_NUM, g_spi_tx_buf, g_spi_rx_buf, 32);

    while (1) {
        /* Wait for semaphore from ISR */
        os_sem_pend(&g_test_sem, OS_TIMEOUT_NEVER);
        spi_cb_obj.txlen = spi_cb_obj.txlen ? spi_cb_obj.txlen:32;
        memcpy(g_spi_tx_buf, g_spi_rx_buf, spi_cb_obj.txlen);
        rc = hal_spi_txrx_noblock(SPI_S_NUM, g_spi_tx_buf, g_spi_rx_buf,
                spi_cb_obj.txlen);
        assert(rc == 0);
        console_printf("Data rcvd %2d: ",g_rx_len);
        for (i = 0; i < g_rx_len; i++)
            console_printf("%2x ", g_spi_rx_buf[i]);
        console_printf("\n");
        /* Toggle the LED */
        hal_gpio_toggle(g_led_pin);
    }
}
#endif

/**
 * init_tasks
 *
 * Called by main.c after sysinit(). This function performs initializations
 * that are required before tasks are running.
 *
 * @return int 0 success; error otherwise.
 */
static void
init_tasks(void)
{
    os_stack_t *pstack;

    (void)pstack;

    /* Initialize global test semaphore */
    os_sem_init(&g_test_sem, 0);

#if defined(SPI_MASTER)
    pstack = malloc(sizeof(os_stack_t)*TASK1_STACK_SIZE);
    assert(pstack);

    os_task_init(&task1, "spim", spim_task_handler, NULL,
            TASK1_PRIO, OS_WAIT_FOREVER, pstack, TASK1_STACK_SIZE);
#endif

#if defined(SPI_SLAVE)
    pstack = malloc(sizeof(os_stack_t)*TASK1_STACK_SIZE);
    assert(pstack);

    os_task_init(&task1, "spis", spis_task_handler, NULL,
            TASK1_PRIO, OS_WAIT_FOREVER, pstack, TASK1_STACK_SIZE);
#endif
}

/**
 * main
 *
 * The main task for the project. This function initializes the packages, calls
 * init_tasks to initialize additional tasks (and possibly other objects),
 * then starts serving events from default event queue.
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
    printf("spi-slave and master \n");
    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    /* Never returns */
    assert(0);

    return rc;
}
