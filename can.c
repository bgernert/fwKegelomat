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

#include "can.h"

volatile uint8_t canAddress;

struct canFlags {
	int newMessage:1;
} __attribute__((packed)) volatile canFlags;

/**
 * Externer Interrupt für den CAN-Bus
 */
#ifdef CANINTRV
ISR(CANINTRV)
{
	canFlags.newMessage = 1;
}
#endif

/**
 * Adresse des CAN-Bus auslesen
 */
uint8_t kegelomat_can_readAddress()
{
	uint8_t addr = 0;

	// Pins als Eingang setzen
	CANADDR_0_DDR &= ~(1 << CANADDR_0_PIN);
	CANADDR_1_DDR &= ~(1 << CANADDR_1_PIN);
	CANADDR_2_DDR &= ~(1 << CANADDR_2_PIN);
	CANADDR_3_DDR &= ~(1 << CANADDR_3_PIN);

	// Interne Pull-Up Widerstände setzen
	CANADDR_0_PORT |= (1 << CANADDR_0_PIN);
	CANADDR_1_PORT |= (1 << CANADDR_1_PIN);
	CANADDR_2_PORT |= (1 << CANADDR_2_PIN);
	CANADDR_3_PORT |= (1 << CANADDR_3_PIN);

	// 1 Takt warten
	asm volatile ("nop");

	// Prüfen ob ein Jumper gesetzt ist, wenn nicht Pullup deaktivieren
	if(!(CANADDR_0_PINREG & (1 << CANADDR_0_PIN)))
		addr |= 0b00000001;
	else
		CANADDR_0_PORT &= ~(1 << CANADDR_0_PIN);

	if(!(CANADDR_1_PINREG & (1 << CANADDR_1_PIN)))
		addr |= 0b00000010;
	else
		CANADDR_1_PORT &= ~(1 << CANADDR_1_PIN);

	if(!(CANADDR_2_PINREG & (1 << CANADDR_2_PIN)))
		addr |= 0b00000100;
	else
		CANADDR_2_PORT &= ~(1 << CANADDR_2_PIN);

	if(!(CANADDR_3_PINREG & (1 << CANADDR_3_PIN)))
		addr |= 0b00001000;
	else
		CANADDR_3_PORT &= ~(1 << CANADDR_3_PIN);

	return addr;
}

/**
 * CAN-Bus initialisieren
 */
void kegelomat_can_init()
{
	// Adresse des CAN-Bus setzen
	canAddress = kegelomat_can_readAddress();

	// CAN initialisieren
	can_init(CANBITRATE);

	// Filter und Masken-Tabelle erstellen
	can_filter_t ownID = {
			.id = canAddress,	// Adresse der Jumper
			.mask = 0x01F,		// Nur die letzen 5 Bits werden berücksichtigt
			.flags = {
				.rtr = 0
			}
	};

	can_filter_t grpID = {
			.id = 0x010,		// Gruppenadresse 0x010 = 16
			.mask = 0x01F,
			.flags = {
				.rtr = 0
			}
	};

	// Filter setzen
	can_set_filter(0, &ownID);
	can_set_filter(2, &grpID);

	// Interrupt für CAN aktivieren
	EIMSK |= (1 << CANINTR);				// Externen Interrupt verwenden
	//cli();								// Alle Interrupts löschen
	EICRA &= ~(1 << ISC00) & ~(1 << ISC01);	// Interrupt 0 einschalten und beim low level auslösen
	sei();									// Interrupts aktivieren

	// Flags setzen
	canFlags.newMessage = 0;
}


/**
 * Aufruf in der main-Loop, CAN-Bus Aufgaben abarbeiten
 */
