#include <Wire.h>
#include "pinout.h"
#include "ctrl_display.h"
#include "hbad_memory.h"
#include "./sensors/sensors.h"
#include "./sensors/sensors.cpp"
#include "Service_Mode.h"
#include "./StateControl/StateControl.h"
#include "./StateControl/StateControl.cpp"

int TimeSeries = 0;
volatile short currPos = 1;
unsigned short newIER = 1;
unsigned short newPeep = 5;

long int resetEditModetime;
int ctrlParamChangeInit = 0;
volatile int switchMode = 0;
volatile boolean actionPending = false;
int lastCLK;
int currentCLK;
String saveFlag = "Save  ";
String cancelFlag = "Cancel";
int currentSaveFlag = 1;
int gCtrlParamUpdated = 0;
#define ROT_ENC_FOR_IER (currPos == inex_rati.index)
#define ROT_ENC_FOR_PEEP (currPos == peep_pres.index)

#define READ_FROM_ENCODER ((switchMode == PAR_SELECTED_MODE) \
                           && (currPos >=0 && (currPos < MAX_CTRL_PARAMS) \
                               && (params[currPos].readPortNum == DISP_ENC_CLK)))

#define LCD_DISP_REFRESH_COUNT 5
#define EDIT_MODE_TIMEOUT 5000
#define SEND_CTRL_PARAMS_COUNT 15
#define SEND_SENSOR_VALUES_COUNT 5

int ContrlParamsSendCount = 0;
int SensorValuesSendCount = 0;

ErrorDef_T errorState = ERR_PWR_FAILURE;
bool machineOn = false;
//Need to Integrate into Main Code
bool compressionCycle = false;
bool expansionCycle = false;
bool homeCycle = false;
String rxdata;
int comcnt;

bool gCntrlSerialEventRecvd = false;

byte editSeletIndicator = 0;
byte editScrollIndex = 0;
bool menuChanged = false;
bool editSelectionMade = false;
#define CYLINDER 0
#define HOSP_LINE 1
bool o2LineSelect = CYLINDER;
bool tempO2LineSelect = 0;
char * o2LineString[2] = {"Cylinder", "Hospital line"};
bool o2LineChange = false;
bool powerSupplyFailure = false;
bool gasSuppluFailure = false;
bool mechFailSafeValve = false;
bool errorDisplayFlag = false;

unsigned int sensor_data[MAX_SENSORS] = {0};

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(AUTO_MODE, INPUT_PULLUP);
  pinMode(ASSISTED_MODE, INPUT_PULLUP);
  pinMode(RESET_SWITCH, INPUT_PULLUP);
  pinMode(DISP_ENC_CLK, INPUT);
  pinMode(DISP_ENC_DT, INPUT);
  pinMode(DISP_ENC_SW, INPUT_PULLUP);
  pinMode(ADS115_INT_PIN, INPUT_PULLUP);
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
  //attachInterrupt(digitalPinToInterrupt(DISP_ENC_SW), isr_processStartEdit, HIGH);
  getAllParamsFromMem();
  setup_service_mode();
  displayInitialScreen();
  checkAlarms();
  sensors_init();
  Serial.println("Exiting setup !");
}
boolean runInitDisplay = true;
void  checkAlarms() {
  int pwrSupply = digitalRead(POWER_SUPPLY_FAILURE);
  int gasSupply = digitalRead(GAS_SUPPLY_FAILURE);
  int mechFailSafe = digitalRead(MECH_FAILSAFE_VALVE);
  if (!pwrSupply) {
    errorDisplayFlag = true;
    errorState = ERR_PWR_FAILURE;
    errorDisplay();
  }
  if (!gasSupply) {
    errorDisplayFlag = true;
    errorState = ERR_AIR_SUPPLY;
    errorDisplay();
  } if (!mechFailSafe) {
    errorDisplayFlag = true;
    errorState = ERR_AIR_SUPPLY;
    errorDisplay();
  }

}



