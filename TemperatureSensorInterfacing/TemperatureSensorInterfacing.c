/*
 * SensorInterfacing.c
 *
 * Created: 27-Sep-19 9:04:13 AM
 * 
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <math.h> //included to support power function
#include "lcd.c"

void lcd_port_config (void)
{
	DDRC = DDRC | 0xF7; //all the LCD pin's direction set as output
	PORTC = PORTC & 0x80; // all the LCD pins are set to logic 0 except PORTC 7
}

void port_init()
{
	lcd_port_config();
}

void init_devices (void)
{
	cli(); //Clears the global interrupts
	port_init();
	sei(); //Enables the global interrupts
}

unsigned int temperatureReading(){
	
	unsigned int temperatureValue = 0;
	
	return temperatureValue;
}

int main(void)
{
	init_devices();
	
	lcd_init();
	
	//This is just an example listed below. You can update each of the variables however you want
	int row = 1;
	int column = 3;
	int sensorVal = 500;
	int digits = 4;
	
    while(1)
    {
		//Call function and print them on the LCD
		//Use the following function to print out your sensor reading
		lcd_print(row, column, sensorVal, digits);
    }
}