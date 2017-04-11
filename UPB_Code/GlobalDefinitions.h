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

#ifndef GLOBALDEFINITIONS_H
#define GLOBALDEFINITIONS_H 1
#include "Protocol_defines.h"
#include "Pins_Init.h"
	typedef unsigned char		uint8;
	typedef unsigned int		uint16;	

	//bit macro
	#define bit_get(p,m)		((p) & (m))
	#define bit_set(p,m)		((p) |= (m))
	#define bit_clear(p,m)		((p) &=~(m))
	#define bit_flip(p,m)		((p) ^= (m))
	#define BIT(x)				(0x01 << (x)) 
	//
	#define false		0
	#define true 		1
	#define ON 			1
	#define OFF 		0
	#define On 			1
	#define Off 		0
	#define HIGH 		1
	#define LOW			0
	#define Pressed 	0
	#define Released 	1
	//
	#define _NOP()		__asm__ __volatile__ ("nop")
	#define NOP()		_NOP()
	#define TMR0		TCNT0
	#define TMR1		TCNT1
	#define TMR2		TCNT2
	
	/////////////////////////////////////////////////////////////
	//ALL INVERTED
	/////////////////////////////////////////////////////////////
	#define Red_LED_On			bit_clear(PORTB,BIT(Red_Led))
	#define Red_LED_Off			bit_set(PORTB,BIT(Red_Led))
	#define Red_LED_Toggle		bit_flip(PORTB,BIT(Red_Led))
	#define Green_LED_On		bit_clear(PORTB,BIT(Green_Led))
	#define Green_LED_Off		bit_set(PORTB,BIT(Green_Led))
	#define Green_LED_Toggle	bit_flip(PORTB,BIT(Green_Led))

	#define Button0_Pressed !bit_get(PINC,BIT(Button0_Pin))
	#define Button1_Pressed !bit_get(PINC,BIT(Button1_Pin))
	#define Button2_Pressed !bit_get(PINC,BIT(Button2_Pin))
	
	/////////////////////////////////////////////////////////////
	//Dimmer Light
	/////////////////////////////////////////////////////////////
	#define Disable_TMR1_compare_Light_int	bit_clear(TIMSK,BIT(OCIE1B))
	#define Enable_TMR1_compare_Light_int	bit_set(TIMSK,BIT(OCIE1B))
	#define Activate_Load_Triac				bit_set(PORTD,BIT(Dimmer_Output_Pin))
	#define Deactivate_Load_Triac			bit_clear(PORTD,BIT(Dimmer_Output_Pin))
	/////////////////////////////////////////////////////////////
	//UART 
	/////////////////////////////////////////////////////////////
	#define Enable_UART 					bit_set(UCSRB,BIT(RXCIE))
	#define Disable_UART 					bit_clear(UCSRB,BIT(RXCIE))
	
	#define NID_UART 						UART_Buffer[0]
	#define DID_UART 						UART_Buffer[1]
	#define Reserve_UART 					UART_Buffer[2]
	#define MDID_UART 						UART_Buffer[3] //Arg[0]
	#define Arg_UART(i) 					UART_Buffer[3+i]
	#define UARTCommand 					MDID_UART //same!
	
	#define Link_UART 						(Reserve_UART>>7 & 0b000000001)
	#define ACK_UART						(Reserve_UART>>4 & 0b000000111)
	#define ACK_Pulse_UART 					(ACK_UART & 0b000000001)
	#define ID_Pulse_UART 					(ACK_UART & 0b000000010)
	#define ACK_Message_UART 				(ACK_UART & 0b000000100) 
	
	/////////////////////////////////////////////////////////////
	// Zero Cross definitions
	/////////////////////////////////////////////////////////////
	#define Zero_Cross_Signal 			bit_get(PIND,BIT(Zero_Cross_Pin))//((PIND >> Zero_Cross_Pin) & 0x01)
	#define Enable_Zerocross_Interrupt 	bit_set(GICR,BIT(INT0))	
	#define Disable_Zerocross_Interrupt	bit_clear(GICR,BIT(INT0))
	#define ZeroCross_Sample_Number		5	//too much will delay offset!!!
	
	/////////////////////////////////////////////////////////////
	//Timer definitions
	/////////////////////////////////////////////////////////////
	#define TMR1_on				((TCCR1B & 0b00000111) != 0)		
	#define Prescaler_Timer1	8	//MUST Set manually!!!	
	#define Start_Timer1 		(TCCR1B = 0b00000010)	//Prescaler:8 overflow at 8Mhz 65mS Res: 1uS
	#define Stop_Timer1			TCCR1B = 0
	#define Reset_Timer1		TCNT1 = 0
	#define Enable_CompA_Int 	bit_set(TIMSK,BIT(OCIE1A));	//Enable interrupt on compare 1A
	
	/////////////////////////////////////////////////////////////
	//Signal Comparator
	/////////////////////////////////////////////////////////////
	#define Disable_Comp_Int	bit_clear(ACSR,BIT(ACIE))
	#define Enable_Comp_Int		bit_set(ACSR,BIT(ACIE))
	#define Enable_Comparator	bit_clear(ACSR,BIT(ACD))	//Analog Comparator Disable is "0"->switch on
	#define Disable_Comparator	bit_set(ACSR,BIT(ACD)) 
	
	#define Comparator_Output	    bit_get(ACSR,BIT(ACO)) 
	#define Signal_Input 			(!Comparator_Output)	//Signal is higher than Vref
	#define Signal_Input_Inverted 	Comparator_Output

	
	/////////////////////////////////////////////////////////////
	//AC Power wave
	/////////////////////////////////////////////////////////////
	#define Sine_Wave_Min_Period 	19000	//10mS * 0.95 = 9.5mS = 9500uS
	#define Sine_Wave_Max_Period 	21000	//10mS * 1.05 = 10.5mS = 10500uS
	
	/////////////////////////////////////////////////////////////
	//Noise Threshold
	/////////////////////////////////////////////////////////////
	#define Min_Noise_Threshold 	40	//60 40 27//~0,5V perfect	
	#define Noise_Threshold_Step	16	//25 16 20
	
	/////////////////////////////////////////////////////////////
	//Fire Triacs
	/////////////////////////////////////////////////////////////
	#define Triac_On_Response_Time	100 //uS	//must be >50us!
	//#define Triac_Trigger_On		bit_set  (PORTD,BIT(Signal_Generate_Pin))
	//#define Triac_Trigger_Off		bit_clear(PORTD,BIT(Signal_Generate_Pin))
	#define Triac_Trigger_On		bit_set  (DDRD,BIT(Signal_Generate_Pin));	bit_set  (PORTD,BIT(Signal_Generate_Pin))
	#define Triac_Trigger_Off		bit_clear(DDRD,BIT(Signal_Generate_Pin));	bit_clear(PORTD,BIT(Signal_Generate_Pin))
	
	/*
	//DONT OVERLAP ZEROCROSS WITH UPB_SIGNAL

	Offset values
	maybe Offset has to be corrected
	50Hz
	AGC_start: 7620 uS
	Signal Offset: 7980 uS

	60Hz
	AGC_start: 6893,3 uS
	Signal Offset: 7253,3 uS
	*/
	#define AGC_start			15240
	#define Offset_Nominal		15960	//For TX! optimal position
	#define Pulse_Spacing		( Pos1_L - Pos0_L )			//160 us
	#define Half_Pulse_Width	( ( Pos0_U - Pos0_L ) / 2 )	//40 us

	//Signal1:80uS, gap:80uS, Signal2:80uS, gap:80uS, Signal3:80
	//gap:80uS, Signal4:80uS
	#define Pos_Steps	160		//!!! 80 uS !!!!
	#define Pos0_L (0*Pos_Steps) //	 0 uS
	#define Pos0_U (1*Pos_Steps) // 80 uS
	#define Pos1_L (2*Pos_Steps) //160 uS
	#define Pos1_U (3*Pos_Steps) //240 uS
	#define Pos2_L (4*Pos_Steps) //320 uS
	#define Pos2_U (5*Pos_Steps) //400 uS
	#define Pos3_L (6*Pos_Steps) //480 uS
	#define Pos3_U (7*Pos_Steps) //560 uS

	/////////////////////////////////////////////////////////////
	//////////////////////////////////////////
	////  On going...
	//////////////////////////////////////////
	/////////////////////////////////////////////////////////////

	//For atmega8:#define Enable_Comp_int				bit_set(ACSR,BIT(ACIE))

	//For atmega8:#define Enable_Zerocross_interrupt	bit_set(GICR,BIT(INT1))
	//For atmega8:#define Disable_Zerocross_interrupt	bit_clear(GICR,BIT(INT1))

	
	
	#define Low_Byte(x)				( (unsigned char) (x) )
	#define High_Byte(x)			( (unsigned char) ( ( (unsigned int) (x) ) >> 8) )
	
	#define UART_Receiving			(bit_get(UCSR0A,BIT(RXC0)) >> 7 )	//if (UART_Receiving == 1)

	//Not universal!
	//#define Start_Timer2 (TCCR2B = 0b00000111)	//Prescaler:1024 overflow at 8Mhz 32mS Res: 128uS
			
	

	//16Mhz
	//AGC will end sooner!
	#define Allowed_Negative_Delay	0	//40us To RX pulses before offset
	#define Max_Delay_For_Both		255 //calced from |-40|+40=80us=160clk

	
	
#endif