void errorDisplay()
{
  if (errorDisplayFlag)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("       ERROR");

    switch (errorState)
    {
      case (ERR_PWR_FAILURE):
        {
          lcd.setCursor(0, 1);
          lcd.print(POWER_FAILURE_MSG1);
          lcd.setCursor(0, 2);
          lcd.print(POWER_FAILURE_MSG2);
        }
        break;
      case (ERR_PATIENT_DISCONN):
        {
          lcd.setCursor(0, 1);
          lcd.print(POWER_FAILURE_MSG1);
          lcd.setCursor(0, 2);
          lcd.print(POWER_FAILURE_MSG2);
        }
        break;
      case (ERR_PEEP):
         {
          lcd.setCursor(0, 1);
          lcd.print(POWER_FAILURE_MSG1);
          lcd.setCursor(0, 2);
          lcd.print(POWER_FAILURE_MSG2);
        }
        break;
      case (ERR_AIR_SUPPLY):
         {
          lcd.setCursor(0, 1);
          lcd.print(POWER_FAILURE_MSG1);
          lcd.setCursor(0, 2);
          lcd.print(POWER_FAILURE_MSG2);
        }
        break;
      case (ERR_TV):
         {
          lcd.setCursor(0, 1);
          lcd.print(POWER_FAILURE_MSG1);
          lcd.setCursor(0, 2);
          lcd.print(POWER_FAILURE_MSG2);
        }
        break;
      default:
        break;
    }
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
    runInitDisplay = true;
    errorDisplayFlag = false;
    //lcd.clear();
  }
  delay(1000);
}
void displayRunTime()
{
  String row0 = "       STATUS       ";
  String row1 = "TV:XXXmL      BPM:YY";
  String row2 = "FiO2:XX%   IER:YYYYY";
  String row3 = "PEEP:XX IP:YY  EP:ZZ";
  
  if (runInitDisplay)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(row0);
 #if 1
    lcd.setCursor(0,1);
    lcd.print(row1);
    lcd.setCursor(0,2);
    lcd.print(row2);
    lcd.setCursor(0,3);
    lcd.print(row3);
     
    
 #endif
    runInitDisplay = false;
  }


  #ifdef OLDSTYLE
  // cleanRow(1); cleanRow(2); cleanRow(3);
  String row1 = "TV:";
  row1 += params[TIDAL_VOL].value_curr_mem;
  while (row1.length() < 6)
  {
    row1 += " ";
  }
  row1 += "mL BPM:";
  row1 += params[BPM].value_curr_mem;

  while (row1.length() < 20)
  {
    row1 += " ";
  }
  lcd.setCursor(0, 1);
  lcd.print(row1);

  String row2 = "FiO2:21";
  //row2 += (PS_ReadSensorValueX10(O2)) / 10;
  row2 += "%";
  //Serial.println((PS_ReadSensorValueX10(O2)) / 10);
  while (row2.length() < 8)
  {
    row2 += " ";
  }
  row2 += "     IER:1:";
  row2 += params[IE_RATIO].value_curr_mem;
  lcd.setCursor(0, 2);
  lcd.print(row2);


  String row3 = "PEEP:";
  row3 += params[PEEP_PRES].value_curr_mem;

  while (row3.length() < 13)
  {
    row3 += " ";
  }

  row3 += "  IP:";
  row3 += sensor_data[SENSOR_PRESSURE_A0];
  while (row3.length() < 14)
  {
    row3 += " ";
  }

  row3 += " EP:";
  row3 += sensor_data[SENSOR_PRESSURE_A1];
  while (row3.length() < 20)
  {
    row3 += " ";
  }
  lcd.setCursor(0, 3);
  lcd.print(row3);

  #else
//  lcdinsertword("        Status            ");
  //lcdinsertword("TV:"+  params[TIDAL_VOL].value_curr_mem);
  //lcdinsertword( "mL BPM:" + params[BPM].value_curr_mem);
  //lcdinsertword("FiO2:21%");

