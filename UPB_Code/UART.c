/****************************************************************************** 
 * ++ 
 * Author:    Tran Hoang Luan (LuanTH)
 * Module Name  :  main.c
 * Description: receiving and transmitting data with a format that was written
 *				 to communicating with embedded web server and UPB-PC
 *					software.
 *
 * Mod. History :	16.Sep.12	- Tran Hoang Luan  
 *									+	File first created 
 * --    
*******************************************************************************/ 
#include "UART.h"

//variable
//
//volatile unsigned char UARTCommand = Null_cmd;
unsigned char UART_TX 		= 0;
volatile unsigned char Need_To_TX_UART = 0;
volatile unsigned char UART_Buffer[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//NID+DID+Reserve
volatile unsigned char UART_Payload_Length = 0;
volatile unsigned char UART_Payload_Index = 0;
volatile unsigned char UART_Sync_Flag = false;
FILE uartstd = FDEV_SETUP_STREAM(uart_char_tx, NULL,_FDEV_SETUP_WRITE);

//prototype 
void uart_Init(void);
void uart_char_tx(unsigned char);

//

void uart_char_tx(unsigned char chr){
	if (chr == '\n')
		uart_char_tx('\r');
    while (bit_is_clear(UCSRA,UDRE)) {}; //cho den khi bit UDRE=1 
    UDR=chr;
}

//
//
//
void uart_Init(void)
{
	//set baud,38.4k ung voi f=16Mhz(25), 8mhz(12)
	UBRRH=0;  
	UBRRL=25;
	//set khung truyen va kich hoat bo nhan du lieu
	UCSRA=0x00;
	UCSRC=(1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);// truyen 8 bit	
	UCSRB=(1<<RXEN)|(1<<TXEN);//|(1<<RXCIE); //Chi khoi dong bo thu phat ,ngat RX se duoc set sau
	stdout=&uartstd;
}
//
//
//
ISR(SIG_UART_RECV){ //trinh phuc vu ngat USART 
	//In ma ASCII cua phim duoc nhan
	uint8_t UDATA = UDR;
	if(UART_Sync_Flag)
	{
		if(UART_Payload_Length==0)
		{
			UART_Payload_Length = UDATA;
			UART_Payload_Index = 0;
		}
		else
		{
			UART_Buffer[UART_Payload_Index++] = UDATA;
			if(--UART_Payload_Length==0)
			{
				Need_To_TX_UART = 1;
				UART_TX = 1;//uart TX not button
				UART_Sync_Flag = false;			
			}
		}
	}
	//don't change the order
	if((UDATA==UART_Sync_Byte) && (!UART_Sync_Flag))
		UART_Sync_Flag = true;
}
