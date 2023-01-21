/*
 * Sketch11.ino
 *
 * Created: 8/14/2022 8:43:20 PM
 * Author: dlcks
 */ 
//#define __MY_DEBUG_LOOP__
#define __MY_DEBUG_LOG__

#include "max6675.h"
#include "steamgen1.h"
#include <arduino.h>
#include "Timer.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "hmi_driver.h"
#include <EEPROM.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#define FIRMWARE_VER 921
#define CRC16_ENABLE 0
#define QUEUE_MAX_SIZE 256

#define SD_FILE_EN 0

#define DEBUG_MSG
#define hmi_baudrate 9600
#define sys_baudrate 19200

int g_iDelayTime = 1000;
int g_iswPressed = -1;
byte g_byteMuxValue = 0;

// ����� ���� ��
volatile int Recipe_Set_Tempe =	SettingValue[0][0];
volatile int Press_Set_TankTempe = SettingValue[0][1]; // ���� ��ũ
volatile int Press_Set_OccurTempe = SettingValue[0][2]; // ����
volatile int Recipe_Set_Input = SettingValue[0][3];
volatile int Steam_Set_Input = SettingValue[0][4];
volatile int Delay_Set_Time = SettingValue[0][5];
volatile int Motor_upTime = SettingValue[0][6];
volatile int Delay_Set_Time2 = SettingValue[0][7];

// Start,Stop,Reset ���� ���� flag ����ؼ� �����ϱ�
volatile int Stop_tmp = 0;
volatile int Stop_moter = 0;
volatile int Stop_Stream = 0;
volatile int Stop_Recipe = 0;
int Reset_tmp = 0;
volatile int Start_tmp = 0;

uint8 water_first = 0;
uint8 steam_first = 0;
uint8 recipe_first = 0;

uint8 pump_tmp = 0;
uint8 water_level_tmp = 0;
volatile int Tempe_tmp = 0;
volatile int Tempe_tmp1 = 0;
int ready_tmp = 0;
int lcd_tmp = 0;

// LCD ���� value
unsigned char  cmd_buffer[CMD_MAX_SIZE];
unsigned int timer_tick_last_update = 0;
qsize  size = 0;
unsigned char ch;

uint8_t EEPROMtmp = 0;
 
// ���� ȭ�� ID
static unsigned int current_screen_id = 0;

// ȭ�� ������Ʈ
static unsigned char update_en = 0;

// ���� Ÿ�̸� ī��Ʈ
byte leds = 0;
volatile unsigned int  timer_tick_count = 0;
Timer ts;

MAX6675 thermocouple (pinTemps_SCK, pinTempsCS_1, pinTemps_SO); // steam
MAX6675 thermocouple2(pinTemps_SCK, pinTempsCS_2, pinTemps_SO); // recipe
MAX6675 thermocouple3(pinTemps_SCK, pinTempsCS_3, pinTemps_SO); // water

int pressuretempe = thermocouple.readCelsius(); // 
int steamtempe = thermocouple3.readCelsius();
int rmtempe = thermocouple2.readCelsius();

void NOTIFYHandShake(void)
{
}
void ProcessMessage( PCTRL_MSG msg, unsigned int size )
{
	unsigned char cmd_type = msg->cmd_type;  // ��ɾ� Ÿ��
	unsigned char ctrl_msg = msg->ctrl_msg;  // ��� �޼���
	unsigned char control_type = msg->control_type;    // ������ ����
	unsigned int screen_id = PTR2U16(&msg->screen_id);    // ��ũ��(ȭ��) ID
	unsigned int control_id = PTR2U16(&msg->control_id);  // ��Ʈ��(��ư, �����̴�, ���... ) ID
	unsigned long value = PTR2U32(msg->param);     // ���� ������

	//debug_msg("ProcessMessage\r\n");

	switch(cmd_type)
	{
		case NOTIFY_TOUCH_PRESS:    // ��ġ ����
		case NOTIFY_TOUCH_RELEASE:  // ��ġ ���ȴٰ� ������
		// NotifyTouchXY(cmd_buffer[1],PTR2U16(cmd_buffer+2),PTR2U16(cmd_buffer+4));
		break;
		case NOTIFY_CONTROL:
		{
			if(ctrl_msg==MSG_GET_CURRENT_SCREEN)  // ȭ�麯�� �˸�
			{
				NotifyScreen(screen_id);  // ����� ȭ������ ��ȯ
			}
			else
			{
				switch(control_type)
				{
					case kCtrlButton:  // ��ư ��Ʈ��
					NotifyButton(screen_id,control_id,msg->param[1]);
					break;
					default:
					break;
				}
			}
			break;
		}
		case NOTIFY_HandShake:  // Handshake
		NOTIFYHandShake();
		break;
		// 		case kCtrlText:                                                         //�ı��ؼ�
		// 		NotifyText(screen_id,control_id,msg->param);
		break;
		default:
		break;
	}
}

void LCD_with_MCU()
{
	if (Serial.available())
	{
		ch = Serial.read();
		queue_push(ch);
	}
	size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE);
	// ���ŵ� ��ɾ����� �Ǵ�
	if(size>0&&cmd_buffer[1]!=0x07)
	{// ��ɾ� ó��
		ProcessMessage((PCTRL_MSG)cmd_buffer, size);
	}
	else if(size>0&&cmd_buffer[1]==0x07)
	{// �ý��� ���� ��ɾ�
		Serial.println("System Reset");
	}
	// ts.update();

	//delay(10);
}

int read_AmbientSensor(int pinNum) // ���� ���� �б�
{
	int ambientValue;
	ambientValue = analogRead(pinNum);
	return ambientValue;
}

void Mux_select_Do(int which,bool Do)
{
	if(Do) bitSet(leds,which);
	else bitClear(leds,which);
	
	digitalWrite(pin_Latch,LOW);
	shiftOut(pin_Data,pin_Clock,LSBFIRST,leds);
	digitalWrite(pin_Latch,HIGH);
}