//lcdinsertword("IER:1:"+ params[IE_RATIO].value_curr_mem);

  //lcdinsertword("PEEP:"+ params[PEEP_PRES].value_curr_mem);
  //lcdinsertword("IP:" + (String)read_and_clear_sensor_data(SENSOR_PRESSURE_A0));
  //lcdinsertword("harnoor");

  //Serial.print ("row1.indexOf(X,0)");
  //Serial.print (row1.indexOf("X",0));
  //Serial.print("params[IE_RATION].value_curr_mem=");
  //Serial.println( params[IE_RATIO].value_curr_mem );
  
//insertWord(1,row1.indexOf("X",0),(String)params[TIDAL_VOL].value_curr_mem,5);
insertWord(1,row1.indexOf("X",0),(String)(int)sensor_data[SENSOR_DP_A2],3);//dummy

//insertWord(1,row1.indexOf("Y",0),(String)params[BPM].value_curr_mem,5);


//insertWord(1,row1.indexOf("Y",0),(String)"77",2);//dummy

insertWord(1,row1.indexOf("Y",0),(String)"7",2);//dummy

insertWord(2,row2.indexOf("X",0),"21",2);//dummy fio2
//insertWord(2,row2.indexOf("Y",0),(String)params[IE_RATIO].value_curr_mem,5);
insertWord(2,row2.indexOf("Y",0),(String)"1:1.2",5);//dummy ier
//insertWord(3,row3.indexOf("X",0),(String) params[PEEP_PRES].value_curr_mem,5);
insertWord(3,row3.indexOf("X",0),(String) "20",2);//peep dummy
insertWord(3,row3.indexOf("Y",0),(String)(int)( sensor_data[SENSOR_PRESSURE_A0]),2);
insertWord(3,row3.indexOf("Z",0),(String)(int)( sensor_data[SENSOR_PRESSURE_A0]),2);//dummy for EP



  #endif
}


/* Project Main loop */

void loop() {
  RT_Events_T eRTState = RT_NONE;

  checkAlarms();
  
  enable_sensor(PRESSURE_A0 | DP_A2);
  for(int index = 0; index < MAX_SENSORS; index++) {
    sensor_data[index] = read_sensor_data(index);
  }
  displayRunTime();
  //checkSendDataToGraphicsDisplay();

  eRTState = encoderScanUnblocked();
  if (eRTState == RT_BT_PRESS) {
    if (millis() - resetEditModetime > 500) {
      Serial.println("Entering Edit mode!");
      resetEditModetime = millis();
      displayEditMenu();
      gCtrlParamUpdated = 1;
      editSeletIndicator = 0;
      editScrollIndex = 0;
      runInitDisplay = true;
      resetEditModetime = millis();
    }
  }
  if (gCntrlSerialEventRecvd == true) {
    gCntrlSerialEventRecvd = false;
    Ctrl_ProcessRxData();
  }
  Ctrl_StateMachine_Manager(&sensor_data[0]);
  if (digitalRead(RESET_SWITCH) == LOW) {
    //reset switch.
    if (machineOn == true) {
      machineOn = false;
      Ctrl_Stop();
    } else if(machineOn == false) {
      machineOn = true;
      Ctrl_Start();
    }
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
  }
  delay(3);
}


typedef enum {
  E_EXIT,
  E_TV,
  E_BPM,
  E_FiO2,
  E_IER,
  E_PEEP,
  E_PIP,
  E_O2_INPUT,
  MAX_EDIT_MENU_ITEMS
} editMenu_T;

char* mainEditMenu[MAX_EDIT_MENU_ITEMS] = {"EXIT EDIT MENU", "TV", "BPM", "FiO2", "IER", "PEEP", "PIP", "O2 in"};
editMenu_T currentEditMenuIdx = MAX_EDIT_MENU_ITEMS;



