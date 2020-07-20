#include <EEPROM.h>
#include <jm_Wire.h>
#include "pin_new.h"
#include "./lcd_display/ctrl_display.h"
#include "./memory/memory.cpp"
#include "./sensors/sensormanager.h"
#include "./sensors/sensormanager.cpp"
#include "./lcd_display/service_mode.h"
#include "./lcd_display/service_mode.cpp"
#include "./state_control/statecontrol.h"
#include "./state_control/statecontrol.cpp"
#include "./encoder/encoder.c"
#include "./lcd_display/ctrl_display.cpp"
#include  <avr/wdt.h>
#include "debug.h"  //to control debug related utilities, refer to the macro "VENT_DEBUG_LEVEL"

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

const uint8_t wdog_timer = WDTO_8S;

displayManager dM;

ErrorDef_T gErrorState = NO_ERR;

void setup() 
{
  int err = 0;
  WDT_Clear();
  Serial.begin(115200);

  VENT_DEBUG_ERROR("Initialization Started", 0);

  err = WDT_Cookie_Check();
  if (err == -1)
  {
    VENT_DEBUG_ERROR("WDOG Cookie check failed", err);
  }
  
  WDT_Set(wdog_timer); 
  VENT_DEBUG_ERROR("WDOG Timer enabled for value", wdog_timer);
    
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
  //pinMode(14, INPUT_PULLUP);
  //pinMode(15, INPUT_PULLUP);
  lcd.begin(LCD_LENGTH_CHAR, LCD_HEIGHT_CHAR);
  VENT_DEBUG_ERROR("LCD Module Init Done", 0);
  
  Wire.setClock(4000000L);
  Wire.begin();
  VENT_DEBUG_ERROR("Wire Init Done", 0);
  
  hbad_mem.begin();
  VENT_DEBUG_ERROR("Ext EEPROM Init Done", 0);
  
  Serial3.begin(115200);
  VENT_DEBUG_ERROR("Serial3 Init Done", 0);
  
  getAllParamsFromMem();
  VENT_DEBUG_ERROR("Parameter read from EEPROM Done", 0);
  
  err = sM.init();
  VENT_DEBUG_ERROR("Sensors Init Done", 0);

  if (err < 1)
  {
    VENT_DEBUG_ERROR("Sensors Init *Failed*", 0);
    //while(1);
  }
  
  delay(1000);

  attachInterrupt(digitalPinToInterrupt(DISP_ENC_SW), isr_processSwitch, FALLING );
  attachInterrupt(digitalPinToInterrupt(DISP_ENC_CLK), isrEncoderClk, RISING  );
  attachInterrupt(digitalPinToInterrupt(DISP_ENC_DT), isrEncoderDt, RISING  );
  VENT_DEBUG_ERROR("Enable Rotator Button Interrupts Done", 0);
  
  displayInitialScreen(dM);
  VENT_DEBUG_ERROR("Initial Screen Setup Done ", 0);
  
  checkAlarms();
  VENT_DEBUG_ERROR("Check Power Done ", 0);
  
  setup_service_mode();
  VENT_DEBUG_ERROR("Service Mode Set ", 0);
  
  sendDefaultParams();
  VENT_DEBUG_ERROR("Param Set Default - Done ", 0);

  VENT_DEBUG_ERROR("Initialization Complete ", 0);

  
} //end of setup

void sendDefaultParams()
{
  VENT_DEBUG_FUNC_START();
  
  Ctrl_send_packet(tidl_volu.parm_name, params[E_TV].value_curr_mem);
  delay(100);
  Ctrl_send_packet(resp_rate.parm_name, params[E_BPM].value_curr_mem);
  delay(100);
  Ctrl_send_packet(inex_rati.parm_name, params[E_IER].value_curr_mem);
  delay(100);
  sM.enable_sensor(O2);
  
  VENT_DEBUG_FUNC_END();
}

