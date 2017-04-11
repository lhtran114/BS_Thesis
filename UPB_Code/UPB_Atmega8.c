/****************************************************************************** 
 * ++ 
 * Author:    Tran Hoang Luan (LuanTH)
 * Module Name  :  main.c
 * Description:
 *
 * Mod. History :	16.Sep.12	- Tran Hoang Luan  
 *									+	File first created 
 * --    
*******************************************************************************/ 

//No main.h !

#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>


#include "Dimmer_Light.h"
#include "Button_Input.h"
#include "UART.h"
#include "Init_Comparator.h"
#include "Fire_Triacs.h"
#include "GlobalDefinitions.h"	//includes Protocol_defines.h #include "Pins_Init.h"

//#include "I2C.h"
#include "RX.h"
#include "Signal_Capturing.h"
#include "Sync_Reference_delays.h"
#include "Timers.h"
#include "TX.h"
#include "Vref.h"
#include "Zero_Cross.h"
#include "EEPROM.h"
#include "Sensor_Input.h"
//#include "myLCD.h"

//Prototype
extern void TX_Multi_Packet(unsigned char command);

/******************************************************************************
 * ++ 
 * Method name: main
 * Description: ...
 * Parameters:	none
 * Return values: ...
 * Modifications:	16.Sep.12 - Tran Hoang Luan 
 *								+ Function first created
 * -- 
******************************************************************************/
int main(void)
{	
	//init somethings
	Pins_Init();
	Init_Timers();//Only init, dont start them!
	Zero_Cross_Init();//only init
	Init_Comparator();//Only init, dont enable it yet
	Init_Vref();
	Sensor_Init();
	uart_Init();
	set_sleep_mode(SLEEP_MODE_IDLE);	//all peripheral clocks are still running	

	init_Variable_buffers();//in RX.c
	
	Enable_Zerocross_Interrupt;

	wdt_enable(WDTO_30MS);	
	//ZeroCross'll always reset this, NEED TO CUT PW after change&programming!!!

	sei();	//Enable global interrupts
	Green_LED_On;
	//internal loop
	while(true)
	{	
	
		RX();	//RX always	
		if (RX_overtime_flag == 1)
			RX_overtime_flag = 0;
		//ONE CYCLE EVERY HALF WAVE!	
		
		if (Need_To_TX_UART == 1 && !SETUP_mode)
		{
			Need_To_TX_UART = 0;
			TX_Multi_Packet(UARTCommand);//Load NID,DID,SID,MDID,Args	
		}	
		
		
		if (Need_To_TX_Button_Action == 1 && !SETUP_mode)
		{
			Need_To_TX_Button_Action = 0;
			TX_Multi_Packet(Button_Command[Button_Command_Index]);//Load NID,DID,SID,MDID,Args			
		}	
		
	}
	return 0;
}	//main


///

void TX_Multi_Packet(unsigned char command)	//only called once!
{
	TXmode = 1;	//TX
	
	Header_Byte[0] = 0;
	Header_Byte[1] = 0;
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//Link Packet Enabled/Disabled
	if (Link_UART)
		bit_set( Header_Byte[0], BIT(7) );
	else
		bit_clear( Header_Byte[0], BIT(7) );
	//if (DID == 0x00) LINK BIT IS CLEARED IN TX() !!!	

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//Acknowledge Message Request
	if (ACK_Message_UART)
		bit_set( Header_Byte[1], BIT(6) );
	else
		bit_clear( Header_Byte[1], BIT(6) );	
		
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//ID Pulse Request
	if (ID_Pulse_UART)
		bit_set( Header_Byte[1], BIT(5) );
	else
		bit_clear( Header_Byte[1], BIT(5) );
		
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@	
	//ACK Pulse Request
	if (ACK_Pulse_UART)
		bit_set( Header_Byte[1], BIT(4) );
	else
		bit_clear( Header_Byte[1], BIT(4) );	
		
/*	
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@	
	//Transmit Count, how many times will be TXed 
	Header_Byte[1] &= 0b11110000;		//Clear CNT & SEQ
	Header_Byte[1] |= (How_many_times_to_TX - 1) << 2;
	
	//1-4, dont have to recalc

	for (unsigned char i=0; i<How_many_times_to_TX ;i++)
	{		
		//Transmit Sequence
		Header_Byte[1] &= 0b11111100;	//Clear SEQ
		Header_Byte[1] |= i;

		TX(command);	//This will also Recalc Checksum
		
		//1 ACK might acour, then not empty half is the next
		//Wait empty_halfwaves in Next TX()

		//For Repeater this MUST be known exactly!
		
	}	//for
*/		
	TX(command);	//This will also Recalc Checksum	
	TXmode = 0;	//TX
}	//TX_Multi_Packet

