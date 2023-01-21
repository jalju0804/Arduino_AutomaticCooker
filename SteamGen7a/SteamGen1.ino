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

//bool g_fStartButtonOnePressed = false;
//bool g_fRecipeButtonOnePressed = false;
//bool g_fSteamButtonOnePressed = false;


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

int readMux_eachSensor (const byte which)
{
  // select correct MUX channel
  digitalWrite (pin_muxAddress_A, (which & 1) ? HIGH : LOW);  // low-order bit
  digitalWrite (pin_muxAddress_B, (which & 2) ? HIGH : LOW);
  digitalWrite (pin_muxAddress_C, (which & 4) ? HIGH : LOW);  // high-order bit
  // now read the sensor
  return analogRead (pin_muxOnCH);
}  // end of readSensor

byte readMux_AllChannel()
{
    // show all 8 sensor readings
  byte muxValue = 0; // 0x3ff - All released.....	
  
  //--no txd digitalWrite(pin_muxCE, LOW);    
  mux_readValue = 1023;
  for (byte i = 0; i < 8; i++)
    {
      Serial.print ("Mux Sensor(ch) ");
      Serial.print (i);
      Serial.print (" reads: ");
	  mux_readValue = readMux_eachSensor (i);
      Serial.println (mux_readValue);	
	    
	  if (mux_readValue < mux_effective_value)
			muxValue = muxValue | (B00000001 << i);
    }
	return muxValue;
}

byte readMux_SwChannel()
{
  //--no txd digitalWrite(pin_muxCE, LOW);  
    // show switch 3 sensor readings
  mux_readValue = 1023;
  byte swValue = 0; // 0x3ff - All released.....
  for (byte i = 0; i < 8; i++)
    {
		if ((i != 4) && (i != 6)) // Exclude 4, 6 : Water Level Sensor......
		{
#if defined (__MY_DEBUG_LOG__)
		  Serial.print ("Mux Sensor(switch-ch) ");
		  Serial.print (i);
		  Serial.print (" reads: ");
		  mux_readValue = readMux_eachSensor (i);
		  Serial.println (mux_readValue);     ///// ????????      
#else
		  mux_readValue = readMux_eachSensor (i);
#endif
		  if (mux_readValue < mux_effective_value)
		  {

			  // B0000 0000
			  // 0-1 : Start switch			  // 1-2 : Water switch
			  // 2-4 : Steam switch			  // 3-8 : Large switch
			  // 5-32 : Small switch		  // 7-128 : Medium switch
			  swValue = swValue | (B00000001 << i);			  
		  }
		}
    }
    return swValue;
}

// Read Tempe. Sensor ( Water1 / Recipe2 Box )....
float read_TemperatureSensor(const byte which)
{
  //float tempe;
  float tempe = -1;
  if (which == 1)
  {
      // Water Box....
#if defined (__MY_DEBUG_LOG__)      
      Serial.print("C 1= ");
      tempe = thermocouple.readCelsius();
      Serial.println(tempe);
#else
      tempe = thermocouple.readCelsius();
#endif      
  }
  else
  {
      // Recipe Box....
#if defined (__MY_DEBUG_LOG__)            
      Serial.print("C 2= ");
      tempe = thermocouple2.readCelsius();
      Serial.println(tempe);
#else
      tempe = thermocouple2.readCelsius();
#endif            
  }
  return tempe;
}

int read_AmbientSensor(const byte pinNum)
{
  int ambientValue;
  //int ambientValue;  
#if defined (__MY_DEBUG_LOG__NO)      
  Serial.print("Ambient Sensor(pin-");  
  Serial.print(pinNum);      Serial.print("): ");  
#endif
  ambientValue = analogRead(pinNum);
#if defined (__MY_DEBUG_LOG__NO)      
  Serial.println(ambientValue); 
#endif
  if (ambientValue <= ambient_sensor_effective_value)
  {  
	  Serial.print("Ambient Sensor(pin-");
	  Serial.print(pinNum);      Serial.print("): ");
	  Serial.println(ambientValue); 
  }
  return ambientValue;  
}

void  SolValve34_Control(int valvNum, int ishigh)
{
	if (valvNum == 3) // Sol-Valve 3
	{
		if (ishigh == HIGH)
			// set PB6 high
			PORTB |= (1 << PORTB6);
		else
			// set PB6 low
			PORTB &= ~(1 << PORTB6);
	}
	else if (valvNum == 4)	// Sol-Valve 4
	{
		if (ishigh == HIGH)
			// set PB7 high
			PORTB |= (1 << PORTB7);
		else
			// set PB7 low
			PORTB &= ~(1 << PORTB7);
	}
}

