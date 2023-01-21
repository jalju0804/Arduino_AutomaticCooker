
// Define the Pins

///// Button Control /////
// " 74HC4051M " Mux chip for multi-Switch......
// Start , Water , Steam
// Large , Middle , Small ( Current,  Not Use..... )
// 수위센서1 -- (S357_1 = x6[2], 수위센서2 -- (S357_2 = X4[1]) (Water Position Sensor 2EA)
int pin_muxOnCH = A5;   // PC5(ADC5)  220815
int pin_Data = 2;   // PD4(XCK) 220815
int pin_Latch = 3;   // PD3(INT1) 220815
int pin_Clock = 4;   // PD2(INT0) 220815
int pin_muxCE = 1;   // PD1(TXD) ---- Mux: INH pin

int mux_chnnel_Heater1 = 0; // Water Box 220815
int mux_chnnel_Heater2 = 1; // Steam Box 220815
int mux_chnnel_Heater3 = 2; // Recipe Box 220815
int mux_chnnel_Heater4 = 3; // new 220815

int mux_chnnel_SOL1 = 4; // Water Input 220815
int mux_chnnel_SOL2 = 5; // Steam Input 220815
int mux_chnnel_SOL3 = 6; // Recipe Water Input(육수 보충) 220815
int mux_chnnel_SOL4 = 7; // Recipe Input(조리) 220815

int pin_WaterLevelSensor_1 = A3; // Recipe Water 220815 pc3
int pin_WaterLevelSensor_1_2 = A2; // new 220815 pc2
int pin_WaterLevelSensor_2 = A4; // Water Box 220815

//int powerButton = 0;

/*
/// OP-LED /////
int pinOp_LED = 0;      // PD0 - Red <=> Green 

// Heater 
int pinHeater_1 = A4;     // PC4(ADC4) -- Water Box 
int pinHeater_2 = A3;     // PC3(ADC3) -- Steam Box
int pinHeater_3 = A2;     // PC2(ADC2) -- Recipe Box
220815 */

// Ambient(Proximity) Sensor.... 근접센서 
int pinAmbient_sens3 = A1;     // PC1(ADC1) -- 상 220815
int pinAmbient_sens1 = A0;     // PC0(ADC0) -- 하 220815
int pinAmbient_sens2 = A7;     // ADC7      -- 중 220815
 
 /*
// Sol. Valve....
int pinSolvalv_1 = 8;     // PB0 -- Water Input
int pinSolvalv_2 = 9;     // PB1 -- Steam Input
//--Not operating int pinSolvalv_3 = 14;     // PB6 -- Recipe Water Input(육수 보충)
//--Not operating int pinSolvalv_4 = 15;     // PB7 -- Recipe Input(조리)
*/
 
// Pressure Sensor  for Steam Box.....
int pinPress_sens = A6;     // ADC6 220815

// Temprature Sensor ......
// " MAX6675 " chip multi-Drop connected.......
int pinTempsCS_1 = 10;     // PB2 220815
int pinTempsCS_2 = 9;     // PB1 220815
int pinTempsCS_3 = 8;	   // PB0 220815

int pinTemps_SO = 12;     // PB4 220815
int pinTemps_SCK = 13;     // PB5 220815

// MOtor 
int pin_MotorEF = 7;   // PD7(AIN1)     ---> Error Flag..... (open drain) 220815
int pin_MotorPwm1 = 6;   // PD6(AIN0)   ---> Motor UP move 220815
int pin_MotorPwm2 = 5;   // PD5(OCO0B) ---> Motor Down move 220815

//-- const int motor_moveloop_delay = 10;
const int motor_moveloop_delay = 20;

