/****************************************************************************** 
 * ++ 
 * Author:    Tran Hoang Luan (LuanTH)
 * Module Name  :  Pins_Init.c
 * Description:
 *
 * Mod. History :	16.Sep.12	- Tran Hoang Luan  
 *									+	File first created 
 * --    
*******************************************************************************/ 
#include "Pins_Init.h"

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

void Pins_Init(void)
{
	//bit_set(SFIOR,BIT(PUD));	//Disable Pull ups
	//bit_clear(SFIOR,BIT(PUD));	//Enable  Pull ups	
	
	//output:1 input:0	
	DDRB = 0b00001111;
	DDRC = 0b00000000;
	DDRD = 0b00010010;
	//DDRD = 0b00011010;
	//To pull ups for Buttons
	PORTB = 0b11111111;
	PORTC = 0b01111000;//pull up for 3 buttons!
	PORTD = 0b00000011;
}