void Delay_And_Check(int delay_Time, int chk_mode)
{
	if (delay_Time > 0)
	{
		for (int i = 0; i < delay_Time; i++)
		{
			switch(chk_mode)
			{
				case 0: // 육수 투입 중.....
					//--20201117 no check....control_RecipeBox (10); // temp.
					break;
				case 1:	// 그냥 대기 중.... ( Water Heater On.....==> check...)
					recipewater_Level_minCount = 7; // 대기중일 때는, 6초 기다리지 않고 바로 투입되게....					
					control_RecipeBox (10); // temp.					
					control_WaterBox (30); // press.
					break;
				case 2:	// 스팀 투입 중....
					control_RecipeBox (10); // temp.				
					//--20201117 no check....control_SteamBox (10); // temp.
					control_WaterBox (30); // press. ++20201117			
					break;				
				case 3:	// ------ None....Motor Down move 중....
					//control_WaterBox (30); // press.				
					//control_SteamBox (10); // temp.
					break;					
				default:
					break;
			}		
			delay(970);
		}				
	}
	else  // Motor Down move 중.....
	{		
		// Motor Down move 중.....All checking....
		control_RecipeBox (10); // temp.		
			
		control_WaterBox (30); // press.	
		control_SteamBox (10); // temp.
		
		//--20201117 delay(delay_Time); // 모터 하강시,  올라간 만큼 내려가지 않음.
		delay(motor_moveloop_delay); // 0 --> 20으로 변경.....
	}		
}

// Water Box Control.....
void control_WaterBox(int addMax_value)
{
	//--no txd digitalWrite(pin_muxCE, LOW);	  
	// check Water Level Sensor.....
	#if defined (__MY_DEBUG_LOG__)
	Serial.print("WaLevel-4= ");
	int waterLevel = readMux_eachSensor (4);
	Serial.println (waterLevel);
	#else
	int waterLevel = readMux_eachSensor (4);
	#endif
	if (waterLevel >= mux_effective_value)
	{
		waterBox_readyOk = false;

	    wwater_Level_maxCount = 0;
		if (wwater_Level_minCount++ > 6)
		{
			wwater_Level_minCount = 8; // no meaning...
			// 물 투입....
			digitalWrite(pinSolvalv_1, HIGH); // Open Sol. Valve....
		}		
		
		//20200915 When Level low, check tempe.
		int ipressValue = analogRead(pinPress_sens);
		if (ipressValue >= (steamBox_Press_Margin_MAX + addMax_value))
		{
			// Heater 멈춤.....
			digitalWrite(pinHeater_1, LOW);  // Off Heater....
		}
	}
	else
	{
		wwater_Level_minCount = 0;
		// 물 투입 정지....
		digitalWrite(pinSolvalv_1, LOW); // Close Sol. Valve....
					
		//if (wwater_Level_maxCount++ > 3)
		//{
			//wwater_Level_maxCount = 5; // no meaning...
			//
			//// 물 투입 정지....
			//digitalWrite(pinSolvalv_1, LOW); // Close Sol. Valve....
		//}
		// check Pressure  Sensor.....
		#if defined (__MY_DEBUG_LOG__)
		Serial.print("Pressure Sensor Value: ");
		int ipressValue = analogRead(pinPress_sens);
		Serial.println(ipressValue);
		#else
		int ipressValue = analogRead(pinPress_sens);
		#endif
		if (ipressValue < steamBox_Press_Margin_MIN)
		{
			waterBox_readyOk = false;
			
			// Heater 가열.....
			digitalWrite(pinHeater_1, HIGH);  // On Heater....
			
			Serial.println("Water Low Pressure(Heater On.!!");
		}
		else
		{
			if (ipressValue > (steamBox_Press_Margin_MAX + addMax_value))
			{
				// Heater 멈춤.....
				digitalWrite(pinHeater_1, LOW);  // Off Heater....
				Serial.println("Water Heater Off !!");
			}
			// Water Box OK.(Ready)....
			waterBox_readyOk = true;
			Serial.println("Water Box Ready Ok!!");
		}
	}
}

// Steam Box Control.....
void control_SteamBox(int addMax_value)
{
	// Check Temp. Sensor......
	float waterTempe = read_TemperatureSensor(1);
	
	//////////// for test only
	read_TemperatureSensor(2);	
	
	if (waterTempe <= steamBox_Tempe_MIN)
	{
		steamBox_readyOk = false;
		// Heater 가열.....
		digitalWrite(pinHeater_2, HIGH);  // On Heater....
	}
	else
	{	
		if (waterTempe >= (steamBox_Tempe_MAX + addMax_value))
		{
			// Heater 멈춤.....
			digitalWrite(pinHeater_2, LOW);  // Off Heater....
		}
		steamBox_readyOk = true;
		Serial.println("Steam Box Ready Ok!!");
	}
}