void  SolValve_Control(int valvNum, int ishigh)
{
	if (valvNum == 1) // Sol-Valve 1  QB
	{
		if (ishigh == HIGH)
			{
				Mux_select_Do(6,1);
			}
		else
			{
				Mux_select_Do(6,0);
			}
	}
	else if (valvNum == 2)	// Sol-Valve 2 QF
	{
		if (ishigh == HIGH)
			{
				Mux_select_Do(2,1);
			}
		else
			{
				Mux_select_Do(2,0);
			}
	}
	else if (valvNum == 3)	// Sol-Valve 3  QC
	{
		if (ishigh == HIGH)
			{
				Mux_select_Do(5,1);
			}
		else
			{
				Mux_select_Do(5,0);
			}
	}
	else if (valvNum == 4)	// Sol-Valve 4 QE
	{
		if (ishigh == HIGH)
			{
				Mux_select_Do(3,1);
			}
		else
			{
				Mux_select_Do(3,0);
			}
	}
}

void  Heater_Control(int HeaterNum, int ishigh) //heater 4 == ����
{
	if ( HeaterNum == 1) // Heater 1  QH
	{
		if (ishigh == HIGH)
		Mux_select_Do(0,1);
		else
		Mux_select_Do(0,0);
	}
	else if ( HeaterNum == 2)	// Heater 2 QG
	{
		if (ishigh == HIGH)
		Mux_select_Do(1,1);
		else
		Mux_select_Do(1,0);
	}
	else if ( HeaterNum == 3)	// Heater 3 QA
	{
		if (ishigh == HIGH)
		Mux_select_Do(7,1);
		else
		Mux_select_Do(7,0);
	}
	else if ( HeaterNum == 4)	// Heater 4 QD
	{
		if (ishigh == HIGH)
		Mux_select_Do(4,1);
		else
		Mux_select_Do(4,0);
	}
}

void Delay_And_Check(int delay_Time, int chk_mode) //
{
	if(Reset_tmp) return;
	byte byteMuxValue = 0;
	bool fLoopBreak = false;
	int  steamBtnPress_Count = 0;
	
	while(Stop_tmp)
	{
		if(Reset_tmp) return;
		LCD_with_MCU();
	}
	if(Reset_tmp) return; // stop ����
	
	if (delay_Time > 0)
	{
		for (int i = 0; i < delay_Time; i++)
		{
			switch(chk_mode)
			{
				case 0: 
					LCD_with_MCU();
					while(Stop_tmp)
					{
						if(Reset_tmp) return;
						LCD_with_MCU();
					}
					if(Reset_tmp) return;
					Tempe_Read();
					control_Boxs();
				break;
				case 1:	// �׳� ��� ��.... ( Water Heater On.....==> check...)
					LCD_with_MCU();
					while(Stop_tmp)
					{
						if(Reset_tmp) return;
						LCD_with_MCU();
					}
					if(Reset_tmp) return;
					Tempe_Read();
					control_Boxs();
				break;
				case 2:	// ���� ���� ��....
					LCD_with_MCU();
					while(Stop_tmp)
					{
						if(Reset_tmp) return;
						LCD_with_MCU();
					}
					if(Reset_tmp) return;
					if(Stop_Stream)
					{
						Stop_Stream = 0;
						SolValve_Control(2,HIGH);
					}
					Tempe_Read();
					control_Boxs();
				break;
			}
			delay(495); // 
			//if ((chk_mode == 7) && (fLoopBreak == true)) // for Loop ����..... : Only  [Steam �߰�] ���....
			//break;
		}
	}
	else  // Motor Down move ��.....
	{
		Tempe_Read();
		control_Boxs();
		delay(motor_moveloop_delay); // 0 --> 20���� ����.....
	}
}
void control_Boxs()
{
	control_RecipeBox(0);
	control_SteamBox(0);
	control_WaterBox(0);
	return;
}
// Water Box Control.....
void control_WaterBox(int addMax_value) // heater 1 /press set occur/ steam
{
	//--no txd digitalWrite(pin_muxCE, LOW);
	// check Water Level Sensor.....
		int ipressValue = analogRead(pinPress_sens); // ���� �Ƴ��α� ����
		
		if (ipressValue == 0) // High(1024) = �з��� ���������� �Ʒ� low(0) �з��� ���������� ����
		{
		     Heater_Control(1,LOW);
		}
		else
		{
		 	if(steamtempe > Press_Set_OccurTempe)
		 		{
		 			Heater_Control(1,LOW);	
		 		}
		 	else if(steamtempe <= Press_Set_OccurTempe)
		 		{
		 			Heater_Control(1,HIGH);
				}
				
			if(steamtempe > Press_Set_OccurTempe - 3) waterBox_readyOk = true;
			else if(steamtempe <= Press_Set_OccurTempe - 3)waterBox_readyOk = false;
		}
					
		int waterLevel_min = analogRead(pin_WaterLevelSensor_1);
		int waterLevel_max = analogRead(pin_WaterLevelSensor_1_2);
		
		if(water_level_tmp == 0)
		{
				if (waterLevel_min >= mux_effective_value && waterLevel_max >= mux_effective_value ) // ����1-1 && ���� 1-2 off
				{
					SolValve_Control(1,HIGH);
					Heater_Control(4,HIGH); // ����
					pump_tmp = 1;
				}
				else water_level_tmp = 1;
		}
		else
		{
			if(waterLevel_max < mux_effective_value)//���� 1-2 low -> detact
			{
				if(pump_tmp == 3) Heater_Control(4,HIGH); // ����
				else if(pump_tmp == 2 || pump_tmp == 4) Heater_Control(4,LOW); // ����
				SolValve_Control(1,LOW);
				pump_tmp = 2;
			}
			if(waterLevel_min < mux_effective_value) //���� 1-1 low -> detact
			{
				if(pump_tmp == 3) Heater_Control(4,HIGH); // ����
				else if(pump_tmp == 2 || pump_tmp == 4) Heater_Control(4,LOW); // ����
				SolValve_Control(1,HIGH);
				pump_tmp = 1;
			}
		}
		
		
// 		if(water_first)
// 		{
// 			
// 			if (ipressValue == 0) // High(1024) = �з��� ���������� �Ʒ� low(0) �з��� ���������� ����
// 			{
// 				// Heater ����.....
// 				Heater_Control(1,LOW);
// 			}
// 			else
// 			{
// 				if(steamtempe >= Press_Set_OccurTempe)
// 				{
// 					waterBox_readyOk = true;
// 					Heater_Control(1,LOW);
// 					water_first = 2;
// 				}
// 				else if(steamtempe < Press_Set_OccurTempe)
// 				{
// 					waterBox_readyOk = false;
// 					if(water_first == 2)Heater_Control(1,HIGH);
// 				}
// 			}
// 		}
// 		else
// 		{
// 			if (ipressValue == 0) // High(1024) = �з��� ���������� �Ʒ� low(0) �з��� ���������� ����
// 			{
// 				// Heater ����.....
// 				Heater_Control(1,LOW);
// 			}
// 			else
// 			{
// 				if(steamtempe >= Press_Set_OccurTempe-4)
// 				{
// 					waterBox_readyOk = true;
// 					Heater_Control(1,LOW);
// 					water_first = 1;
// 				}
// 				else if(steamtempe < Press_Set_OccurTempe)
// 				{
// 					waterBox_readyOk = false;
// 					Heater_Control(1,HIGH);
// 				}
// 			}
// 			
// 		}


}

