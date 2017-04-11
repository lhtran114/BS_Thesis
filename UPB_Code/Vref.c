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

#include "Vref.h"
	
volatile unsigned char Pre_State = 0;


void Init_Vref(void)
{
	//TMR2 PWM, Filtered For VREF!!
	TCCR2 = (1<<WGM21)|(1<<WGM20)|(1<<COM21)|(1<<CS20);	//Fast Pwm Clear OC2 on Compare Match, set OC2 at BOTTOM,			
	/*TCCR2 = (1<<CS22)|(1<<CS21)|(1<<CS20);
	TCNT2 = 100;
	bit_set(TIMSK,BIT(TOIE2));*/
}

void Set_Ext_Vref_To_Current_Level (void)
{
	OCR2 = Current_Noise_Threshold;
}
