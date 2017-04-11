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

#ifndef TX_H
#define TX_H 1

	//Includes
	#include "GlobalDefinitions.h"
	#include <avr/sleep.h>
	#include <avr/io.h>
	#include <util/delay.h>
	#include "Button_Input.h"

	//Prototypes
	extern void TX (unsigned char TX_Argument);

	//Global vars
	extern volatile unsigned char TXmode;
	extern unsigned char Payload_Byte[18];	//for max length
	extern unsigned char Header_Byte[5];		//CTL01, CTL1, NID, DID, SID
	extern unsigned char Checksum_Byte;
	extern volatile unsigned char Double_Bit_out_flag;
	extern unsigned char Unit_ID_fast;
	extern unsigned char Unit_Network_ID_fast;
	
	extern volatile unsigned char Number_of_Register;

#endif