// Steam Box Control.....
void control_SteamBox(int addMax_value) // heater 2 /press set tank / water
{
	// Check Temp. Sensor......	
	if (pressuretempe <= Press_Set_TankTempe) // steamBox_Tempe_MIN
	{
		// Heater ����.....
		Heater_Control(2,HIGH);
		//digitalWrite(pinHeater_2, HIGH);  // On Heater....22 08 19
	}
	else
	{
		if (pressuretempe > ( Press_Set_TankTempe )) // steamBox_Tempe_MAX + addMax_value
		{
			// Heater ����.....
			Heater_Control(2,LOW);
			//digitalWrite(pinHeater_2, LOW);  // Off Heater....
		}
	}
	
	if(pressuretempe <= Press_Set_TankTempe - 3) steamBox_readyOk = false;
	else if(pressuretempe > Press_Set_TankTempe- 3) steamBox_readyOk = true;
}
// Recipe Box Control.....
void control_RecipeBox(int addMax_value) //
{
	int recipeLevel = analogRead(pin_WaterLevelSensor_2);
	
 	if (recipeLevel >= mux_effective_value) // ON
	 {
		recipeBox_readyOk = false;
		Heater_Control(4,HIGH); // ����
		SolValve_Control(3,HIGH);
		pump_tmp = 3;
	 }
	 
	 if(recipeLevel < mux_effective_value) // OFF
	 {
		 if(pump_tmp == 1) Heater_Control(4,HIGH); // ����
		 else if(pump_tmp == 2 || pump_tmp == 4) Heater_Control(4,LOW); // ����
		 SolValve_Control(3,LOW);
		 pump_tmp = 4;
	 }
 
// 	 	// Check Temp. Sensor......
		 if (rmtempe <= Recipe_Set_Tempe)
		 {
		 	// Heater ����.....
		 	Heater_Control(3,HIGH);
		 }
		 else
		 {
		 	if (rmtempe > (Recipe_Set_Tempe))
		 	{
			 	// Heater ����.....
			 	Heater_Control(3,LOW);
	 		}
		 }
		
		if(rmtempe > Recipe_Set_Tempe - 3) recipeBox_readyOk = true;
		else if(rmtempe <= Recipe_Set_Tempe -3) recipeBox_readyOk = false;
		 
		 
		// ������ ����....
		//SolValve34_Control(3, HIGH); // Open Sol. Valve....
		// ���� �µ� ��½ð� ������ ����,   3�� ��������  Solvalve Toggle........

		// ����������  High-Low�� ���������� �ݺ��ؼ�,  6���̻� Low�� ���ӵ� ��쿡��  ���� ����.....
// 		if (recipewater_Level_minCount++ > 6000)
// 		{
// 			recipewater_Level_minCount = 6001; // no meaning...
// 			SolValve_Control(3,HIGH);
// // 			switch(recipewater_Input_count)
// // 			{
// // 				case 0:
// // 				case 1:
// // 				// ������ ����....
// // 				SolValve_Control(3, HIGH); // Open Sol. Valve....
// // 				recipewater_Input_count++;
// // 				break;
// // 				case 2:
// // 				case 3:
// // 				// ������ ���� ����....
// // 				SolValve_Control(3, LOW); // Close Sol. Valve....
// // 				recipewater_Input_count++;
// // 				break;
// // 				case 4:
// // 				case 5:
// // 				case 6:
// // 				case 7:
// // 				recipewater_Input_count++;
// // 				break;
// // 				case 8:
// // 				default:
// // 				recipewater_Input_count = 0;
// // 				break;
// // 			}
// 		}
		
		//20200915 When Level LOw,  check tempe.
//  		float recipeTempe = thermocouple2.readCelsius();
// 		if (recipeTempe >= ( Recipe_Set_Tempe + 5))
// 		{
// 			// Heater ����.....
// 			Heater_Control(3,LOW);
// 			//digitalWrite(pinHeater_3, LOW);  // Off Heater.... 22 08 19
// 		}
// 		else Heater_Control(3,HIGH); 
// 	}
		 return; 
}
void reset_seq()
{
	analogWrite(pin_MotorEF, HIGH); // ���� OFF
	analogWrite(pin_MotorPwm2,LOW);
	SolValve_Control(4, LOW); // ���� OFF
	SolValve_Control(2, LOW); // ���� OFF
	delay(100);
	return;
}
void motor_move(const int step) // ��� �ø��� ���� �������� ���� �ӵ��� 23�� base -> top // 220820
{
	//digitalWrite(pin_MotorPwm1,HIGH); // LOW = �ð����
	//analogWrite(pin_MotorPwm2,controlpwm[i]);
	int Speed;
	bool fSenseChecked = false;

	motor_updownmove_Count = 27 * Motor_upTime;
	if(motor_updownmove_Count > 600) motor_updownmove_Count = 600;
	motor_updownmove_Count_to_Top = 600 - motor_updownmove_Count; // 1200 = ���� ��ü �ð�
	
	switch(step)
	{
		case 0:  // "BASE" Position init & check...
		for (Speed = motor_updownmove_Count * 3; Speed >= 1; Speed--)
		{
			Tempe_Read();
			control_Boxs();
			LCD_with_MCU();
			while(Stop_tmp)
			{
				if(Reset_tmp) return;
				LCD_with_MCU();
			}
			if(Reset_tmp) return;
			if (read_AmbientSensor(pinAmbient_sens1) < ambient_sensor_effective_value) // '��'�� ���Ͱ� ������, Stop.....
 			break;
			//Serial.println(motorEF ? "HIGH" : "LOW");
			digitalWrite(pin_MotorEF,LOW);
			digitalWrite(pin_MotorPwm1, LOW); // ��������
			//--200822 analogWrite(pin_MotorPwm1, motor_moving_Speed - 40); // Backward to BASE(��-����).....
			analogWrite(pin_MotorPwm2, motor_moving_Speed - 20); // Backward to BASE(��-����).....
			delay(motor_moveloop_delay);
		}
		//digitalWrite(pin_MotorPwm1, HIGH);
		digitalWrite(pin_MotorEF,HIGH);    // Stop
		break;
		case 1:  // move to Middle Position (Up-->)
		for (Speed = motor_updownmove_Count; Speed >= 1; Speed--)
		{
			Tempe_Read();
			control_Boxs();
			if (read_AmbientSensor(pinAmbient_sens2) < ambient_sensor_effective_value) // '��'�� ���Ͱ� ������, Stop.....
			  break;
			LCD_with_MCU();
			while(Stop_tmp)
			{
				if(Reset_tmp) return;
				LCD_with_MCU();
			}
			if(Reset_tmp) return;
			digitalWrite(pin_MotorEF,LOW);    // Forward (Up....)
			digitalWrite(pin_MotorPwm1, HIGH);  // Forward (Up....)
			analogWrite(pin_MotorPwm2, motor_moving_Speed);
			delay(motor_moveloop_delay);
		}
		Tempe_Read();
		control_Boxs();
		//digitalWrite(pin_MotorPwm1, LOW);
		digitalWrite(pin_MotorEF,HIGH);    // Stop
		break;
		case 2:  // move to Top Position (Up-->)
		for (Speed = motor_updownmove_Count_to_Top; Speed >= 1; Speed--)
		{
			Tempe_Read();
			control_Boxs();
			LCD_with_MCU();
			while(Stop_tmp)
			{
				if(Reset_tmp) return;
				LCD_with_MCU();
			}
			if(Reset_tmp) return;
			if (read_AmbientSensor(pinAmbient_sens3) < ambient_sensor_effective_value) // '��-����'�� ���Ͱ� ������, Stop.....
			  break;
			digitalWrite(pin_MotorEF,LOW);    
			digitalWrite(pin_MotorPwm1, HIGH);  // Forward (Up....)	
			analogWrite(pin_MotorPwm2, motor_moving_Speed);
	
			delay(motor_moveloop_delay);
		}
		
		//analogWrite(pin_MotorPwm1, HIGH);   // Stop
		digitalWrite(pin_MotorEF,HIGH);    // Stop
		break;
		case 3:  // move to Middle Position (Down-->)
		for (Speed = motor_updownmove_Count_to_Top; Speed >= 1; Speed--)
		{
			LCD_with_MCU();
			while(Stop_tmp)
			{
				if(Reset_tmp) return;
				LCD_with_MCU();
			}
			if(Reset_tmp) return;
			digitalWrite(pin_MotorEF,LOW);    
			digitalWrite(pin_MotorPwm1, LOW); // ��������
			analogWrite(pin_MotorPwm2, motor_moving_Speed);  // Backward
			if (read_AmbientSensor(pinAmbient_sens2) < ambient_sensor_effective_value) // '��'�� ���Ͱ� ������, Stop.....
			  break;
			delay(motor_moveloop_delay);
		}
		//digitalWrite(pin_MotorPwm1, HIGH);
		digitalWrite(pin_MotorEF,HIGH);    // Stop   
		break;
		case 4:  // move to Base Position (Down-->)
		//for (Speed = motor_updownmove_Count * 3; Speed >= 1; Speed--)
		for (Speed = motor_updownmove_Count; Speed >= 1; Speed--)
		{
			Tempe_Read();
			control_Boxs();
			LCD_with_MCU();
			while(Stop_tmp)
			{
				if(Reset_tmp) return;
				LCD_with_MCU();
			}
			if(Reset_tmp) return;
			digitalWrite(pin_MotorEF, LOW);    
			digitalWrite(pin_MotorPwm1, LOW);
			analogWrite(pin_MotorPwm2, motor_moving_Speed);  // Backward
			if (read_AmbientSensor(pinAmbient_sens1) < ambient_sensor_effective_value) // '��'�� ���Ͱ� ������, Stop.....
			{
				fSenseChecked = true;
				break;
			}
			delay(motor_moveloop_delay);
		}
		if (fSenseChecked == false) //++20201120 : �ϰ��� ��,  ó�� ��ġ�� �������� ���� ��� ����.....1���� Try......
		{
			delay(100);
			for (Speed = motor_updownmove_Count_downMore; Speed >= 1; Speed--)
			{
				LCD_with_MCU();
				while(Stop_tmp)
				{
					if(Reset_tmp) return;
					LCD_with_MCU();
				}
				if(Reset_tmp) return;
				digitalWrite(pin_MotorEF,LOW);   
				digitalWrite(pin_MotorPwm1, LOW);
				analogWrite(pin_MotorPwm2, motor_moving_Speed);  // Backward
				if (read_AmbientSensor(pinAmbient_sens1) < ambient_sensor_effective_value) // '��'�� ���Ͱ� ������, Stop.....
				break;
				delay(motor_moveloop_delay);
			}
			
		}
		
		//digitalWrite(pin_MotorPwm1, HIGH);
		digitalWrite(pin_MotorEF,HIGH);    // Stop
		break;
		case 10:  // reset �� ������ ��������
		for (Speed = motor_updownmove_Count * 3; Speed >= 1; Speed--)
		{
			if (read_AmbientSensor(pinAmbient_sens1) < ambient_sensor_effective_value) // '��'�� ���Ͱ� ������, Stop.....
			break;
			digitalWrite(pin_MotorEF,LOW);
			digitalWrite(pin_MotorPwm1, LOW); // ��������
			analogWrite(pin_MotorPwm2, motor_moving_Speed - 20); // Backward to BASE(��-����).....
			delay(motor_moveloop_delay);
		}	
		digitalWrite(pin_MotorEF,HIGH);    // Stop
		break;
	}
}

