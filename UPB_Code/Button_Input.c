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

#include "Button_Input.h"

///////////////////////////////////////////////////////////////////////////
//define
#define Single_Tap_Threshold	5 //~ 250mS < t < 750 mS
	//based on 50Hz, 10mS ZeroCross: 25<t<75
	//Double tap is not implemented
#define Double_Tap_Delay 		20
#define Hold_Threshold			80	//>750mS

//Set internal Pullups to Buttons! -> Pins_Init.c
#define Number_of_Buttons		2
#define Init_Arrays				{0,0}	//Number_of_Buttons

#define Trigger_1Tap	1	//indexes
#define Trigger_2Tap	2 	//double tap not implemented
#define Trigger_Hold	3
#define Trigger_Release	4
///////////////////////////////////////////////////////////////////////////////////

//variables

unsigned char Button_TX = 0;
unsigned char DID_Button = 0;
unsigned char _2Trigger_Hold = 0;
volatile unsigned char Need_To_TX_Button_Action = 0;

unsigned char Button_Command[Number_of_Command] = {Start_Setup_Mode_cmd,Stop_Setup_Mode_cmd,
						Auto_Address_cmd,Get_Register_Value_cmd,Set_Register_Value_cmd,
						Activate_Link_cmd,Deactivate_Link_cmd,Blink_cmd,Toggle_cmd};//9 commands
unsigned char Button_Command_Index = 0;//0-8
//VOLATILE??
static unsigned char Button_State_Count[Number_of_Buttons] = Init_Arrays;
static unsigned char Button_Release_Delay_Count[Number_of_Buttons] = Init_Arrays;
static unsigned char Single_Tap_Active[Number_of_Buttons] = Init_Arrays;
static unsigned char Double_Tap_Active[Number_of_Buttons] = Init_Arrays;
static unsigned char Hold_Active[Number_of_Buttons] = Init_Arrays;

//Prototypes
#warning "For debug"
static void Button_Triggered (unsigned char , unsigned char );
//static void Button_Triggered (unsigned char , unsigned char );
static void Button_Analyse (unsigned char ,unsigned char );

//--------------------------------------------------------------------------
// Analysing buttons
//--------------------------------------------------------------------------
static void Button_Analyse (unsigned char Button_State,unsigned char Button_Number)
{
	if (Button_State)
	{
		//Dont let it overflow
		if (Button_State_Count[Button_Number]<255 && !_2Trigger_Hold)
			Button_State_Count[Button_Number]++;
		//if both buttons are pressed for ~Hold time enter setup mode
		if (Button_State_Count[0] >= Hold_Threshold - 15
		&& Button_State_Count[1] >= Hold_Threshold - 15)
		{
			Button_State_Count[0] = 0;	//Reset count
			Button_State_Count[1] = 0;
			_2Trigger_Hold = true;
			if (SETUP_mode)
				Disable_SETUP_mode();
			else
				Enable_SETUP_mode();
		}
		//double tap
		if(Single_Tap_Active[Button_Number]&&//button already press before
		Button_Release_Delay_Count[Button_Number]<=Double_Tap_Delay)
			Double_Tap_Active[Button_Number]=true;

		if (Button_State_Count[Button_Number] == Hold_Threshold) //if bigger constant sending
		{
				Hold_Active[Button_Number] = 1;
				Button_Triggered(Trigger_Hold, Button_Number);
		}			
	}
	else
	{
		if (Single_Tap_Active[Button_Number] && Button_Release_Delay_Count[Button_Number]<255)
			Button_Release_Delay_Count[Button_Number]++;
		//
		if (Button_State_Count[Button_Number] > Single_Tap_Threshold &&
		Button_State_Count[Button_Number] < Hold_Threshold)
		{
			if(Double_Tap_Active[Button_Number])
			{	
				Button_Release_Delay_Count[Button_Number]=0;		
				Single_Tap_Active[Button_Number]=false;
				Double_Tap_Active[Button_Number]=false;
				Button_Triggered(Trigger_2Tap, Button_Number);
			}	
			else
				Single_Tap_Active[Button_Number] = true;
		}
		//
		if(Single_Tap_Active[Button_Number]	&&
			Button_Release_Delay_Count[Button_Number]>Double_Tap_Delay)	
		{	
			Single_Tap_Active[Button_Number]=false;
			Button_Release_Delay_Count[Button_Number]=0;	
			Button_Triggered(Trigger_1Tap, Button_Number);
		}
		//
		Button_State_Count[Button_Number] = 0;				//Reset count
		_2Trigger_Hold = false;
		//
		if (Hold_Active[Button_Number]) Button_Triggered(Trigger_Release, Button_Number);	//Release
		Hold_Active[Button_Number] = 0;
	}

}//Button_Analyse

//--------------------------------------------------------------------------

void Step_Button_counters (void)	//Called from ZeroCross_counter
{
	if (TXmode == 1) return;	//For safety wait until not TXing
	Button_Analyse(Button0_Pressed,0);
	Button_Analyse(Button1_Pressed,1);
}

//--------------------------------------------------------------------------
#warning "For debug"
static void Button_Triggered (unsigned char Trigger_Type, unsigned char Button_number)
//static void Button_Triggered (unsigned char Trigger_Type, unsigned char Button_number)
{
	//Trigger_Type is index:
	//Trigger_1Tap 1, Trigger_Hold 3, Trigger_Release 4, 
	if(Trigger_Type == Trigger_1Tap)
	{
		switch (Button_number)	//0,1
		{		
			case 0:
				if(Button_Command_Index==0)
					Button_Command_Index = Number_of_Command-1;
				else
					Button_Command_Index--;
				break;
			case 1:
				Need_To_TX_Button_Action = 1;
				Button_TX = 1;//TX from button
				break;
			default:
				return;	
		}		
	}
	if(Trigger_Type == Trigger_2Tap)
	{
		switch (Button_number)	//0,1
		{		
			case 0:
				Local_Light_Level_Set(0,0);//(level,fade)
				break;
			case 1:
				Local_Light_Level_Set(100,0);//(level,fade)
				break;
			default:
				return;	
		}		
	}
	else if(Trigger_Type == Trigger_Hold)
	{
		switch (Button_number)	//0,1
		{		
			case 0:// level down
				Local_Light_Level_Set(0,2);//(level,fade)				
				break;
			case 1://level up
				Local_Light_Level_Set(100,2);//(level,fade)				
				break;
			default:
				return;
		}
	}
	else if(Trigger_Type == Trigger_Release)
	{
		switch (Button_number)	//0,1
		{		
			case 0:
				Local_Light_Level_Set_Stop_Fading();				
				break;
			case 1:
				Local_Light_Level_Set_Stop_Fading();				
				break;
			default:
				return;
		}
	}
}
