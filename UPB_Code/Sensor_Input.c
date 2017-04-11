
//include
#include "Sensor_Input.h"

//definition
#define Num_of_Sample 20

//protorype
void Sensor_Init(void);
void Read_Sensor(void);
uint16_t ADC_Convert(unsigned char);

//variable
volatile unsigned char Need_To_Read_Sensor = 0;
volatile uint16_t Light_Value = 0;
volatile uint16_t Temp_Value = 0;
////////////
//sensor init
////////////
//Thiet lap cho ADC prescaler = 8
void Sensor_Init(void)
{
	ADCSRA |= (1<<ADEN)|(1<<ADPS2)/*|(1<<ADPS1)|(1<<ADPS0)*/;//prescale = 8
	ADMUX |= (1<<REFS0);
}
///////////////////////////////////////////////////////////////////////////////////////////////
//Read ADC chanel
///////////////////////////////////////////////////////////////////////////////////////////////
uint16_t ADC_Convert(unsigned char adc_chanel)
{
	ADMUX = adc_chanel|(1<<REFS0);
	ADCSRA |= (1<<ADSC);	//start convert
	loop_until_bit_is_clear(ADCSRA,ADSC);
	return ADCW;
}


////////////////////////
//update ADC value
////////////////////////
void Read_Sensor(void)
{
	Light_Value = ADC_Convert(Light_Sensor_Pin);//*0.488;
	Temp_Value = 0;
	for(unsigned char i=0;i<Num_of_Sample;i++)
		Temp_Value += ADC_Convert(Temp_Sensor_Pin);
	Temp_Value /= Num_of_Sample;
	Temp_Value *= 500/1024;//500/1024;
}











