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
#ifndef RX_H
#define RX_H 1

	//Includes
	#include "GlobalDefinitions.h"
	#include <avr/sleep.h>
	#include <avr/io.h>
	#include <util/delay.h>
	#include <avr/interrupt.h>	//For reset, Cli(	
#include "Sensor_Input.h"
#include "Dimmer_Light.h"
#include "UART.h"
	#include "Sync_Reference_Delays.h"

	//Prototypes
	extern void RX(void);
	extern void init_Variable_buffers(void);
	extern unsigned char Random_Byte(void);
	extern void Disable_SETUP_mode(void);				//also called from ZeroCross count
	extern void Wait_1_empty_halfwaves (void);		//Also for Tx!
	extern unsigned char Calced_checksum (void);		//For TX & RX
	extern void Wait_New_Halfwave (void);
	extern void Enable_SETUP_mode(void);
	extern void TX_Multi_Packet(unsigned char command);
	unsigned char Collect_Double_Bit (void);	//Returns 255 if overtime, 0-3 normally

	//Global vars
	extern unsigned char Payload_Byte[18];	//for max length
	extern unsigned char Header_Byte[5];		//CTL0, CTL1, NID, DID, SID
	extern unsigned char Checksum_Byte;

	extern volatile unsigned char TXmode;
	extern volatile unsigned char RX_overtime_flag;
	extern volatile unsigned char current_half;
	extern volatile unsigned char Double_Bit_buffer;
	extern volatile unsigned char Double_Bit_in_flag;
	extern volatile unsigned int  Bit_in_Rel_Time;
	extern volatile unsigned char SETUP_mode;
	extern volatile unsigned char CAP_active;
	
	extern volatile unsigned char ID_Exist[32];
	extern volatile unsigned char ACK_Pulse_Flag;

	extern unsigned char TMR_Setup;
	extern unsigned char Device_Status_Register;
	extern unsigned char Setup_Registers_number;

	extern volatile unsigned int Setup_5min_Divider_16bit;
	extern unsigned char Unit_ID_fast;
	extern unsigned char Unit_Network_ID_fast;
	extern unsigned char LED_Options_Fast;

	//TEMP!!!! Swap with define

#endif
