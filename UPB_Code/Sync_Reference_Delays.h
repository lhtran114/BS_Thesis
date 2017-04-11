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


#ifndef SYNC_REFERENCE_DELAYS_H
#define SYNC_REFERENCE_DELAYS_H 1

	//Includes
	#include "GlobalDefinitions.h"
	#include "Zero_Cross.h"
	#include "RX.h"
	#include "Signal_Capturing.h"

	//Prototypes
	unsigned char Get_Sync_And_Ref_Delays(void);

	//Global vars
	extern unsigned char Positive_Delay;
	extern unsigned char Negative_Delay;


#endif