void Start_Cooking()
{
	motor_move(0);
	// ���� ����	
	// ����� :..To..(up)Middle (Step : 1).....
	delay(300);
	LCD_with_MCU();
	while(Stop_tmp) 
	{
		if(Reset_tmp) {
			reset_seq();
			return;
		}
		LCD_with_MCU();
	}
	if(Reset_tmp) {
		reset_seq();
		return;
	}
	
	SetTextValue(current_screen_id,19,"mv1 start");
	motor_move(1);
	delay(300);
	SetTextValue(current_screen_id,19,"mv1 end");
	LCD_with_MCU();
	while(Stop_tmp)
	{
		if(Reset_tmp) {
			reset_seq();
			return;
		}
		LCD_with_MCU();
	}
	if(Reset_tmp) {
		reset_seq();
		return;
	}
	//control_Boxs();
	
	// Heater ����.....
	//Heater_Control(3,HIGH);
	
	delay(300);
	
	// Water Heater ����.....
	//Heater_Control(1,HIGH);
	//control_Boxs();	
	delay(300);
	
	LCD_with_MCU();
	while(Stop_tmp)
	{
		if(Reset_tmp) {
			reset_seq();
			return;
		}
		LCD_with_MCU();
	}
	if(Reset_tmp) {
		reset_seq();
		return;
	}
	
	
	SolValve_Control(4, HIGH); // Open Sol.4(����-������) Valve....
	SetTextValue(current_screen_id,19,"Rp start");
	Delay_And_Check(Recipe_Set_Input,0); // ����-������ ����
	if(Reset_tmp) {
		reset_seq();
		return;
	}
	SetTextValue(current_screen_id,19,"Rp end");
	// ���� ���� ����:
	SolValve_Control(4, LOW); // Close Sol.4(����-������) Valve....

	LCD_with_MCU();
	while(Stop_tmp)
	{
		if(Reset_tmp) {
			reset_seq();
			return;
		}
		LCD_with_MCU();
	}
	if(Reset_tmp) {
		reset_seq();
		return;
	}
	
	SetTextValue(current_screen_id,19,"T1 start");
	Tempe_Read();
	control_Boxs();
	Delay_And_Check(Delay_Set_Time,1);
	if(Reset_tmp) {
		reset_seq();
		return;
	}
	SetTextValue(current_screen_id,19,"T1 end");
	Tempe_Read();
	control_Boxs();
	
	// ����� :..To..(up)Top (Step : 2).....
	SetTextValue(current_screen_id,19,"mv2 start");
	motor_move(2);
	if(Reset_tmp) {
		reset_seq();
		return;
	}
	SetTextValue(current_screen_id,19,"mv2 end");

	//Steam.... Heater ����.....
	//Heater_Control(2,HIGH);
	//control_Boxs();
	
	delay(1000); // 10 sec
	
	// ���� ���� ����:
	SolValve_Control(2,HIGH);
	
	SetTextValue(current_screen_id,19,"stm start");
	Tempe_Read();
	control_Boxs();
	Delay_And_Check(Steam_Set_Input,2); // ���� ���� ��
	if(Reset_tmp) {
		reset_seq();
		return;
	}
	SetTextValue(current_screen_id,19,"stm end");
	control_RecipeBox(0);

	// ���� ���� ����:
	SolValve_Control(2,LOW);
	Tempe_Read();
	control_Boxs();
	SetTextValue(current_screen_id,19,"T2 start");
	
	Delay_And_Check(Delay_Set_Time2,1);
	if(Reset_tmp) {
		reset_seq();
		return;
	}
	SetTextValue(current_screen_id,19,"T2 end");
	Tempe_Read();
	control_Boxs();
	
	// ����ϰ� :..To..(down)Middle (Step : 3).....
	SetTextValue(current_screen_id,19,"mv4 start");
	motor_move(4);
	if(Reset_tmp) {
		reset_seq();
		return;
	}
	SetTextValue(current_screen_id,19,"mv4 end");
	delay(150);

	SetTextValue(current_screen_id,19,"All end");
}