void printEditMenu( void)
{
  String strOnLine234;
#if SERIAL_PRINTS
  Serial.println(editSeletIndicator);
  Serial.println(editScrollIndex);
#endif
  lcd.clear();
  for (int i = 0; i <= LCD_HEIGHT_CHAR - 1; i++) //menuItems[menuIdx].menuLength; i++)
  {
    lcd.setCursor(0, i);
    if (editSeletIndicator == i)
    {
      strOnLine234 = ">";
    }
    else
    {
      strOnLine234 = " ";
    }
    strOnLine234 += mainEditMenu[editScrollIndex + i];
    if (editScrollIndex + i != 0)
    {
      strOnLine234 += ":";
    }
    switch (editScrollIndex + i)
    {
      case (E_TV):
        strOnLine234 += params[TIDAL_VOL].value_curr_mem;
        strOnLine234 += "mL";
        break;
      case (E_BPM):
        strOnLine234 += params[BPM].value_curr_mem;
        break;
      case (E_FiO2):
        strOnLine234 += params[FIO2_PERC].value_curr_mem;
        strOnLine234 += "%";
        break;
      case (E_IER):
        strOnLine234 += " 1:";
        strOnLine234 += params[IE_RATIO].value_curr_mem;
        break;
      case (E_PEEP):
        strOnLine234 += params[PEEP_PRES].value_curr_mem;
        strOnLine234 += "cmH2O";
        break;
      case (E_PIP):
        strOnLine234 += params[PEAK_PRES].value_curr_mem;
        strOnLine234 += "cmH2O";
        break;
      case (E_O2_INPUT):
        strOnLine234 += o2LineString[o2LineSelect];
        break;
    }

    lcd.print (strOnLine234);
#if SERIAL_PRINTS
    Serial.println(strOnLine234);
#endif
  }
}


void moveUpEdit()
{
  editSeletIndicator++;
  /*
     check wrap around of select indicator
  */
  if ((editSeletIndicator >= MAX_EDIT_MENU_ITEMS) ||
      (editSeletIndicator > LCD_HEIGHT_CHAR - 1))
  {
    editSeletIndicator = min(LCD_HEIGHT_CHAR - 1, MAX_EDIT_MENU_ITEMS);
    if (editSeletIndicator == LCD_HEIGHT_CHAR - 1)
    {
      editScrollIndex++;
    }
    if ((editScrollIndex + editSeletIndicator) >= MAX_EDIT_MENU_ITEMS)
    {
      editScrollIndex = MAX_EDIT_MENU_ITEMS - editSeletIndicator - 1;
    }
  }
}

void moveDownEdit()
{
  /*
     check wrap around of select indicator
  */
  if (editSeletIndicator == 0 )
  {
    if (editScrollIndex > 0)
    {
      editScrollIndex--;
    }
  }
  else
  {
    editSeletIndicator--;
  }
}

void selectionEdit()
{
  //  if ((editSeletIndicator + editScrollIndex) != MAX_EDIT_MENU_ITEMS)
  //  {
  //    //we are already in intial edit menu
  //    lcd.clear();
  //    lcd.setCursor(0, 1);
  //    lcd.print("You selected:");
  //    lcd.setCursor(0, 2);
  //    lcd.print(mainEditMenu[editSeletIndicator + editScrollIndex]);
  //    delay(1000);
  //    lcd.clear();
  //  }
  lcd.clear();
  editSelectionMade = true;
}



void editModeEncoderInput(void)
{
  RT_Events_T eRTState = RT_NONE;
  eRTState = encoderScanUnblocked();
  switch (eRTState)
  {
    case RT_INC:
      moveUpEdit();
      break;
    case   RT_DEC:
      moveDownEdit();
      break;
    case   RT_BT_PRESS:
      selectionEdit();
      break;
  }
  if (eRTState != RT_NONE)
  {
    resetEditModetime = millis();
    menuChanged = true;
  }
}