// Recipe Box Control.....
void control_RecipeBox(int addMax_value)
{
	//--no txd digitalWrite(pin_muxCE, LOW);	  
    // check Water Level Sensor.....
#if defined (__MY_DEBUG_LOG__)                        
    Serial.print("ReLevel-6= ");            
    int recipeLevel = readMux_eachSensor (6);
    Serial.println(recipeLevel);            
#else
    int recipeLevel = readMux_eachSensor (6);
#endif        
    if (recipeLevel >= mux_effective_value)
    {
      recipeBox_readyOk = false;
	  
      // 육수물 투입....
      //SolValve34_Control(3, HIGH); // Open Sol. Valve....      	
	  // 육수 온도 상승시간 단축을 위해,   3초 간격으로  Solvalve Toggle........

	  // 수위센서가  High-Low를 순간적으로 반복해서,  6초이상 Low가 지속될 경우에만  육수 투입.....
	  if (recipewater_Level_minCount++ > 6)
	  {
		  recipewater_Level_minCount = 8; // no meaning...		  
		  switch(recipewater_Input_count)  
		  {
			  case 0:
			  case 1:
				// 육수물 투입....
				SolValve34_Control(3, HIGH); // Open Sol. Valve....
				recipewater_Input_count++;
				break;
			  case 2:
			  case 3:
				// 육수물 투입 정지....
				SolValve34_Control(3, LOW); // Close Sol. Valve....
				recipewater_Input_count++;
				break;	
			  case 4:
			  case 5:
			  case 6:	
			  case 7:				  
				recipewater_Input_count++;
				break;			  
			  case 8:	
			  default:
				recipewater_Input_count = 0;
				break;  	  
		  }	
	  }
		
	  //20200915 When Level LOw,  check tempe.
	  float recipeTempe = read_TemperatureSensor(2);
	   if (recipeTempe >= (recipeBox_Tempe_MAX + addMax_value))
	   {
		   // Heater 멈춤.....
		   digitalWrite(pinHeater_3, LOW);  // Off Heater....
	   }
    }
    else
    {
	  recipewater_Input_count = 0;
	  
	  recipewater_Level_minCount = 0;
	  
	  //// 육수물 투입 정지....
	  SolValve34_Control(3, LOW); // Close Sol. Valve....
				
  
      // Check Temp. Sensor......
      float recipeTempe = read_TemperatureSensor(2);
      if (recipeTempe <= recipeBox_Tempe_MIN)
      {
        recipeBox_readyOk = false;
        // Heater 가열.....
        digitalWrite(pinHeater_3, HIGH);  // On Heater....
      }
      else
      {
		if (recipeTempe < recipeBox_Tempe_MIDD)  
		{
			// Heater 가열..... ( 90 도 이하일 경우,  Heater On....  )
			digitalWrite(pinHeater_3, HIGH);  // On Heater....
		}
		else
		{
			if (recipeTempe > (recipeBox_Tempe_MAX + addMax_value))
			{
			  // Heater 멈춤.....
			  digitalWrite(pinHeater_3, LOW);  // Off Heater....
			  // RE-check Recipe Level Sensor.....
			  //recipeLevel = readMux_eachSensor (6);
			  //if (recipeLevel < mux_effective_value) // Re-Checking......(Confirm!!)
			  //{
			  //}
			}
		}
        // Recipe Box OK.(Ready)....
        recipeBox_readyOk = true;
        Serial.println("Recipe Box Ready Ok!!");		
      }
    }
}

