/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#ifndef WBLE400_H
#define WBLE400_H

#define LED_START      18
#define LED_0          18
#define LED_1          19
#define LED_2          20
#define LED_3          21
#define LED_4          22
#define LED_STOP       22

#define BSP_LED_0      LED_0
#define BSP_LED_1      LED_1
#define BSP_LED_2      LED_2
#define BSP_LED_3      LED_3
#define BSP_LED_4      LED_4

#define BUTTON_START   16
#define BUTTON_0       16
#define BUTTON_1       17
#define BUTTON_STOP    17
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BSP_BUTTON_0   BUTTON_0
#define BSP_BUTTON_1   BUTTON_1

#define BUTTONS_NUMBER 2
#define LEDS_NUMBER    5
#define LEDS_INV_MASK  0

#define BSP_BUTTON_0_MASK (1<<BSP_BUTTON_0)
#define BSP_BUTTON_1_MASK (1<<BSP_BUTTON_1)
#define BUTTONS_MASK   (BSP_BUTTON_0_MASK | BSP_BUTTON_1_MASK)

#define BUTTONS_LIST { BUTTON_0, BUTTON_1 }
#define LEDS_LIST { LED_0, LED_1, LED_2, LED_3, LED_4 }

#define BSP_LED_0_MASK    (1<<LED_0)
#define BSP_LED_1_MASK    (1<<LED_1)
#define BSP_LED_2_MASK    (1<<LED_2)
#define BSP_LED_3_MASK    (1<<LED_3)
#define BSP_LED_4_MASK    (1<<LED_4)
#define LEDS_MASK         (BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK | BSP_LED_3_MASK | BSP_LED_4_MASK)

#define RX_PIN_NUMBER  11
#define TX_PIN_NUMBER  9
#define CTS_PIN_NUMBER 8
#define RTS_PIN_NUMBER 10
#define HWFC           true

#define SER_CONN_ASSERT_LED_PIN     LED_0

// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      NRF_CLOCK_LFCLKSRC_XTAL_20_PPM

#endif