void Stop_Cooking()
{
	SetTextValue(current_screen_id,19,"Stop");
	analogWrite(pin_MotorEF, HIGH); // ���� OFF
	analogWrite(pin_MotorPwm2,LOW);
	SolValve_Control(4, LOW); // ���� OFF
	SolValve_Control(2,LOW); // ���� OFF
	Stop_tmp = 1;
	//Stop_moter = 1;
	Stop_Recipe = 1;
	Stop_Stream = 1;
}

void Reset_Cooking()
{
	Stop_tmp = 1;
	SetTextValue(current_screen_id,19,"Reset");
	reset_seq();
	delay(100);
	SetTextValue(current_screen_id,19,"motor replace");
	motor_move(10); // ���� ���ڸ�
	
	//control_Boxs();
	Start_tmp = 0;
	Reset_tmp = 1;
}

void SendChar(uchar t)
{
	// �Ƶ��̳�� �ø��� 1����Ʈ ������ ����
	Serial.write(t);
	// kykim-���� �Ƶ��̳밡 �ƴ϶�� ����ϴ� ����ũ�� ���μ����� �°� �����ϼ���.
}
void BasicSetting(unsigned int screen_id)
{
	SetTextInt32(screen_id, Recipe_Set_Tempebox, SettingValue[screen_id][0],0x0, 0x0);
	SetTextInt32(screen_id, Pressure_Set_Tempebox, SettingValue[screen_id][1],0x0, 0x0);
	SetTextInt32(screen_id, Water_Set_Tempbox, SettingValue[screen_id][2],0x0, 0x0);
	SetTextInt32(screen_id, Recipe_Set_Timebox, SettingValue[screen_id][3],0x0, 0x0);
	SetTextInt32(screen_id, Steam_Set_Timebox, SettingValue[screen_id][4],0x0, 0x0);
	SetTextInt32(screen_id, Delay_Set_TimeBox, SettingValue[screen_id][5],0x0, 0x0);
	SetTextInt32(screen_id, Moter_Set_Timebox, SettingValue[screen_id][6],0x0, 0x0);
	SetTextInt32(screen_id, Delay_Set_Timebox2, SettingValue[screen_id][7],0x0, 0x0);
	
	pressuretempe = thermocouple.readCelsius();
	steamtempe = thermocouple2.readCelsius();
	rmtempe = thermocouple3.readCelsius();
	SetTextInt32(screen_id,Recipe_Current_Tempebox,rmtempe,0x0, 0x0);
	SetTextInt32(screen_id,Water_Current_Tempbox,steamtempe,0x0, 0x0);
	SetTextInt32(screen_id,Pressure_Current_Tempebox,pressuretempe,0x0, 0x0);
}

