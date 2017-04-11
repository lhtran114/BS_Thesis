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
#ifndef PROTOCOL_DEFINES_H
#define PROTOCOL_DEFINES_H 1

	/*
	#define MIDH					0
	#define MIDL					0
	#define PIDH					0
	#define PIDL					0
	#define Signal_Strength			0
	#define Noise_Level				0
	#define UPBID_checksum			0
	#define All_Setup_Regs_checksum		0
    */

	#define Netw_PWD_Addr_H			Network_Password
	#define Netw_PWD_Addr_L			( Network_Password + 1 )
	#define Network_Password_MSB	Read_EEPROM( Netw_PWD_Addr_H )
	#define Network_Password_LSB	Read_EEPROM( Netw_PWD_Addr_L )
	#define Network_password_match 	(Arg[1] == Network_Password_MSB && Arg[2] == Network_Password_LSB)

	#define ACK_pulse_position		3
	#define ACK_ID_pulse_position 	ACK_pulse_position

	#define Sync_Byte 				0b10010110	//2,1,1,2 halfs
	
	#define LINK_bit				(Header_Byte[0] >> 7 & 0b00000001)
	#define Repeater_Req			(Header_Byte[0] >> 5 & 0b00000011)
	#define ABS_Packet_Length		(Header_Byte[0] & 0b00011111)	//5bit -> 6-24 Byte
	//#define Reserved				(Header_Byte[1] >> 7 & 0b00000001)
	#define ACK_Req					(Header_Byte[1] >> 4 & 0b00000111)
	#define Transmit_Cnt			(Header_Byte[1] >> 2 & 0b00000011)
	#define Transmit_Seq			(Header_Byte[1] & 0b00000011)

	#define ACK_Pulse_Req			(ACK_Req & 0b000000001)
	#define ID_Pulse_Req			(ACK_Req & 0b000000010)
	#define ACK_Message_Req			(ACK_Req & 0b000000100)
	//Can be all 3 at the same time

	#define Payload_Length 			(ABS_Packet_Length - 6)	//0-18 Byte: MDID + MDA
	//-6 because Checksum also substracted!!!!
	
	#define Arg						Payload_Byte			//Arg-es start from 1!
	#define Arg_length 				(Payload_Length - 1)	//Arg[1],Arg[2]...Arg[17]->Arg[0] is MDID

	#define CTL0	Header_Byte[0]
	#define CTL1	Header_Byte[1]
	#define NID		Header_Byte[2]		//0-255 Network ID
	#define DID		Header_Byte[3]		//0-255 included specials, Destination ID
	#define SID		Header_Byte[4]		//1-250 Source ID
	#define MDID	Payload_Byte[0]		//Argument[0] of payload
	#define MDA		Payload_Byte[1]		//The start of Message Data Arguments Field 0-17 Byte
	#define CHK		Checksum_Byte

	//MDID
	//#define MSID 	(MDID >> 5 & 0b00000111)	//Message Set
	//#define MID 	(MDID & 0b00011111)			//Message Identifier/Extended Set ID
	//#define ESID 	MID							//Message Identifier/Extended Set ID

	//#define SETUP_mode Write_enabled	//used together

	//____________UPB Core Command Set_________________________
	#define Null_cmd					0x00
	#define Write_Enable_cmd			0x01
	#define Write_Protect_cmd			0x02
	#define Start_Setup_Mode_cmd		0x03
	#define Stop_Setup_Mode_cmd			0x04
	#define Get_Setup_Time_cmd			0x05
	#define Auto_Address_cmd			0x06
	#define Get_Device_Status_cmd		0x07
	#define Set_Device_Control_cmd		0x08
	//#define Unused					0x09-0x0A
	#define Add_Link_cmd				0x0B
	#define Delete_Link_cmd				0x0C
	#define Transmit_This_Message_cmd	0x0D
	#define Device_Reset_cmd			0x0E
	#define Get_Device_Signature_cmd	0x0F
	#define Get_Register_Value_cmd		0x10
	#define Set_Register_Value_cmd		0x11
	//#define Unused					0x12-0x1F


	//____________Device_Control_Command_Set_For_Requesting!___
	#define Activate_Link_cmd			0x20
	#define Deactivate_Link_cmd			0x21
	#define Goto_cmd					0x22
	#define Fade_Start_cmd				0x23
	#define Fade_Stop_cmd				0x24
	#define Blink_cmd					0x25
	#define Indicate_cmd				0x26	//Not used in WS1D
	#define Toggle_cmd					0x27	//Not used in WS1D
	//#define Unused					0x28–0x2F
	#define Report_State_cmd			0x30
	#define Report_Status_cmd			Report_State_cmd	//The same!
	#define Store_State_cmd				0x31
	#define Store_Preset_cmd			Store_State_cmd		//The same!
	//#define Unused					0x32–0x3F
	#define Report_Sensor_cmd			0x3F	//My custom one!

	//____________UPB_Core_Report_Set_For_Answering!___________
	#define ACK_Response				0x80
	//#define Unused					0x81–0x84
	#define Setup_Time_Report			0x85
	#define Device_State_Report			0x86
	#define Device_Status_Report		0x87
	//#define Unused					0x88–0x8E
	#define Device_Signature_Report		0x8F
	#define Register_Values_Report		0x90
	#define RAM_Values_Report			0x91
	#define Raw_Data_Report				0x92
	#define Heartbeat_Report			0x93
	//#define Unused					0x94–0x9F	
	#define Device_Sensor_Report		0x9F	//My custom one!
	
	//Start addresses in eeprom
	#define Network_ID			0x00	//1-255
	#define Unit_ID				0x01	//1-250
	#define Network_Password	0x02	//2Byte MSB first
	#define UPB_Options			0x04
	#define UPB_Version			0x05	//002
	#define Manufacturer_ID		0x06	//2Byte MSB first
	#define Product_ID			0x08	//2Byte MSB first
	#define Firmware_Version	0x0A	//2Byte MSB first
	#define Serial_Number		0x0C	//4Byte MSB first
	#define Network_Name		0x10	//16Byte
	#define Room_Name			0x20	//16Byte
	#define Device_Name			0x30	//16Byte
	#define Configuration_Regs	0x40	//Limited only by MCU
	
	//__TX_component__
	//Double Tap not implemented
	#define Button0_Single_Tap_Level	0x7A	//0x7B: Rate
	#define Button0_Double_Tap_Level	0x7C	//0x7D: Rate
	#define Button1_Single_Tap_Level	0x7E	//0x7F: Rate
	#define Button1_Double_Tap_Level	0x80	//0x81: Rate
	
	//Config: 5Bytes: LinkID, 1-Tap, 2-Tap, Hold, Release
	#define Button0_Config	0x70
	#define Button1_Config	0x75

	#define LED_Options		0x8B

	//TX component addresses in EEPROM + Def value
	//One command: 3Byte: 
	#define	TX_command_Offset	0x90
	/*
	#define	TX_command_0	0x90	//Goto Off 
	#define	TX_command_1	0x93	//Goto On 
	#define	TX_command_2	0x96	//Fade Down 
	#define	TX_command_3	0x99	//Fade Up 
	#define	TX_command_4	0x9C	//Fade Stop 
	#define	TX_command_5	0x9F	//Deactivate 
	#define	TX_command_6	0xA2	//Activate 
	#define	TX_command_7	0xA5	//Snap Off 
	#define	TX_command_8	0xA8	//Snap On 
	#define	TX_command_9	0xAB	//Quick Off 
	#define	TX_command_10	0xAE	//Quick On 
	#define	TX_command_11	0xB1	//Slow Off 
	#define	TX_command_12	0xB4	//Slow On 
	#define	TX_command_13	0xB7	//Blink 
	*/
	#define Auto_Off_Timer_LinkID_Address	0x89
	#define Auto_Off_Timer_Command_Address	0x8A

	#define Auto_Off_Timer		0x8F	//only from v5.5 PCS FW

	#define Tx_Control			0x8E

	#define Dimmer_Options		0x8D	//WS1D model

	//length of registers
	#define Network_Password_Length		2
	#define Manufacturer_ID_Length		2
	#define Product_ID_Length			2
	#define Firmware_Version_Length		2
	#define Serial_Number_Length		4
	#define Network_Name_Length			16
	#define Room_Name_Length			16
	#define Device_Name_Length			16

	//________The Scratch-Pad Registers________
	#define Last_On_Level			0xF9
	#define Setup_Mode_Counter		0xFA
	#define WERR_Counter			0xFB
	#define POR_Counter				0xFC
	#define BOR_Counter				0xFD
	#define WDT_Counter				0xFE
	#define MCLR_Counter			0xFF
	//#define Reset_Light_Level		Last_On_Level

	//LinkID-s 16x3 Byte, 16 links

	#define LinkID_length 			3

	//3Byte -> Link# 1-250,Ligth level 0-100,Fade rate 0-15 def:255

	//Start values! - Universal for dimmers
	//if LinkID = 0 or 255 -> empty

	#define LinkID_1				0x40
	#define LinkID_start			LinkID_1

	/*
	#define LinkID_2	0x43
	#define LinkID_3	0x46
	#define LinkID_4	0x49
	#define LinkID_5	0x4C
	#define LinkID_6	0x4F
	#define LinkID_7	0x52
	#define LinkID_8	0x55
	#define LinkID_9	0x58
	#define LinkID_10	0x5B
	#define LinkID_11	0x5E
	#define LinkID_12	0x61
	#define LinkID_13	0x64
	#define LinkID_14	0x67
	#define LinkID_15	0x6A
	#define LinkID_16	0x6D
	*/

#endif
