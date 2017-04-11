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
#ifndef BUTTON_INPUT_H
#define BUTTON_INPUT_H 1

	//Includes
	#include <avr/io.h>
	#include "GlobalDefinitions.h"
	#include "EEPROM.h"
	#include "TX.h"
	#include "Zero_Cross.h"
	#include "RX.h"

	//definition	
	#define Number_of_Command 9
	//Prototypes
	extern void Step_Button_counters (void);	//Called from ZeroCross_counter
	//void Generate_TX_Message (unsigned char , unsigned char );

	//void Button_Triggered (unsigned char , unsigned char );

	//Global vars
	extern unsigned char Button_TX;
	extern unsigned char Button_Command[Number_of_Command];
	extern unsigned char DID_Button;
	extern volatile unsigned char Need_To_TX_Button_Action;
	extern unsigned char TX_Payload_length_Button;
	extern unsigned char Button_Command_Index;//0-7
	//extern unsigned char Button_Command[];	

#endif
