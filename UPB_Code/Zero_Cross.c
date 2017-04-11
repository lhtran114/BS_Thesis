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

//include file
#include "Zero_Cross.h"
#include "Fire_Triacs.h"
#include "RX.h"
//#include "TX.h"
#include "Sensor_Input.h"
#include "Button_Input.h"
#include "UART.h"
#include "Dimmer_Light.h"
//
volatile unsigned char Current_Half;
volatile unsigned char CAP_Active;
/******************************************************************************
 * ++ 
 * Method name: Pins_Init
 * Description: ...
 * Parameters:	none
 * Return values: none
 * Modifications:	16.Sep.12 - Tran Hoang Luan 
 *								+ Function first created
 * -- 
******************************************************************************/

void Zero_Cross_Init(void)
{	
	
	Current_Half = LOW;
	CAP_Active = 0;
	//INT0 For ZeroCross!
	//INT0 sense control, Interrupt enabled by macro NOT here!
	MCUCR=(1<<ISC00);//Any logical change on INT0 will generate an interrupt
}
/******************************************************************************
 * ++ 
 * Method name: Step_5min_SetupMode
 * Description: setup mode auto off after 5 minutes. In 30 seconds left,
 * 		setup led will be blinked to remind user.
 * Parameters:	none
 * Return values: none
 * Modifications:	01.Dec.2012 - Tran Hoang Luan 
 *								+ Function first created
 * -- 
******************************************************************************/
static inline void Step_5min_SetupMode(void)
{
	if (SETUP_mode != 1) return;	//Not in setup mode, dont step

	//5min = 300sec/10e-3= 30000
	//10mS x 256 = 2.56sec

	if (Setup_5min_Divider_16bit != 0)
	{
		Setup_5min_Divider_16bit--;
		if(Setup_5min_Divider_16bit <= 3000)//30 seconds left
		{
			if((Setup_5min_Divider_16bit % 50) == 0)//blinking setup led every 0.5 second
				Red_LED_Toggle;
		}
	}
	else
		Disable_SETUP_mode();	//Reset counters, clear SETUP_mode

}
/******************************************************************************
 * ++ 
 * Method name: ISR(INT1_vect)
 * Description: ...
 * Parameters:	none
 * Return values: none
 * Modifications:	16.Sep.12 - Tran Hoang Luan 
 *								+ Function first created
 * -- 
******************************************************************************/
ISR(INT0_vect)	//Zero crossing interrupt
{
	//ONLY TRIGGERED IF LEVEL CHANGE ON INPUT!!!! -> 
	//Trully triggered only at zerocrosses!

	//Filter Zero Cross =================================================
	if(TMR1_on && (TMR1<Sine_Wave_Min_Period)) return;
	if(TMR1_on && (TMR1>Sine_Wave_Max_Period)) return;
	
	//oversample to make sure zero cross is >15us
	for (unsigned char i=0;i<ZeroCross_Sample_Number;i++) 
	{
		if (Zero_Cross_Signal == Current_Half)
			return;	//compare with the old value
					//current_half is now obsolete!!!		
		//AT STARTUP only will sync with positive input, because current_half = 0
	}
	
	//at here, we sure this is real zero cross!!!

	//restart timer 1 before charging cap (maybe charging cap will cause some faulse))
	Reset_Timer1;
	Start_Timer1;
	
	//reset watchdog!
	wdt_reset();
	
	//change state of current half wave -> NEED for RX!!
	if (Zero_Cross_Signal)//High
		Current_Half = HIGH;
	else
		Current_Half = LOW;
		
	//after real zero cross, we dont want to re interrupt
	Disable_Zerocross_Interrupt;//it will be enabled after timer1 comp A
	
	//Disable comparator totally, also in Comp change ISR!!
	Disable_Comp_Int;		//Dont change order! (@@ I need to ask Arped Toth why :|)
	Disable_Comparator;
	
	/////////////////////////////////////
	////////////////*********************
	//do what you want here
	UART_Data_Clear;
	Step_Button_counters();
	Step_Current_Output();
	Step_5min_SetupMode();
	////////////////*********************
	/////////////////////////////////////
	
	// *****************vref file*******************
	//reset noise threshold!
	Current_Noise_Threshold = Min_Noise_Threshold;
	//send out new noise threshold
	Set_Ext_Vref_To_Current_Level();
	
	//if Txmode -> we need to charge capacitor!
	if (TXmode == 1 && Double_Bit_out_flag == 1 && Double_Bit_buffer < 4)
	{
		_delay_us(500);
		Fire_Triacs();	//Charge CAP
		CAP_Active = 1;	
	}
}
