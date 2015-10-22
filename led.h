/*
 *  Copyright (c) 2015 Björn Gernert <mail@bjoern-gernert.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SRC_LED_H_
#define SRC_LED_H_

#include <avr/io.h>

#include "configure.h"

typedef enum {
	LED_A = 0,
	LED_B = 1,
	LED_C = 2,
	LED_D = 3
} kegelomat_led;

void kegelomat_led_init();

void kegelomat_led_timer_call();

void kegelomat_led_on(kegelomat_led led);

void kegelomat_led_off(kegelomat_led led);

void kegelomat_led_blink(kegelomat_led led, uint16_t limit);

void kegelomat_led_heartbeat(kegelomat_led led, uint16_t limit);

#endif /* SRC_LED_H_ */
