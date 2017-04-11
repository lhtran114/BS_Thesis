/****************************************************************************** 
 * ++ 
 * Author:    Tran Hoang Luan (LuanTH)
 * Module Name  :  UART.h
 * Description: header file be used for UART.c
 *
 * Mod. History :	16.Sep.12	- Tran Hoang Luan  
 *									+	File first created 
 * --    
*******************************************************************************/ 
#ifndef UART_H
#define UART_H 1

	//Includes
	#include <avr/io.h>
	#include <avr/interrupt.h>
	#include <util/delay.h>
	#include <stdio.h>
	#include "GlobalDefinitions.h"
	//definition
	#define UART_Sync_Byte 55
	#define UART_Data_Clear UART_Sync_Flag=false;UART_Payload_Length=0
	//prototype 
	extern void uart_Init(void);
	extern void uart_char_tx(unsigned char);
	//variable
	//
	//extern volatile unsigned char UARTCommand;
	extern unsigned char UART_TX;
	extern volatile unsigned char Need_To_TX_UART;
	extern volatile unsigned char UART_Buffer[21];
	extern volatile unsigned char UART_Payload_Length;
	extern volatile unsigned char UART_Payload_Index;
	extern volatile unsigned char UART_Sync_Flag;
	extern FILE uartstd;
	
#endif