void motor_move(const int step)
{
  int Speed;
  bool fSenseChecked = false;
  
  Serial.print("Motor Move : Motor Error Flag= ");    
  int motorEF = digitalRead(pin_MotorEF); // Check always ??????
  Serial.println(motorEF ? "HIGH" : "LOW");     
  switch(step)
  {
    case 0:  // "BASE" Position init & check... 
      for (Speed = motor_updownmove_Count * 3; Speed >= 1; Speed--)
      {
        if (read_AmbientSensor(pinAmbient_sens1) < ambient_sensor_effective_value) // '하'에 모터가 있으면, Stop.....
          break;
        //motorEF = digitalRead(pin_MotorEF); // Check always ??????
		//if (motorEF == LOW)
		//{
		//	Serial.println("LOW Triggerred. 000.....!!");  
		//	break;
		//}
		
		//Serial.println(motorEF ? "HIGH" : "LOW");     
        digitalWrite(pin_MotorPwm1, HIGH);
        //--200822 analogWrite(pin_MotorPwm1, motor_moving_Speed - 40); // Backward to BASE(하-원점).....
	    analogWrite(pin_MotorPwm1, motor_moving_Speed - 20); // Backward to BASE(하-원점).....
        delay(motor_moveloop_delay);
      }
      digitalWrite(pin_MotorPwm1, HIGH);
      analogWrite(pin_MotorPwm2, 255);    // Stop
      break;
    case 1:  // move to Middle Position (Up-->)
      for (Speed = motor_updownmove_Count; Speed >= 1; Speed--)
      {
        //if (read_AmbientSensor(pinAmbient_sens2) < ambient_sensor_effective_value) // '중'에 모터가 있으면, Stop.....
        //  break;
 
        analogWrite(pin_MotorPwm1, motor_moving_Speed);
        digitalWrite(pin_MotorPwm2, HIGH);  // Forward (Up....)
        delay(motor_moveloop_delay);
      }        
      analogWrite(pin_MotorPwm1, 255);   // Stop
      digitalWrite(pin_MotorPwm2, HIGH);   
      break;
    case 2:  // move to Top Position (Up-->)
      for (Speed = motor_updownmove_Count_to_Top; Speed >= 1; Speed--)
      {
        //if (read_AmbientSensor(pinAmbient_sens3) < ambient_sensor_effective_value) // '상-원점'에 모터가 있으면, Stop.....
        //  break;
	  
        analogWrite(pin_MotorPwm1, motor_moving_Speed);
        digitalWrite(pin_MotorPwm2, HIGH);  // Forward (Up....)
		if ((Speed % 100) == 0)
			Delay_And_Check(0, 3);
		else
			delay(motor_moveloop_delay);
      }        
      analogWrite(pin_MotorPwm1, 255);   // Stop
      digitalWrite(pin_MotorPwm2, HIGH);       
      break;
    case 3:  // move to Middle Position (Down-->) 
      for (Speed = motor_updownmove_Count_to_Top; Speed >= 1; Speed--)
      {
        digitalWrite(pin_MotorPwm1, HIGH);
        analogWrite(pin_MotorPwm2, motor_moving_Speed);  // Backward
        //if (read_AmbientSensor(pinAmbient_sens2) < ambient_sensor_effective_value) // '중'에 모터가 있으면, Stop.....
        //  break;        
		if ((Speed % 100) == 0)
			Delay_And_Check(0, 3);
		else
			delay(motor_moveloop_delay);
      }
      digitalWrite(pin_MotorPwm1, HIGH);
      analogWrite(pin_MotorPwm2, 255);    // Stop
      break;
    case 4:  // move to Base Position (Down-->)
      //for (Speed = motor_updownmove_Count * 3; Speed >= 1; Speed--)
	  for (Speed = motor_updownmove_Count; Speed >= 1; Speed--)
      {
        digitalWrite(pin_MotorPwm1, HIGH);
        analogWrite(pin_MotorPwm2, motor_moving_Speed);  // Backward
        if (read_AmbientSensor(pinAmbient_sens1) < ambient_sensor_effective_value) // '하'에 모터가 있으면, Stop.....
		{
			fSenseChecked = true;
            break; 
		}
		if ((Speed % 100) == 0)
			Delay_And_Check(0, 3);
		else   
			delay(motor_moveloop_delay);
      }
	  if (fSenseChecked == false) //++20201120 : 하강할 때,  처음 위치로 내려오지 않을 경우 있음.....1번만 Try......
	  {
		  delay(100);
		  for (Speed = motor_updownmove_Count_downMore; Speed >= 1; Speed--)
		  {
			  digitalWrite(pin_MotorPwm1, HIGH);
			  analogWrite(pin_MotorPwm2, motor_moving_Speed);  // Backward
			  if (read_AmbientSensor(pinAmbient_sens1) < ambient_sensor_effective_value) // '하'에 모터가 있으면, Stop.....
				  break;
			  delay(motor_moveloop_delay);
		  }		  
	  }
      digitalWrite(pin_MotorPwm1, HIGH);
      analogWrite(pin_MotorPwm2, 255);    // Stop    
      break;                  
  }
}

// Large,middle,small 3 button 조리 시작.....>>>go! go! go!
void lms_start_ButtonPress(int joriMode)
{
	// 조리 시작
	Serial.println("lms_start_ButtonPress--Start---!!");
	
	// 용기상승 :..To..(up)Middle (Step : 1).....
	motor_move(1);

	delay(150);
	
	/////// 육수 다시 투입.....15 ~  sec.
	//육수물 Heater를 일단 무조건 킨다... ==> 그리고,  나중에  control_RecipeBox 함수에서  컨트롤.....
	// Heater 가열.....
	digitalWrite(pinHeater_3, HIGH);  // On Heater....
		
	delay(150);
	
	// Water Heater 가열.....
	digitalWrite(pinHeater_1, HIGH);  // On Heater....	
			
	delay(150);
				
	SolValve34_Control(4, HIGH); // Open Sol.4(조리-육수물) Valve....
	switch(joriMode)
	{
		case large_ButtonPressed_mode:
			Delay_And_Check(17, 0); // 17 sec. ( 650ml add... ) recipe-water
			break;
		case middle_ButtonPressed_mode:
			Delay_And_Check(17, 0);  // 17 sec. ( 650ml add... )
			break;
		case small_ButtonPressed_mode:
			Delay_And_Check(12, 0);  // 12 sec. ( 430ml add... )
			break;
	}
	// 육수 투입 정지:
	SolValve34_Control(4, LOW); // Close Sol.4(조리-육수물) Valve....
	//////////////////////////////	

	// Water Heater 가열.....
	//digitalWrite(pinHeater_1, HIGH);  // On Heater....
	
	Delay_And_Check(5, 1); 
	//200911  10 sec. delay (Stop)
	Delay_And_Check(4, 1);	
	
	// 용기상승 :..To..(up)Top (Step : 2).....
	motor_move(2);
	
	//Steam.... Heater 가열.....
	digitalWrite(pinHeater_2, HIGH);  // On Heater....
	
	delay(1000);	
		
	// 스팀 투입 시작:
	digitalWrite(pinSolvalv_2, HIGH); // Open Sol.2 Valve....
	switch(joriMode)
	{
		case nobutton_Normal_mode:
		case large_ButtonPressed_mode:
			Delay_And_Check(90, 2); // 45 sec. steam ==> 70 sec.(20200915)
			break;
		case middle_ButtonPressed_mode:
			Delay_And_Check(90, 2);  // 45 sec.
			break;
		case small_ButtonPressed_mode:
			Delay_And_Check(90, 2);  // 45 sec.
			break;
	}
	// 스팀 투입 정지:
	digitalWrite(pinSolvalv_2, LOW); // Close Sol.2 Valve....

	delay(150);
	//control_SteamBox();
	
	// 용기하강 :..To..(down)Middle (Step : 3).....
	motor_move(3);
	
	delay(150);

	// 용기하강 :..To..(down)Base (Step : 4).....
	motor_move(4);

	delay(150);

	// All checking ........

	// Check Motor.....
	//motor_move(0);

	Serial.println("lms_start_ButtonPress--End...---!!");
}


