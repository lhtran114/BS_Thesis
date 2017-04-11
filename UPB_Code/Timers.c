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


#include "Timers.h"
#include "Fire_Triacs.h"

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void Init_Timers(void)
{
	//Enable Timer1 compareA int
	TCCR1A = 0;		//Normal mode, not CTC, need to manually clear
					//Wont automatically clear on compare -> good for Phase split
	OCR1A = AGC_start;	//Always start ISR there!!
	Enable_CompA_Int;	//Enable interrupt on compare 1A
	
	//Start of timer + Prescaler setting in MACRO
}	//init_Timers

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static inline void Do_Automatic_Noise_Level (void)
{
	//Do AGC here
	//AGC_start < t < Offset
	while ( TMR1 < Offset_Nominal )
	{
		//Accurate threshols level cannot be shown on soundcard scope!!!

		if (Signal_Input)	//Signal (now noise) is higher than Vref
		{
		//Increase_noise_threshold
		//Slew rate: ?V/uS based on RC & PWM frequency

		//Check for overflow
		if (255 - Current_Noise_Threshold < Noise_Threshold_Step)
			Current_Noise_Threshold = 255;	
		else
			Current_Noise_Threshold += Noise_Threshold_Step;

		//Set noise threshold, Vref set time: ?? !!
		Set_Ext_Vref_To_Current_Level(); //
		
		//Some delay is needed, or max will be reached instantly
		
		//Not based on TMR1, use simple delay
		_delay_us(15);	//RC filter has time to adjust
		
		//Offset-AGC_start ~ 360uS, must rise to 255 with Noise_steps @ this time!
		}
	}

}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


//_________________Timer1____________________

ISR(TIMER1_COMPA_vect)	//Timer1 Compare A Handler
{
	//ALWAYS GETS HERE AT AGC_start !!!
	
	//No Cli()!!! Auto-disabled!! 	

	//AGC_start < Offset < UPB window

	//RX mode: Start AutoGainControl
	//TX mode: Wait for Offset -> short time -> no sleep
//Disable_UART;
	if (TXmode != 1)	//RXmode -> Enable pulse sensing
	{
		Enable_Comparator;	//Dont change order!
		Disable_Comp_Int;
		Do_Automatic_Noise_Level();
		Enable_Comp_Int;
	}
	else	//TXmode -> If pulse needed, discharge CAP(s) in correct time
	{		
		if (CAP_Active == 1 && Double_Bit_out_flag == 1 && Double_Bit_buffer < 4)
		{
			//Make sure comparator is Disabled in ZeroCross ISR!
			//Offset for TX!  offset+40uS
			while(TMR1 < Offset_Nominal + Half_Pulse_Width);	//Wait for the correct time
			for (unsigned char i=0; i<Double_Bit_buffer ;i++)	//0-3 x 160uS
			{
				_delay_us(160);	//max 48uS precise @16Mhz
			}
			Fire_Triacs();		//This will discharge cap(s)
			Double_Bit_out_flag = 0;
			CAP_Active = 0;		//CAP depleted
		}
	}
	Enable_Zerocross_Interrupt;	//Cant enable it sooner/later	
	//sei() NEVER ADD THIS!!!!!!!!!
}	//ISR(TIMER1_COMPA_vect)

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

//Dimmer triac fire, if Off: this interrupt disabled
ISR(TIMER1_COMPB_vect)	//RX timeout, Timer2 Compare A Handler
{
	//if (isOverHeated)
	//	return;
	
	Activate_Load_Triac;	//Macro
	//This is only for triggering!, ZeroCross will switch of triac!
	_delay_us(Triac_On_Response_Time);
	Deactivate_Load_Triac;	//at the end of previous half
	

}	//ISR(TIMER1_COMPB_vect)

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

//_________________Timer2_8bit_______________

//Debug
//For debug generate 50Hz square
ISR(TIMER2_COMP_vect)	//RX timeout, Timer2 Compare A Handler
{
/*
	//Used for else
	if (bit_get(PORTD,BIT(3)) )
		bit_clear(PORTD,BIT(3));
	else
		bit_set(PORTD,BIT(3));
*/
}	//ISR(TIMER2_COMPA_vect)

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