// 스팀용기  압력 : 압력이  3k를 넘으면 터질 가능성이 잇어서  1.5~ 2.0k 유지 예정(안정성....)
// Input : (20mA....) 0V ~ 5V로 입력되는 것같은데,  확인해야 함.........
// 압력 센서를 읽어서,  물통의   히터를 가열/정지 결정.....
// test시 온도센서를 달아서,   110 ~ 125도 사이일 경우의  압력값을 확인 후,  가열/멈춤.....결정.....
//const int steamBox_Press_Margin_MIN = 250; // 
//const int steamBox_Press_Margin_MAX = 300; // 
//const int steamBox_Press_Margin_MIN = 100; //
//const int steamBox_Press_Margin_MAX = 200; //
//-- 1.5 bar ( ... 250 )  start......
//-- 2 bar ( ... 300 ) stop...
// 230 - 108도
// 260 - 115도
const int steamBox_Press_Margin_MIN = 245; // 245 ~ 275도 유지 110도
const int steamBox_Press_Margin_MAX = 275; // 116도


 // 물통 수위 sensor : Low / High
// NO use : const float waterBox_Tempe_MIN = 125; // 물통 : 125 ~ 150도 유지
// NO use : const float waterBox_Tempe_MAX = 150; // 

// 육수통 수위  sensor : Low / High
//const int recipeBox_Tempe_MIN = 90; // 육수통 : 90 ~ 95도 유지
const int recipeBox_Tempe_MIN = 86; // 육수통 : 86 ~ 95도 유지
const int recipeBox_Tempe_MIDD = 90; // 육수통 : 86 ~ 95도 유지
const int recipeBox_Tempe_MAX = 95; // 

//--20200915 const int steamBox_Tempe_MIN = 260; // 스팀 통 : 260 ~ 290도 유지
//--20200915 const int steamBox_Tempe_MAX = 290; //
//--20201105 const int steamBox_Tempe_MIN = 390; // 스팀 통 : 260 ~ 290도 유지
//--20201105const int steamBox_Tempe_MAX = 420; // 490 (140..)
//t1 const int steamBox_Tempe_MIN = 150; // 스팀 통 : 260 ~ 290도 유지
//t1 const int steamBox_Tempe_MAX = 170; // 490 (140..)
const int steamBox_Tempe_MIN = 110; // 스팀 통 : 110 ~ 130도 유지 20220813 수정
const int steamBox_Tempe_MAX = 130; // 490 (140..)

// 모터위치확인 근접센서 : High / Low..

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

//--origin const int motor_updownmove_Count = 60;
//--2.5sec test const int motor_updownmove_Count = 250;
//--20200817 const int motor_updownmove_Count = 350;
//- test only const int motor_updownmove_Count = 1300;
//--const int motor_updownmove_Count = 350;

//--200911 const int motor_updownmove_Count = 750;
//--200911 const int motor_updownmove_Count_to_Top = 500;
//--1105 const int motor_updownmove_Count = 650;
//--1105 const int motor_updownmove_Count_to_Top = 400;
//t1 const int motor_updownmove_Count = 350;
//t1 const int motor_updownmove_Count_to_Top = 100;
//t2 const int motor_updownmove_Count = 450;
//t2 const int motor_updownmove_Count_to_Top = 200;
int motor_updownmove_Count;
int motor_updownmove_Count_to_Top;
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

//each value definition
//steam twice
#define Recipe_Current_Tempebox		2
#define Recipe_Set_Tempebox			3
#define Pressure_Current_Tempebox	4
#define Pressure_Set_Tempebox		5
#define	Water_Current_Tempbox		6
#define	Water_Set_Tempbox			12
#define	Recipe_Set_Timebox			15
#define	Steam_Set_Timebox			24
#define Delay_Set_TimeBox			35
#define Moter_Set_Timebox			36
#define Delay_Set_Timebox2			40

#define Recipe_Tempe_Up				16
#define Recipe_Tempe_Down			17
#define Pressure_Tempe_Up			18
#define	Pressure_Tempe_Down			25
#define Water_Tempe_Up				19
#define Water_Tempe_Down			26
#define Recipe_Time_Up				22
#define Recipe_Time_Down			27
#define Steam_Time_Up				23
#define Steam_Time_Down				28
#define Delay_Time_Up				31
#define Delay_Time_Down				33
#define Moter_Time_Up				32
#define Moter_Time_Down				34
#define Delay_Time2_Up				41
#define Delay_Time2_Down			42

