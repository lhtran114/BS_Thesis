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

#include "Sync_Reference_Delays.h"
#include "UART.h"

//extern
unsigned char Positive_Delay = 0;
unsigned char Negative_Delay = 0;

unsigned char Validate_Pos_Set_One_Delay (unsigned int ,unsigned int );

//Positive_Delay, Negative_Delay is 8bit -> 255*0,5uS TMR -> 127uS max delay
//in PLCbus delay is ~20-40uS

unsigned char Get_Sync_And_Ref_Delays(void)
{
	//Valid if Sync: 2,1,1,2
	
	//From 2,1 we extract the delay caused by half wave polarity & LC filter
	Positive_Delay = 0;	 //Must reset Delays to RX new ones!!
	Negative_Delay = 0;

	Double_Bit_in_flag = 0;		//For safety

	Wait_1_empty_halfwaves();	//sleeps whole halfwave through
	//IF WAITING FOR 1 empty, must stay here in a loop, cant go back!!!

	//----------------------------------
	//Wait_New_Halfwave();

	//updated by interrupt, this is where the most of the time stays
	while (Double_Bit_in_flag != 1 && !Need_To_TX_Button_Action && !Need_To_TX_UART)	
		sleep_mode();
	
	
	if (Need_To_TX_Button_Action)
		return 0;	//Sync failed need to TX now!

	//we are still in the current half wave!!!! @comparator interrupt
	Wait_New_Halfwave();	//jump to the neerest ZeroCross!

	//if nothing received in this half wave return to RX(),to main
	//and recall Sync

	/*
	if (Double_Bit_in_flag != 1)
		return 0;	//failed to sync
	*/

	//Here we got pulse #2 possibly
	if ( Validate_Pos_Set_One_Delay(Bit_in_Rel_Time, Pos2_L + Pos_Steps / 2) != 1)	//nominal used!!!
		return 0;	//Sync failed

	//Double_Bit_in_flag == 1	
	Double_Bit_in_flag = 0;		//Reset input buffer for next halfwave

	//----------------------------------

	//Now need to RX pulse #1
	Wait_New_Halfwave();
//here ok!

	//if nothing received in this half wave return to RX(),to main
	//and recall Sync
	if (Double_Bit_in_flag != 1)
		return 0;	//failed to sync

	//Here we got pulse #1 possibly
	if ( Validate_Pos_Set_One_Delay(Bit_in_Rel_Time, Pos1_L + Pos_Steps / 2) != 1)
		return 0;	//Sync failed

	//Double_Bit_in_flag == 1	
	Double_Bit_in_flag = 0;		//Reset input buffer for next halfwave
	
	//----------------------------------
	//2,1,1,2
	//Got 2 reference delays, now we can receive pulses easily
	
	//Waits new half, collects result, decodes using correct delay!, clears flag
	if ( Collect_Double_Bit() != 1 )	//255 if overtime
		return 0;
	
	//Waits new half, collects result, decodes using correct delay!, clears flag
	if ( Collect_Double_Bit() != 2 )	//255 if overtime
		return 0;

	return 1; //default, Sync completed!!

}

unsigned char Validate_Pos_Set_One_Delay (unsigned int Rel_Position,unsigned int Nominal_Mid_Pos)
{
	//Sync pulses fired at optimal (0--fire(40uS)--80us) time	
	//+-40uS (Low/High) is the window
	
	//SIGNAL ONLY CAN LAG!!!!! fired in the middle
	if ( Rel_Position < Nominal_Mid_Pos )
	{

		return 0;  //Pulses only can LAG!!!
	}

	if ( Rel_Position - Nominal_Mid_Pos > Max_Delay_For_Both) return 0;

	//invert because pulse was RXed in previous half!!!!!
	//only need to invert here!
	if (!Current_Half)   //Positive half wave
		Positive_Delay = (char) (Rel_Position - Nominal_Mid_Pos);
	else				//Negative half wave
		Negative_Delay = (char) (Rel_Position - Nominal_Mid_Pos);
	
	return 1;	//Default: return success
}

