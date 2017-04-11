/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    http://www.gnu.org/licenses
    
    Creator: Arpad Toth (mrx23dot) - 2012
	//---------------------------------------------------------
	Editor: Tran Hoang Luan (hoangluan01@gmail.com) - Nov/2012
*/

#ifndef EEPROM_H
#define EEPROM_H 1

	//Includes
	#include "GlobalDefinitions.h"
	#include <avr/io.h>
	#include <avr/eeprom.h>	//Not int driven
	#include <util/delay.h>

	//Prototypes
	extern void Write_EEPROM(unsigned char address, unsigned char value);
	extern unsigned char Read_EEPROM(unsigned char address);

	//Global vars

#endif
