//#include <Wire.h>
#include <jm_Wire.h>
//#include "pinout.h"
#include "pin_new.h"
#include "./lcd_display/ctrl_display.h"
#include "./memory/memory.cpp"
//#include "./libraries/wire/src/Wire1.cpp"
//#include "./libraries/wire/src/Wire1.h"
#include "./sensors/sensormanager.h"
#include "./sensors/sensormanager.cpp"
#include "./lcd_display/service_mode.h"
#include "./lcd_display/service_mode.cpp"
#include "./state_control/statecontrol.h"
#include "./state_control/statecontrol.cpp"
#include "./encoder/encoder.c"
#include "./lcd_display/ctrl_display.cpp"

int TimeSeries = 0;

int ctrlParamChangeInit = 0;
volatile int switchMode = 0;
volatile boolean actionPending = false;
#define READ_FROM_ENCODER ((switchMode == PAR_SELECTED_MODE) \
                           && (currPos >=0 && (currPos < MAX_CTRL_PARAMS) \
                               && (params[currPos].readPortNum == DISP_ENC_CLK)))

#define LCD_DISP_REFRESH_COUNT 5
#define EDIT_MODE_TIMEOUT 5000000
#define SEND_CTRL_PARAMS_COUNT 15
#define SEND_SENSOR_VALUES_COUNT 5

int ContrlParamsSendCount = 0;
int SensorValuesSendCount = 0;

bool machineOn = false;
//Need to Integrate into Main Code
bool compressionCycle = false;
bool expansionCycle = false;
bool homeCycle = false;
String rxdata;
int comcnt;

bool gCntrlSerialEventRecvd = false;

bool powerSupplyFailure = false;
bool gasSuppluFailure = false;
bool mechFailSafeValve = false;

displayManager dM;

ErrorDef_T gErrorState = NO_ERR;