void kegelomat_can_doWork()
{
#ifdef CANINTRV
	if(canFlags.newMessage == 1)
	{
		canFlags.newMessage = 0;
#endif
		if(can_check_message())
		{
			can_t msg;
			if(can_get_message(&msg))
			{
				// LED_C blinken lassen
				kegelomat_led_blink(LED_C, 1);

				// Alle Nachrichten müssen mind. 1 Byte lang sein
				// (Aktion + Optional{0...6})
				if(msg.length > 0)
				{
					switch(msg.id >> 5)
					{
					// Keepalive Nachricht senden
					case CAN_MSG_SEND_ALIVE:
						kegelomat_can_send_keepalive();
						break;

					// Relais dauerhaft setzen
					case CAN_MSG_SET_RELAY:
						if(msg.length > 3)
						{
							uint16_t data = (msg.data[0] << 8) | msg.data[1];
							uint16_t mask = (msg.data[2] << 8) | msg.data[3];
							kegelomat_relay_fixed_set(data, mask);
						} else {
							kegelomat_led_blink(LED_D, 1);
							uint8_t err = ERR_MISSING_PARAMETER;
							kegelomat_can_send_msg(CAN_MSG_ERROR, &err, 1);
						}
						break;

					// Relais begrenzt drücken
					case CAN_MSG_TSET_RELAY:
						if(msg.length > 6)
						{
							uint16_t data = (msg.data[0] << 8) | msg.data[1];
							uint16_t mask = (msg.data[2] << 8) | msg.data[3];
							uint32_t time = ((uint32_t)msg.data[4] << 24) | ((uint32_t)msg.data[5] << 16) | (msg.data[6] << 8) | msg.data[7];
							kegelomat_relay_timed_set(data, mask, time);
						} else {
							kegelomat_led_blink(LED_D, 1);
							uint8_t err = ERR_MISSING_PARAMETER;
							kegelomat_can_send_msg(CAN_MSG_ERROR, &err, 1);
						}
						break;

					// Gesamtpunktestand setzen
					case CAN_MSG_SSET_RELAY:
						if(msg.length > 1)
						{
							uint16_t data = (msg.data[0] << 8) | msg.data[1];
							kegelomat_relay_score_set((data % 10000));
						} else {
							kegelomat_led_blink(LED_D, 1);
							uint8_t err = ERR_MISSING_PARAMETER;
							kegelomat_can_send_msg(CAN_MSG_ERROR, &err, 1);
						}
						break;

					// Fehler
					default:
						kegelomat_led_blink(LED_D, 2);
						uint8_t errmsg[3];
						errmsg[0] = ERR_UNKNOWN_FUNCTION;
						errmsg[1] = (msg.id >> 8);
						errmsg[2] = (msg.id & 0xff);
						kegelomat_can_send_msg(CAN_MSG_ERROR, errmsg, 3);
						break;
					}
				} else {
					kegelomat_led_blink(LED_D, 1);
					uint8_t err = ERR_CAN;
					kegelomat_can_send_msg(CAN_MSG_ERROR, &err, 1);
				}
			}
		}
#ifdef CANINTRV
	}
#endif
}

void kegelomat_can_send_msg(uint8_t type, uint8_t* data, uint8_t length)
{
	can_t msg;

	uint16_t addr = (type << 5) | canAddress;
	msg.id = addr;
	msg.flags.rtr = 0;
	if(data != NULL)
		memcpy(msg.data, data, length);
	msg.length = length;

	can_send_message(&msg);

	kegelomat_led_blink(LED_C, 1);
}

void kegelomat_can_send_bootup()
{
	uint8_t data[3];
	data[0] = FW_VERSION;		// Version der Firmware
	data[1] = 0b00000000;		// Stellung Relais 9-16 //TODO
	data[2] = 0b00000000;		// Stellung Relais 1-8	//TODO
	kegelomat_can_send_msg(CAN_MSG_BOOTUP, data, 3);
}

void kegelomat_can_send_keepalive()
{
	uint8_t data[3];
	data[0] = FW_VERSION;		// Version der Firmware
	data[1] = 0b00000000;		// Stellung Relais 9-16 //TODO
	data[2] = 0b00000000;		// Stellung Relais 1-8	//TODO
	kegelomat_can_send_msg(CAN_MSG_KEEPALIVE, data, 3);
}
