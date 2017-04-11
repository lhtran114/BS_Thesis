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

#include "TX.h"
#include "RX.h"	//for random byte
#include "Protocol_defines.h"
#include "EEPROM.h"
#include "Zero_Cross.h"
//#include "Sensor_Input.h"

volatile unsigned char Double_Bit_out_flag = 0;
//
//
volatile unsigned char Number_of_Register = 0;		

/* !!!!!!!!!!!!!!!!!!!!!
ONLY Double_Bit_in_flag IS 
CLEARED AT ZEROCROSS!!!!!

Double_Bit_out_flag IS ALWAYS 
CLEARED AFTER THE PULSE SEND OUT!!!!

TX: wake up at TMR1= Offset, fire pulse, 
clear Double_Bit_out_flag, RETURN to BITOUT 
waiting! Set Double_Bit_out_flag again and 
zerocross can charge for next half
*/ 

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
/*Not needed
static void Swap_Bytes(unsigned char x, unsigned char y)
{
	unsigned char TEMP_SWAP = 0;
	
	TEMP_SWAP = x;
	x = y;
	y = TEMP_SWAP;

}	//Swap_Bytes
*/
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

static void Send_Byte( unsigned char output_Byte_buffer)
{

//check timing very carefully!!!
//musnt leave any halfwaves out!
//or overtime on RX side!

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

	for (unsigned char i=0;i<4;i++)
		{
			//MSB first
			Double_Bit_buffer = output_Byte_buffer >> ( (3-i)*2 ) & 0b00000011;
					
			Double_Bit_out_flag = 1;
			
			while(Double_Bit_out_flag == 1)	//Will be cleared by pulse out
				sleep_mode();	
			
			//Now here we are after pulse out and zerocross!!
			//Calculate the the next DoubleBit out, set flag
			//BEFORE zerocross-> so it can charge the cap
			//and we dont miss out a halfwave!
					
		}	//for

}	//Send_Byte()

