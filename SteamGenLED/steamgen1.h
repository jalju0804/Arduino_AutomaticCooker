
// Define the Pins

///// Button Control /////
// " 74HC4051M " Mux chip for multi-Switch......



//int powerButton = 0;


/// OP-LED /////
int pinOp_LED = 0;      // PD0 - Red <=> Green

// Heater 
int pinHeater_1 = A4;     // PC4(ADC4) -- Water Box
int pinHeater_2 = A3;     // PC3(ADC3) -- Steam Box
int pinHeater_3 = A2;     // PC2(ADC2) -- Recipe Box

// Ambient(Proximity) Sensor....
int pinAmbient_sens3 = A1;     // PC1(ADC1) -- 상
int pinAmbient_sens1 = A0;     // PC0(ADC0) -- 하
int pinAmbient_sens2 = A7;     // ADC7      -- 중

// Sol. Valve....
int pinSolvalv_1 = 8;     // PB0 -- Water Input
int pinSolvalv_2 = 9;     // PB1 -- Steam Input
//--Not operating int pinSolvalv_3 = 14;     // PB6 -- Recipe Water Input(육수 보충)
//--Not operating int pinSolvalv_4 = 15;     // PB7 -- Recipe Input(조리)

// Pressure Sensor  for Steam Box.....
int pinPress_sens = A6;     // ADC6 

// Temprature Sensor ......
// " MAX6675 " chip multi-Drop connected.......
int pinTempsCS_1 = 11;     // PB3 
int pinTempsCS_2 = 10;     // PB2 
int pinTemps_SO = 12;     // PB4 
int pinTemps_SCK = 13;     // PB5 

// MOtor 
int pin_MotorEF = 7;   // PD7(AIN1)     ---> Error Flag..... (open drain)
int pin_MotorPwm1 = 6;   // PD6(AIN0)   ---> Motor UP move
int pin_MotorPwm2 = 5;   // PD5(OCO0B) ---> Motor Down move


// Motor Control
// 250 = Stop speed,  0 = Max speed
const int motor_speed_MAX = 0;   // ?????????
const int motor_speed_Middle = 128;
const int motor_speed_MIN = 255;  // ????????

//-- Origin const int motor_moving_Speed = 128; 
//-- very slow....... const int motor_moving_Speed = 64; // ask for lhh 1/2 (128 ==> 64)
const int motor_moving_Speed = 128; // ask for lhh 1/2 (128 ==> 64)
const int motor_moving_ToBase_Speed = 30;

const int motor_move_Base = 0;


//t1 const int motor_updownmove_Count = 350;
//t1 const int motor_updownmove_Count_to_Top = 100;
//t2 const int motor_updownmove_Count = 450;
//t2 const int motor_updownmove_Count_to_Top = 200;
const int motor_updownmove_Count = 750;
const int motor_updownmove_Count_to_Top = 450;
const int motor_updownmove_Count_downMore = 350;

enum {
	nobutton_Normal_mode = 0,
	
	large_ButtonPressed_mode = 1,
	middle_ButtonPressed_mode = 2,
	small_ButtonPressed_mode = 3
	};

boolean waterBox_readyOk = false; 
boolean recipeBox_readyOk = false; 
boolean steamBox_readyOk = false; 

int jori_Mode = nobutton_Normal_mode;

//const int  mux_effective_value = 120;
//--const int  mux_effective_value = 320;

const int  mux_effective_value = 60;
const int  ambient_sensor_effective_value = 60;

const int led_toggle_Interval = 2; // 3.0 sec.
int       led_toggle_Count = 0; 
bool      led_Mode = false;

bool	  start_btn_pressed = false;

int   recipewater_Input_count = 0;


int	  recipewater_Level_minCount = 0;
int	  recipewater_Level_maxCount = 0;

int	  wwater_Level_minCount = 0;
int	  wwater_Level_maxCount = 0;




