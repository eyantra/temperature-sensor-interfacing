/*
 * SensorInterfacing.c
 *
 * Created: 27-Sep-19 9:04:13 AM
 * Modified: 24-Oct-19 10:02:10 AM
 * Author: Debdut
 * Modifier: Debdut
 */

#define F_CPU 14745600UL
//#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include "lcd.h"
#include "th.h"

void port_init();
void init_devices(void);

char lcd_buff[6];

void port_init()
{
	lcd_init();
	th_init();
}

void init_devices(void)
{
	cli(); //Clears the global interrupts
	port_init();
	sei(); //Enables the global interrupts
}

int main(void)
{
	init_devices();
	
	lcd_cursor(1,3);
	lcd_string("HACTOBER-2K19");
	lcd_cursor(2,4);
	lcd_string("OPENSOURCE");
	_delay_ms(2000);
	lcd_clear();
	
	lcd_cursor(1,6);
	lcd_string("DHT11");
	lcd_cursor(2,7);
	lcd_string(":");
	
	while (1)
	{
		/*
		//Debugging code
		if(th_getData())
		{
			lcd_cursor(2,1);
			lcd_string("   ");
			
			lcd_cursor(2,3);
			lcd_string("TEMP:");
			lcd_print(2,8,dht11.TEMP,2);
			lcd_string(".");
			lcd_print(2,11,dht11.TEMP_F,2);
			lcd_wr_char(0xdf);
			lcd_string("C");
			_delay_ms(1000);
		
			lcd_cursor(2,3);
			lcd_string("HUMI:");
			lcd_print(2,8,dht11.HUM,3);
			lcd_string(".");
			lcd_print(2,12,dht11.HUM_F,2);
			lcd_string("%");
			_delay_ms(1000);
		}
		else
		{
			lcd_cursor(2,1);
			lcd_string("ER");
			_delay_ms(500);
		}
		*/
		
		/*
		//Float data printing code
		lcd_cursor(2,3);
		lcd_string("TEMP:");
		lcd_string(dtostrf(temperatureReadingFloat(), 6, 2, lcd_buff));
		lcd_wr_char(0xdf);
		lcd_string("C");
		_delay_ms(1000);
		
		lcd_cursor(2,3);
		lcd_string("HUMI:");
		lcd_string(dtostrf(humidityReadingFloat(), 6, 2, lcd_buff));
		lcd_string("% ");
		_delay_ms(1000);
		*/
		
		//Integer data printing code
		lcd_cursor(2,4);
		lcd_string("TEMP:");
		lcd_print(2,10,temperatureReading(),2);
		lcd_wr_char(0xdf); //DEGREE SYMBOL
		lcd_string("C");
		_delay_ms(1000);
		
		lcd_cursor(2,4);
		lcd_string("HUMI:");
		lcd_print(2,10,temperatureReading(),3);
		lcd_string("% ");
		_delay_ms(1000);
	}
}