void NotifyScreen(unsigned int screen_id)
{
	//TODO: ����� �ڵ� �߰�
	current_screen_id = screen_id; // ���� ȭ�� ��ȣ ����
	if(screen_id > 1 && screen_id < 11) BasicSetting(screen_id);
		
	if(screen_id >= 11 && screen_id <= 19)
	{
		Recipe_Set_Tempe =	SettingValue[screen_id-9][0];
		Press_Set_TankTempe = SettingValue[screen_id-9][1];
		Press_Set_OccurTempe = SettingValue[screen_id-9][2];
		Recipe_Set_Input = SettingValue[screen_id-9][3];
		Steam_Set_Input = SettingValue[screen_id-9][4];
		Delay_Set_Time = SettingValue[screen_id-9][5];
		Motor_upTime = SettingValue[screen_id-9][6];
		Delay_Set_Time2 = SettingValue[screen_id-9][7];
	}
}
void NotifyButton(unsigned int screen_id, unsigned int control_id, unsigned char  state)
{
	//TODO: ����� �ڵ� �߰�
	//debug_msg("NotifyButton\r\n");
	if(screen_id >= 11 && screen_id <= 19)
	{
// 		if ((steamBox_readyOk == true) && (recipeBox_readyOk == true))
// 		{
			if(control_id == 2) //start
			{
				if(state == 1)
				{
					if(Stop_tmp && Reset_tmp == 0) Stop_tmp = 0;
					if(steamBox_readyOk && waterBox_readyOk)
					{
						if(recipeBox_readyOk)
						{			
							if(Start_tmp == 0)
							{
								Start_tmp = 1;
								Start_Cooking();
								Reset_tmp = 0;
								Start_tmp = 0;
							}
						}
					}
				}
			}
				else if(control_id == 3)// stop
			{
				if(state == 1)
				{
					if(Stop_tmp == 0 && Reset_tmp == 0) Stop_Cooking();
				}
			}
				else if(control_id == 9)// reset
			{
				if(state == 1)
				{
					if(Reset_tmp == 0) Reset_Cooking();
				}
			}
		/*}*/
	}
	
	//��������
	if(screen_id == 20)
	{
		//���� ���� sol
		if(control_id == 2) // on
		{
			if(state == 1)
			{
				SolValve_Control(2,HIGH); // ���� on 
			}
		}
		else if(control_id == 3) //off
		{
			if(state == 1)
			{
			    SolValve_Control(2,LOW); // ���� OFF
			}
		}
		
		//���� ���� sol
		if(control_id == 4) //on
		{
			if(state == 1)
			{
				SolValve_Control(4, HIGH); // ���� on 
			}
		}
		else if(control_id == 7) //off
		{
			if(state == 1)
			{
				SolValve_Control(4, LOW); // ���� OFF 
			}
		}
		
		// ������ �� ���� SOL
		if(control_id == 5) //on
		{
			if(state == 1)
			{
				int recipeLevel = analogRead(pin_WaterLevelSensor_2);
				if(recipeLevel >= mux_effective_value) // OFF
				{
					SolValve_Control(3, HIGH); // ���� on  1�� ����
				}
			}
		}
		else if(control_id == 10) //off
		{
			if(state == 1)
			{
				SolValve_Control(3, LOW); // ���� OFF  1�� ����
			}
		}
		
		//������ �� ���� sol
		if(control_id == 6) //on
		{
			if(state == 1)
			{
				int waterLevel_max = analogRead(pin_WaterLevelSensor_1_2);
				int waterLevel_min = analogRead(pin_WaterLevelSensor_1);
				
				if (waterLevel_min >= mux_effective_value && waterLevel_max >= mux_effective_value )
				{
					SolValve_Control(1,HIGH);
				}
			}
			
		}
		else if(control_id == 11) //off
		{
			if(state == 1)
			{
				SolValve_Control(1,LOW); // ���� OFF 
			}
		}
		
		//����
		if(control_id == 16) //on
		{
			if(state == 1)
			{
				if (read_AmbientSensor(pinAmbient_sens2) < ambient_sensor_effective_value) {
					digitalWrite(pin_MotorEF,HIGH);
					return;
				}
				if (read_AmbientSensor(pinAmbient_sens3) < ambient_sensor_effective_value) {
					digitalWrite(pin_MotorEF,HIGH);
					return;
				}
				digitalWrite(pin_MotorEF, LOW);
				digitalWrite(pin_MotorPwm1, HIGH);  // Forward (Up....)
				analogWrite(pin_MotorPwm2, motor_moving_Speed);
				delay(motor_moveloop_delay);
			}
			else digitalWrite(pin_MotorEF,HIGH);
		}
		else if(control_id == 17) //off
		{
			if(state == 1)
			{
				if (read_AmbientSensor(pinAmbient_sens1) < ambient_sensor_effective_value){
					 digitalWrite(pin_MotorEF,HIGH);
					 return;
				}
				digitalWrite(pin_MotorEF, LOW);
				digitalWrite(pin_MotorPwm1, LOW);  // Forward (down....)
				analogWrite(pin_MotorPwm2, motor_moving_Speed);
				delay(motor_moveloop_delay);
			}
			else digitalWrite(pin_MotorEF,HIGH);
		}
	}
	//�����µ�
	if(screen_id > 1 && screen_id < 11)
	{
			if(control_id==Recipe_Tempe_Up)
		{
			if( state == 1 )
			{
				SettingValue[screen_id][0]++;
				SettingValue[screen_id][0] = SettingValue[screen_id][0] % 101;
				SetTextInt32(screen_id, Recipe_Set_Tempebox, SettingValue[screen_id][0],0x0, 0x0);
				eeprom_update_word((uint16_t *)SettingValueptr[screen_id][0],SettingValue[screen_id][0]);
				
			}
		}
		else if(control_id == Recipe_Tempe_Down) 
		{
			if( state == 1 )
			{
				SettingValue[screen_id][0]--;
				if(SettingValue[screen_id][0] < 1) SettingValue[screen_id][0] = 100;
				SettingValue[screen_id][0] = SettingValue[screen_id][0] % 101;
				SetTextInt32(screen_id, Recipe_Set_Tempebox, SettingValue[screen_id][0],0x0, 0x0);
				eeprom_update_word((uint16_t *)SettingValueptr[screen_id][0],SettingValue[screen_id][0]);
			}

		}
		// �з���ũ
		if(control_id==Pressure_Tempe_Up) 
		{
			if( state == 1 )
			{
				SettingValue[screen_id][1]++;
				SettingValue[screen_id][1] = SettingValue[screen_id][1] % 1001;
				SetTextInt32(screen_id, Pressure_Set_Tempebox, SettingValue[screen_id][1],0x0, 0x0);		
				eeprom_update_word((uint16_t *)SettingValueptr[screen_id][1],SettingValue[screen_id][1]);			
			}
			
		}
		else if(control_id == Pressure_Tempe_Down)
		{
			if( state == 1 )
			{
				SettingValue[screen_id][1]--;
				if(SettingValue[screen_id][1] < 1) SettingValue[screen_id][1] = 1000;
				SettingValue[screen_id][1] = SettingValue[screen_id][1] % 1001;
				SetTextInt32(screen_id, Pressure_Set_Tempebox, SettingValue[screen_id][1],0x0, 0x0);			
				eeprom_update_word((uint16_t *)SettingValueptr[screen_id][1],SettingValue[screen_id][1]);
			}
		}
		//�����߻�
		if(control_id==Water_Tempe_Up) 
		{
			if( state == 1 )
			{
				SettingValue[screen_id][2]++;
				SettingValue[screen_id][2] = SettingValue[screen_id][2] % 131;
				SetTextInt32(screen_id, Water_Set_Tempbox, SettingValue[screen_id][2],0x0, 0x0);
				eeprom_update_word((uint16_t *)SettingValueptr[screen_id][2],SettingValue[screen_id][2]);
			}
		}
		else if(control_id == Water_Tempe_Down) 
		{
			if( state == 1 )
			{
				SettingValue[screen_id][2]--;
				if(SettingValue[screen_id][2] < 1) SettingValue[screen_id][2] = 130;
				SettingValue[screen_id][2] = SettingValue[screen_id][2] % 131;
				SetTextInt32(screen_id, Water_Set_Tempbox,SettingValue[screen_id][2],0x0, 0x0);
				eeprom_update_word((uint16_t *)SettingValueptr[screen_id][2],SettingValue[screen_id][2]);
			}

		}
		//�����ð�
		if(control_id==Recipe_Time_Up)
		{
			if( state == 1 )
			{
				SettingValue[screen_id][3]++;
				SettingValue[screen_id][3] = SettingValue[screen_id][3] % 61;
				SetTextInt32(screen_id, Recipe_Set_Timebox, SettingValue[screen_id][3],0x0, 0x0);
				eeprom_update_word((uint16_t *)SettingValueptr[screen_id][3],SettingValue[screen_id][3]);
			}
		}
		else if(control_id == Recipe_Time_Down)
		{
			if( state == 1 )
			{
				SettingValue[screen_id][3]--;
				if(SettingValue[screen_id][3] < 1) SettingValue[screen_id][3] = 60;
				SettingValue[screen_id][3] = SettingValue[screen_id][3] % 61;
				SetTextInt32(screen_id, Recipe_Set_Timebox, SettingValue[screen_id][3],0x0, 0x0);
				eeprom_update_word((uint16_t *)SettingValueptr[screen_id][3],SettingValue[screen_id][3]);
			}

		}
		//�����ð�
		if(control_id==Steam_Time_Up)
		{
			if( state == 1 )
			{
					SettingValue[screen_id][4]++;
					SettingValue[screen_id][4] = SettingValue[screen_id][4] % 181;
					SetTextInt32(screen_id, Steam_Set_Timebox, SettingValue[screen_id][4],0x0, 0x0);
					eeprom_update_word((uint16_t *)SettingValueptr[screen_id][4],SettingValue[screen_id][4]);
			}
		}
		else if(control_id == Steam_Time_Down)
		{
			if( state == 1 )
			{
					SettingValue[screen_id][4]--;
					SettingValue[screen_id][4] = SettingValue[screen_id][4] % 181;
					if(SettingValue[screen_id][4] < 1) SettingValue[screen_id][4] = 180;
					SetTextInt32(screen_id,  Steam_Set_Timebox, SettingValue[screen_id][4],0x0, 0x0);
					eeprom_update_word((uint16_t *)SettingValueptr[screen_id][4],SettingValue[screen_id][4]);
			}
		}
		//���� �ð�
		if(control_id==Delay_Time_Up)
		{
			if( state == 1 )
			{
				SettingValue[screen_id][5]++;
				SettingValue[screen_id][5] = SettingValue[screen_id][5] % 181;
				SetTextInt32(screen_id, Delay_Set_TimeBox, SettingValue[screen_id][5],0x0, 0x0);
				eeprom_update_word((uint16_t *)SettingValueptr[screen_id][5],SettingValue[screen_id][5]);
			}
		}
		else if(control_id == Delay_Time_Down)
		{
			if( state == 1 )
			{
				SettingValue[screen_id][5]--;
				SettingValue[screen_id][5] = SettingValue[screen_id][5] % 181;
				if(SettingValue[screen_id][5] < 1) SettingValue[screen_id][5] = 180;
				SetTextInt32(screen_id,  Delay_Set_TimeBox, SettingValue[screen_id][5],0x0, 0x0);
				eeprom_update_word((uint16_t *)SettingValueptr[screen_id][5],SettingValue[screen_id][5]);
			}

		}
		//��� ���
		if(control_id==Moter_Time_Up)
		{
			if( state == 1 )
			{
				SettingValue[screen_id][6]++;
				SettingValue[screen_id][6] = SettingValue[screen_id][6] % 31;
				SetTextInt32(screen_id, Moter_Set_Timebox, SettingValue[screen_id][6],0x0, 0x0);
				eeprom_update_word((uint16_t *)SettingValueptr[screen_id][6],SettingValue[screen_id][6]);
			}
		}
		else if(control_id == Moter_Time_Down)
		{
			if( state == 1 )
			{
				SettingValue[screen_id][6]--;
				if(SettingValue[screen_id][6] < 1) SettingValue[screen_id][6] = 30;
				SettingValue[screen_id][6] = SettingValue[screen_id][6] % 31;
				SetTextInt32(screen_id,  Moter_Set_Timebox, SettingValue[screen_id][6],0x0, 0x0);
				eeprom_update_word((uint16_t *)SettingValueptr[screen_id][6],SettingValue[screen_id][6]);
			}

		}
		//���� �ð�2
		if(control_id==Delay_Time2_Up)
		{
			if( state == 1 )
			{
				SettingValue[screen_id][7]++;
				SettingValue[screen_id][7] = SettingValue[screen_id][7] % 181;
				SetTextInt32(screen_id, Delay_Set_Timebox2, SettingValue[screen_id][7],0x0, 0x0);
				eeprom_update_word((uint16_t *)SettingValueptr[screen_id][7],SettingValue[screen_id][7]);
			}
		}
		else if(control_id == Delay_Time2_Down)
		{
			if( state == 1 )
			{
				SettingValue[screen_id][7]--;
				SettingValue[screen_id][7] = SettingValue[screen_id][7] % 181;
				if(SettingValue[screen_id][7] < 1) SettingValue[screen_id][7] = 180;
				SetTextInt32(screen_id,  Delay_Set_Timebox2, SettingValue[screen_id][7],0x0, 0x0);
				eeprom_update_word((uint16_t *)SettingValueptr[screen_id][7],SettingValue[screen_id][7]);
			}

		}
	}
}