long unsigned lastDisplayTime = 0;
int oldValue;
void showSaveSelectedParam()
{
  RT_Events_T eRTState = RT_NONE;
  eRTState = encoderScanUnblocked();
  switch (eRTState)
  {
    case RT_INC:
      currentSaveFlag = false;
      break;
    case   RT_DEC:
      currentSaveFlag = true;
      break;
    case   RT_BT_PRESS:
      switchMode = PAR_SAVE_MODE;
      saveSelectedParam();
      editSelectionMade = false;
      break;
  }
  if (eRTState != RT_NONE)
  {
    resetEditModetime = millis();
  }
  if ((millis() - lastDisplayTime > 500) ||
      (eRTState != RT_NONE))
  {
    lastDisplayTime = millis();
    //Serial.println("in showSaveSelectedParam");
    //Serial.println(currPos);
    if (o2LineChange)
    {
      // change tempO2LineSelect
      o2LineChanger();
    }
    else
    {
      lcd.setCursor(8, 0);
      lcd.print(params[currPos].parm_name);

      if (currPos == FIO2_PERC)
      {
        lcd.setCursor(0, 2);
        lcd.print("Set using FiO2 valve");
      }
      else if (currPos == PEEP_PRES)
      {
        lcd.setCursor(0, 2);
        lcd.print("Set using PEEP valve");
      }
      else
      {
        if (currPos == inex_rati.index || currPos == peep_pres.index) {
          abc();
          return;
        }
        params[currPos].value_new_pot = analogRead(params[currPos].readPortNum);
        lcd.setCursor(0, 1);
        lcd.print("New: ");
        lcd.print("   ");
        lcd.setCursor(8, 1);
        int actualValue = getCalibValue(params[currPos].value_new_pot, currPos);
        printPadded(actualValue);
        lcd.setCursor(15, 1);
        lcd.print(params[currPos].units);
        lcd.setCursor(0, 2);
        lcd.print("Old: ");
        lcd.setCursor(8, 2);
        printPadded(params[currPos].value_curr_mem);
        lcd.setCursor(15, 2);
        lcd.print(params[currPos].units);
        lcd.setCursor(0, 3);
        if (currentSaveFlag == true) {
          lcd.print(SAVE_FLAG_CHOSEN);
          lcd.print("    ");
          lcd.print(CANC_FLAG);
        } else {
          lcd.print(SAVE_FLAG);
          lcd.print("    ");
          lcd.print(CANC_FLAG_CHOSEN);
        }

        int diffValue = abs(oldValue - params[currPos].value_new_pot);
        //Serial.print("diffValue "); Serial.println(diffValue);
        if (diffValue > 5)
        {
          resetEditModetime = millis();
        }
        oldValue = params[currPos].value_new_pot;
      }
    }
    //Serial.println("exiting showSaveSelectedParam");
  }
}

void displayEditMenu(void)
{
  menuChanged = true;
  bool continueEditModeFlag = true;
  editSelectionMade = false;
  currentSaveFlag = false;
  do {
    if (editSelectionMade == false)
    {
      //display prameters if changed.
      if (menuChanged)
      {
        printEditMenu();
        menuChanged = false;
      }
      //get parameters changes in unblocked manner.
      editModeEncoderInput();
    }
    else
    {
      //act on the input
      switch (editSeletIndicator + editScrollIndex)
      {
        case (E_EXIT):
          continueEditModeFlag = false;
          break;
        case (E_TV):
          currPos = TIDAL_VOL;
          break;
        case (E_BPM):
          currPos = BPM;
          break;
        case (E_FiO2):
          currPos = FIO2_PERC;
          break;
        case (E_IER):
          currPos = IE_RATIO;
          break;
        case (E_PEEP):
          currPos = PEEP_PRES;
          break;
        case (E_PIP):
          currPos = PEAK_PRES;
          break;
        case (E_O2_INPUT):
          o2LineChange = true;
          break;
        default:
          break;
      }
      if (continueEditModeFlag == true)
      {
        switchMode = PAR_SELECTED_MODE;
        showSaveSelectedParam();
      }
    }
    delay(1);
  } while (((millis() - resetEditModetime) < EDIT_MODE_TIMEOUT) && (continueEditModeFlag));
}

