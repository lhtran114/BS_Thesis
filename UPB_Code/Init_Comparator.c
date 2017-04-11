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
#include "init_Comparator.h"


void Init_Comparator(void)
{
	// + input: AIN0 : Vref
	// - input: AIN1 : signal
	
	//Comparator Enable/Disable by define

	//ADCSRB - don set this
	ACSR = (1<<ACD)|(1<<ACIE)|(1<<ACIS1);//0b10001010
	//Disable comparator,No intern Vref,interrupt disabled,
	//Falling edge interrupt

	//Comparator functions enabled by ZeroCross/Timer interrupts, not here

	/*	//fPWM->RC filter->Vref used
	//Enable internal Vref 1.1V
	bit_set(ACSR,BIT(ACBG));	//need 40uS to stabilise
	*/

	/*
	INIT BUT DONT START, (Dont enable interrupt)!!!
	disable int before setting!
	Vref on -> need time to stabilise
	*/

	//DIDR1=0b00000011;	//Disable Digital input for AN1/0 pin -> save power
	//just has in ATmega88 -> Atmega16 doesn't has!
}