// 조리 시작.....>>>go! go! go!
void start_ButtonPress(int joriMode)
{
    // 조리 시작
    Serial.println("start_ButtonPress--Start---!!");
            
    // 용기상승 :..To..(up)Middle (Step : 1).....
    motor_move(1);
	
	//육수물 Heater를 일단 무조건 킨다... ==> 그리고,  나중에  control_RecipeBox 함수에서  컨트롤.....
	// Heater 가열.....
	digitalWrite(pinHeater_3, HIGH);  // On Heater....
		
	delay(150);
	
    // 육수 투입 시작: 
    SolValve34_Control(4, HIGH); // Open Sol.4(조리-육수물) Valve....  
	switch(joriMode)
	{
		case nobutton_Normal_mode:
		case large_ButtonPressed_mode:
		    //--delay(12000); // 12 sec. recipe-water
			Delay_And_Check(19, 0); // 19 sec.(750ml) recipe-water
			break;
		case middle_ButtonPressed_mode:
			Delay_And_Check(8, 0);  // 8 sec.
			break;
		case small_ButtonPressed_mode:
			Delay_And_Check(4, 0);  // 4 sec.
			break;
	}
    // 육수 투입 정지: 
    SolValve34_Control(4, LOW); // Close Sol.4(조리-육수물) Valve....   
    
	delay(150);
		   
	// Water Heater 가열.....
	digitalWrite(pinHeater_1, HIGH);  // On Heater....
		   
	Delay_And_Check(5, 1);
	Delay_And_Check(4, 1);  // 10 sec. Stop.....200911	
		   
    // 용기상승 :..To..(up)Top (Step : 2).....
    motor_move(2);   	

	//Steam.... Heater 가열.....
	digitalWrite(pinHeater_2, HIGH);  // On Heater....
	
	delay(1000);  // 10 sec. Stop.....200911
	    
    // 스팀 투입 시작: 
    digitalWrite(pinSolvalv_2, HIGH); // Open Sol.2 Valve....  
	switch(joriMode)
	{
		case nobutton_Normal_mode:
		case large_ButtonPressed_mode:
			Delay_And_Check(15, 2); // 15 sec. steam
			break;
		case middle_ButtonPressed_mode:
			Delay_And_Check(15, 2);  // 15 sec.
			break;
		case small_ButtonPressed_mode:
			Delay_And_Check(10, 2);  // 10 sec.
			break;
	}
    // 스팀 투입 정지: 
    digitalWrite(pinSolvalv_2, LOW); // Close Sol.2 Valve....            

	delay(150);
	//control_SteamBox();
	
    // 용기하강 :..To..(down)Middle (Step : 3).....
    motor_move(3);   
    
    delay(150);

    // 용기하강 :..To..(down)Base (Step : 4).....
    motor_move(4);         

    delay(150);

    // All checking ........

    // Check Motor.....
    //motor_move(0);

    Serial.println("start_ButtonPress--End...---!!");
}

// Recipe-Water Input Button.....>>>
void water_ButtonPress()
{
    // Input "Recipe-water"......
    Serial.println("water_ButtonPress-----!!");    

	// 용기상승 :..To..(up)Middle (Step : 1).....
	motor_move(1);
	
	//육수물 Heater를 일단 무조건 킨다... ==> 그리고,  나중에  control_RecipeBox 함수에서  컨트롤.....
	// Heater 가열.....
	digitalWrite(pinHeater_3, HIGH);  // On Heater....	   
	
	delay(150);
		
	// 육수 투입 시작:
	SolValve34_Control(4, HIGH); // Open Sol.4(조리-육수물) Valve....

	Delay_And_Check(2, 0);	// 2 sec. ( 30ml add... )
	// 육수 투입 정지:
	SolValve34_Control(4, LOW); // Close Sol.4(조리-육수물) Valve....
	
    // 용기하강 :..To..(down)Base (Step : 4).....
    motor_move(4);	
    delay(150);

    // Check Motor.....
    //motor_move(0);	
}