void saveSelectedParam() {
  if (switchMode == PAR_SAVE_MODE) {
    //    Serial.println(params[currPos].value_new_pot);
    lcd.setCursor(0, 3);
    lcd.print(params[currPos].parm_name);
    String command;
    String param;
    if (o2LineChange)
    {
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print(" saved          ");
      if (o2LineSelect != tempO2LineSelect)
      {
        o2LineSelect = tempO2LineSelect;
        if (o2LineSelect == CYLINDER)
        {
          Ctrl_send_packet(OXY_SOLE_HOS_O2_OFF);
          delay(10);
          Ctrl_send_packet(OXY_SOLE_CYL_ONN);
        }
        else //HOSP_LINE
        {
          Ctrl_send_packet(OXY_SOLE_CYL_OFF);
          delay(10);
          Ctrl_send_packet(OXY_SOLE_HOS_O2_ONN);
        }
      }
      digitalWrite(BUZZER_PIN, HIGH);
      delay(500);
      digitalWrite(BUZZER_PIN, LOW);
      return;
    }

    if ((ROT_ENC_FOR_PEEP) ||
        (currPos == FIO2_PERC))
    {
      lcd.setCursor(0, 3);
      lcd.print("   going back....");
      delay(500);
      return;
    }

    if (ROT_ENC_FOR_IER || ROT_ENC_FOR_PEEP) {
      params[currPos].value_curr_mem = getCalibratedParamFromPot(params[currPos]);
      storeParam(params[currPos]);
      Ctrl_send_packet(params[currPos].parm_name, params[currPos].value_curr_mem);
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print(" saved          ");
      digitalWrite(BUZZER_PIN, HIGH);
      delay(500);
      digitalWrite(BUZZER_PIN, LOW);
      return;
    }
    if (currentSaveFlag == 0) {
      lcd.print(" Edit cancelled.");
    } else {

      params[currPos].value_curr_mem = getCalibratedParamFromPot(params[currPos]);
      storeParam(params[currPos]);
      Ctrl_send_packet(params[currPos].parm_name, params[currPos].value_curr_mem);
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print(" saved.....");
      digitalWrite(BUZZER_PIN, HIGH);
    }
    actionPending = false;
    switchMode = DISPLAY_MODE;
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
    cleanRow(3);
    resetEditModetime = millis();
  }
}

void isr_processStartEdit() {
  static unsigned long lastSwitchTime = 0;
  unsigned long switchTime = millis();
  if ((switchTime - lastSwitchTime) < DBNC_INTVL_SW) {
    return;
  }
  switchMode = (switchMode + 1) % 4;
  resetEditModetime = millis();
  actionPending = 1;
  lastSwitchTime = switchTime;
}
/*
   The getCalibValue is for calibrating any integer to a parameter.
   The parameter is input with the help of an index
*/
int getCalibValue(int potValue, int paramIndex) {
  float convVal = map(potValue, 0, POT_HIGH, params[paramIndex].min_val, params[paramIndex].max_val);
  return ((int)(convVal / params[paramIndex].incr) + 1) * params[paramIndex].incr;
}

/*
   The below method is for a specific parameter
*/
int getCalibratedParamFromPot(ctrl_parameter_t param) {
  if (param.readPortNum == DISP_ENC_CLK) {
    return param.value_curr_mem;
  }
  float convVal = map(param.value_new_pot, 0, POT_HIGH, param.min_val, param.max_val);
  return ((int)(convVal / param.incr) + 1) * param.incr;
}

int rectifyBoundaries(int value, int minimum, int maximum) {
  if (value < minimum) {
    return maximum;
  }
  if (value > maximum) {
    return minimum;
  }
  return value;
}


void displayChannelData(sensor_e sensor)
{
  int o2mVReading, o2Unitx10;
  RT_Events_T eRTState = RT_NONE;
#if SERIAL_PRINTS
  Serial.println("we are in diagO2Sensor");
#endif

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
    if (sensor == O2)
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
}

