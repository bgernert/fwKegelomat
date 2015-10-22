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

#ifndef SRC_RELAY_H_
#define SRC_RELAY_H_

#include <avr/io.h>
#include <util/delay.h>

#include "configure.h"
#include "can.h"

#define RELAY_QUEUE_SIZE	8

struct relay_queue_item_t {
	uint8_t type;
	uint16_t data;
	uint16_t mask;
	uint32_t time;
}  __attribute__((packed)) relay_queue_item[RELAY_QUEUE_SIZE];

void kegelomat_relay_init();

struct relay_queue_item_t* kegelomat_relay_queue_put(uint8_t type);

struct relay_queue_item_t* kegelomat_relay_queue_get();

void kegelomat_relay_timer_call();

void kegelomat_relay_set(uint16_t data, uint16_t mask);

void kegelomat_relay_fixed_set(uint16_t data, uint16_t mask);

void kegelomat_relay_timed_set(uint16_t data, uint16_t mask, uint32_t time);

void kegelomat_relay_score_set(uint16_t data);

#endif /* SRC_RELAY_H_ */
