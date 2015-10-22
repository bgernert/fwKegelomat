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

#include "relay.h"

uint16_t relay_current;		// Aktueller Stand der Relais
uint16_t relay_old;			// Alter Stand der Relais

uint8_t relay_queue_read_pointer;
uint8_t relay_queue_write_pointer;
uint8_t relay_queue_first_run;

struct relay_queue_item_t* relay_active_queue_item;

void kegelomat_relay_init()
{
	// Als Ausgang konfigurieren
	RELAY_DDR_DATA |= (1 << RELAY_DDRPIN_DATA);
	RELAY_DDR_SHC |= (1 << RELAY_DDRPIN_SHC);
	RELAY_DDR_STC |= (1 << RELAY_DDRPIN_STC);

	// RESET löscht das Register bei LOW, daher: Immer HEIGH
	RELAY_DDR_RESET |= (1 << RELAY_DDRPIN_RESET);
	RELAY_PORT_RESET |= (1 << RELAY_PORTPIN_RESET);

	// Am Anfang alles auf aus setzen
	asm volatile ("nop");
	kegelomat_relay_set(0x0, 0xffff);

	// Variablen initialisieren
	relay_current = 0;
	relay_old = 0;
	relay_queue_read_pointer = 0;
	relay_queue_write_pointer = 0;
	relay_queue_first_run = 0;
	relay_active_queue_item = NULL;
}

struct relay_queue_item_t* kegelomat_relay_queue_put(uint8_t type)
{
	uint8_t next_item = (relay_queue_write_pointer + 1);
	if(next_item >= RELAY_QUEUE_SIZE)
	{
		next_item = 0;
	}
	if(relay_queue_item[next_item].type == 0)
	{
		relay_queue_write_pointer = next_item;
		relay_queue_item[next_item].type = type;
		relay_queue_item[next_item].data = 0;
		relay_queue_item[next_item].mask = 0;
		relay_queue_item[next_item].time = 0;
		return &relay_queue_item[next_item];
	}
	return NULL;
}

struct relay_queue_item_t* kegelomat_relay_queue_get()
{
	relay_queue_item[relay_queue_read_pointer].type = 0;
	uint8_t next_item = (relay_queue_read_pointer + 1);
	if(next_item >= RELAY_QUEUE_SIZE)
	{
		next_item = 0;
	}
	if(relay_queue_item[next_item].type != 0)
	{
		relay_queue_read_pointer = next_item;
		return &relay_queue_item[next_item];
	}
	return NULL;
}

void kegelomat_relay_set(uint16_t data, uint16_t mask)
{
	// Die ersten 8 Bit überspringen
	// INFO: Entfernen und umschreiben für ext. Relais
	data = (data << 8);
	mask = (mask << 8);
	uint16_t data_old = (relay_current << 8);
	uint16_t data_new = 0;

	// Für die letzten 8 Bit im Byte 'data'
	for(uint8_t i = 0; i < 8; ++i)
	{
		// Das höchtwertige Bit (MSB) untersuchen, dazu
		// 'mask'/'data' mit 0b1000000000000000 (= 0x8000) verunden.
		// Wenn Bit = 1, dann 'true', sonst nicht
		if(mask & 0x8000)
		{
			if(data & 0x8000)
			{
				RELAY_PORT_DATA |= (1 << RELAY_PORTPIN_DATA);
				data_new = (data_new << 1) | 0b1;
			} else {
				RELAY_PORT_DATA &= ~(1 << RELAY_PORTPIN_DATA);
				data_new = (data_new << 1);
			}
		} else {
			if(data_old & 0x8000)
			{
				RELAY_PORT_DATA |= (1 << RELAY_PORTPIN_DATA);
				data_new = (data_new << 1) | 0b1;
			} else {
				RELAY_PORT_DATA &= ~(1 << RELAY_PORTPIN_DATA);
				data_new = (data_new << 1);
			}
		}

		// Daten in das Register schieben.
		RELAY_PORT_SHC |= (1 << RELAY_PORTPIN_SHC);
		asm volatile ("nop");
		RELAY_PORT_SHC &= ~(1 << RELAY_PORTPIN_SHC);
		asm volatile ("nop");

		// Das nächste Bit an die erste Stelle holen (Shift nach Links um 1)
		data = (data << 1);
		data_old = (data_old << 1);
		mask = (mask << 1);
	}

	// Alle 8 Bit sind nun im Register. Jetzt müssen sie noch
	// angewendet werden
	RELAY_PORT_STC |= (1 << RELAY_PORTPIN_STC);
	asm volatile ("nop");
	RELAY_PORT_STC &= ~(1 << RELAY_PORTPIN_STC);

	relay_current = data_new;
}

void kegelomat_relay_fixed_set(uint16_t data, uint16_t mask)
{
	struct relay_queue_item_t* item = kegelomat_relay_queue_put(1);
	if(item != NULL)
	{
		item->data = data;
		item->mask = mask;
	} else {
		kegelomat_led_blink(LED_D, 3);
		uint8_t err = ERR_QUEUE_FULL;
		kegelomat_can_send_msg(CAN_MSG_ERROR, &err, 1);
	}
}

