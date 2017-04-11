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

#include "EEPROM.h"

//Read:unlimited, Write LIMITED!
/*
 The function needs the address of a variable that is to be 
 written in the EEPROM. In other words, it needs the EEPROM 
 memory address of where to write the variable.
 */

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void Write_EEPROM(unsigned char address, unsigned char value)
{
	//EEPROM has limited lifespan of 100,000 writes - reads are unlimited
	eeprom_busy_wait();		//included in eeprom_write_byte
	eeprom_write_byte( (unsigned char *) (unsigned int) address, value );

}	//Write_EEPROM

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

unsigned char Read_EEPROM(unsigned char address)
{
	eeprom_busy_wait();		//included in eeprom_read_byte
	return eeprom_read_byte( (unsigned char *) (unsigned int) address );

}	//Read_EEPROM
