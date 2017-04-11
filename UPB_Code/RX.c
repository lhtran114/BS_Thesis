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

#include "RX.h"
#include "TX.h"	//because needed to send
#include "EEPROM.h"
#include "Protocol_defines.h"
#include "Fire_Triacs.h"
#include "Zero_Cross.h"

//These are externs!
	
//for max length
unsigned char Payload_Byte[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 
//CTL0, CTL1, NID, DID, SID
unsigned char Header_Byte[5] = {0,0,0,0,0}; 
unsigned char Checksum_Byte = 0;
//Faster to read from here than from EEPROM!
unsigned char Link_ID_fast[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 
unsigned char Unit_ID_fast;		//not eeprom, extern
unsigned char Unit_Network_ID_fast = 1;		//not eeprom, extern
//ID was exist
volatile unsigned char ID_Exist[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//
volatile unsigned char ACK_Pulse_Flag = 0;
volatile unsigned int Setup_5min_Divider_16bit = 0;

//unsigned char Setup_Registers_number = 0;

volatile unsigned char SETUP_mode =0;
volatile unsigned char RX_overtime_flag = 0;	//This is extern!
volatile unsigned char TXmode = 0;
volatile unsigned char Double_Bit_buffer = 0;
volatile unsigned char Double_Bit_in_flag = 0;
volatile unsigned int  Bit_in_Rel_Time = 0;

//
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static inline void increase_SETUP_entered_cnt(void)
{
	//Write_EEPROM(Read_EEPROM(Setup_Mode_Counter) + 1, Setup_Mode_Counter); //(value,address???) wrong?
	Write_EEPROM(Setup_Mode_Counter,Read_EEPROM(Setup_Mode_Counter) + 1);//edited by Luan (address,value)
}	//increase_SETUP_entered_cnt

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void init_Variable_buffers(void)	//Read before RX() because it take time to read
{	
	//!!!!DONT MODIFY TOO MUCH, OR VERYFICATION WILL FAIL!!!!!!

	unsigned char tmp=0;
	//Read LinkID-s to a fast array instead reading it every time from EEPROM

	//Also called when EEPROM writed!	

	//EEPROM could be remotely modified, check & correct here critical data!
	//DONT DELETE THIS!

	//___Read Unit_ID into fast SRAM___
	// if not 1-250 set to 1 
	Unit_Network_ID_fast = Read_EEPROM(Network_ID);	//1-255
	Unit_Network_ID_fast = Read_EEPROM(Network_ID);	//1-255
	//Sometimes autochanges Device address!!!
/*
	if (Unit_Network_ID_fast == 0)	//Fatal overwrite error
	{
		Unit_Network_ID_fast = Random_Byte();
		Write_EEPROM(Network_ID , Unit_Network_ID_fast);	//returns 1-250
	}
*/

	Unit_ID_fast = Read_EEPROM(Unit_ID);
	Unit_ID_fast = Read_EEPROM(Unit_ID);	//reads crap!!!
/*
	if (Unit_ID_fast == 0 || Unit_ID_fast > 250)
	{
		Unit_ID_fast = Random_Byte();			//returns 1-250
		Write_EEPROM(Unit_ID , Unit_ID_fast );	
	}
*/
	//_________________________________

	for (unsigned char i=0;i<16;i++)
		Link_ID_fast[i] = Read_EEPROM(LinkID_1 + i*3);

	//__________Last_Level_____________
/*
	//load last on state Power level 0-100%
	tmp = Read_EEPROM(Last_On_Level);	//Restore Last State

	//Ensure that it is 0-100
	if (tmp > 100)
	{
		Write_EEPROM(Last_On_Level , 100);
		tmp = 100;
	}
*/
	//_____Auto_Off_Timer_Fast_____
	//Auto_Off_Timer_Fast = Read_EEPROM(Auto_Off_Timer);
	//Good: 0-240,255 valid Time: 1-240 not handled here!
	//Bad: 241-254	//can be used in the future!
	/*
	if (Auto_Off_Timer_Fast > 240 && Auto_Off_Timer_Fast < 255)
	{
		Write_EEPROM(Auto_Off_Timer , 0xFF);
		Auto_Off_Timer_Fast = 0xFF;
	}
	*/

	//Local_Light_Level_Set(tmp,0); 	//Snap to Last_on_Level!
	
	#warning "debug"!!!!
	//	Local_Light_Level_Set(0,0); 	//Snap to Last_on_Level!

	//Default_FadeRate = 	Read_EEPROM(Dimmer_Options) & 0b00001111;		//0-15
	//Dimmer_Mode = 	(Read_EEPROM(Dimmer_Options) >> 7) & 0b00000001;	//1 or 0	
	//Auto_Report_State_Enabled = 	(Read_EEPROM(Dimmer_Options) >> 4) & 0b00000001;	//1 or 0	
	//LED_Options_Fast = Read_EEPROM(LED_Options);

}	//init_Variable_buffers

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

/* !!!!!!!!!!!!!!!!!!!!!
ONLY Double_Bit_in_flag IS 
CLEARED AT ZEROCROSS!!!!!

Double_Bit_out_flag IS ALWAYS 
CLEARED AFTER THE PULSE SEND OUT!!!!

TX: wake up at TMR1= Offset, fire pulse, 
clear Double_Bit_out_flag, RETURN to BITOUT 
waiting! Set Double_Bit_out_flag again and 
zerocross can charge for next half

CANT CHARGE AFTER Wait_New_Halfwave(); in the new halfwave
because we left zero cross!!!
*/ 

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static inline void Indicate_set(void) 
{
	/*
	if ( Arg[1] > 50 )
		LED
	else
		LED
*/

}	//Indicate_set

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void Enable_SETUP_mode(void)
{
	//Start 5min countdown
	
	//5min = 118 (slow) x 10mS x 256 (fast)
	//10mS x 256 = 2.56sec
	//5min=300sec/10e-3=30000 count
	
	Setup_5min_Divider_16bit = 30000;
Red_LED_On;	
	SETUP_mode = 1;
//	increase_SETUP_entered_cnt();

}	//Enable_SETUP_mode

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void Disable_SETUP_mode(void)	//also called from ZeroCross count
{
	//Stop 5min countdown
//	Setup_5min_Divider_16bit = 0;
Red_LED_Off;
	SETUP_mode = 0;

}	//Disable_SETUP_mode

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

unsigned char Random_Byte(void)	//returns 1-250
{
	//Take a lot of sources to generate a random byte
	//Oscillator calibration value
	//Status Register
	//NID
	unsigned char Temp_Random = 0;	//Dont set start value
	
	Temp_Random =
	(((OSCCAL ^ SREG) & 
	(Current_Half & Unit_ID_fast)) ^ 
	((Checksum_Byte & TMR1) |	
	//(Signal_Strength | Noise_Level))) ^ 
	Header_Byte[2] ));
	
	if (Temp_Random == 0)	//Set min:1
		Temp_Random = 1;

	if (Temp_Random > 250)	//Set max:250
		Temp_Random = 250;
	
	return Temp_Random;

}	//Random_Byte()

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static inline void Reset_device(void)
{

	//Disable All interrupts -> No WDT Reset
	cli();
	while(1);	//Do nothing until WDT reset

}	//Reset_device


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static unsigned char Get_LinkID_Rel_Address(void)
{
	//Received LinkID will be DID

	for (unsigned char i=0; i<16 ;i++)
		if ( Link_ID_fast[i] == DID ) return i;		//! return 0-15 !
	
	return 255;	//default error value!

}	//Get_LinkID_Rel_Address

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static inline void Store_State_to_Link(void)
{
	//Received LinkID will be DID

	//Only set Light level, Dont set FadeRate!
	
	//3Byte -> Link# 1-250,Ligth level 0-100,Faderate 0-15 def:255
	//Get_LinkID_Rel_Address returns 0-15
	
	//Link:			LinkID + 0
	//Ligth level:	LinkID + 1
	//Faderate:		LinkID + 2
	
	//Write_EEPROM( LinkID_start + Get_LinkID_Rel_Address() * 3 + 1 , current_output_lev );
	
}	//Store_State_to_Link

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


static inline void Activate_Link(void)
{
	unsigned char tmp_Level=0, tmp_FadeRate=0;
	//Received LinkID will be DID
	
	//Link:			LinkID + 0
	//Ligth level:	LinkID + 1
	//Faderate:		LinkID + 2
	
	tmp_Level 		= Read_EEPROM( LinkID_start + Get_LinkID_Rel_Address() * 3 + 1 );
	tmp_FadeRate 	= Read_EEPROM( LinkID_start + Get_LinkID_Rel_Address() * 3 + 2 );

	if (tmp_Level > 100)		//For Safety
		tmp_Level = 100;
	//Local_Light_Level_Set(tmp_Level,tmp_FadeRate);

}	//Activate_Link

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static inline void Deactivate_Link(void)
{
	//Received LinkID will be DID
	
	//output to 0% using the Fade Rate parameter of the 
	//“linked” Receive Component
	
	//Can be other Link than the Activating link!!!
	
	//Link:			LinkID + 0
	//Ligth level:	LinkID + 1
	//Faderate:		LinkID + 2

 	//Local_Light_Level_Set(0,Read_EEPROM( LinkID_start + Get_LinkID_Rel_Address() * 3 + 2 ));

}	//Deactivate_Link

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

unsigned char Calced_checksum (void)	//For TX & RX
{
	//Sum all of the bytes of the Packet Header and UPB Message 
	//fields together. Then take the 2’s complement of the sum 
	//and truncate the result to 8-bits

	unsigned char sum = 0,TMP = Payload_Length;		//1Byte enough
	
	for (unsigned char i=0;i<5;i++)
		sum += Header_Byte[i];			//Sum up 5 Header Bytes
	
	for (unsigned char i=0;i<TMP;i++)	//calc only once length, with define
		sum += Payload_Byte[i];

	sum = ~sum;			//Same as XOR 0xFF
	sum++;				//Must add 1
	//sum&= 0xFF;		//only if int!
	
	return sum;
			
}	//Calced_checksum

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static inline void Add_LinkID(unsigned char new_ID)
{
	//Add link, INPUT: 1-250
	
	for (unsigned char i=0;i<16;i++)
	{
		//when empty: LinkID = 255 or 0
		if (Link_ID_fast[i] == 0x00 || Link_ID_fast[i] == 0xFF)	//is it an empty spot?
			{
				Write_EEPROM( (LinkID_start + i*3), new_ID);	//Start adr + new value
				Link_ID_fast[i] = new_ID;				//Assign fast list to
				return;		//Dont store to other empty spaces
			}

	}	//for
	
	//if no empty left: simply return	

}	//Add_LinkID

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static inline void Del_LinkID(unsigned char del_ID)
{
	//Delete link, INPUT: 1-250
	
	for (unsigned char i=0;i<16;i++)
	{
		if (Link_ID_fast[i] == del_ID)
			{
				Write_EEPROM( (LinkID_start + i*3), 0xFF);	//0xFF makes it empty
				Link_ID_fast[i] = 0xFF;	
				//Dont return of first find!		
			}
			
	}	//for
	
}	//Del_LinkID

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void Wait_New_Halfwave (void)
{
	unsigned char TMP = Current_Half;

	//Exit loop after ZeroCross interrupt set new Current_Half
	//Wakes up on ZeroCross int.
	while (TMP == Current_Half)
		sleep_mode();
		
}	//Wait_New_Halfwave

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static inline void ACK_Handler (void)
{
	/*
	WE ARE NOW IN THE SAME HALF WHERE WE COULD NEED
	TO FIRE THE ACK PULSE!!!!
	WE CANT USE THE ZEROCROSS TO CHARGE OUR CAP!!
	NEED TO DO THIS MANUALLY!!
	*/
	TXmode = 1;
	//Only called after last packet of Multipacket
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	//CANT MAKE COMMON BASE, because it is sequential!

	if ( ACK_Pulse_Req )
	{
		//Compromise modularity!
		//We cant use zerocross because we are over it (double bit is collected after zerocross)
		_delay_us(500);	//wait some time after zerocross
		Fire_Triacs();	//Charge CAP!
		CAP_Active = 1;
			
		Double_Bit_buffer = ACK_pulse_position;
		Double_Bit_out_flag = 1;

		while(Double_Bit_out_flag == 1)
			sleep_mode(); 
			//Will exit loop at one zerocross
			//could wake up multiple times, 
			//but 'll go back to sleep
	}

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 
	if ( ID_Pulse_Req )
	{
		//Unit_ID: possible:256half !
		//Right after RXed packet! 256 halfes

		for (unsigned char i=0;i<Unit_ID_fast;i++)
			Wait_New_Halfwave();

		//Compromise modularity!
		//We cant use zerocross because we are over it
	
		_delay_us(500);	//wait some time after zerocross
		Fire_Triacs();	//Charge CAP!
		CAP_Active = 1;

		Double_Bit_buffer = ACK_ID_pulse_position;
		Double_Bit_out_flag = 1;
	
		while(Double_Bit_out_flag == 1)
			sleep_mode(); 
			//Will exit loop at one zerocross
			//could wake up multiple times, 
			//but 'll go back to sleep
	}
		
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	
	//is ACK message needed?, how much delay allowed??
	//is this follows ID pulse
	//"is this follows ID pulse" !!!!!!!!!!!!!
	
	if (ACK_Message_Req) 
		TX(ACK_Response);	//Generate the message

	TXmode = 0;		//Resume

}	//ACK_Handler()

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@



static inline void Process_packet (void)
{

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	
	switch (MDID)
	{

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@_______________________UPB_Core_Command_Set___________________________@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//my comand
		case Report_Sensor_cmd:
			//Payload_Length = 1;		//MDID
			
			if ( Payload_Length != 1 ) return;	//For safety
			TX_Multi_Packet( Device_Sensor_Report );
			break;
//my report
		case Device_Sensor_Report:
			if ( Payload_Length != 5 ) return;	//For safety
			Light_Value = (Arg[1]<<8) + Arg[2];
			Temp_Value = (Arg[3]<<8) + Arg[4];
			printf("Light = %i\nTemp = %i",Light_Value,Temp_Value);
			//print value to LCD
			break;
//////////////////////////////////
//////////////////////////////	
		case Null_cmd:					//Performs no action
			//Payload_Length = 1;		//MDID			
			if ( Payload_Length != 1 ) return;	//For safety
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Write_Enable_cmd:			//Enable Setup mode
			//Payload_Length = 3;		//MDID, Arg[1-2]
			
			//Arg[1] = Password MSB 0-255
			//Arg[2] = Password LSB 0-255
			
			if ( Payload_Length != 3 ) return;	//For safety
			
			if (Network_password_match != 1) return;
			
			Enable_SETUP_mode();
					
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Write_Protect_cmd:			//Disable Setup mode
			//Payload_Length = 1;		//MDID
			
			//No Password required
			
			if ( Payload_Length != 1 ) return;	//For safety

			Disable_SETUP_mode();
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Start_Setup_Mode_cmd:		//Enable Setup mode
			//Payload_Length = 3;		//MDID, Arg[1-2]
			
			//Arg[1] = Password MSB 0-255
			//Arg[2] = Password LSB 0-255
		
			if ( Payload_Length != 3 ) return;	//For safety
			
			if (Network_password_match != 1) return;
			
			Enable_SETUP_mode();
		//	
//		move_LCD(2,1);
//		print_LCD("setup mode enabled!");
		//	
		
			//Other possibility not implemented				
				
			//group of devices by Manufacturer ID and Product ID
//			if (Arg[3] == MIDH && Arg[4] == MIDL && Arg[5] == PIDH && Arg[6] == PIDL)
//				Enable_SETUP_mode();
			
					
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Stop_Setup_Mode_cmd:		//Disable Setup mode
			//Payload_Length = 1;		//MDID
			
			//No Password required
			
			if ( Payload_Length != 1 ) return;	//For safety
			
			Disable_SETUP_mode();
		//	
//		move_LCD(2,1);
//		print_LCD("setup mode disabled!");
		//	
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Get_Setup_Time_cmd:		//
			//Payload_Length = 1;		//MDID
			
			if ( Payload_Length != 1 ) return;	//For safety
			
			if ( Transmit_Cnt == Transmit_Seq )	//TX after Last packet!
				TX_Multi_Packet( Setup_Time_Report );	//Report time clicks left of 5mins
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Auto_Address_cmd:			//new random Unit_ID 1-250 
			//Payload_Length = 1;		//MDID
			
			if ( Payload_Length != 1 ) return;	//For safety
			
			if (SETUP_mode != 1)	return;
			
			//Generate new Unit_ID -> Store in EEPROM & SRAM
			
			Unit_ID_fast = Random_Byte();	//returns 1-250! NOT 0-255
			
//			Write_EEPROM( Unit_ID, Unit_ID_fast );	//address, value

			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Get_Device_Status_cmd:		//
			//Payload_Length = 1;		//MDID
						
			if ( Payload_Length != 1 ) return;	//For safety
			
			if ( Transmit_Cnt == Transmit_Seq )	//TX after Last packet!
				TX_Multi_Packet( Device_Status_Report );		//respond with a Device Status Report 0x87
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Set_Device_Control_cmd:	//Set Device Control Register value
			//Payload_Length = 2;		//MDID, Arg[1]
			
			//Arg[1] = Device Control Register value 0-255
			
			if ( Payload_Length != 2 ) return;	//For safety
			
			//No control register?
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Add_Link_cmd:				//
			//Payload_Length = 2;		//MDID, Arg[1]
			
			//Receiver must be in Setup mode for this
			//Packet Must be Direct Packet!
			//Arg[1] = Link_ID 1-250
			
			if ( Payload_Length != 2 ) return;	//For safety
			
			if (SETUP_mode != 1) return;		//Need to be in Setup mode
			
			if (LINK_bit != 1) return;			//Only when Direct packet
			
			if (Arg[1] == 0 || Arg[1] > 250) return;	//For safety
			
			
			Add_LinkID(Arg[1]);				 //will ignore if no LinkID space left

			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Delete_Link_cmd:			//
			//Payload_Length = 2;		//MDID, Arg[1]
			
			//Receiver must be in Setup mode for this
			//Packet Must be Direct Packet!
			//Arg[1] = Link_ID 1-250
			
			if ( Payload_Length != 2 ) return;	//For safety
			
			if (SETUP_mode != 1) return;		//Need to be in Setup mode
			
			if (LINK_bit != 1) return;			//Only when Direct packet
			
			if (Arg[1] == 0 || Arg[1] > 250) return;	//For safety
			
			Del_LinkID(Arg[1]);					//will ignore if non existing LinkID

			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Transmit_This_Message_cmd:		//Not implemented yet
//			
			//Payload_Length = 7;			//MDID, Arg[1-6], opt: Arg[7-17]
			//Payload min:7 max:18
			
			//Header:	Arg[1]-Arg[5]
			//1VV: 		Arg[6]
			//optional:	Arg[7]-Arg[17]

			if ( Payload_Length < 7	) return;	//For safety
			
			//DID and SID must be stored swapped because TX will swap them
			CTL0 =	Arg[1];
			CTL1 =	Arg[2];
			NID =	Arg[3];
			DID =	Arg[5];
			SID =	Arg[4];
			
			//MDID Transmit_This_Message_cmd
			if ( Transmit_Cnt == Transmit_Seq )	//TX after Last packet!
				TX_Multi_Packet( Transmit_This_Message_cmd );
//			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Device_Reset_cmd:			//
			//Payload_Length = 3;		//MDID, Arg[1-2]
			
			//Arg[1] = Password MSB 0-255
			//Arg[2] = Password LSB 0-255
			
			if ( Payload_Length != 3 ) return;	//For safety
			
			if (Network_password_match != 1) return;
			
			Reset_device();

			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Get_Device_Signature_cmd:	//Answer: Device Signature Report
			//Payload_Length = 1;		//MDID
			
			//Can be custom payload: 24 or normal: 1			
			//if ( Payload_Length != 1 && Payload_Length != 24 ) return;	//For safety
			
			if ( Transmit_Cnt == Transmit_Seq )	//TX after Last packet!
				TX( Device_Signature_Report );
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Get_Register_Value_cmd:	//Answer: Register Values Report
			//Payload_Length = 3;		//MDID, Arg[1-2]
			
			//Arg[1] = Start register address	0-255
			//Arg[2] = number of registers to be retrieved	1-16			
			
			if ( Payload_Length != 3 ) return;	//For safety

			if( Arg[2] == 0 || Arg[2] > 16 ) return;	//For safety

			if ( Transmit_Cnt == Transmit_Seq )	//TX after Last packet!			
				TX_Multi_Packet( Register_Values_Report );
			
			//Password protection handled in TX			

			break;		

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Set_Register_Value_cmd:	//
			//Payload_Length = Not fix	//MDID, Arg[1-2], opt: Arg[3-17]
			
			//Payload min:3 max:18
			//Must be in setup mode!
			
			//Arg[1] = Start register address	0-255
			//Arg[2] = 1vv	value to be set
			//Arg[3-17]
						
			if (SETUP_mode != 1) return;

			//Payload_Length: 0-18 Bytes
			//max 16 bytes to Write -> Payl will be 18
			
			for (unsigned char i=0;i<Payload_Length-2;i++)//Payload_Length-2: MDID and Arg[1]-> Start Address
				Write_EEPROM( Arg[1]+i, Arg[i+2] );

			init_Variable_buffers();	//Update fast access vars (SID,NID..)
			
			break;
			
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@___________________DeviceControlCommandSet_For_Req____________________@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Activate_Link_cmd:			//
			//Payload_Length = 1;		//MDID, +2 Byte Undocumented Dummy!!!!
						
			if ( Payload_Length > 3 ) return;	//For safety
			//MDID + 2 Byte Undocumented Dummy!!!!

			//Must be in Link packet!		
			if (LINK_bit != 1)	return;		 	//Only when Link packet
			
			Activate_Link();					//DID will be the LinkID#
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Deactivate_Link_cmd:		//
			//Payload_Length = 1;		//MDID, +2 Byte Undocumented Dummy!!!!
	
			if ( Payload_Length > 3 ) return;	//For safety
			//MDID + 2 Byte Undocumented Dummy!!!!

			//Must be in Link packet!		
			if (LINK_bit != 1)	return;		 	//Only when Link packet
			
			Deactivate_Link();					//DID will be the LinkID#
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Goto_cmd:					//
			//Payload_Length = 2;		//MDID, Arg[1], opt:Arg[2]
			//Payload min:2 max:3
			//Arg[1] = Level 0-100
			//Arg[2] = FadeRate 0-15, optional, RX can handle if >15
			
			if ( Payload_Length == 1 || Payload_Length > 3) return;	//For safety
			if( Arg[1] > 100 ) return;			//For safety

			//Multichannels are not used!

			//Not specified OR invalid FadeRate -> Use default		
//			if (Payload_Length != 3 || Arg[2] > 15)					
//				Local_Light_Level_Set(Arg[1],Default_FadeRate);
//			else	//Specifying Rate exists
//				Local_Light_Level_Set(Arg[1],Arg[2]);
//			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Fade_Start_cmd:			//Same as Goto, but ignered by not Dimmers
			//Payload_Length = 2;		//MDID, Arg[1], opt:Arg[2]
			//Payload min:2 max:3
			
			//Arg[1] = Level 0-100
			//Arg[2] = FadeRate 0-15, optional, RX can handle if >15
			
			if ( Payload_Length == 1 || Payload_Length > 3) return;	//For safety

			if( Arg[1] > 100 ) return;			//For safety
	Local_Light_Level_Set(Arg[1],Arg[2]);
			//if (Dimmer_Mode != 1) return;		//Only difference from Goto_cmd!!!
			
			//Multichannels are not used!
			
			//Multichannels are not used!

			//Not specified OR invalid FadeRate -> Use default		
//			if (Payload_Length != 3 || Arg[2] > 15)					
//				Local_Light_Level_Set(Arg[1],Default_FadeRate);
//			else	//Specifying Rate exists
//				Local_Light_Level_Set(Arg[1],Arg[2]);
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Fade_Stop_cmd:				//
			//Payload_Length = 1;		//MDID
						
			if ( Payload_Length > 3 ) return;	//For safety
			
			//Local_Light_Level_Set_Stop_Fading();
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Blink_cmd:					//Currently not implemented!
			//Payload_Length = 2;		//MDID, Arg[1]
			
			//Arg[1] = Blink rate 0-255
			
			//Multichannels are not used!
			
			//if ( Payload_Length != 2 ) return;	//For safety
			
			//NOT USED
		//	
//		move_LCD(2,1);
//		print_LCD("Blink on!");
		Light_On;
		//	
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Indicate_cmd:				//e.g. LED
			//Payload_Length = 2;		//MDID, Arg[1]
			//Payload min:2 max:3
			
			//Arg[1] = Level or state 0-100
			//Arg[2] = Faderate NOT used!	0-255
			
			if ( Payload_Length == 1 || Payload_Length > 3) return;	//For safety
			
			if( Arg[1] > 100 ) return;			//For safety
			
			//Multichannels are not used!		
			
			Indicate_set();
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Toggle_cmd:				//Not implemented!
			//Payload_Length = 1;		//MDID
			
			//if ( Payload_Length != 1 ) return;	//For safety
			//if ( Transmit_Cnt != Transmit_Seq ) return;//Set after Last packet!		

			//NOT USED IN WS1D
		//	
		Light_Off;
		//		
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Report_State_cmd:			//
			//Payload_Length = 1;		//MDID
			
			//Must be in Direct packet!
			
			if ( Payload_Length != 1 ) return;		//For safety
			
			if ( LINK_bit != 0 ) return;			//Must be in Direct packet

			if ( Transmit_Cnt == Transmit_Seq )	//TX after Last packet!		
				TX_Multi_Packet( Device_State_Report );
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Store_State_cmd:			//
			//Payload_Length = 3;		//MDID, Arg[1-2]
			
			//Must be in link packet!
			
			//Arg[1] = Password MSB 0-255
			//Arg[2] = Password LSB 0-255
			
			if ( Payload_Length != 3 ) return;		//For safety
			
			if ( LINK_bit != 1 ) return;			//Must be in Link packet
			
			if ( Network_password_match != 1) return;
			
			Store_State_to_Link();
						
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@___________________UPBCoreReport_Set_For_Answering!____________________@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case ACK_Response:				//
			//Payload_Length = 2;		//MDID, Arg[1]
			
			//Arg[1]: old MDID	0–255
			
			if ( Payload_Length != 2 ) return;	//For safety
	
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Setup_Time_Report:			//
			//Payload_Length = 3;		//MDID, Arg[1], Arg[2]

			//Arg[1]: RAM register address of 5min count variable
			//Arg[2]: number of ticks that are left before timing out Setup mode		
			
			if ( Payload_Length != 3 ) return;	//For safety
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Device_State_Report:		//Same as Status report!
			//Payload_Length = 2;		//MDID, Arg[1]
			
			//Arg[1]: output percent 0-100, based on WS1D
			
			if ( Payload_Length != 3 ) return;	//For safety			
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Device_Signature_Report:	//Not used in WS1D??
			{
			
			}
			
			return;			//for debug

			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Register_Values_Report:	//From EEPROM
			
			//Read Rx-ed payload, rewrite it with data from EEPROM
			
////			//At now (untouched payload):
////			//Arg[1] = Start register address	0-255
////			//Arg[2] = number of registers to be retrieved	1-16
			for(unsigned char i=0;i<Number_of_Register;i++)
				printf("%i,",Arg[2+i]);
			for(unsigned char i=0;i<Number_of_Register;i++)
				printf("%c",Arg[2+i]);
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case RAM_Values_Report:			//Report from SRAM
			
			//The password is not in the SRAM!
			
			//Read Rx-ed payload, rewrite it with data from SRAM
			
			//At now (untouched payload):
			//Arg[1] = Start register address	0-255
			//Arg[2] = number of registers to be retrieved	1-16
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Raw_Data_Report:			//ONLY REPORT, No COMMAND!
			//Payload_Length = 2;		//MDID, Arg[1], opt:Arg[2-16]
			//Payload min:2 max:17
			
			//Arg[1-16]: RAW data
						
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Heartbeat_Report:			//
			//Payload_Length = 1;		//MDID
			
			if ( Payload_Length != 1 ) return;	//For safety
			
			break;
			
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		default:
		
			return;						//Not recognized Message ID
						
			break;
			
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	}	//Switch

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

}	//Process_packet



//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static inline unsigned char Valid_DID(void)	//return 1 if valid address
{
	#define Accept_DID 1
	#define Reject_DID 0

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	if (LINK_bit == 1)		//Link Packet (DID will be Link_ID)
	{
		if (DID == 0) return Reject_DID;	//No LinkID=0

		for (unsigned char i=0;i<16;i++)	//Any match from 16 LinkID?
		{
			if (( DID == Link_ID_fast[i] ) && 
				( Link_ID_fast[i] != 0 ) && 
				( Link_ID_fast[i] <= 250 ))
					return Accept_DID;
		}
		
		//return Reject_DID;
	}

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	else	//Direct Packet (DID Field interpreted as a Unit ID)
	{
		if (DID == 0) return Accept_DID;	//Broadcast DID, Accepted by all UPB devices

		//Accepted by all UPB devices that have been manually put into Setup Mode
		if (SETUP_mode == 1 && DID == 0xFE)	
			return Accept_DID;

		//Accepted by all UPB devices who’s Setup Registers are write enabled
		if (SETUP_mode == 1 && DID == 0xFD)
			return Accept_DID;

		//Traditional Destination matches unitID
		if (DID == Unit_ID_fast)
			return Accept_DID;

		//return Reject_DID;
	}
 
	return Reject_DID;	//By default

}	//Valid_DID

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

unsigned char Collect_Double_Bit (void)	//Returns 255 if overtime, 0-3 normally
{
	Wait_New_Halfwave();	//sleeps whole halfwave through
	//read inputs at zerocross
		
	//Current_Half is the opposite, because we are in the new half!!!!!!

	if (Double_Bit_buffer > 3)	//error
		return 255;		//invalid

	if (Double_Bit_in_flag != 1)
		return 255;		//invalid

	//From here valid
	Double_Bit_in_flag = 0;	

	return 	Double_Bit_buffer;		//invalid (?? Luan-> why invalid??)
	
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

unsigned char Collect_Byte(void)	//Needed for TX ack_rx!
{
	unsigned char input_Byte_buffer = 0;
	unsigned char Double_Bit_TMP = 255;

	RX_overtime_flag = 0;	//Reset
	
	//return: input_byte, IF werent overtime
	//return: 0, if there were overtime, this will screw up checksum!
	
/*
WE CAN ONLY READ (then Reset) INCOMING PULSE buffer
at the new halfwaves (next zerocross) -> so we can determinate was 
the an UPB signal RXed or not -> Used for RX time out!!!*/
		
	for (unsigned char i=0;i<4;i++)	//4x Double bit = 1Byte
	{
		//Waits new half, collects result, decodes using correct delay!, clears flag
		Double_Bit_TMP = Collect_Double_Bit();
	
		if ( Double_Bit_TMP > 3 )	//255 if overtime, 0-3 normally
		{
			RX_overtime_flag = 1; 
			return 0;			//werent any signal RXed in prev. half
		}

		input_Byte_buffer |= Double_Bit_TMP << ((3-i)*2);	

	}	//for
	
	return input_Byte_buffer;	//Return Complette RXed byte:

}	//Collect_Byte()


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


void Wait_1_empty_halfwaves (void)		//Also for Tx!
{
	unsigned char count = 0, TMP = TXmode;
	
	TXmode = 0;		//for checking empty halfes, need to set back for TX
	
	//Start from a known point, jump to the closest ZeroCross
//	Wait_New_Halfwave();	//debug too much
	Double_Bit_in_flag = 0;	//Need to be reset (Luan - Don't need to be reset)
	
	//State machine
	while (count < 1)
		{
			Wait_New_Halfwave();
			
			if (Double_Bit_in_flag == 0)	//it was an empty half
				count++;
			else				//There was a pulse, reset state
				{
					count = 0;
					Double_Bit_in_flag = 0;	//Reset buffer
				}
				
		}	//while
		
	TXmode = TMP;	//restore
	
}


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void RX(void)
{
	TXmode = 0;	//RX mode
	RX_overtime_flag = 0;	//For safety

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	
	for (unsigned char i=0;i<5;i++)
		Header_Byte[i] = 0;	
	
	//Clear payload because it has variable length
	for (unsigned char i=0;i<18;i++)
		Payload_Byte[i] = 0x00;
	
	Checksum_Byte = 0x12;	//dummy
			
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	
	//Wait for sync
	if( Get_Sync_And_Ref_Delays() != 1)	//MANUALLY CAPTURES!
		return;

	//if overtime-> returns junk, sets RX_overtime_flag
	//if (RX_overtime_flag == 1) return;	//From RX
	//Reset RX_overtime_flag from main

	//Got sync
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	//Receive Header Bytes
	for (unsigned char i=0;i<5;i++)
	{
		//if overtime-> returns junk, sets RX_overtime_flag
		Header_Byte[i] = Collect_Byte();	
		if (RX_overtime_flag == 1) return;	//From RX
		//Reset RX_overtime_flag from main
	}
	
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	//Quick validate header (basic protocol rules + address )
	if (Repeater_Req != 0x00)	return;	
	//The packet was ment to be for repeater	


	//NID: 0x00 Global!
	if (NID != 0 && NID != Unit_Network_ID_fast) return;//it's ment to other network

	if (Valid_DID() != 1)	return;		//Address doesn't match
	//SID can be 0-255!!

	if (ABS_Packet_Length < 6 || ABS_Packet_Length > 24) return;	
	//Not protocol size packet

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	{//independent block	
		unsigned char TMP = Payload_Length;	//calc only once length, with define
			
		//RX payload 0-18 Bytes (pcs!)
		for (unsigned char i=0;i<TMP;i++)
		{
			//if overtime-> returns junk, sets RX_overtime_flag
			Payload_Byte[i] = Collect_Byte();
			if (RX_overtime_flag == 1) return;	//From RX
			//Reset RX_overtime_flag from main
		}
	}

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	//Receive checksum
	//if overtime-> returns junk, sets RX_overtime_flag
	Checksum_Byte = Collect_Byte();

	if (RX_overtime_flag == 1) return;	//From RX
	//Reset RX_overtime_flag from main


	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	if (Calced_checksum() != Checksum_Byte)	return;	//if checksum fails return

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	//Only send ACK-es after the last packet of Multipackets
	if (LINK_bit == 0 && Transmit_Cnt == Transmit_Seq )	//Last packet, only direct
		ACK_Handler();	//Returns after all ACK-es handled!
	
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	//Process every packet of multipackets, but only answer after the last one
	Process_packet();

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

}	//RX()
 

