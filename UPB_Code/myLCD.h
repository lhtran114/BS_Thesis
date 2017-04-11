#include <util/delay.h>

#define 	sbi(sfr,bit)	sfr|=_BV(bit)
#define 	cbi(sfr,bit)	sfr&=~(_BV(bit))

#define EN 			2
#define RW 			1
#define RS 			0
#define CTRL		PORTA
#define DDR_CTRL	DDRA

#define DATA_O		PORTA
#define DATA_I		PINA
#define DDR_DATA	DDRA
/*
#define LCD8BIT
#define DATA_O		PORTD
#define DATA_I		PIND
#define DDR_DATA	DDRD
*/

char Read2Nib(void){
	char HNib, LNib;
	DATA_O |=0xF0;

    DDR_DATA &=0x0F;
	sbi(CTRL,EN); //enable
	sbi(CTRL,EN); // dong nay chi co tac dung delay vi chan EN da o muc cao roi
	

	HNib=DATA_I & 0xF0;
	cbi(CTRL,EN); //disable

	sbi(CTRL,EN); //enable
	sbi(CTRL,EN); //dong nay chi co tac dung delay vi chan EN da o muc cao roi	
	LNib = DATA_I & 0xF0;
	cbi(CTRL,EN); //disable
	LNib>>=4;
	return (HNib|LNib);
}

void Write2Nib(uint8_t chr){

	uint8_t HNib, LNib, temp_data;	
	temp_data=DATA_O & 0x0F;

	HNib=chr & 0xF0;
	LNib=(chr<<4) & 0xF0;		
		
	DATA_O =(HNib |temp_data);	
	sbi(CTRL,EN); //enable
	cbi(CTRL,EN); //disable	
	
	DATA_O =(LNib|temp_data);		
	sbi(CTRL,EN); //enable
	cbi(CTRL,EN); //disable
}

void Write8Bit(uint8_t chr){
	DATA_O=chr;   //out 8 bits to DATA Line
	sbi(CTRL,EN); //enable
	cbi(CTRL,EN); //disable
}

void wait_LCD(void){	
	#ifdef LCD8BIT	
		while(1){
			cbi(CTRL,EN); //disable before doing sth
			cbi(CTRL,RS); //RS=0 mean the following data is COMMAND (not normal DATA)
			sbi(CTRL,RW); //the Direction of this COMMAND is from LCD -> chip
			
			DDR_DATA=0xFF;//Ready to Out
			DATA_O=0xFF; //Out the COMMAND, 0xFF means "Read Status Bit" COMMAND	
			sbi(CTRL,EN); //enable
			
			DDR_DATA=0x00;//Ready to IN
			if(bit_is_clear(DATA_I,7)) break;
		}
		cbi(CTRL,EN); //disable for next step	
		cbi(CTRL,RW); //ready for next step	
		DDR_DATA=0xFF;//Ready to Out
	#else
		char temp_val;
		while(1){
			cbi(CTRL,RS); //RS=0 mean the following data is COMMAND (not normal DATA)
			sbi(CTRL,RW); //the Direction of this COMMAND is from LCD -> chip
			temp_val=Read2Nib();
			if (bit_is_clear(temp_val,7)) break;
		}
		cbi(CTRL,RW); //ready for next step	
		DDR_DATA=0xFF;//Ready to Out		
	#endif
	
	//_delay_ms(1);
	
}


void init_LCD(void){
	DDR_CTRL=0xFF;
	DDR_DATA=0xFF;
//Function set------------------------------------------------------------------------------
	cbi(CTRL,RS);   // the following data is COMMAND
	cbi(CTRL, RW); // AVR->LCD
	cbi(CTRL, EN);
	#ifdef LCD8BIT	
		Write8Bit(0x38);
		wait_LCD();
	#else
		sbi(CTRL,EN); //enable
		sbi(DATA_O, 5); 
		cbi(CTRL,EN); //disable
		wait_LCD();		
		Write2Nib(0x28);//
		wait_LCD();	
	#endif
//Display control-------------------------------------------------------------------------	
	cbi(CTRL,RS); // the following data is COMMAND 
	#ifdef LCD8BIT	
		Write8Bit(0x0E);
		wait_LCD();
	#else
		Write2Nib(0x0E);
		wait_LCD();			
	#endif
//Entry mode set------------------------------------------------------------------------
	cbi(CTRL,RS); // the following data is COMMAND 
	#ifdef LCD8BIT
		Write8Bit(0x06);
		wait_LCD();
	#else
		Write2Nib(0x06);
		wait_LCD();
	#endif
}

void home_LCD(void){
	cbi(CTRL,RS); // the following data is COMMAND 
	#ifdef LCD8BIT	
		Write8Bit(0x02);
		wait_LCD();	
	#else
		Write2Nib(0x02);
		wait_LCD();
	#endif	
}
void move_LCD(uint8_t y,uint8_t x){
	uint8_t Ad;
	Ad=64*(y-1)+(x-1)+0x80; //
	cbi(CTRL,RS); // the following data is COMMAND
	#ifdef LCD8BIT	
		Write8Bit(Ad);
		wait_LCD();
	#else
		Write2Nib(Ad);
		wait_LCD();
	#endif	
}

void clr_LCD(void){
	cbi(CTRL,RS); //RS=0 mean the following data is COMMAND (not normal DATA)
	#ifdef LCD8BIT	
		Write8Bit(0x01);
		wait_LCD();	
	#else
		Write2Nib(0x01);
		wait_LCD();	
		
	#endif	
}

void putChar_LCD(uint8_t chr){
	sbi(CTRL,RS); //this is a normal DATA	
	#ifdef LCD8BIT	
		Write8Bit(chr);
		wait_LCD();	
	#else
		Write2Nib(chr);
		wait_LCD();	
	#endif		
}

void print_LCD(char* str){
	 unsigned char i;
	 for (i=0; str[i]!=0; i++) putChar_LCD(str[i]);	 
}