void setup() {
  lcd.createChar(DP_FI, fiChar);
   lcd.createChar(DP_UP_TR, upTriaChar);
  lcd.createChar(DP_DW_TR, dwnTriaChar);
  lcd.createChar(DP_EM_DN_TR, emDnChar);
   lcd.createChar(DP_EM_UP_TR, emUpChar);

  pinMode(O2_CYN_SWITCH, INPUT_PULLUP);
  pinMode(O2_HOSP_SWITCH, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(AUTO_MODE, INPUT_PULLUP);
  pinMode(ASSISTED_MODE, INPUT_PULLUP);
  pinMode(RESET_SWITCH, INPUT_PULLUP);
  pinMode(DISP_ENC_CLK, INPUT_PULLUP);
  pinMode(DISP_ENC_DT, INPUT_PULLUP);
  pinMode(DISP_ENC_SW, INPUT_PULLUP);
  pinMode(ADS115_INT_PIN, INPUT_PULLUP);
  pinMode(ADS115_INT_PIN_1, INPUT_PULLUP);
  pinMode(POWER_SUPPLY_FAILURE, INPUT_PULLUP);
  pinMode(GAS_SUPPLY_FAILURE, INPUT_PULLUP);
  pinMode(MECH_FAILSAFE_VALVE, INPUT_PULLUP);
  lcd.begin(LCD_LENGTH_CHAR, LCD_HEIGHT_CHAR);
  Wire.setClock(4000000L);
  Wire.begin();
  hbad_mem.begin();
  Serial.begin(115200);
  Serial2.begin(115200);
  Serial3.begin(115200);
  attachInterrupt(digitalPinToInterrupt(DISP_ENC_SW), isr_processSwitch, FALLING );
  attachInterrupt(digitalPinToInterrupt(DISP_ENC_CLK), isrEncoderClk, RISING  );
  attachInterrupt(digitalPinToInterrupt(DISP_ENC_DT), isrEncoderDt, RISING  );
  getAllParamsFromMem();
  sM.init();
  delay(1000);
  displayInitialScreen(dM);
  checkAlarms();
  setup_service_mode();
  sendDefaultParams();
  Serial.println("Exiting setup !");
} //end of setup

void sendDefaultParams(){
  Ctrl_send_packet(tidl_volu.parm_name,params[E_TV].value_curr_mem);
  delay(100);
  Ctrl_send_packet(resp_rate.parm_name,params[E_BPM].value_curr_mem);
  delay(100);
  Ctrl_send_packet(inex_rati.parm_name,params[E_IER].value_curr_mem);
  delay(100);
  sM.enable_sensor(O2);
}

void checkAlarms() {
  gErrorState = NO_ERR;
  int gasSupply = digitalRead(GAS_SUPPLY_FAILURE);
  int mechFailSafe = digitalRead(MECH_FAILSAFE_VALVE);
  int oxySupply = digitalRead(O2_CYN_SWITCH);
  int hospSwitch = digitalRead(O2_HOSP_SWITCH);
  
if(breathCount > 2){
  if (machineOn == true && oxySupply == LOW && hospSwitch == LOW) {
   //gErrorState = ERR_OXY;
  }
  if (bvmFailure) {
    gErrorState = ERR_BVM;
  }
#if 0
  else {
    if (peepErr != 0) {
      gErrorState = ERR_PEEP;
   }
    if (tviErr != 0) {
      gErrorState = ERR_TV;
    }
  }
#endif  
  if (NO_ERR != gErrorState)
    dM.errorDisplay(gErrorState);
 }
  
}

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

float data_sensors[MAX_SENSORS] = {0};
/* Project Main loop */

void loop() {
  int index = 0;
  int err = 0;
  
  checkAlarms();

  for (; index < MAX_SENSORS; index++) {
    err = sM.read_sensor_data(index, (float *) & (data_sensors[index]));
    if (err) {
     // Serial.print("ERROR ");
     // Serial.print(err);
     // Serial.print(": sensor read error for ");
     // Serial.println(index);
    }
#if 0    
    Serial.print("Index: ");
    Serial.println(sensor_data[index]);
#endif
  }

  
  //Serial.print("freespace :");
  //Serial.println(freeMemory());
  if (NO_ERR == gErrorState) {
    dM.displayManagerloop(&data_sensors[0], sM);
  } else {
    dM.errorDisplay(gErrorState);
    gErrorState = NO_ERR;
  }

  if (gCntrlSerialEventRecvd == true) {
    gCntrlSerialEventRecvd = false;
    Ctrl_ProcessRxData();
  }
  Ctrl_StateMachine_Manager(&data_sensors[0], sM, dM);
  if (digitalRead(RESET_SWITCH) == LOW) {
    //reset switch.
    if (machineOn == true) {
      machineOn = false;
      Serial3.print(commands[STPR_STP]);
      Ctrl_Stop();
       breathCount = 0;
      gErrorState = NO_ERR;

    } else if (machineOn == false) {
      machineOn = true;
      Ctrl_Start();
      breathCount++;
     // Serial.print("MachineON");
    }
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
  }
}

void displayChannelData(sensor_e sensor)
{
  int index = 0;
  int err = 0;
#if 1
  int o2mVReading, o2Unitx10;
  RT_Events_T eRTState = RT_NONE;
#if SERIAL_PRINTS
  Serial.println("we are in diagO2Sensor");
#endif

  float sensor_data[MAX_SENSORS] = {0};
  checkAlarms();
  for (; index < MAX_SENSORS; index++) {
    err = sM.read_sensor_data(index, (float *) & (sensor_data[index]));
    if (err) {
     // Serial.print("ERROR ");
     // Serial.print(err);
     // Serial.print(": sensor read error for ");
     // Serial.println(index);
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Input for:");
  lcd.print(menuItems[currentMenuIdx].menu[seletIndicator + scrollIndex - 1 ]);
  lcd.setCursor(0, 1);

  lcd.print("current reading:");

  while (RT_NONE == eRTState)
  {
    String disp = "";
    lcd.setCursor(0, 2);
    o2Unitx10 = sensor_data[sensor];
    disp += (((float)o2Unitx10) / 10);
    if (sensor == SENSOR_O2)
    {
      disp += ("%   ");
    }
    else
    {
      disp += ("cmH2O  ");
    }
    while (disp.length() < LCD_LENGTH_CHAR)
    {
      disp += " ";
    }
    lcd.print(disp);
    for (int wait = 0; wait < 200; wait += 20)
    {
      eRTState = encoderScanUnblocked();
      if (eRTState != RT_NONE)
      {
        break;
      }
      delay (20);
    }
  }
  switch (eRTState)
  {
    case RT_INC:
    case RT_DEC:
#if SERIAL_PRINTS
      Serial.println("leave without changes from diagO2Sensor");
#endif
      break;
    case   RT_BT_PRESS:
      //diagSaveCalibData(O2,o2mVReading, o2Unitx10);
#if SERIAL_PRINTS
      Serial.println("save diagO2Sensor");
#endif
      break;
  }
#endif
}

void diagO2Sensor(void)
{
  displayChannelData(SENSOR_O2);
}
void diagAds1115(void)
{
  displayChannelData(SENSOR_PRESSURE_A0);
  displayChannelData(SENSOR_PRESSURE_A1);
  displayChannelData(SENSOR_DP_A0);
  displayChannelData(SENSOR_DP_A1);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ads1115 validated");
  delay(2000);
}
void sensorstatus(void)
{
  while (1) {
    RT_Events_T eRTState = encoderScanUnblocked();
      if (eRTState == RT_BT_PRESS)
      {
        break;
      }
     //lcd.clear();
     lcd.setCursor(6, 1);
     lcd.print("    ");
     lcd.setCursor(6, 2);
     lcd.print("    ");
     lcd.setCursor(3,0);
     lcd.print("Sensor millivolt");
     lcd.setCursor(0, 1);
     lcd.print("P0:");
     lcd.print(sM.read_sensor_rawvoltage(SENSOR_PRESSURE_A0));
    
     lcd.setCursor(10, 1);
     lcd.print("dp0:");
     lcd.print(sM.read_sensor_rawvoltage(SENSOR_DP_A0));
    
    
     lcd.setCursor(0, 2);
     lcd.print("P1:");
     lcd.print(sM.read_sensor_rawvoltage(SENSOR_PRESSURE_A1));
    
     lcd.setCursor(10, 2);
     lcd.print("dp1:");
     lcd.print(sM.read_sensor_rawvoltage(SENSOR_DP_A1));
    
     lcd.setCursor(7, 3);
     lcd.print("O2: ");
     lcd.print(sM.read_sensor_rawvoltage(SENSOR_O2));
    //lcd.print(o2value);
    delay(500);
  }
}


String rxdata_buff;
void serialEvent3() {
  while (Serial3.available()) {
    char inChar = (char)Serial3.read();
    Serial.print("R");
    if (inChar == '$') {
      comcnt = 1;
      rxdata_buff = "";
    }
    if  (comcnt >= 1) {
      rxdata_buff += inChar;
      comcnt = comcnt + 1;
      if (inChar == '&') {
        if (comcnt >= 10) {
          Ctrl_store_received_packet(rxdata_buff);
          gCntrlSerialEventRecvd = true;
          Serial.println("");
          Serial.println(rxdata_buff);
        }
      }
    }
  }
}
