/*
 * th.h
 *
 * Created: 20-Oct-19 03:56:54 PM
 * Modified: 26-Oct-19 08:36:06 PM
 * Author: Debdut
 * Modifier: Debdut
 */ 

#ifndef TH_H_
#define TH_H_

#define DHT_PORT PORTL //SENSOR PORT REG
#define DHT_PD DDRL //SENSOR PORT DATA DIRECTION REG
#define DHT_PIN PINL6 //SENSOR PIN
#define DHT_PDR PINL //SENSOR INPUT DATA REG

/*
 * Clock Cycles and Time related formulas:
 * Clock Cycles per Micro Second = CPU Frequency / 1MHz { EX: 16000000UL / 1000000UL = 16 cycles }
 * Micro Seconds to Clock Cycles = Micro Seconds * Clock Cycles per Micro Second { EX: 1000 * (16000000UL / 1000000UL) = 16000 cycles }
 * Clock Cycles to Micro Seconds = Clock Cycles / Clock Cycles per Micro Second { EX: 16000 / (16000000UL / 1000000UL) = 1000 us }
 */
#ifndef _CT_F
#define _CT_F

#define _CPM() (F_CPU / 1000000UL) //CLOCK CYCLES PER MICRO SECOND
#define _MTC(us) ((us) * _CPM()) //MICRO SECONDS TO CLOCK CYCLES
#define _CTM(cl) ((cl) / _CPM()) //CLOCK CYCLES TO MICRO SECONDS

#endif

#define TH_MIN_INTERVAL 1000 //MINIMUM INTERVAL BETWEEN 2 RETRIES
#define TH_TIMEOUT -1 //TH_TIMEOUT TO -1
#define TH_MAX_CYCLE _MTC(1000) //CYCLES IN 1000 MICRO SECONDS

#include <stdbool.h>

void th_port_config(void);
void th_init();

bool th_wake();
bool th_getData();
uint8_t temperatureReading();
uint8_t humidityReading();
float temperatureReadingFloat();
float humidityReadingFloat();

uint32_t th_getPulse(bool);

//Sensor final data store structure
typedef struct TH
{
	uint8_t TEMP;
	uint8_t TEMP_F;
	uint8_t HUM;
	uint8_t HUM_F;
	uint8_t CHK;
} th;

th dht11; //DATA STORE
bool th_lastResult; //LAST RESULT STATUS
uint8_t th_rc = 0; //RETRY COUNT

//Function to config sensor port
void th_port_config(void)
{
	DHT_PD |= (1 << DHT_PIN); //SET PORT DIRECTION TO OUTPUT
	//DHT_PORT |= (1 << DHT_PIN); //WRITE HIGH TO OUTPUT
}

//Function to initialize sensor
void th_init()
{
	th_port_config();
	_delay_ms(1000);
}

//Function to wake sensor and return wake status
bool th_wake()
{
	th_lastResult = true; //SET LAST RESULT STATUS TO TRUE
	
	//Sending wakeup signal
	DHT_PD |= (1 << DHT_PIN); //SET PORT DIRECTION TO OUTPUT
	DHT_PORT |= (1 << DHT_PIN); //WRITE HIGH TO OUTPUT
	_delay_ms(1); //DELAY FOR 1ms
	DHT_PORT &= ~(1 << DHT_PIN); //WRITE LOW TO OUTPUT
	_delay_ms(20); //DELAY FOR 18ms
	DHT_PORT |= (1 << DHT_PIN); //WRITE HIGH TO OUTPUT
	_delay_us(55); //DELAY FOR 40us
	DHT_PORT &= ~(1 << DHT_PIN); //WRITE LOW TO OUTPUT (DISABLE INTERNAL PULLUP) 
	/*
	   WARNING: IF THE SENSOR HAS A 5K OHM PULL UP RESISTOR CONNECTED 
	   BETWEEN DATA PIN AND VCC THEN ATMAGA2560 INTERNAL 50K PULLUP 
	   IS NOT NECESSARY, THEREFORE DISABLE INTERNAL PULLUP AND  IF
	   NOT THEN DON'T DISABLE IT
	 */
	
	//Set pin as input
	DHT_PD &= ~(1 << DHT_PIN); //SET PORT DIRECTION TO INPUT

	//Timing critical code is starting
	cli(); //CLEAR INTERRUPT
	
	//Listen for a 80us low pulse then 80us high pulse if success then sensor responded
	if(th_getPulse(0) == TH_TIMEOUT)
	{
		th_lastResult = false;
	}
	if(th_getPulse(1) == TH_TIMEOUT)
	{
		th_lastResult = false;
	}
	
	//Retry mechanism to retry for max 3 times if sensor doesn't responds
	if(th_lastResult)
	{
		th_rc = 0;
		return true;
	}
	else
	{
		th_rc++;
		if(th_rc > 2)
		{
			th_rc = 0;
			return false;
		}
		else
		{
			//Timing critical code is ending
			sei(); //SET INTERRUPT
			_delay_ms(TH_MIN_INTERVAL);
			return th_wake();
		}
	}
}