void diagO2Sensor(void)
{
  displayChannelData(SENSOR_O2);
}
void diagAds1115(void)
{
  displayChannelData(SENSOR_PRESSURE_A0);
  displayChannelData(SENSOR_PRESSURE_A1);
  displayChannelData(SENSOR_DP_A2);
  displayChannelData(SENSOR_DP_A3);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ads1115 validated");
  delay(2000);
}
void diagSolStatus(void)
{
  //  Serial.println("we are in diagSolStatus");
}


String rxdata_buff;
void serialEvent2() {
  while (Serial2.available()) {
    char inChar = (char)Serial2.read();
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
        }
      }
    }
  }
}

void o2LineChanger() {
  RT_Events_T eRTState;
  lastDisplayTime = millis();
  resetEditModetime = millis();
  do {
    eRTState = encoderScanUnblocked();
    if (eRTState != RT_NONE)
    {
      resetEditModetime = millis();
    }
    switch (eRTState)
    {
      case RT_INC:
        tempO2LineSelect = 1;
        break;
      case   RT_DEC:
        tempO2LineSelect = 0;
        break;
      case   RT_BT_PRESS:
        currentSaveFlag = true;
        switchMode = PAR_SAVE_MODE;
        saveSelectedParam();
        editSelectionMade = false;
        o2LineChange = false;
        return;
        break;
    }
    if ((millis() - lastDisplayTime > 500) ||
        (eRTState != RT_NONE))
    {
      lcd.setCursor(0, 1);
      if (tempO2LineSelect == 0)
      {
        lcd.print("< O2 Cylinder >  ");
        lcd.setCursor(0, 2);
        lcd.print("  Hospital Line  ");
      }
      else
      {
        lcd.print("  O2 Cylinder    ");
        lcd.setCursor(0, 2);
        lcd.print("< Hospital Line > ");
      }
      lastDisplayTime = millis();
    }
  } while ((millis() - resetEditModetime) < EDIT_MODE_TIMEOUT);
}

void abc() {
  RT_Events_T eRTState;
  lastDisplayTime = millis();
  int oldIER = params[inex_rati.index].value_curr_mem;
  resetEditModetime = millis();
  do {
    int incr = 0;
    eRTState = encoderScanUnblocked();
    if (eRTState != RT_NONE)
    {
      resetEditModetime = millis();
    }
    switch (eRTState)
    {
      case RT_INC:
        incr++;
        break;
      case   RT_DEC:
        incr--;
        break;
      case   RT_BT_PRESS:
        currentSaveFlag = true;
        switchMode = PAR_SAVE_MODE;
        saveSelectedParam();
        editSelectionMade = false;
        return;
        break;
    }
    if ((millis() - lastDisplayTime > 500) ||
        (incr != 0))
    {
      lastDisplayTime = millis();
      if (ROT_ENC_FOR_IER) {
        newIER = rectifyBoundaries(newIER + incr, inex_rati.min_val, inex_rati.max_val);
        //      cleanRow(1);
        ; lcd.setCursor(VALUE1_DISPLAY_POS, 1);
        lcd.setCursor(3, 1);
        lcd.print("New IER  1:");
        lcd.print(newIER);
        lcd.setCursor(3, 2);
        lcd.print("Old IER  1:");
        lcd.print(oldIER);
        params[inex_rati.index].value_new_pot = newIER;
        params[inex_rati.index].value_curr_mem = newIER;
      } else if (ROT_ENC_FOR_PEEP) {
        newPeep = rectifyBoundaries(newPeep + incr * peep_pres.incr, peep_pres.min_val, peep_pres.max_val);
        params[peep_pres.index].value_new_pot = newPeep;
        params[peep_pres.index].value_curr_mem = newPeep;
        lcd.setCursor(VALUE1_DISPLAY_POS, 1);
        printPadded(newPeep);
        lcd.setCursor(0, 3);
        lcd.print("set using PEEP valve");
      }
      incr = 0;
    }
    lastCLK = currentCLK;
  } while ((millis() - resetEditModetime) < EDIT_MODE_TIMEOUT);
}
