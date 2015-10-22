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

#ifndef SRC_CAN_H_
#define SRC_CAN_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include "configure.h"
#include "led.h"
#include "relay.h"
#include "../contrib/can.h"

extern volatile uint8_t canAddress;

uint8_t kegelomat_can_readAddress();

void kegelomat_can_init();

void kegelomat_can_doWork();

void kegelomat_can_send_msg(uint8_t type, uint8_t* data, uint8_t length);

void kegelomat_can_send_bootup();

void kegelomat_can_send_keepalive();

#endif /* SRC_CAN_H_ */
