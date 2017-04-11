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

//Includes
#include "Dimmer_Light.h"

//IF SOMETHING DOESNT WORK USE VOLATILE!!!!!!!!!!!!!!!
//non volatile
unsigned int  Divider = 0;
unsigned char current_output_lev = 0;
unsigned char desired_output_lev = 0;
unsigned char FadeRate = 0;
unsigned char Default_FadeRate = 0;
unsigned char Dimmer_Mode = 1;
unsigned char Auto_Report_State_Enabled = 0;

//-----------------------------------------------------------------------@

static inline void Apply_current_output_lev(void)
{
	//Lookup table
	//0% by disabling TMR compare

	//Max delay must be less than AGC_start!
/* OLD
	const unsigned int Percent_To_SinPW_To_TMR_Delay[100] =	
	{
		(AGC_start-40), 15189, 15170, 15144, 15110, 15069, 
		15021, 14965, 14902, 14832, 14755, 14670, 
		14579, 14481, 14376, 14264, 14146, 14021, 
		13890, 13753, 13609, 13460, 13305, 13144, 
		12978, 12807, 12630, 12448, 12262, 12071, 
		11876, 11676, 11473, 11265, 11054, 10840, 
		10622, 10402, 10178, 9953, 9724, 9494, 
		9262, 9028, 8793, 8557, 8319, 8081, 
		7843, 7604, 7365, 7127, 6889, 6651, 
		6415, 6180, 5946, 5714, 5484, 5255, 
		5030, 4806, 4586, 4368, 4154, 3943, 
		3735, 3532, 3332, 3137, 2946, 2760, 
		2578, 2401, 2230, 2064, 1903, 1748, 
		1599, 1455, 1318, 1187, 1062, 944, 
		832, 727, 629, 538, 453, 376, 306, 
		243, 187, 139, 98, 64, 38, 19, 8, 4	//will triac trigger on minimum?
	}; 
*/

/* Used for 20Mhz frequency
	const unsigned int Percent_To_SinPW_To_TMR_Delay[100] =	
	{
		(AGC_start-240), 14992, 14978, 14959, 14935, 
		14905, 14870, 14830, 14784, 14734, 14678, 
		14616, 14550, 14479, 14403, 14322, 14237, 
		14147, 14052, 13952, 13849, 13741, 13628, 
		13512, 13392, 13268, 13140, 13009, 12874, 
		12736, 12594, 12450, 12302, 12152, 12000, 
		11845, 11687, 11527, 11366, 11202, 11037, 
		10871, 10703, 10533, 10363, 10192, 10020, 
		9848, 9675, 9503, 9330, 9157, 8985, 8813, 
		8642, 8472, 8303, 8135, 7968, 7803, 7640, 
		7478, 7318, 7161, 7006, 6853, 6703, 6556, 
		6411, 6270, 6132, 5997, 5865, 5738, 5614, 
		5493, 5377, 5265, 5157, 5053, 4954, 4859, 
		4769, 4683, 4602, 4526, 4455, 4389, 4328, 
		4272, 4221, 4176, 4135, 4100, 4070, 4046, 
		4027, 4014, 4005, 4003 	//will triac trigger on minimum?
	}; 
*/

//Used for 16Mhz frequency 50hz
	const unsigned int Percent_To_SinPW_To_TMR_Delay[100] =	
	{
		(AGC_start-192), 11993,11982,11967,11948,11924
		,11896,11864,11827,11787,11742,11692,11640,11583
		,11522,11457,11389,11317,11241,11161,11079,10992
		,10902,10809,10713,10614,10512,10407,10299,10188
		,10075,9960,9841,9721,9600,9476,9349,9221,9092
		,8961,8829,8696,8562,8426,8290,8153,8016,7878
		,7740,7602,7464,7325,7188,7050,6913,6777,6642
		,6508,6374,6242,6112,5982,5854,5728,5604,5482
		,5362,5244,5128,5016,4905,4797,4692,4590,4491
		,4394,4301,4212,4125,4042,3963,3887,3815,3746
		,3681,3620,3564,3511,3462,3417,3376,3340,3308
		,3280,3256,3236,3221,3211,3204,3202,	//will triac trigger on minimum?
	}; 

	//current_output_lev:0-100 [0]:Disable, [1-1]-[100-1]: ~1-100%

	if ( current_output_lev > 100 )		//For safety!
		return;

	if ( current_output_lev == 0)		//0%
		Disable_TMR1_compare_Light_int;

	else	//~1%-100% -> [0]-[99]
	{
		OCR1B = Percent_To_SinPW_To_TMR_Delay[current_output_lev-1];
		//First load compare value then enable interrupt!
		Enable_TMR1_compare_Light_int;	//Macro
	}
	
}	//Apply_current_output_lev

//------------------------------------------------------------------------

static inline void FrameRate_To_Divider(void)
{
	//This function wont be called if FrameRate is 0 (snap)
	
	//How many half should be left out when counting, Start from 0!
	const unsigned int FrameRate_To_Divider_Table[15] =
	{0, 1, 2, 4, 6, 9, 19, 29, 59, 119, 299, 599, 899, 1799, 3599};
	
	if (FadeRate > 15) 
		FadeRate = Default_FadeRate;	//For safety

	//1-15 shifted to 0-14, compensate
	Divider = FrameRate_To_Divider_Table[ FadeRate - 1 ];
	
}	//FrameRate_To_Divider

//------------------------------------------------------------------------

void Step_Current_Output(void)
{

	if (current_output_lev == desired_output_lev) return;	
	//nothing to do with the output

	if (Divider > 0)	//need to be 0 for next step
	{
		Divider--;
		return;
	}
	else		//time for a step, up or down?
	{
		if ( desired_output_lev > current_output_lev )	
			current_output_lev++; 
		else		
			current_output_lev--;

		//Apply only if current output has changed
		Apply_current_output_lev();

		//Reload the Divider for next output lev step
		FrameRate_To_Divider(); 
	}

}	//Step_Current_Output

//------------------------------------------------------------------------


void Set_Output_level(void)
{
	//------------------------------------------------------------------------

	//if NOT Dimmer mode ouput 
	//only can be 0/100 off/on with no delay

	if (Dimmer_Mode != 1)
	{
		FadeRate = 0;	//Snap: no delay
	
		//Round output to 100% or 0%
		if (desired_output_lev > 50)
			desired_output_lev = 100;
		else
			desired_output_lev = 0;
	}

	//------------------------------------------------------------------------

	if (FadeRate == 0)	//Snap
	{
		current_output_lev = desired_output_lev;
		Apply_current_output_lev();
		return;		//Nothing else to do if snap
	}

	FrameRate_To_Divider();	//if FrameRate > 15 default will be used

	/*current_output_lev will change to 
	desired in steps (ZeroCross based)
	Next ZeroCross will apply new level*/

}	//Set_Output_level


void Local_Light_Level_Set(unsigned char Level, unsigned char FadeRate_new)
{

	if (Level > 100) Level = 100;	//Safety
	if (FadeRate > 15) 
		FadeRate = Default_FadeRate;	//For safety

	//Make it faster
	if (desired_output_lev == Level && FadeRate == FadeRate_new)
		return;

	desired_output_lev = Level;
	
	FadeRate = FadeRate_new;	//>15 is handled!					
				
	Set_Output_level();

	//Start_Auto_Off_Timer();	//This also checks is it enabled!

}

void Local_Light_Level_Set_Stop_Fading(void)
{
	desired_output_lev = current_output_lev;
}