// Steam Input Button.....>>>
void steam_ButtonPress()
{
    // Input "Steam"......
    Serial.println("steam_ButtonPress-----!!");   
	     
    // 용기상승 :..To..(up)Middle (Step : 1).....
    motor_move(1);
    delay(150);
    
    // 용기상승 :..To..(up)Top (Step : 2).....
    motor_move(2);
    
    delay(150);
	
	// Water Heater 가열.....
	digitalWrite(pinHeater_1, HIGH);  // On Heater....
	
    delay(150);
		
	//Steam.... Heater 가열.....
	digitalWrite(pinHeater_2, HIGH);  // On Heater....	
		
    delay(150);
			
    // 스팀 투입 시작:
    digitalWrite(pinSolvalv_2, HIGH); // Open Sol.2 Valve....

	Delay_And_Check(15, 2);	// 15 sec.
	
    // 스팀 투입 정지:
    digitalWrite(pinSolvalv_2, LOW); // Close Sol.2 Valve....

    delay(150);
		
    // 용기하강 :..To..(down)Middle (Step : 3).....
    motor_move(3);    
    delay(150);

    // 용기하강 :..To..(down)Base (Step : 4).....
    motor_move(4);
    delay(150);

    // Check Motor.....
    //motor_move(0);
}

/********* 8 MHz*************/
void setup() {
  
  // put your setup code here, to run once:
  Serial.begin(115200);

  // LED pin define(0 pin)
  pinMode(pinOp_LED, OUTPUT);       digitalWrite(pinOp_LED, HIGH);  //  Operator LED....(R e d)...

  // Mux. Pin Init....(4, 3, 2)
  pinMode (pin_muxAddress_A, OUTPUT); 
  pinMode (pin_muxAddress_B, OUTPUT); 
  pinMode (pin_muxAddress_C, OUTPUT); 

  // Sol. Valve 1,2,3,4 (8,9, 14, 15) - Relay
  pinMode (pinSolvalv_1, OUTPUT); digitalWrite(pinSolvalv_1, LOW);  // Close Sol. Valve....
  pinMode (pinSolvalv_2, OUTPUT); digitalWrite(pinSolvalv_2, LOW);  //

  // set as outputs
  //(XTAL1 & XTAL2) => PORTB
  DDRB |= (1 << DDB6) | (1 << DDB7);

  SolValve34_Control(3, LOW); 
  SolValve34_Control(4, LOW);

  //pinMode (pinSolvalv_3, OUTPUT); digitalWrite(pinSolvalv_3, LOW);  //
  //pinMode (pinSolvalv_4, OUTPUT); digitalWrite(pinSolvalv_4, LOW);  //

  // Heater 1,2,3 (A4, A3, A2) - Relay
  pinMode (pinHeater_1, OUTPUT); digitalWrite(pinHeater_1, LOW);  // Off Heater....
  pinMode (pinHeater_2, OUTPUT); digitalWrite(pinHeater_2, LOW);
  pinMode (pinHeater_3, OUTPUT); digitalWrite(pinHeater_3, LOW);

  // Pressure Sensor ( Steam Box , A6) ....
  pinMode(pinPress_sens, INPUT);  

  // Motor Control with EF + PWM. ( 7, + 6, 5 )......
  pinMode(pin_MotorEF, INPUT);  // Motordriver EF (Error Flag - Open Drain)
  
  pinMode(pin_MotorPwm1, OUTPUT);  // Motordriver IN1
  pinMode(pin_MotorPwm2, OUTPUT);  // Motordriver IN2
  digitalWrite(pin_MotorPwm1, LOW);
  digitalWrite(pin_MotorPwm2, LOW);


  // Ambient(Proximity) Sensor(A0, A7, A1 : 용기 확인)
  pinMode(pinAmbient_sens1, INPUT);  
  pinMode(pinAmbient_sens2, INPUT);  
  pinMode(pinAmbient_sens3, INPUT);  


  // Motor Error Flag 확인 ??
  int motorEF = digitalRead(pin_MotorEF);
  if (motorEF == 0)
    Serial.println("Motor EF Error!! (LOW)??"); 
  else
    Serial.println("Motor EF No-Error (HIGH)-----!!"); 
  // Motor 위치 확인
  motor_move(0); // move Base....

  if ((steamBox_readyOk == true) && (recipeBox_readyOk == true))
    digitalWrite(pinOp_LED, HIGH);  //  Operator LED....(G r e e n)...
  else
    digitalWrite(pinOp_LED, LOW);  //  Operator LED....(G r e e n)...    
  led_toggle_Count	= 0;
  
  //육수물 Heater를 일단 무조건 킨다... ==> 그리고,  나중에  control_RecipeBox 함수에서  컨트롤.....
  // Heater 가열.....
  digitalWrite(pinHeater_3, HIGH);  // On Heater....	
  
  recipewater_Level_minCount = 0;  
  wwater_Level_minCount = 0; 
}

