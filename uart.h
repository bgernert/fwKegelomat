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

#ifndef SRC_UART_H_
#define SRC_UART_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

#include "configure.h"
#include "led.h"
#include "can.h"

void kegelomat_uart_init();

void kegelomat_uart_doWork();

void kegelomat_uart_reset_status();

void kegelomat_uart_error();

bool kegelomat_uart_check_digit(char* c);

#endif /* SRC_UART_H_ */