//Function to get sensor data
bool th_getData()
{
	uint32_t cycles[80]; //ARRAY WILL BE CONSIST PULSE DATA IN CLOCK CYCLES { EX: 10110011... --> _microSecToCycle(50,70,50,28,50,70,50,70,50,28,50,28,50,70,50,70,...) (TOTAL PULSE COUNT: 40*2) }
	uint8_t th_data[5] = {0,0,0,0,0}; //ARRY WILL BE CONSIST TEMPORARY SENSOR DATA
	
	if(th_wake()) //CHECK IF SENSOR RESPONDS
	{
		//Store cycles for each pulses from entire pulse train of 80 pulses for 40bits of data to analyze it later 
		for(int i = 0; i < 80; i += 2)
		{
			cycles[i] = th_getPulse(0);
			cycles[i + 1] = th_getPulse(1);
		}
	
		//Timing critical code is ending
		sei(); //SET INTERRUPT

		/*
		   Inspect pulses and determine which ones are 0 (high state cycle count < low
		   state cycle count), or 1 (high state cycle count > low state cycle count)
		*/
		for(int i = 0; i < 40; ++i)
		{
			uint32_t lowCycles = cycles[2 * i];
			uint32_t highCycles = cycles[2 * i + 1];
			if ((lowCycles == TH_TIMEOUT) || (highCycles == TH_TIMEOUT)) //CHECK IF ANY OF PULSE CONSISTS TH_TIMEOUT(-1) CYCLE
			{
				th_lastResult = false;
				return th_lastResult;
				//break;
			}
			
			th_data[i / 8] <<= 1; //SHIFT LEFT FOR 1 ROOM { EX: 1011 --> 0110 }
			
			//Check the low and high cycle times to see if the bit is a 0 or 1.
			if (highCycles > lowCycles) //CHECK IF HIGH CYCLE IS GREATER THAN 50us LOW CYCLE COUNT, SO BIT MUST BE A 1
			{
				th_data[i / 8] |= 1; //REPLACE SHIFTED NEW BIT WITH 1 { EX: 0110 --> 0111 }
			}
		}
		
		//Check for data received with checksum if data has no error then copy all data to sensor structure variable and return true
		if(th_data[4] == ((th_data[0] + th_data[1] + th_data[2] + th_data[3]) & 0xFF))
		{
			dht11.HUM = th_data[0]; //HUMIDITY INTEGER PART
			dht11.HUM_F = th_data[1]; //HUMIDITY FRACTION PART
			dht11.TEMP = th_data[2]; //TEMPERATURE INTEGER PART
			dht11.TEMP_F = th_data[3]; //TEMPERATURE FRACTION PART
			dht11.CHK = th_data[4]; //CHECKSUM
			
			return true;
		}
		else return false;
	}
	else return false;
}

//Function to get temperature reading
uint8_t temperatureReading()
{
	if(th_getData()) //IF ABLE TO GET DATA THEN RETURN TEMPERATURE
		return dht11.TEMP;
	else
		return (uint8_t)NAN;
}

//Function to get humidity reading
uint8_t humidityReading()
{
	if(th_getData()) //IF ABLE TO GET DATA THEN RETURN HUMIDITY
		return dht11.HUM;
	else
		return (uint8_t)NAN;
}

//Function to get temperature reading in fraction
float temperatureReadingFloat()
{
	if(th_getData()) //IF ABLE TO GET DATA THEN RETURN TEMPERATURE IN FLOAT
		return dht11.TEMP + dht11.TEMP_F * 0.01f;
	else
		return NAN;
}

//Function to get humidity reading in fraction
float humidityReadingFloat()
{
	if(th_getData()) //IF ABLE TO GET DATA THEN RETURN HUMIDITY IN FLOAT
		return dht11.HUM + dht11.HUM_F * 0.01f;
	else
		return NAN;
}

//Function to get pulse duration in clock cycles
uint32_t th_getPulse(bool level)
{
	uint16_t cycles = 0; //FOR COUNTING CLOCK CYCLES
	
	//Check pulse levels in sensor pin with given pulse level and increase cycle count until pulse level changes compare to given pulse level
	while(((DHT_PDR & (1 << DHT_PIN)) >> DHT_PIN) == level)
	{
		if(cycles++ >= TH_MAX_CYCLE)
		{
			return TH_TIMEOUT; //IF MAX CYCLES EXCEEDED THEN RETURN TH_TIMEOUT
		}
	}

	return cycles;
}

#endif