void loop()
{
	g_byteMuxValue = 0;
	g_byteMuxValue = readMux_SwChannel();
	Serial.print("g_byteMuxValue1 : "); Serial.println(g_byteMuxValue); 
   
	//Serial.print("Pressure Sensor Value: ");
	//int ipressValue = analogRead(pinPress_sens);
	//Serial.println(ipressValue);
	  
	switch(g_byteMuxValue)
	{
	case 1: // Start Button Pressed.....
		Serial.println("Start Button Pressed!!"); 	
		if ((steamBox_readyOk == true) && (recipeBox_readyOk == true)) // Only If All ready.........
		{
			if (start_btn_pressed == false)
				start_btn_pressed = true;
			else
			{	
				start_btn_pressed = false;
				Serial.println("Start Button Real ------- Continue Pressed !!"); 
				jori_Mode = nobutton_Normal_mode;
				//--skip for error  start_ButtonPress(jori_Mode);
			}
							
			/*--200911 if (read_AmbientSensor(pinAmbient_sens1) < ambient_sensor_effective_value) // '하'에 모터가 있으면, Starting.....
			{
				jori_Mode = nobutton_Normal_mode;
				start_ButtonPress(jori_Mode);
			}
			else
			{
				// Motor 위치 확인
				motor_move(0); // move Base....
			}  */
		}
		jori_Mode = nobutton_Normal_mode;  
		break;
	case 2: // Water-Recipe Button Pressed.....
		start_btn_pressed = false;
		if (recipeBox_readyOk == true)
			water_ButtonPress();
		jori_Mode = nobutton_Normal_mode;  
		break;
	case 4: // Steam Button Pressed.....  
		start_btn_pressed = false;
		if (steamBox_readyOk == true)
			steam_ButtonPress();
		jori_Mode = nobutton_Normal_mode;  	  	  
		break;    
	case 8: // Large Button Pressed........
		start_btn_pressed = false;
		Serial.println("Large Button Pressed!!"); 
		jori_Mode = large_ButtonPressed_mode;
		//if (g_byteMuxValue & B00100000) //large + small ==> All test mode --> '1'
		//	function_AlltestMode(1);
		//else if(g_byteMuxValue & B10000000) //large + middle ==> All test mode --> '0'
		//	function_AlltestMode(0);
		//else
		//{
			if ((steamBox_readyOk == true) && (recipeBox_readyOk == true)) // Only If All ready.........
				lms_start_ButtonPress(jori_Mode);
		//}
		break;				
	case 32: // Small Button Pressed........
		start_btn_pressed = false;
		Serial.println("Small-Button Pressed!!");
		jori_Mode = small_ButtonPressed_mode;
		//if (g_byteMuxValue & B00001000) //small + large ==> All test mode --> '1'
		//	function_AlltestMode(1);
		//else if (g_byteMuxValue & B10000000) // small + middle ==> All test mode --> '2'
		//	function_AlltestMode(2);		
		//else
		//{
			if ((steamBox_readyOk == true) && (recipeBox_readyOk == true)) // Only If All ready.........
				lms_start_ButtonPress(jori_Mode);
		//}
		break;		
	case 128: // Medium Button Pressed........
		start_btn_pressed = false;
		jori_Mode = middle_ButtonPressed_mode;
		Serial.println("Medium Button Pressed!!");
		//if (g_byteMuxValue & B00100000) //middle + small ==> All test mode --> '2'
		//	function_AlltestMode(2);
		//else if (g_byteMuxValue & B00001000) // middle + large ==> All test mode --> '0'
		//	function_AlltestMode(0);		
		//else
		//{
			if ((steamBox_readyOk == true) && (recipeBox_readyOk == true)) // Only If All ready.........
				lms_start_ButtonPress(jori_Mode);			
		//}
		break;		
	case 160: // middle + samll.........
		//function_AlltestMode(2);	
		break;				
	case 136: // large + middle.........
		 // 육수 투입 정지:
		 SolValve34_Control(4, LOW); // Close Sol.4(조리-육수물) Valve....
		//function_AlltestMode(0);	
		break;
	case 40: // large + small.........
		//--function_AlltestMode(1);
		SolValve34_Control(4, HIGH); // Open Sol.4(조리-육수물) Valve....		
		break; 
	case 0: // 
	default: // "-1" - No Pressed....& Normal...State......
		start_btn_pressed = false;	  
		break;
	}
	
	recipewater_Input_count = 0;
	
	control_WaterBox(0);
	control_RecipeBox(0);
	control_SteamBox(0);	
	
	if ((steamBox_readyOk == true) && (recipeBox_readyOk == true)) // Only If All ready.........
	{
		digitalWrite(pinOp_LED, HIGH);  //  Operator LED....(G r e e n)...
		led_Mode = false;
		led_toggle_Count = 0;
	}
	else
	{
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
	}    
	delay(g_iDelayTime);  // RealTime..(Urgent!!).......    
}