void kegelomat_relay_timed_set(uint16_t data, uint16_t mask, uint32_t time)
{
	struct relay_queue_item_t* item = kegelomat_relay_queue_put(2);
	if(item != NULL)
	{
		item->data = data;
		item->mask = mask;
		item->time = time;
	} else {
		kegelomat_led_blink(LED_D, 3);
		uint8_t err = ERR_QUEUE_FULL;
		kegelomat_can_send_msg(CAN_MSG_ERROR, &err, 1);
	}
}

void kegelomat_relay_score_set(uint16_t data)
{
	struct relay_queue_item_t* item = kegelomat_relay_queue_put(3);
	if(item != NULL)
	{
		item->data = data;
	} else {
		kegelomat_led_blink(LED_D, 3);
		uint8_t err = ERR_QUEUE_FULL;
		kegelomat_can_send_msg(CAN_MSG_ERROR, &err, 1);
	}
}

void kegelomat_relay_timer_call()
{
	if(relay_active_queue_item == NULL)
	{
		if((relay_active_queue_item = kegelomat_relay_queue_get()) == NULL)
		{
			return;
		}
		relay_queue_first_run = 0;
	}

	switch(relay_active_queue_item->type)
	{
		// Relais unbegrenzt setzen
		case 1:
			kegelomat_relay_set(relay_active_queue_item->data, relay_active_queue_item->mask);

			uint8_t msg[4];
			msg[0] = (relay_current >> 8);
			msg[1] = relay_current & 0xff;
			msg[2] = (relay_active_queue_item->mask >> 8);
			msg[3] = relay_active_queue_item->mask & 0xff;

			kegelomat_can_send_msg(CAN_MSG_ANSWER_SET_RELAY, msg, 4);
			relay_active_queue_item = NULL;
			break;

		// Relais begrenzt setzen
		case 2:
			if(relay_queue_first_run == 0)
			{
				relay_old = relay_current;
				kegelomat_relay_set(relay_active_queue_item->data, relay_active_queue_item->mask);

				uint8_t msg[4];
				msg[0] = (relay_current >> 8);
				msg[1] = relay_current & 0xff;
				msg[2] = (relay_active_queue_item->mask >> 8);
				msg[3] = relay_active_queue_item->mask & 0xff;

				kegelomat_can_send_msg(CAN_MSG_ANSWER_TSET_RELAY, msg, 4);

				++relay_queue_first_run;
			}

			--relay_active_queue_item->time;
			if(relay_active_queue_item->time <= 0)
			{
				kegelomat_relay_set(relay_old, 0xffff);
				relay_active_queue_item = NULL;
			}
			break;

		// Gesamtpunktestand setzen
		case 3:
			if(relay_queue_first_run == 0)
			{
				uint16_t data = (1 << RELAY_BUTTON_SCORE_DEL);
				uint16_t mask = (1 << RELAY_BUTTON_SCORE_DEL);

				relay_active_queue_item->time = RELAY_BUTTON_SCORE_TIME_ON;
				kegelomat_relay_set(data, mask);
				++relay_queue_first_run;
			} else if(relay_queue_first_run == 1) {
				--relay_active_queue_item->time;
				if(relay_active_queue_item->time <= 0)
				{
					relay_active_queue_item->time = RELAY_BUTTON_SCORE_TIME_OFF;
					++relay_queue_first_run;
				}
			} else if(relay_queue_first_run == 2){
				uint16_t data = 0;
				uint16_t mask = (1 << RELAY_BUTTON_SCORE_DEL)
								| (1 << RELAY_BUTTON_SCORE_1)
								| (1 << RELAY_BUTTON_SCORE_10)
								| (1 << RELAY_BUTTON_SCORE_100)
								| (1 << RELAY_BUTTON_SCORE_1000);
				kegelomat_relay_set(data, mask);
				++relay_queue_first_run;
			} else if(relay_queue_first_run == 3) {
				--relay_active_queue_item->time;
				if(relay_active_queue_item->time <= 0)
				{
					++relay_queue_first_run;
				}
			} else {
				uint16_t score = relay_active_queue_item->data;
				uint16_t data = 0;
				uint16_t mask = (1 << RELAY_BUTTON_SCORE_1)
								| (1 << RELAY_BUTTON_SCORE_10)
								| (1 << RELAY_BUTTON_SCORE_100)
								| (1 << RELAY_BUTTON_SCORE_1000);
				uint8_t n0 = score % 10;				// Einer
				uint8_t n1 = (score % 100) / 10;		// Zehner
				uint8_t n2 = (score % 1000) / 100;		// Hunderter
				uint8_t n3 = (score % 10000) / 1000;	// Tausender
				if(n0 > 0)
				{
					data |= (1 << RELAY_BUTTON_SCORE_1);
					score = (score - 1);
				}
				if(n1 > 0)
				{
					data |= (1 << RELAY_BUTTON_SCORE_10);
					score = (score - 10);
				}
				if(n2 > 0)
				{
					data |= (1 << RELAY_BUTTON_SCORE_100);
					score = (score - 100);
				}
				if(n3 > 0)
				{
					data |= (1 << RELAY_BUTTON_SCORE_1000);
					score = (score - 1000);
				}

				if(data != 0)
				{
					relay_active_queue_item->data = score;
					relay_active_queue_item->time = RELAY_BUTTON_SCORE_TIME_ON;
					kegelomat_relay_set(data, mask);
					relay_queue_first_run = 1;
				} else {
					relay_active_queue_item = NULL;
				}
			}
			break;

		// Unbekannt
		default:
			relay_active_queue_item = NULL;
			break;
	}
}
