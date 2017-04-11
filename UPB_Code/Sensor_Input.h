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
#ifndef SENSOR_INPUT_H
#define SENSOR_INPUT_H 1

	//Includes
	#include <avr/io.h>
	#include "GlobalDefinitions.h"

	//protorype
	extern void Sensor_Init(void);
	extern void Read_Sensor(void);
	extern uint16_t ADC_Convert(unsigned char);

	//variable
	extern volatile unsigned char Need_To_Read_Sensor;
	extern volatile uint16_t Light_Value;
	extern volatile uint16_t Temp_Value;

#endif
