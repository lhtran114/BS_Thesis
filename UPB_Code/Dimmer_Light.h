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
#define Light_On	bit_set  (PORTD,BIT(Dimmer_Output_Pin))
#define Light_Off	bit_clear(PORTD,BIT(Dimmer_Output_Pin))


#ifndef DIMMER_H
#define DIMMER_H 1

//include
	#include "GlobalDefinitions.h"
	#include <avr/io.h>
	//#include "ZeroCross_Counters.h"

	//Prototypes
	void Set_Output_level(void);
	void Step_counters( void );
	void Step_Current_Output(void);
	void Local_Light_Level_Set_Stop_Fading(void);
	void Local_Light_Level_Set(unsigned char , unsigned char );

	//Global vars
	//extern unsigned char current_output_lev;
	extern unsigned char Default_FadeRate;
	extern unsigned char Dimmer_Mode;
	extern unsigned char Auto_Report_State_Enabled;
	extern unsigned char desired_output_lev;

#endif
