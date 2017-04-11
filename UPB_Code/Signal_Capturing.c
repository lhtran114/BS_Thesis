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
#include "Signal_Capturing.h"

//Interrupt with comparator
#define Pulse_Samples			5	//Crystal based
#define Min_Pulse_Lenght		3

//Extern
unsigned char Current_Noise_Threshold = Min_Noise_Threshold;	//from define

static inline unsigned char Analyze_Pulse_Position (unsigned int );	//first pulse's time received

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

ISR(ANA_COMP_vect)	//Data input interrupt, Analog Comparator Handler
{
	//No Cli()!!! Auto-disabled!! 
	
	//Signal went High!
	
	unsigned int Saved_Time = TMR1;	
	//Save the 16bit time to Saved_Time for further analyzes	
	
	if (TXmode == 1)	//safety
		return;

	//We are here after Offset after AGC, but make sure!	
	if (Saved_Time < Offset_Nominal || Saved_Time > Offset_Nominal + Pos3_U + Max_Delay_For_Both)
		return;		//it was invalid trigger

	unsigned char Valid_cnt = 0;
	
	//Time window matched, now Over-sample input
	//filterint doesnt affect result!
	for(unsigned char i=0;i<Pulse_Samples;i++)
	{
		if(Signal_Input) 
			Valid_cnt++;
	}
	
	if (Valid_cnt < Min_Pulse_Lenght)
		return;		//return from interrupt, input pulse was to short

	//Pulse long enough!
	
	Bit_in_Rel_Time = Saved_Time - Offset_Nominal;	//Includes delay!!

	Double_Bit_buffer = Analyze_Pulse_Position( Bit_in_Rel_Time );

	Double_Bit_in_flag = 1;		//this is a valid pulse

	//Double_Bit_in_flag needed to be 1 even if Double_Bit_buffer is rubbish
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	//Disable comparator totally, also in ZeroCross int!!
	//if no input this interrupt wont be called!

	Disable_Comp_Int;		//Dont change order
	Disable_Comparator;		//Now we dont need it
	
	//sei() NEVER ADD THIS!!!!!!!!!

}	//ISR(ANALOG_COMP_vect)

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static inline unsigned char Analyze_Pulse_Position (unsigned int Rel_Pulse_Pos)	//first pulse's time received
{
	//returns pulse number, normally 0-3, default invalid: 255
	//Windows defined in globaldefs
	//Dont need to set Double_Bit_in_flag here!
	
	//Rel_Pulse_Pos = 0 + delay

	if (Current_Half)   	//Positive half wave delay
		Rel_Pulse_Pos -= Positive_Delay;
	else					//Negative half wave delay
		Rel_Pulse_Pos -= Negative_Delay;

	//Now 'Rel_Pulse_Pos' is Relative!!! 0-...
	
	//4 windows Upper+Lower Limits
		 if (Pos0_L<= Rel_Pulse_Pos && Rel_Pulse_Pos <= Pos0_U) return 0;
	else if (Pos1_L < Rel_Pulse_Pos && Rel_Pulse_Pos <= Pos1_U) return 1;
	else if (Pos2_L < Rel_Pulse_Pos && Rel_Pulse_Pos <= Pos2_U) return 2;
	else if (Pos3_L < Rel_Pulse_Pos && Rel_Pulse_Pos <= Pos3_U) return 3;

	return 255;	//didnt match any window
}