///////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////
static inline void ACK_Receive (void)
{
	/*
	WE ARE NOW IN AFTER PULSE OUT AND BEFORE ZEROCROSS
	SO NEED TO DISABLE TXMODE AND WAIT TO NEXT HALF
	TO GET ACK_PULSE
	*/
	unsigned char Double_Bit_TMP = 255;	
	TXmode = 0;
	Wait_New_Halfwave();

	if ( ACK_Pulse_Req )
	{
		Collect_Double_Bit();
		if(Double_Bit_buffer == ACK_pulse_position)	
		{
			ACK_Pulse_Flag = 1;
			printf("y");
		}
		else
		{
			ACK_Pulse_Flag = 0;		
			printf("n");
		}
	}

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 
	if ( ID_Pulse_Req )
	{
		//Unit_ID: possible:256half !
		for (unsigned int i=0;i<256;i++)
		{
			Double_Bit_TMP = Collect_Double_Bit();
			if(Double_Bit_TMP == ACK_ID_pulse_position)				
				bit_set(ID_Exist[i/8],BIT(i%8));
			else
				bit_clear(ID_Exist[i/8],BIT(i%8));
		}
		//send to UART
		for (unsigned char i=0;i<32;i++)	
			printf("%i ",ID_Exist[i]);
	}
		
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
}	//ACK_Receive()

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void TX (unsigned char TX_Argument)
{
	//got MDID from TX argument, in switch we just do the extra calcs needed
	//at this point keep the RX Header+Payload untouched! itt will be needed
	//At the end give the new MDID values
	//Set Arg[x]-s before calling TX()

	unsigned char TX_Payload_length = 1;	//Only MDID in Payload (Default)
										
	TXmode = 1;	//TX
	if(Button_TX)//TX by button
	{
		Button_TX = 0;
		NID = Unit_Network_ID_fast;	//Default
		DID = DID_Button;
	//Can be changed below
	}	
	else if(UART_TX)
	{
		UART_TX = 0;
		NID = NID_UART;
		DID = DID_UART;
		//for(unsigned char i=1;i<=16;i++)
		//	Arg[i] = Arg_UART(i);
	}
	else//Auto_TX
	{
		NID = Unit_Network_ID_fast;	//Default
		DID = SID;//respone for device TX
		//clear ACK
		bit_clear( Header_Byte[1], BIT(6) );
		bit_clear( Header_Byte[1], BIT(5) );
		bit_clear( Header_Byte[1], BIT(4) );
	}

	MDID = TX_Argument; 
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


	switch (TX_Argument) 
	{

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@_______________________UPB_Core_Command_Set___________________________@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//my comand
		case Report_Sensor_cmd:
			//For Button Transmit component!
			//TX_Payload_length = 1;	//MDID: Default
			break;
//my report
		case Device_Sensor_Report:
			TX_Payload_length = 5;//MDID + Light_SS + Temp_SS			
			Read_Sensor();
			Arg[1] = Light_Value>>8;
			Arg[2] = Light_Value;
			Arg[3] = Temp_Value>>8;
			Arg[4] = Temp_Value;
			break;
//////////////////////////////////
//////////////////////////////		
		case Null_cmd:					//Performs no action
			//TX_Payload_length = 1;	//MDID: Default
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Write_Enable_cmd:			//Enable Setup mode
			TX_Payload_length = 3;		//MDID, Arg[1-2]
			
			//Arg[1] = Password MSB 0-255
			//Arg[2] = Password LSB 0-255
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Write_Protect_cmd:			//Disable Setup mode
			//TX_Payload_length = 1;	//MDID: Default
			
			//No Password required
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Start_Setup_Mode_cmd:		//Enable Setup mode
			TX_Payload_length = 3;		//MDID, Arg[1-2]
			Arg[1] = Arg_UART(1);//Arg[1] = Password MSB 0-255
			Arg[2] = Arg_UART(1);//Arg[2] = Password LSB 0-255
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Stop_Setup_Mode_cmd:		//Disable Setup mode
			//TX_Payload_length = 1;	//MDID: Default
			
			//No Password required
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Get_Setup_Time_cmd:		//
			//TX_Payload_length = 1;	//MDID: Default
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Auto_Address_cmd:			//new random Unit_ID 1-250 
			//TX_Payload_length = 1;	//MDID: Default
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Get_Device_Status_cmd:		//
			//TX_Payload_length = 1;	//MDID: Default
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Set_Device_Control_cmd:	//Set Device Control Register value
			TX_Payload_length = 2;		//MDID, Arg[1]
			
			//Arg[1] = Device Control Register value 0-255
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Add_Link_cmd:				//
			TX_Payload_length = 2;		//MDID, Arg[1]
			
			//Receiver must be in Setup mode for this
			//Packet Must be Direct Packet!
			//Arg[1] = Link_ID 1-250
			
			if (Arg[1] == 0 || Arg[1] > 250) return;	//For safety
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Delete_Link_cmd:			//
			TX_Payload_length = 2;		//MDID, Arg[1]
			
			//Receiver must be in Setup mode for this
			//Packet Must be Direct Packet!
			//Arg[1] = Link_ID 1-250
			
			if (Arg[1] == 0 || Arg[1] > 250) return;	//For safety
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Transmit_This_Message_cmd:		//Not implemented yet
			/*
			TX_Payload_length = 7;			//MDID, Arg[1-6], opt: Arg[7-17]
			//Payload min:7 max:18
			
			//Header:	Arg[1]-Arg[5]
			//1VV: 		Arg[6]
			//optional:	Arg[7]-Arg[17]
			*/
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Device_Reset_cmd:			//
			TX_Payload_length = 3;		//MDID, Arg[1-2]
			
			//Arg[1] = Password MSB 0-255
			//Arg[2] = Password LSB 0-255
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Get_Device_Signature_cmd:	//Answer: Device Signature Report
			//TX_Payload_length = 1;	//MDID: Default
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Get_Register_Value_cmd:	//Answer: Register Values Report
			TX_Payload_length = 3;		//MDID, Arg[1-2]
			
Arg[1] = Arg_UART(1);//Start register address	0-255
Arg[2] = Arg_UART(2);//number of registers to be retrieved	1-16
	
Number_of_Register = Arg[2];		
			if( Arg[2] == 0 || Arg[2] > 16 ) return;	//For safety
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
/////////////////
//edited By Ryu!
///////////
		case Set_Register_Value_cmd:	//
			TX_Payload_length = 3;		//MDID, Arg[1-2], opt: Arg[3-17]
			
			TX_Payload_length = 1 + Reserve_UART;//MDID + Arg
			for(int i=1;i<=Reserve_UART;i++)
				Arg[i] = Arg_UART(i);
			//Payload min:3 max:18
			//Must be in setup mode!
			
			//Arg[1] = Start register address	0-255
			//Arg[2] = 1vv	value to be set
			//Arg[3-17]
			
			break;
			
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@___________________DeviceControlCommandSet_For_Req____________________@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Activate_Link_cmd:			//
			//TX_Payload_length = 1;	//MDID: Default
			
			//Must be in Link packet!		
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Deactivate_Link_cmd:		//
			//TX_Payload_length = 1;	//MDID: Default

			//Must be in Link packet!		
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Goto_cmd:					//
			TX_Payload_length = 3;		//MDID, Arg[1], opt:Arg[2]
			//Payload min:2 max:3
			
			//DID can be LinkID if Link packet!!! Link:1-250
			//is linkBIT overrided later?
			//DID = 0;	//Report to everyone
			//SID is set later			


			//Arg[1] = Level 0-100
			//Arg[2] = FadeRate 0-15, optional, RX can handle if >15

			
			if( Arg[1] > 100 ) return;	//For safety
			if( Arg[2] > 15 ) return;	//For safety, why send bigger if not used?!

			//Multichannels are not used!
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Fade_Start_cmd:			//Same as Goto, but ignered by not Dimmers
			TX_Payload_length = 3;		//MDID, Arg[1], opt:Arg[2]
			//Payload min:2 max:3
			
			//Arg[1] = Level 0-100
			//Arg[2] = FadeRate 0-15, optional, RX can handle if >15
Arg[1] = Arg_UART(1);//level
Arg[2] = Arg_UART(2);//fade	
			if( Arg[1] > 100 ) return;	//For safety
			if( Arg[2] > 15 ) return;	//For safety, why send bigger if not used?!

			//Multichannels are not used!
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Fade_Stop_cmd:				//
			//TX_Payload_length = 1;	//MDID: Default
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Blink_cmd:					//Currently not implemented!
			TX_Payload_length = 2;		//MDID, Arg[1]
			
			//Arg[1] = Blink rate 0-255
			
			//Multichannels are not used!
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Indicate_cmd:				//e.g. LED
			TX_Payload_length = 2;		//MDID, Arg[1]
			//Payload min:2 max:3
			
			//Arg[1] = Level or state 0-100
			//Arg[2] = Faderate NOT used!	0-255
			
			if( Arg[1] > 100 ) return;	//For safety
			
			//Multichannels are not used!		
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Toggle_cmd:				//Not implemented!
			//TX_Payload_length = 1;	//MDID: Default
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Report_State_cmd:			//
			//TX_Payload_length = 1;	//MDID: Default
			
			//Must be in Direct packet!
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Store_State_cmd:			//
			TX_Payload_length = 3;		//MDID, Arg[1-2]
			
			//Must be in link packet!
			
			//Arg[1] = Password MSB 0-255
			//Arg[2] = Password LSB 0-255
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@___________________UPBCoreReport_Set_For_Answering!____________________@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case ACK_Response:				//
			TX_Payload_length = 2;		//MDID, Arg[1]
			
			//Arg[1]: old MDID	0–255
			//order important!
			
			Arg[1] = MDID;	//Message Data ID of the received Packet
			
			DID = 0;	//Report to everyone
			//SID is set later		

			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Setup_Time_Report:			//
			TX_Payload_length = 3;		//MDID, Arg[1], Arg[2]

			//Arg[1]: RAM register address of 5min count variable
			//Arg[2]: number of ticks that are left before timing out Setup mode
						
			//pointers are 16bits!
	
			//you CANNOT fit the 16 bit 
			//value of a pointer address 
			//into an 8 bit variable !!!!!!!!!!!!!!!!!!
			
			//random
			//NO WAY TO REPORT ANY ADDRESS BECAUSE ALL ADRR-S ARE 16BITS!!!!!
			//maybe just on PIC!
			Arg[1] = 0b10101101;	//CRAP!! CANT send the int var's 16bit address in 1Byte!!!

			Arg[2] = High_Byte( Setup_5min_Divider_16bit );	//Slow
			
			DID = 0;	//Report to everyone
			//SID is set later		

			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Device_State_Report:		//Same as Status report!
			TX_Payload_length = 2;		//MDID, Arg[1]
			
			//Arg[1]: output percent 0-100, based on WS1D
			
			//Arg[1] = desired_output_lev;
			
			DID = 0;	//Report to everyone
			//SID is set later		
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Device_Signature_Report:
			{
/*				TX_Payload_length = 18;
				//Header cleared at TX() end
				unsigned int Temp2 = 0;
			
				DID = SID;	//Report to everyone
				//SID is set later		
					
				Arg[1] = Random_Byte();			//returns 1-250, NOT 0-255
				Arg[2] = Random_Byte() >> (Arg[2]&0x07)&0b10101100;	//Mix some shit
				Arg[3] = 64;					//Signal_Strength, not implemented now
				Arg[4] = 0;						//Noise Level at the device 0-5
				
				//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			
				//-----Calc UPBID_checksum-----
				Temp2 = 0;
				for (unsigned char i=0;i<64;i++)
				{
					Temp2 = (unsigned int) Read_EEPROM(i);
				}
				 
				Temp2 = ~Temp2;			//Same as XOR 0xFF
				Temp2++;				//Must add 1
				
				Arg[5] = High_Byte(Temp2);		//16-bit checksum of UPBID
				Arg[6] = Low_Byte(Temp2);
			
				//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
				
				//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
				
				//-----Calc All_Setup_Regs_checksum-----
				Temp2 = 0;
				for (unsigned char i=0;i<192;i++)
				{
					Temp2 = (unsigned int) Read_EEPROM(i);
				}
				 
				Temp2 = ~Temp2;			//Same as XOR 0xFF
				Temp2++;				//Must add 1
												
				Arg[7] = High_Byte(Temp2);		//16-bit checksum of all Setup Registers
				Arg[8] = Low_Byte(Temp2);
				
				//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			
				//the number of Setup Registers that exist 
				Arg[9] = 192;	//For Sure! 64 UPBID+128 //Setup_Registers_number;	
				
				//8bytes Diagnostic Information that is To Be Determined, BUT send all
				//Clear
				for (unsigned char i=10;i<18;i++)	//smaller
					Arg[i] = 0;
				
				
				Arg[10] = 0x88;	
				Arg[11] = 0x0B;
				
				//Arg[12] = 0;
				//Arg[13] = 0;
				//Arg[14] = 0;
				//Arg[15] = 0;
				//Arg[16] = 0;
				//Arg[17] = 0;
						
*/			
			}			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Register_Values_Report:	//From EEPROM
			
			//Read Rx-ed payload, rewrite it with data from EEPROM
			
			//At now (untouched payload):
			//Arg[1] = Start register address	0-255
			//Arg[2] = number of registers to be retrieved	1-16
			{
				unsigned char Length = Arg[2];	//1-16, Arg[2] will be rewritten

				if (Length == 0 || Length > 16) return;		//For safety
				
				TX_Payload_length = 2 + Arg[2];	//MDID, Start Address, length
				
				//Arg[1] STAYS
				//Protect Password if not in Setup mode
				//Check for every byte

				for (unsigned char i=0;i<Length;i++)
				{
					if (SETUP_mode == 1)	//No need to protect password
						Arg[i+2] = Read_EEPROM(Arg[1] + i);	//Arg[2] - Arg[17]
								
					else	//Protect Password
						
						if ( 	(Arg[1] + i) == Netw_PWD_Addr_L || 
								(Arg[1] + i) == Netw_PWD_Addr_H	)					
									Arg[i+2] = 0x00;	//No need to read EEPROM
						else
							Arg[i+2] = Read_EEPROM(Arg[1] + i);	//Arg[2] - Arg[17]
				}	//For
				
			}	//Block


			DID = 0;	//Report to everyone
			//SID is set later		
				
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case RAM_Values_Report:			//Report from SRAM
			
			//The password is not in the SRAM!
			
			//Read Rx-ed payload, rewrite it with data from SRAM
			
			//At now (untouched payload):
			//Arg[1] = Start register address	0-255
			//Arg[2] = number of registers to be retrieved	1-16
			{
				unsigned char Length = Arg[2];	//1-16, Arg[2] will be rewritten

				if (Length == 0 || Length > 16) return;		//For safety
				
				TX_Payload_length = 2 + Arg[2];	//MDID, Start Address, length
				
				//Arg[1] STAYS
				//No need to protect password

				for (unsigned char i=0;i<Length;i++)
				{
					Arg[i+2] = Read_EEPROM(Arg[1] + i);	//Arg[2] - Arg[17]
				}
				
			}	//Block
			
			DID = 0;	//Report to everyone
			//SID is set later		
			
			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Raw_Data_Report:			//ONLY REPORT, No COMMAND!
			//TX_Payload_length = 2;	//MDID, Arg[1], opt:Arg[2-16]
			//Payload min:2 max:17
			
			//Arg[1-16]: RAW data
			//Fully custom
			//manually give Destination (DID)
			//DID = 0;	//Report to everyone
			//SID is set later

			break;

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		case Heartbeat_Report:			//
			//TX_Payload_length = 1;	//MDID: Default
			
			DID = 0;	//Report to everyone
			//SID is set later
			
			break;
			
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		default:
			
			return;		//Not recognized Message ID
			
			break;
			
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	}	//Switch

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	
	//dont request any ack for now
	
	//The UPB Transmit Control Register 
	//Link Packet Enable	
	//ACK MSG Req
	//ID Pulse Req
	//ACK pulse req
	//TX count

	//NID ALWAYS loaded with this device's at the top
	//but can be overwrited when needed by commands	
	
	SID = Unit_ID_fast;				 	//This device's address, only Destionation 
	//DID cannot be set here!!! when sending Link packet -> DID is uniq! (=LinkID)
	
	//#define CTL0	Header_Byte[0]
	//#define CTL1	Header_Byte[1]
	//CTL0/1 also set in TX_Multi_Packet() (Header_Byte[0/1] )!!!!!!
	//Only set payload here!!!!!
	
	//if DID is 0x00 (broadcast) must be Direct packet!
	if (DID == 0x00)
		CTL0 &= 0b01111111;
	
	CTL0 &= 0b11100000;				//Linkbit,Rep could be set!!!!
	CTL0 |= TX_Payload_length + 6;	//length min:6 (CTL0/1,NID,DID,SID,CHK) + 0-18
	//CTL1 is fully set in TX_Multi_Packet!!!!!!!!!!!!


	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	Checksum_Byte = Calced_checksum();	//Calc new checksum

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	
	//Send out Bytes -> Double bits!!
	//CANT USE COMMON BASE WITH RX -> DIFFERENT LENGTH!!!

	//wait 2 + RANDOM EMPTY halfwaves !!!!!!!!!!

	Wait_1_empty_halfwaves ();	//RX will be auto switched on then off

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	
	Send_Byte( Sync_Byte );	//Transmit Sync Byte
	
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	for (unsigned char i=0;i<5;i++)	//Transmit Header Bytes: 5
		Send_Byte( Header_Byte[i] );			
	
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		
	//Transmit Payload Bytes: 0-18 

	for (unsigned char i=0; i<TX_Payload_length ;i++)	//0-18 default:1 (MDID)
		Send_Byte( Payload_Byte[i] );
	
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	//Transmit checksum Byte: 1 Byte
	Send_Byte( Checksum_Byte );	
	//
	
	//check ACK pulse
	//Only receive ACK-es after the last packet of Multipackets
	if (LINK_bit == 0 && Transmit_Cnt == Transmit_Seq )	//Last packet, only direct
		ACK_Receive();
	
/////////////////////////////
/////////////
//////////////////////////	

}	//TX()