void checkAlarms() 
{
  VENT_DEBUG_FUNC_START();
  gErrorState = NO_ERR;

  int oxySupply = digitalRead(O2_CYN_SWITCH);
  int hospSwitch = digitalRead(O2_HOSP_SWITCH);
  
  if(breathCount > 2)
  {
    if (machineOn == true && oxySupply == LOW && hospSwitch == LOW) 
    {
       //gErrorState = ERR_OXY;
    }
    if (bvmFailure) 
    {
      gErrorState = ERR_BVM;
    }
    if (NO_ERR != gErrorState)
    {
      dM.errorDisplay(gErrorState);
      VENT_DEBUG_ERROR("Incorrect State", gErrorState);
    }
  }
  VENT_DEBUG_FUNC_END();
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

static unsigned long endtime = 0;
#define PRINT_PROCESSING_TIME 1
/* Project Main loop */
void loop() {
  
  int index = 0;
  int err = 0;
#if PRINT_PROCESSING_TIME
  unsigned long starttime = millis();
  Serial.print("L:strt_ts ");
  Serial.println(starttime);
#endif
  checkAlarms();
  wdt_reset();
  VENT_DEBUG_FUNC_START();

  sM.capture_sensor_data();

  for (; index < MAX_SENSORS; index++) 
  {
    err = sM.read_sensor_data(index, (float *) &(data_sensors[index]));
    if (err) 
    {
      VENT_DEBUG_ERROR("Sensors Read Failed for", index);
      VENT_DEBUG_ERROR("Error Code", err);
    }
  }
#if PRINT_PROCESSING_TIME
  Serial.print("sensor module processing time:");
  Serial.println((millis()-starttime));
  unsigned long dstarttime = millis();
#endif
  //VENT_DEBUG_ERROR("Error State: ", gErrorState);
  if (NO_ERR == gErrorState) 
  {
    dM.displayManagerloop(&data_sensors[0], sM);
  } 
  else 
  {
    dM.errorDisplay(gErrorState);
    gErrorState = NO_ERR;
  }
#if PRINT_PROCESSING_TIME
  Serial.print("display module processing time:");
  unsigned long ctrlsm_starttime = millis();  
  Serial.println((ctrlsm_starttime-dstarttime));
#endif
  if (gCntrlSerialEventRecvd == true) {
    gCntrlSerialEventRecvd = false;
    Ctrl_ProcessRxData();
  }
  
  Ctrl_StateMachine_Manager(&data_sensors[0], sM, dM);
#if PRINT_PROCESSING_TIME
  Serial.print("Ctrl_StateMachine_Manager processing time:");
  Serial.println(millis()- ctrlsm_starttime);
#endif  
  if (digitalRead(RESET_SWITCH) == LOW) 
  {
    //reset switch.
    if (machineOn == true) 
    {
      machineOn = false;
      Serial3.print(commands[STPR_STP]);
      Ctrl_Stop();
      breathCount = 0;
      gErrorState = NO_ERR;

    } 
    else if (machineOn == false) 
    {
      machineOn = true;
      Ctrl_Start();
      breathCount++;
    }
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
  }
  
  wdt_reset();  //Reset watchdog timer in case there is no failure in the loop
  VENT_DEBUG_ERROR("End of main process loop ", 0);
#if PRINT_PROCESSING_TIME  
  Serial.print("Main loop processing time:");
  endtime = millis();
  Serial.println((endtime - starttime));
  Serial.print("L:stp_ts ");
  Serial.println(endtime);
#endif  
  VENT_DEBUG_FUNC_END();
}


void displayChannelData(sensor_e sensor)
{
  unsigned int index = 0;
  int err = 0;
  RT_Events_T eRTState = RT_NONE;
  float sensor_data[MAX_SENSORS] = {0};
  int o2Unitx10;

  VENT_DEBUG_FUNC_START();
  
  checkAlarms();
  
  for (; index < MAX_SENSORS; index++) {
    err = sM.read_sensor_data((sensor_e)index, &(sensor_data[index]));
    if (err) {
       VENT_DEBUG_ERROR("Reading of Sensor Data Failed for Sensor", index);
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
      break;
    case RT_BT_PRESS:
      break;
    case RT_NONE:
      break;
    default:
      break;
  }

  VENT_DEBUG_FUNC_END();
}

void diagO2Sensor(void)
{
  VENT_DEBUG_FUNC_START();
  displayChannelData(SENSOR_O2);
  VENT_DEBUG_FUNC_END(); 
}


void diagAds1115(void)
{
  VENT_DEBUG_FUNC_START();
  displayChannelData(SENSOR_PRESSURE_A0);
  displayChannelData(SENSOR_PRESSURE_A1);
  displayChannelData(SENSOR_DP_A0);
  displayChannelData(SENSOR_DP_A1);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ads1115 validated");
  delay(2000);
  VENT_DEBUG_FUNC_END(); 
}

void sensorstatus(void)
{
  while (1) 
  {
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
#if 1
void serialEvent3() 
{
  VENT_DEBUG_FUNC_START();
  while (Serial3.available()) 
  {
    char inChar = (char)Serial3.read();
    Serial.print("Ro");
    if (inChar == '$') 
    {
      comcnt = 1;
      rxdata_buff = "";
    }
    if  (comcnt >= 1) 
    {
      rxdata_buff += inChar;
      comcnt = comcnt + 1;
      if (inChar == '&') 
      {
        if (comcnt >= 10) 
        {
          Ctrl_store_received_packet(rxdata_buff);
          gCntrlSerialEventRecvd = true;
          VENT_DEBUG_INFO("Received Packet", rxdata_buff);
        }
      }
    }
  }
  VENT_DEBUG_FUNC_END(); 
}
#endif
/*************************************** WDOG Functionality *********************************************/

void WDT_Clear(void)
{
  VENT_DEBUG_FUNC_START();
  noInterrupts();
  
  /* Clear WDRF in MCUSR */
  MCUSR &= ~(1<<WDRF);

  WDTCSR = (1 << WDCE);
  WDTCSR = 0;
  
  interrupts();
  VENT_DEBUG_FUNC_END();
}

void WDT_Set(const uint8_t val)
{
  VENT_DEBUG_FUNC_START();
  noInterrupts();
  
  /* Clear WDRF in MCUSR */
  MCUSR &= ~(1<<WDRF);

  /*WDOG Timer and Reset enable*/
  wdt_enable(val);
  
  /* Interrupt Enable */
  WDTCSR |= ((1 << WDCE) | (1 << WDIE));
  
  interrupts();
  VENT_DEBUG_FUNC_END();
}

int WDT_Cookie_Check(void)
{
  VENT_DEBUG_FUNC_START();
  char cookie = 'w';
  int result = 0;

  result = eeprom_ext_rw(EEPROM_WDT_DATA, &cookie, sizeof(char), EEPROM_READ); //Read the WDOG Cookie
  if (result != 0)
  {
    VENT_DEBUG_ERROR("Read of EEPROM failed in Watchdog", result);
    result = -1;
    goto wdog_err;
  }

  if (cookie == 'w')
  {
    VENT_DEBUG_ERROR("Reset due to WDOG - Cookie Detected", result);
    cookie ='r';
    result = eeprom_ext_rw(EEPROM_WDT_DATA, &cookie, sizeof(char), EEPROM_WRITE); //write "r" respresenting watchdog cookie is cleared
    if (result != 0)
    {
       VENT_DEBUG_ERROR("Write of EEPROM failed in Watchdog", result);
       result = -1;
       goto wdog_err;
    }
    
    result = eeprom_ext_rw(EEPROM_WDT_DATA + 1, &cookie, sizeof(char), EEPROM_READ); //read the state of the machine when the machine was reset by watchdog timer
    if (result == 0)
    {
      if (cookie == "r")//machine running when the watchdog triggered
      {
        machineOn == true;
      }
      else if(cookie == "s") //machine was in stop state when the watchdog triggered
      {
        machineOn = false;
      }
      else
      {
        VENT_DEBUG_ERROR("Invalid cookie present for the state in the EEPROM WDT: ", -1);
      }
    }
    else
    {
      VENT_DEBUG_ERROR("error reading the state from the EEPROM ", -1);
    }
    /* Update the State to Run the Machine */
    //TODO
  }
  else
  {
     VENT_DEBUG_ERROR("Regular Boot without WDT reset", result);
  }

wdog_err:
  return result;

}

//interrupt service routine for the watchdog timer
ISR(WDT_vect)
{
  VENT_DEBUG_FUNC_START();
  char data = 'w';
  int result = 0;
  result = eeprom_ext_rw(EEPROM_WDT_DATA, &data, sizeof(char), EEPROM_WRITE); //write "w" respresenting watchdog reset to EEPROM at the address EEPROM_WDT_DATA
  
  if(result == 0)
  {
    VENT_DEBUG_INFO("Write to EEPROM successful by the watchdog", result);
  }
  else
  {
    VENT_DEBUG_ERROR("Write to EEPROM failed in Watchdog", result);
  }
  //writing the state of the machine in the EEPROM
  if (machineOn == true)
  {
    data = "r"; // "r" respresents run state when the watchdog got triggered
  }
  else
  {
    data = "s"; //"s" respresents stop state when the watchdog was triggered
  }
  result = eeprom_ext_rw(EEPROM_WDT_DATA+1, &data, sizeof(char), EEPROM_WRITE); //write the state of the machine so that the machine can resume from the original state
  
  if(result == 0)
  {
    VENT_DEBUG_INFO("Writing state to EEPROM successful by the watchdog", result);
  }
  else
  {
    VENT_DEBUG_ERROR("Writing state to EEPROM failed in Watchdog", result);
  }
  
  VENT_DEBUG_FUNC_END();
}
