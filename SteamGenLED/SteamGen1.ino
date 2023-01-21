//#define __MY_DEBUG_LOOP__
#define __MY_DEBUG_LOG__

#if defined (__AVR__)
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#elif defined (__arm__)

#else
#error "Processor architecture is not supported."
#endif
#include "Arduino.h"



// Lib....
#include "max6675.h"

#include "steamgen1.h"

//#define  __MY_DEBUG_TRACE__
//#define  __MY_WATCHDOG__
//#define  __SLEEP_MODE_PWR_DOWN__


MAX6675 thermocouple (pinTemps_SCK, pinTempsCS_1, pinTemps_SO);
MAX6675 thermocouple2(pinTemps_SCK, pinTempsCS_2, pinTemps_SO);

const int wdt_timeout_count_MAX = 101; // w d t - time_out : 800 sec....(12 minutes)
int wdt_timeout_count = 0; 
volatile int iwdt_timeout_mode = 0;

int mux_readValue = 1023;

//--200911 int g_iDelayTime = 1500;
int g_iDelayTime = 1000;
int g_iswPressed = -1;
byte g_byteMuxValue = 0;




 ISR (PCINT1_vect)
{
#if defined (__MY_WATCHDOG__)	
	iwdt_timeout_mode = 0;
#endif
}

ISR(WDT_vect)
{
}

void watchdogOn() {
#if defined (__MY_WATCHDOG__)
	wdt_reset();

	// Clear the reset flag, the WDRF bit (bit 3) of MCUSR.
	MCUSR = MCUSR & B11110111;
	
	// Set the WDCE bit (bit 4) and the WDE bit (bit 3)
	// of WDTCSR. The WDCE bit must be set in order to
	// change WDE or the watchdog prescalers. Setting the
	// WDCE bit will allow updtaes to the prescalers and
	// WDE for 4 clock cycles then it will be reset by
	// hardware.
	WDTCSR = WDTCSR | B00011000;

	// Set the watchdog timeout prescaler value to 1024 K
	// which will yeild a time-out interval of about 8.0 s.
	WDTCSR = B00100001;

	// Enable the watchdog timer interupt.
	WDTCSR = WDTCSR | B01000000;
	//MCUSR = MCUSR & B11110111;
#endif	
}

ISR (TIMER1_COMPA_vect)
{
	TCNT1 = 0;			// manually reset Timer Count reset......
}


/********* 8 MHz*************/
void setup() {
  
  // put your setup code here, to run once:
  Serial.begin(115200);

  // LED pin define(0 pin)
  pinMode(pinOp_LED, OUTPUT);       digitalWrite(pinOp_LED, HIGH);  //  Operator LED....(R e d)...


}

void loop()
{
	g_byteMuxValue = 0;


		// LED High - Low Toggle.....
		if (led_toggle_Count >= led_toggle_Interval)
		{
			Serial.print("LED Toggle.....[");	Serial.print(led_Mode);		Serial.println("] !!"); 
			led_toggle_Count = 0;
			digitalWrite(pinOp_LED, led_Mode ? HIGH : LOW);
			led_Mode = !led_Mode;
		}			
		else
			led_toggle_Count++;

	delay(g_iDelayTime);  // RealTime..(Urgent!!).......    
}

void function_AlltestMode(int mode)
{
	  
	  Serial.println("Led On....!!");	
	  digitalWrite(pinOp_LED, HIGH); 
}

/* Motor PWM Ref...
 *  
 *  Fyi, the logic to get the TLE5205-2 to move is as follows:
OPEN
- IN1 = LOW
- IN2 = LOW

CLOSE
- IN1 = LOW
- IN2 = HIGH

BRAKE (slow close)
- IN1 = HIGH
- IN2 = LOW

OFF
- IN1 = HIGH
- IN2 = HIGH
 *  
 *  
 *  
void Motor_Pwm()
{  
 for (Speed = 150; Speed >= 1; Speed--)
 {
   analogWrite(Motortreiber1_IN1_Pin, Speed);
   digitalWrite(Motortreiber1_IN2_Pin, HIGH);  // Foward
   delay(200);
 }
 delay(1000);
 analogWrite(Motortreiber1_IN1_Pin, 255);   // Stop
 digitalWrite(Motortreiber1_IN2_Pin, HIGH);
 delay(2000);
 
 for (Speed = 150; Speed >= 1; Speed--)
 {
   digitalWrite(Motortreiber1_IN1_Pin, HIGH);
   analogWrite(Motortreiber1_IN2_Pin, Speed);  // Backward
   delay(200);
 }
 delay(1000);
 digitalWrite(Motortreiber1_IN1_Pin, HIGH);
 analogWrite(Motortreiber1_IN2_Pin, 255);    // Stop
 delay(2000);
}*/
 
void dipswitchRead()
{
  /*
  int tmpDipValue = 0;

  // Read DIP Switch....
    Serial.print("DIP 1 Value:");
  //tmpDipValue = analogRead(pinDIP_1);
  //tmpDipValue = (digitalRead(pinDIP_1) == 1)?0:1;
  tmpDipValue = (digitalRead(pinDIP_1) == 1) ? 0 : 1;
  dipswitchValue = tmpDipValue;
  Serial.print(tmpDipValue);
    Serial.print(" \t");

    Serial.print("DIP 2 Value:");
    //tmpDipValue = analogRead(pinDIP_2);
  tmpDipValue = (digitalRead(pinDIP_2) == 1) ? 0 : 1;
  dipswitchValue += (tmpDipValue << 1);
    Serial.println(tmpDipValue);

  Serial.print("DIP 3 Value:");
  //tmpDipValue = analogRead(pinDIP_3);
  tmpDipValue = (digitalRead(pinDIP_3) == 1) ? 0 : 1;
  dipswitchValue += (tmpDipValue << 2);
  Serial.print(tmpDipValue);
  Serial.print(" \t");

    Serial.print("DIP 4 Value:");
    //tmpDipValue = analogRead(pinDIP_4);
  tmpDipValue = (digitalRead(pinDIP_4) == 1) ? 0 : 1;
  dipswitchValue += (tmpDipValue << 3);
    Serial.println(tmpDipValue);

    Serial.print("DIP Switch Value:");
    Serial.println(dipswitchValue); */
}


