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


#ifndef ZEROCROSS_H
#define ZEROCROSS_H 1

	//Includes	
	#include <avr/io.h>
	#include <avr/interrupt.h>
	#include <avr/wdt.h>
	#include <util/delay.h>	
	#include "GlobalDefinitions.h"
	#include "Vref.h"
	//#include "myLCD.h"
	//Prototypes
	void Zero_Cross_Init(void);

	//Global vars
	extern volatile unsigned char Current_Half;
	extern volatile unsigned char TXmode;
	extern volatile unsigned char Double_Bit_Out_Flag;
	extern volatile unsigned char CAP_Active;
	extern volatile unsigned char Need_To_TX_Button_Action;

#endif