void Tempe_Read()
{
	pressuretempe = thermocouple.readCelsius();
	rmtempe = thermocouple2.readCelsius();
	steamtempe = thermocouple3.readCelsius();
	return;
}

void Tempe_updateAndCheckReady()
{	
	if(++Tempe_tmp1 > 500)
	{
		Tempe_tmp1 = 0;
		Tempe_Read();
	}
	
	if(current_screen_id > 1 && current_screen_id < 11)
	{
		if(++Tempe_tmp > 500)
		{
			Tempe_tmp = 0;
			
			SetTextInt32(current_screen_id,Recipe_Current_Tempebox,rmtempe,0x0, 0x0);
			SetTextInt32(current_screen_id,Water_Current_Tempbox,steamtempe,0x0, 0x0);
			SetTextInt32(current_screen_id,Pressure_Current_Tempebox,pressuretempe,0x0, 0x0);
		}	
	}
	
	if(++ready_tmp > 30000) // �� 1��
	{
		ready_tmp = 0;
		//control_Boxs();
		if(Reset_tmp)
		{
			if ((steamBox_readyOk == true) && (recipeBox_readyOk == true))
			{
				SetTextValue(current_screen_id,19,"Cooking Ready!");
				Reset_tmp = 0;
			}
			else SetTextValue(current_screen_id,19,"Please wait...");
		}
		else
		{			
			if(current_screen_id == 0)
			{
				if ((steamBox_readyOk == true) && (recipeBox_readyOk == true))
					SetTextValue(0, 22, "Cooking Ready!");
			}
			
			if(current_screen_id >= 11 && current_screen_id <=19)
			{
				if ((steamBox_readyOk == true) && (recipeBox_readyOk == true))
				{
					SetTextValue(current_screen_id,19,"Cooking Ready!");
					LCD_with_MCU();
				}
				else
				{
					SetTextValue(current_screen_id,19,"Please wait...");
				}
			}
		}
		
		
// 		if ((steamBox_readyOk == true) && (recipeBox_readyOk == true)) // Only If All ready.........
// 		{
// 			if(current_screen_id == 0)SetTextValue(0, 22, "Cooking Ready!");
// 			lcd_tmp = 1;
// 		}
// 		else
// 		{
// 			if(current_screen_id == 0)SetTextValue(0, 22, "Please Wait...");
// 			lcd_tmp = 0;
// 		}
// 		
	}
}