void function_AlltestMode(int mode)
{
/*	  int testInterval = 3000;
	  
	  Serial.println("function_AlltestMode!!");	
	  delay(testInterval);
	  
	  Serial.println("Led Off....!!");
	  digitalWrite(pinOp_LED, LOW);
	  
	  readMux_AllChannel();
	  
	  read_AmbientSensor(pinAmbient_sens1);
	  read_AmbientSensor(pinAmbient_sens2);
	  read_AmbientSensor(pinAmbient_sens3);
	  
	  read_TemperatureSensor(1);
	  read_TemperatureSensor(2);
	  
	  Serial.print("Pressure Sensor Value: ");
	  int ipressValue = analogRead(pinPress_sens);
	  Serial.println(ipressValue);
	  
	  Serial.println("Led On....!!");
	  digitalWrite(pinOp_LED, HIGH);

	  Serial.println("Heater 1,2,3 test!!");	
	  Serial.println("Heater 1(PC4(ADC4) -- Water Box) test!!");	    
	  digitalWrite(pinHeater_1, HIGH);  delay(testInterval); digitalWrite(pinHeater_1, LOW);  // Off Heater....
	  
	  Serial.println("Heater 2(PC3(ADC3) -- Steam Box) test!!");		  
	  digitalWrite(pinHeater_2, HIGH);  delay(testInterval); digitalWrite(pinHeater_2, LOW);  // Off Heater...
	  
	  Serial.println("Heater 3(PC2(ADC2) -- Recipe Box) test!!");	
	  digitalWrite(pinHeater_3, HIGH);  delay(testInterval); digitalWrite(pinHeater_3, LOW);  // Off Heater....
	  
	  Serial.println("SolValve 1,2,3,4 test!!");	  
	  Serial.println("SolValve 1 (8, PB0 -- Water Input) test!!");		  
	  // 물 투입....
	  digitalWrite(pinSolvalv_1, HIGH); delay(testInterval);
	  // 물 투입 정지....
	  digitalWrite(pinSolvalv_1, LOW); // Close Sol. Valve....
	  
	  Serial.println("SolValve 2 (9, PB1 -- Steam Input) test!!");	
	  // 스팀 투입 시작:
	  digitalWrite(pinSolvalv_2, HIGH); delay(testInterval);
	  // 스팀 투입 정지:
	  digitalWrite(pinSolvalv_2, LOW); // Close Sol.2 Valve....
	  
	  Serial.println("SolValve 3 (14, PB6 -- Recipe Water Input-육수보충) test!!");		  
	  // 육수물 보충...
	  SolValve34_Control(3, HIGH); 
	  delay(testInterval);
	  // 육수물 보충 정지....
	  SolValve34_Control(3, LOW); // Close Sol. Valve....
	  
	  Serial.println("SolValve 4 (15, PB7 -- Recipe-Water Input(조리) test!!");			  
	  // 육수 투입 시작:
	  SolValve34_Control(4, HIGH); // Open Sol.4(조리-육수물) Valve....
	  delay(testInterval);
	  // 육수 투입 정지:
	  SolValve34_Control(4, LOW); // Close Sol.4(조리-육수물) Valve....
	  
	  Serial.println("Led Off....!!");	  
	  digitalWrite(pinOp_LED, LOW);
	  
	  if (mode == 1)
	  {
		  Serial.println("motor 0---- Base..Positioning....!!");
		  motor_move(0);	  delay(testInterval);		  
	  }
	  else if (mode == 2)
	  {		  
		  Serial.println("motor 1,2,3,4, test!!");	 
		  //Serial.println("motor Base..Positioning....!!");	 	  
		  //motor_move(0);	  delay(testInterval); 
	  
		  Serial.println("motor base-to-middle(up) test!!");	  
		  motor_move(1);	  delay(testInterval); 
	  
		  Serial.println("motor middle-to-top(up) test!!");	  
		  motor_move(2);	  delay(testInterval); 
	  
		  Serial.println("motor Top-to-middle(down) test!!");	  
		  motor_move(3);	  delay(testInterval); 
	  
		  Serial.println("motor middle-to-base(down) test!!");	  
		  motor_move(4);	  
	  }
	  
	  Serial.println("Led On....!!");	
	  digitalWrite(pinOp_LED, HIGH); */
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


/*********************
If you’re running an ATmega328 without an external oscillator, you might want to use the crystal pins (XTAL1 & XTAL2).

These pins are also known as PB6 and PB7. They are not supported by the Arduino IDE but you can control them with direct port manipulation on port B.

void setup() {
	// set as outputs
	DDRB |= (1 << DDB6) | (1 << DDB7);
}

void loop() {
	// set PB6 high
	PORTB |= (1 << PORTB6);
	delay(1000);
	// set PB6 low
	PORTB &= ~(1 << PORTB6);
	delay(1000);
	
	// both high
	PORTB |= (1 << PORTB6) | (1 << PORTB7);
	delay(1000);
	// both low
	PORTB &= ~((1 << PORTB6) | (1 << PORTB7));
	delay(1000);
}
*********************/