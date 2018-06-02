/* 
 * The first version of the production board uses a NRF51822-S4AT module with
 * integrated antenna.
 *
 * This board has no push buttons nor leds, nor a LF XTAL.
 */

#ifndef S4AT_H
#define S4AT_H

#define BUTTONS_NUMBER 0
#define LEDS_NUMBER    0
#define LEDS_INV_MASK  0

#define NRF_CLOCK_LFCLKSRC      NRF_CLOCK_LFCLKSRC_RC_250_PPM_TEMP_4000MS_CALIBRATION

#endif
