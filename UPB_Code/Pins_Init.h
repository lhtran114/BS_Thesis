/****************************************************************************** 
 * ++ 
 * Author:    Tran Hoang Luan (LuanTH)
 * Module Name  :  Pins_Init.h
 * Description:
 *
 * Mod. History :	16.Sep.12	- Tran Hoang Luan  
 *									+	File first created 
 * --    
*******************************************************************************/ 

#ifndef PINS_INIT_H
#define PINS_INIT_H 1

//NEW_BOARD OLD_BOARD
#define NEW_BOARD
	//Includes
	#include <avr/io.h>
	
	//definition
	//PORTB
	#define Red_Led 0
	#define Green_Led 1
	#define Blue_Led 2
	#define PWM_Noise_Threshole 3
	////
	
	//PORTC was preserved!
	#define Light_Sensor_Pin 0
	#define Temp_Sensor_Pin 1
	#define Button0_Pin	3
	#define Button1_Pin	4
	#define Button2_Pin	5
	//PORTD
	#define Zero_Cross_Pin 2
///////////////////////////////////
 //choose board	
	#ifdef NEW_BOARD
		#define Signal_Generate_Pin 3
		#define Dimmer_Output_Pin 4
	#endif
	#ifdef OLD_BOARD	
		#define Signal_Generate_Pin 4
		#define Dimmer_Output_Pin 3
	#endif
/////////////////////
	#define Signal_Detect_Pin 7	
	
	//Prototypes
	void Pins_Init(void);// initial all pin of MCU

	//Global vars	

#endif