void setup()
{
	// Ÿ�̸� ������ ������Ʈ �ð�
	
	  queue_reset();
 	  Serial.begin(38400);
	  delay(300);	
	
	  pinMode(pin_WaterLevelSensor_1,INPUT);
	  pinMode(pin_WaterLevelSensor_2,INPUT);
	  pinMode(pin_WaterLevelSensor_1_2,INPUT);
	  
	  //pinMode(pin_muxOnCH,OUTPUT);
	  pinMode(pin_Data,OUTPUT); // dataPin
	  pinMode(pin_Latch,OUTPUT); // latchPin
	  pinMode(pin_Clock,OUTPUT); // clockPin
	  
	  digitalWrite(pin_Data, LOW);
	  digitalWrite(pin_Latch, LOW);
	  digitalWrite(pin_Clock, LOW);	
		
	  // Motor Control with EF + PWM. ( 7, + 6, 5 )......
	  pinMode(pin_MotorEF, OUTPUT);  // Motordriver EF (Error Flag - Open Drain)
	  digitalWrite(pin_MotorEF,HIGH); // ���� x
		
	  pinMode(pin_MotorPwm1, OUTPUT);  // Motordriver IN1
	  pinMode(pin_MotorPwm2, OUTPUT);  // Motordriver IN2 pwm
	  digitalWrite(pin_MotorPwm1, LOW);
	  digitalWrite(pin_MotorPwm2, LOW);

	   // Ambient(Proximity) Sensor(A0, A7, A1 : ��� Ȯ��)
	  pinMode(pinAmbient_sens1, INPUT);
	  pinMode(pinAmbient_sens2, INPUT);
	  pinMode(pinAmbient_sens3, INPUT);

	  if ((steamBox_readyOk == true) && (recipeBox_readyOk == true))
	  	SetTextValue(0, 22, "Cooking Ready!");
	  else
	  //digitalWrite(pinOp_LED, LOW);  //  Operator LED....(G r e e n)...
	  
	  led_toggle_Count	= 0;
	  
	  //������ Heater�� �ϴ� ������ Ų��... ==> �׸���,  ���߿�  control_RecipeBox �Լ�����  ��Ʈ��.....
	  // Heater ����.....
	  
	 // Heater_Control(3,HIGH);
	  
	  Heater_Control(1,LOW);
	  Heater_Control(2,LOW);
	  Heater_Control(3,LOW);
	  Heater_Control(4,LOW);
	  SolValve_Control(1,LOW);
	  SolValve_Control(2,LOW);
	  SolValve_Control(3,LOW);
	  SolValve_Control(4,LOW);
	  recipewater_Level_minCount = 0;
	  wwater_Level_minCount = 0;
	  
	//Write and Read value in EEPROM 
	EEPROMtmp = eeprom_read_word((uint16_t*)1);
	if(EEPROMtmp != 1)
	{
		EEPROMtmp = 1;
		for(int i =2;i<11;i++)
			for(int j = 0;j<7;j++)
				eeprom_write_word((uint16_t*)SettingValueptr[i][j],SettingValue[i][j]);
		eeprom_write_word((uint16_t*)EEPROMtmp,1);
	}
	else
	{
		for(int i =2;i<11;i++)
			for(int j = 0;j<7;j++)
				SettingValue[i][j] = eeprom_read_word((uint16_t*)SettingValueptr[i][j]);
	}
}

void loop()
{ 
	//control_Boxs();
 	LCD_with_MCU();
 	Tempe_updateAndCheckReady();
	control_Boxs();
}



