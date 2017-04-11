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

#include "Fire_Triacs.h"

/******************************************************************************
 * ++ 
 * Method name: Fire_Triacs
 * Description: this function open triac to charge or discharge cap
 * Parameters:	none
 * Return values: none
 * -- 
******************************************************************************/

void Fire_Triacs (void)
{	
	Triac_Trigger_On;
	_delay_us(Triac_On_Response_Time);
	Triac_Trigger_Off;
	
	//After this: Triac(s) will auto 
	//stop charging when current < Ihold

}	//Fire_Triacs