uint16_t SettingValue[11][8]=
{
	{0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0}, 
	{95,120,110,19,55,30,7,30}, //	Rm 육수온도 압력탱크(steam) 스팀발생(water) 육수시간 스팀시간 딜레이2 모터시간 딜레이1
	{95,120,110,12,55,30,7,30}, //	Nd
	{95,120,110,7,55,30,7,30}, //	Cup
	{95,120,110,19,55,30,7,30}, //	Rice
	{95,120,110,19,55,30,7,30},//	Six
	{95,120,110,19,55,30,7,30},//	Free1
	{95,120,110,19,55,30,7,30},//	Free2
	{95,120,110,19,55,30,7,30},//	Free3
	{95,120,110,19,55,30,7,30}//    Free4
};

uint8_t SettingValueptr[11][8]=
{
	{0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0},
	{4,8,12,16,20,24,28,256}, //	Rm 육수온도 압력탱크 스팀발생 육수시간 스팀시간 딜레이1 모터시간 딜레이2
	{32,36,40,44,48,52,56,260}, //	Nd
	{60,64,68,72,76,80,84,264}, //	Cup
	{88,92,96,100,104,108,112,268}, //	Rice
	{116,120,124,128,132,136,140,272},//	Six
	{144,148,152,156,160,164,168.276},//	Free1
	{172,176,180,184,188,192,196.280},//	Free2
	{200,204,208,212,216,220,224,284},//	Free3
	{228,232,236,240,244,248,252,288}//		Free4
};

// uint8_t Rm_recipe_tempe = 95; // 육수
// uint8_t Rm_water_tempe = 0; // 압력 탱크
// uint8_t Rm_steam_tempe = 110; // 스팀 발생
// uint8_t Rm_recipe_time = 19; 
// uint8_t Rm_steam_time = 55; 
// 
// uint8_t Nd_recipe_tempe = 95;
// uint8_t Nd_water_tempe = 0;
// uint8_t Nd_steam_tempe = 110;
// uint8_t Nd_recipe_time = 12;
// uint8_t Nd_steam_time = 55;
// 
// uint8_t Cup_recipe_tempe = 95;
// uint8_t Cup_water_tempe = 0;
// uint8_t Cup_steam_tempe = 110;
// uint8_t Cup_recipe_time = 7;
// uint8_t Cup_steam_time = 55;
// 
// uint8_t Rice_recipe_tempe = 95;
// uint8_t Rice_water_tempe = 0;
// uint8_t Rice_steam_tempe = 110;
// uint8_t Rice_recipe_time = 19;
// uint8_t Rice_steam_time = 55;
// 
// uint8_t Six_recipe_tempe = 95;
// uint8_t Six_water_tempe = 0;
// uint8_t Six_steam_tempe = 110;
// uint8_t Six_recipe_time = 19;
// uint8_t Six_steam_time = 55;
// 
// uint8_t Free1_recipe_tempe = 95;
// uint8_t Free1_water_tempe = 0;
// uint8_t Free1_steam_tempe = 110;
// uint8_t Free1_recipe_time = 19;
// uint8_t Free1_steam_time = 55;
// 
// uint8_t Free2_recipe_tempe = 95;
// uint8_t Free2_water_tempe = 0;
// uint8_t Free2_steam_tempe = 110;
// uint8_t Free2_recipe_time = 19;
// uint8_t Free2_steam_time = 55;
// 
// uint8_t Free3_recipe_tempe = 95;
// uint8_t Free3_water_tempe = 0;
// uint8_t Free3_steam_tempe = 110;
// uint8_t Free3_recipe_time = 19;
// uint8_t Free3_steam_time = 55;
// 
// uint8_t Free4_recipe_tempe = 95;
// uint8_t Free4_water_tempe = 0;
// uint8_t Free4_steam_tempe = 110;
// uint8_t Free4_recipe_time = 19;
// uint8_t Free4_steam_time = 55;