#include "ctrl_display.h"

static const char * o2LineString[2] = {"Cylinder", "HospitalLine"};
String saveFlag = "Save  ";
String cancelFlag = "Cancel";

// macros here
#define ROT_ENC_FOR_IER (_currItem == inex_rati.index)
#define ROT_ENC_FOR_PEEP (_currItem == peep_pres.index)
#define EDIT_MENU_TIMEOUT 5000
int oldValue;
extern bool machineOn;

extern bool refreshfullscreen_inhale ;
extern bool refreshfullscreen_exhale ;
extern unsigned long exhale_refresh_timeout ;


byte fiChar[] = {
  B11110,
   B10000,
   B10001,
   B11100,
   B10001,
   B10001,
   B10001,
   B00000

};
 
byte upArrowChar[] = {
  B11111,
  B11011,
  B10001,
  B01010,
  B11011,
  B11011,
  B11011,
  B11111
};
byte upTriaChar[] = {
  B00100,
  B01110,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000
};
  byte dwnTriaChar[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B01110,
  B00100
};
  byte emDnChar[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B10001,
  B01010,
  B00100
};
byte upInvChar[] = {
  B00000,
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00000
};
byte emUpChar[] = {
  B00100,
  B01010,
  B10001,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000
};

typedef enum 
{
  DP_FI=0,
  DP_UP,
  DP_UP_TR,
  DP_DW_TR,
  DP_EM_DN_TR,
  DP_UP_INV,
   DP_EM_UP_TR
}DP_CH_T;

void displayManager::setDisplayParam(eDisplayPrm param, float value) {
  VENT_DEBUG_FUNC_START();
  
  VENT_DEBUG_INFO ("Param", value);
  
  switch(param) {
    case DISPLAY_TVE:
      m_display_tve = value;
      break;
    case DISPLAY_TVI:
      m_display_tvi = value;
      break;
    case DISPLAY_PEEP:
      m_display_peep = value;
      break;
    case DISPLAY_PLAT:
      m_display_plat = value;
      break;
    case DISPLAY_PIP:
      m_display_pip = value;
      break;
  }
  VENT_DEBUG_FUNC_END();
}

float displayManager::getDisplayParam(eDisplayPrm param) {
  switch(param) {
    case DISPLAY_TVI:
      return  m_display_tvi;
    default:
      return -1;
  }
 }

void displayManager::drawSensorValueMenu(RT_Events_T eRTState) {
    _lastSubEditMenuTime = millis();
    bool bCalibrate = false;
    m_sM->enable_sensor(PRESSURE_A0 | DP_A0 | O2 | PRESSURE_A1 | DP_A1);
    switch (eRTState)
    {
      case RT_INC:
        _currentSaveFlag = false;
        break;
      case   RT_DEC:
        _currentSaveFlag = true;
        break;
      case   RT_BT_PRESS:
        bCalibrate = true;
        _bBack2EditMenu = true;
        break;
	  case RT_NONE:
			break;
	  default:
			break;
     }
    if ((millis() - _lastDisplayTime > 500) ||
        (eRTState != RT_NONE))
    {
      //lcd.clear();
      lcd.setCursor(6, 1);
      //lcd.print("    ");
      lcd.setCursor(6, 2);
      //lcd.print("    ");
      lcd.setCursor(3,0);
      lcd.print("Sensor Pressure");
      lcd.setCursor(0, 1);
      lcd.print("P0:");
      lcd.print(m_sM->read_sensor_pressurevalues(SENSOR_PRESSURE_A0));
      lcd.setCursor(10, 1);
      lcd.print("dp0:");
      lcd.print(m_sM->read_sensor_pressurevalues(SENSOR_DP_A0));
      lcd.setCursor(0, 2);
      lcd.print("P1:");
      lcd.print(m_sM->read_sensor_pressurevalues(SENSOR_PRESSURE_A1));
      lcd.setCursor(10, 2);
      lcd.print("dp1:");
      lcd.print(m_sM->read_sensor_pressurevalues(SENSOR_DP_A1));
      lcd.setCursor(0, 3);
      if (true == _currentSaveFlag) {
        lcd.print(CALIB_FLAG_CHOSEN);
        lcd.print("  ");
        lcd.print(CANC_FLAG);
      } else {
        lcd.print(CALIB_FLAG);
        lcd.print("  ");
        lcd.print(CANC_FLAG_CHOSEN);
      }
    }
  if (bCalibrate) {
    if (_currentSaveFlag == 0) {
      lcd.print(" .....cancelled");
      m_sM->enable_sensor(0);
    } else {
      m_sM->enable_sensor(0);
      m_sM->start_calibration();
      m_sM->enable_sensor(0);
      lcd.setCursor(0, 3);
      lcd.print(" calibrated....");
      digitalWrite(BUZZER_PIN, HIGH);
      delay(500);
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}

void displayManager::drawSensorvoltageMenu(RT_Events_T eRTState) {
  _lastSubEditMenuTime = millis();
  m_sM->enable_sensor(PRESSURE_A0 | DP_A0 | O2 | PRESSURE_A1 | DP_A1);
  switch (eRTState)
  {
    case RT_INC:
      break;
    case   RT_DEC:
      break;
    case   RT_BT_PRESS:
      m_sM->enable_sensor(0);
      _bBack2EditMenu = true;
      break;
	case RT_NONE:
			break;
	default:
			break;
   }
  if ((millis() - _lastDisplayTime > 500) ||
      (eRTState != RT_NONE))
  {
     int o2value = analogRead(OXYGEN_ANALOG_PIN);
     //lcd.clear();
     lcd.setCursor(6, 1);
     lcd.print("    ");
     lcd.setCursor(6, 2);
     lcd.print("    ");
     lcd.setCursor(1,0);
     lcd.print("Sensor mV,Press2Exit");
     lcd.setCursor(0, 1);
     lcd.print("P0:");
     lcd.print(m_sM->read_sensor_rawvoltage(SENSOR_PRESSURE_A0));
    
     lcd.setCursor(10, 1);
     lcd.print("dp0:");
     lcd.print(m_sM->read_sensor_rawvoltage(SENSOR_DP_A0));
    
     lcd.setCursor(0, 2);
     lcd.print("P1:");
     lcd.print(m_sM->read_sensor_rawvoltage(SENSOR_PRESSURE_A1));
    
     lcd.setCursor(10, 2);
     lcd.print("dp1:");
     lcd.print(m_sM->read_sensor_rawvoltage(SENSOR_DP_A1));
    
     lcd.setCursor(7, 3);
     lcd.print("O2: ");
     lcd.print(o2value);
  }
}


void displayManager::drawUpdateO2_InputMenu(RT_Events_T eRTState) {
  static bool tempO2LineSelect;
  bool bSave = false;
  switch (eRTState)
  {
    case RT_INC:
      tempO2LineSelect = 1;
      break;
    case   RT_DEC:
      tempO2LineSelect = 0;
      break;
    case   RT_BT_PRESS:
      bSave = true;
      _bBack2EditMenu = true;
      break;
	case RT_NONE:
			break;
	default:
			break;
   }
  if (((millis() - _lastDisplayTime) > 300) ||
      (eRTState != RT_NONE))
  {
    _lastDisplayTime = millis();
	VENT_DEBUG_INFO("O2 Changer", 0);

    lcd.setCursor(0, 1);
    if (tempO2LineSelect == 0)
    {
      lcd.print("< O2 Cylinder >  ");
      lcd.setCursor(0, 2);
      lcd.print("  HospitalLine  ");
    }
    else
    {
      lcd.print("  O2 Cylinder    ");
      lcd.setCursor(0, 2);
      lcd.print("< HospitalLine > ");
    }
  }
  if (true == bSave) {
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print(" saved          ");

    _o2LineSelect = tempO2LineSelect;
    if (_o2LineSelect == CYLINDER)
    {
      //Ctrl_send_packet(OXY_SOLE_HOS_O2_OFF);
      //delay(10);
      Ctrl_send_packet(OXY_SOLE_CYL_ONN);
    }
    else //HOSP_LINE
    {
      //Ctrl_send_packet(OXY_SOLE_CYL_OFF);
      //delay(10);
      Ctrl_send_packet(OXY_SOLE_HOS_O2_ONN);
    }
    params[_currItem].value_curr_mem = _o2LineSelect;
    storeParam(params[_currItem]);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
 }
}

void displayManager::drawUpdatePEEPorIERMenu(RT_Events_T eRTState) {
  int oldIER = params[inex_rati.index].value_curr_mem;
  static int incr = 0;
  bool bSave = false;
  switch (eRTState)
  {
    case RT_INC:
      incr++;
      break;
    case   RT_DEC:
      incr--;
      break;
    case   RT_BT_PRESS:
       bSave = true;
      _bBack2EditMenu = true;
      if (ROT_ENC_FOR_IER)
          params[inex_rati.index].value_curr_mem = _newIER;
      if (ROT_ENC_FOR_PEEP)
          params[peep_pres.index].value_curr_mem = _newPeep;
      break;
	case RT_NONE:
			break;
	default:
			break;
  }
  if ((millis() - _lastDisplayTime > 300) ||
      (incr != 0)) {
    _lastDisplayTime = millis();
    if (ROT_ENC_FOR_IER) {
      _newIER = rectifyBoundaries(_newIER + incr, inex_rati.min_val, inex_rati.max_val);
      lcd.setCursor(VALUE1_DISPLAY_POS, 1);
      lcd.setCursor(3, 1);
      lcd.print("New IER  1:");
      lcd.print(_newIER);
      lcd.setCursor(3, 2);
      lcd.print("Old IER  1:");
      lcd.print(oldIER);
      params[inex_rati.index].value_new_pot = _newIER;
    } else if (ROT_ENC_FOR_PEEP) {
      _newPeep = rectifyBoundaries(_newPeep + incr * peep_pres.incr, peep_pres.min_val, peep_pres.max_val);
      params[peep_pres.index].value_new_pot = _newPeep;
      lcd.setCursor(VALUE1_DISPLAY_POS, 1);
      printPadded(_newPeep);
      lcd.setCursor(0, 3);
      lcd.print("set using PEEP valve");
    }
    incr = 0;
  }

  if ( true == bSave ) {
    params[_currItem].value_curr_mem = getCalibratedParamFromPot(params[_currItem]);
    storeParam(params[_currItem]);
    Serial.print(_currItem);
    Ctrl_send_packet(params[_currItem].parm_name, params[_currItem].value_curr_mem);
    lcd.setCursor(0, 3);
    lcd.print("                    ");
    lcd.setCursor(0, 3);
    lcd.print(" saved          ");
    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
    return;
  }
}

void displayManager::drawUpdateFiO2Menu(RT_Events_T eRTState)
{
  bool bSave = false;
  if ( 0 == _o2_en_flag) {
    unsigned int _o2_en_flag = sM.get_enable_sensors();
    if ((_o2_en_flag & O2) == 0) { 
      _o2_en_flag = _o2_en_flag | O2;
      sM.enable_sensor(_o2_en_flag);
    }
  }
  switch (eRTState)
  {
    case RT_INC:
      _currentSaveFlag = false;
      break;
    case   RT_DEC:
      _currentSaveFlag = true;
      break;
    case   RT_BT_PRESS:
      _o2_en_flag = 0;
      _bBack2EditMenu = true;
      bSave = true;
      break;
	case RT_NONE:
			break;
	default:
			break;
  }
  if ((millis() - _lastDisplayTime > 300) ||
      (eRTState != RT_NONE))
    {
      _lastDisplayTime = millis();
      lcd.setCursor(0, 0);
      lcd.print(" Live O2  : ");
      float data = 0.0;
      sM.read_sensor_data(SENSOR_O2,&data);
      lcd.print((int)data);
      lcd.print("%");
      Serial.print("reading from Pot :");
      params[_currItem].value_new_pot = analogRead(params[_currItem].readPortNum);
      Serial.println(params[_currItem].value_new_pot);
      lcd.setCursor(0, 1);
      lcd.print(" New      : ");
      lcd.print(" ");
      lcd.setCursor(11, 1);
      int actualValue = getCalibValue(params[_currItem].value_new_pot, _currItem);
      printPadded(actualValue);
      //lcd.setCursor(15, 1);
      lcd.print(params[_currItem].units);
      lcd.setCursor(0, 2);
      lcd.print(" Old      : ");
      lcd.setCursor(11, 2);
      printPadded(params[_currItem].value_curr_mem);
      //lcd.setCursor(15, 2);
      lcd.print(params[_currItem].units);
      lcd.setCursor(0, 3);
      if (true == _currentSaveFlag) {
        lcd.print(SAVE_FLAG_CHOSEN);
        lcd.print("   ");
        lcd.print(CANC_FLAG);
      } else {
        lcd.print(SAVE_FLAG);
        lcd.print("   ");
        lcd.print(CANC_FLAG_CHOSEN);
      }
    
      int diffValue = abs(oldValue - params[_currItem].value_new_pot);
      if (diffValue > 5)
      {
        /*Special case update for pot inputs*/
        _lastSubEditMenuTime = millis();
      }
      oldValue = params[_currItem].value_new_pot;
    }
    
    if (bSave) {
      if (_currentSaveFlag == 0) {
        lcd.print(" Edit cancelled.");
      } else {
        params[_currItem].value_curr_mem = getCalibratedParamFromPot(params[_currItem]);
        storeParam(params[_currItem]);
        //Ctrl_send_packet(params[_currItem].parm_name, params[_currItem].value_curr_mem);
        lcd.setCursor(0, 3);
        lcd.print("                    ");
        lcd.setCursor(0, 3);
        lcd.print(" saved.....");
        digitalWrite(BUZZER_PIN, HIGH);
        delay(500);
        digitalWrite(BUZZER_PIN, LOW);
      }
    }

  if (true == _bBack2EditMenu) {
    lcd.setCursor(0, 3);
    lcd.print("   going back....");
    delay(100);
  }
}
void displayManager::drawDefaultItemUpdateMenu( RT_Events_T eRTState)
{
  bool bSave = false;
  switch (eRTState)
  {
    case RT_INC:
      _currentSaveFlag = false;
      break;
    case   RT_DEC:
      _currentSaveFlag = true;
      break;
    case   RT_BT_PRESS:
      _bBack2EditMenu = true;
      bSave = true;
      break;
	case RT_NONE:
		break;
	default:
		break;
  }
  if ((millis() - _lastDisplayTime > 500) ||
      (eRTState != RT_NONE))
  {
    _lastDisplayTime = millis();
    lcd.setCursor(8, 0);
    lcd.print(params[_currItem].parm_name);
    Serial.print("reading from Pot :");
    params[_currItem].value_new_pot = analogRead(params[_currItem].readPortNum);
    Serial.println(params[_currItem].value_new_pot);
    lcd.setCursor(0, 1);
    lcd.print("New: ");
    lcd.print("   ");
    lcd.setCursor(8, 1);
    int actualValue = getCalibValue(params[_currItem].value_new_pot, _currItem);
    printPadded(actualValue);
    lcd.setCursor(15, 1);
    lcd.print(params[_currItem].units);
    lcd.setCursor(0, 2);
    lcd.print("Old: ");
    lcd.setCursor(8, 2);
    printPadded(params[_currItem].value_curr_mem);
    lcd.setCursor(15, 2);
    lcd.print(params[_currItem].units);
    lcd.setCursor(0, 3);
    if (true == _currentSaveFlag) {
      lcd.print(SAVE_FLAG_CHOSEN);
      lcd.print("    ");
      lcd.print(CANC_FLAG);
    } else {
      lcd.print(SAVE_FLAG);
      lcd.print("    ");
      lcd.print(CANC_FLAG_CHOSEN);
    }
  
    int diffValue = abs(oldValue - params[_currItem].value_new_pot);
    if (diffValue > 5)
    {
      /*Special case update for pot inputs*/
      _lastSubEditMenuTime = millis();
    }
    oldValue = params[_currItem].value_new_pot;
  }

  if (bSave) {
    if (_currentSaveFlag == 0) {
      lcd.print(" Edit cancelled.");
    } else {
      params[_currItem].value_curr_mem = getCalibratedParamFromPot(params[_currItem]);
      storeParam(params[_currItem]);
      Ctrl_send_packet(params[_currItem].parm_name, params[_currItem].value_curr_mem);
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print(" saved.....");
      digitalWrite(BUZZER_PIN, HIGH);
      delay(500);
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}


void displayManager::drawEditMenu( void)
{
  String strOnLine234;
#if SERIAL_PRINTS
  VENT_DEBUG_INFO("Indicator", _editSeletIndicator);
  VENT_DEBUG_INFO("ScrollIndex", _editScrollIndex);
#endif
  lcd.clear();
  for (int i = 0; i <= LCD_HEIGHT_CHAR - 1; i++) //menuItems[menuIdx].menuLength; i++)
  {
    lcd.setCursor(0, i);
    if (_editSeletIndicator == i)
    {
      strOnLine234 = ">";
    }
    else
    {
      strOnLine234 = " ";
    }
    strOnLine234 += mainEditMenu[_editScrollIndex + i];

    switch (_editScrollIndex + i)
    {
      case (E_TV):
        strOnLine234 += params[E_TV].value_curr_mem;
        strOnLine234 += "mL";
        break;
      case (E_BPM):
        strOnLine234 += params[E_BPM].value_curr_mem;
        break;
      case (E_FiO2):
        strOnLine234 += params[E_FiO2].value_curr_mem;
        strOnLine234 += "%";
        break;
      case (E_IER):
        strOnLine234 += "1:";
        strOnLine234 += params[E_IER].value_curr_mem;
        break;
      case (E_PEEP):
        strOnLine234 += params[E_PEEP].value_curr_mem;
        strOnLine234 += "cmH2O";
        break;
      case (E_PIP):
        strOnLine234 += params[E_PIP].value_curr_mem;
        strOnLine234 += "cmH2O";
        break;
      case (E_O2_INPUT):
        _o2LineSelect = params[E_O2_INPUT].value_curr_mem;
        strOnLine234 += o2LineString[_o2LineSelect];
        break;
      case (SHOW_VOL):
        break;
    }

    lcd.print (strOnLine234);
#if SERIAL_PRINTS
    VENT_DEBUG_INFO("String", strOnLine234);
#endif
  }
}

/*utility to show the states in string*/
String displayManager::dpStatusString(STATE State) {
  switch(State)  {
    case STATUS_MENU:
      return "STATUS_MENU";
    case STATUS_MENU_TO_EDIT_MENU:
      return "STATUS_MENU_TO_EDIT_MENU";
    case EDIT_MENU:
      return "EDIT_MENU";
    case EDIT_MENU_TO_SUB_EDIT_MENU:
      return "EDIT_MENU_TO_SUB_EDIT_MENU";
    case SUB_EDIT_MENU:
      return "SUB_EDIT_MENU";
    case SUB_EDIT_MENU_TO_EDIT_MENU:
      return "SUB_EDIT_MENU_TO_EDIT_MENU";
    case EDIT_MENU_TO_STATUS_MENU:
        return "EDIT_MENU_TO_STATUS_MENU";
    default:
      return "InCorrect State";
  }
}
void displayManager::moveUpEdit()
{
  _editSeletIndicator++;
  /*
     check wrap around of select indicator
  */
  if ((_editSeletIndicator >= MAX_EDIT_MENU_ITEMS) ||
      (_editSeletIndicator > LCD_HEIGHT_CHAR - 1))
  {
    _editSeletIndicator = min(LCD_HEIGHT_CHAR - 1, MAX_EDIT_MENU_ITEMS);
    if (_editSeletIndicator == LCD_HEIGHT_CHAR - 1)
    {
      _editScrollIndex++;
    }
    if ((_editScrollIndex + _editSeletIndicator) >= MAX_EDIT_MENU_ITEMS)
    {
      _editScrollIndex = MAX_EDIT_MENU_ITEMS - _editSeletIndicator - 1;
    }
  }
}

void displayManager::moveDownEdit()
{
  /*
     check wrap around of select indicator
  */
  if (_editSeletIndicator == 0 )
  {
    if (_editScrollIndex > 0)
    {
      _editScrollIndex--;
    }
  }
  else
  {
    _editSeletIndicator--;
  }
}

/*
   The getCalibValue is for calibrating any integer to a parameter.
   The parameter is input with the help of an index
*/
int displayManager::getCalibValue(int potValue, int paramIndex) {
  float convVal = map(potValue, 0, POT_HIGH, params[paramIndex].max_val, params[paramIndex].min_val);
  return ((int)(convVal / params[paramIndex].incr) + 1) * params[paramIndex].incr;
}

/*
   The below method is for a specific parameter
*/
int displayManager::getCalibratedParamFromPot(ctrl_parameter_t param) {
  if (param.readPortNum == DISP_ENC_CLK) {
    return param.value_new_pot;
  }
  float convVal = map(param.value_new_pot, 0, POT_HIGH, param.max_val, param.min_val);
  return ((int)(convVal / param.incr) + 1) * param.incr;
}

int displayManager::rectifyBoundaries(int value, int minimum, int maximum) {
  if (value < minimum) {
    return maximum;
  }
  if (value > maximum) {
    return minimum;
  }
  return value;
}

void displayManager::editModeEncoderInput(void)
{
  RT_Events_T eRTState = RT_NONE;
  _bEditItemSelected = false;
  _bRefreshEditScreen = false;
  eRTState = encoderScanUnblocked();
  switch (eRTState)
  {
    case RT_INC:
      VENT_DEBUG_INFO("RT_INC", 0);
      moveUpEdit();
      break;
    case   RT_DEC:
      VENT_DEBUG_INFO("RT_DEC", 0);
      moveDownEdit();
      break;
    case   RT_BT_PRESS:
      VENT_DEBUG_INFO("RT_BT_PRESS", 0);
      _bEditItemSelected = true;
      break;
    case RT_NONE:
      break;
  }
  if (eRTState != RT_NONE)
  {
    _bRefreshEditScreen = true;
  }
}

void displayManager::handleItemUpdate()
{
  RT_Events_T eRTState;
  eRTState = encoderScanUnblocked();
  if (eRTState != RT_NONE)
  {
    _lastSubEditMenuTime = millis();
  }
 switch(_currItem) {
  case (E_TV):
    drawDefaultItemUpdateMenu(eRTState);
    break;
  case (E_BPM):
    drawDefaultItemUpdateMenu(eRTState);
    break;
  case (E_FiO2):
    drawUpdateFiO2Menu(eRTState);
    break;
  case (E_IER):
    drawUpdatePEEPorIERMenu(eRTState);
    break;
  case (E_PEEP):
    drawUpdatePEEPorIERMenu(eRTState);
    break;
  case (E_PIP):
    drawDefaultItemUpdateMenu(eRTState);
    break;
  case (E_O2_INPUT):
    drawUpdateO2_InputMenu(eRTState);
    break;
  case (SHOW_VOL):
    drawSensorvoltageMenu(eRTState);
    break;
  case (SHOW_PRESSURE):
    drawSensorValueMenu(eRTState);
    break;
  default:
   {
     VENT_DEBUG_INFO("Unhandled Element", _currItem);
     Serial.println();    
   }
    break;
  }
}

void displayManager::getItemIndx(void) {
	
#if SERIAL_PRINTS
  VENT_DEBUG_INFO("Indicator", _editSeletIndicator);
  VENT_DEBUG_INFO("ScrollIndex", _editScrollIndex);
#endif

  if (params[_editSeletIndicator + _editScrollIndex].index != (_editSeletIndicator + _editScrollIndex)) {
    VENT_DEBUG_ERROR("Invalid Edit", 0);
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("  Edit Element Error ");
  } else {
    VENT_DEBUG_ERROR("Editing....", 0);
    Serial.println(params[_editSeletIndicator + _editScrollIndex].parm_name);
    _currItem = _editSeletIndicator + _editScrollIndex;
  }
}

void displayManager::stateMachine(void) {
  switch (_dpState) {
    case STATUS_MENU_TO_EDIT_MENU:
    {
      _bRefreshEditScreen = true;
      _bEditItemSelected = false;
      _lastEditMenuTime = millis();
      _dpState = EDIT_MENU;
      drawEditMenu();      
    }
    break;
    case EDIT_MENU:
    {
      if ((millis() - _lastEditMenuTime) > EDIT_MENU_TIMEOUT) {
        VENT_DEBUG_ERROR("Edit Menu Timeout", 0);
        _dpState = EDIT_MENU_TO_STATUS_MENU;
        return ;
      }
      editModeEncoderInput();
      if (true == _bEditItemSelected)
      {
        getItemIndx();
        if (E_EXIT == _currItem)
          _dpState = EDIT_MENU_TO_STATUS_MENU;
        else
          _dpState = EDIT_MENU_TO_SUB_EDIT_MENU;
      } 
      else
      {
        //No element selected for edit
        //display screen has to be updated if scroll bar is moved up or down
        if (true == _bRefreshEditScreen)
        {
          _lastEditMenuTime = millis();
          drawEditMenu();
        }
      }
      //element selected for edit
    }
    break;
    case EDIT_MENU_TO_SUB_EDIT_MENU:
    {
      lcd.clear();
      _bBack2EditMenu = false;
      _currentSaveFlag = false;
      _dpState = SUB_EDIT_MENU;
      _lastSubEditMenuTime = millis();
    }
    break;
    case SUB_EDIT_MENU:
    {
      handleItemUpdate();
      if (true == _bBack2EditMenu) {
        _dpState = SUB_EDIT_MENU_TO_EDIT_MENU;
        _bBack2EditMenu = false;
      }
      if ((millis() - _lastSubEditMenuTime) > EDIT_MENU_TIMEOUT) {
      VENT_DEBUG_ERROR("SubEdit Menu Timeout", 0);
      _dpState = EDIT_MENU_TO_STATUS_MENU;
      return ;
      }
    }   
    break;
    case SUB_EDIT_MENU_TO_EDIT_MENU:
    {
      _bEditItemSelected = false;
      _currentSaveFlag = false;
      _lastEditMenuTime = millis();
      _bRefreshEditScreen = true;
      _dpState = EDIT_MENU;
      drawEditMenu();
    }
    break;
    default: 
    {
       VENT_DEBUG_ERROR("Unhandled State", _dpState);
    }
  }
}

#if 0
void displayManager::displayRunTime(float *sensor_data)
{
  if (true == _refreshRunTimeDisplay)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("       STATUS");
    _refreshRunTimeDisplay = false;
  }
  // cleanRow(1); cleanRow(2); cleanRow(3);
  String row1 = "TV  :";
  row1 += params[E_TV].value_curr_mem;
  while (row1.length() < 6)
  {
    row1 += " ";
  }
  row1 += "mL    BPM:";
  row1 += params[E_BPM].value_curr_mem;

  while (row1.length() < 20)
  {
    row1 += " ";
  }
  lcd.setCursor(0, 1);
  lcd.print(row1);

  String row2 = "FiO2:";
  row2 += (int)sensor_data[SENSOR_O2];
  row2 += "%";

  while (row2.length() < 8)
  {
    row2 += " ";
  }
  row2 += "     IER:1:";
  row2 += params[E_IER].value_curr_mem;
  lcd.setCursor(0, 2);
  lcd.print(row2);


  String row3 = "PEEP:";
  row3 += params[E_PEEP].value_curr_mem;

  while (row3.length() < 7)
  {
    row3 += " ";
  }

  row3 += "  IP:";
  /*Display needs only decimal*/
  row3 += (int)(sensor_data[SENSOR_PRESSURE_A0]);
  while (row3.length() < 14)
  {
    row3 += " ";
  }

  row3 += " EP:";
  /*Display needs only decimal*/
  row3 += (int)((sensor_data[SENSOR_PRESSURE_A1]));
  while (row3.length() < 20)
  {
    row3 += " ";
  }
  lcd.setCursor(0, 3);
  lcd.print(row3);
}
#endif
#if 0
void displayManager::displayRunTime(float *sensor_data)
{
  if (true == _refreshRunTimeDisplay)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    _refreshRunTimeDisplay = false;
  }

  // row0 start
  String row0 = "TV  :"; // reached 5
  row0 += params[E_TV].value_curr_mem; // reached 8 
  row0 += "mL BPM :"; //reached 16
  row0 += params[E_BPM].value_curr_mem; //reached 18
  row0 += "  "; //reached 20
  lcd.setCursor(0, 0);
  lcd.print(row0);

  // row1 start
  String row1 = "FiO2:"; //reached 5
  row1 += (int)sensor_data[SENSOR_O2]; //reached 7
  row1 += "%"; //reached 8
  row1 += "   IER :1:"; //reached 17
  row1 += params[E_IER].value_curr_mem;
  lcd.setCursor(0, 1);
  lcd.print(row1);

  // row2 start
  String row2 = "PEEP:"; //reached 5
  row2 += params[E_PEEP].value_curr_mem;
  while (row2.length() < 11)
  {
    row2 += " ";
  }

  row2 += "PMAX:";
  /*Display needs only decimal*/
  //row2 += (int)(sensor_data[SENSOR_PRESSURE_A0]);
  row2 += 100;
  while (row2.length() < 20)
  {
    row2 += " ";
  }
  lcd.setCursor(0, 2);
  lcd.print(row2);

  String row3 = "TVi:";
  /*Display needs only decimal*/
  row3 += m_display_tvi;
  row3 += "mL  TVe:";
  row3 += m_display_tve;
  row3 += "mL";
  while (row3.length() < 20)
  {
    row3 += " ";
  }
  lcd.setCursor(0, 3);
  lcd.print(row3);
}
#endif
#if 0
void displayManager::displayRunTime(float *sensor_data)
{
  char str_temp[6];
  char buffer[6];

  if (true == _refreshRunTimeDisplay)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    _refreshRunTimeDisplay = false;
  }

  {
    // row0 start
    String row0 = "TV  "; // reached 5
    row0 += params[E_TV].value_curr_mem; // reached 7
    row0 += " RR "; //reached 9
    row0 += params[E_BPM].value_curr_mem; //reached 11
    row0 += " "; //reached 12
    row0 += "IE 1:";
    row0 += params[E_IER].value_curr_mem;
    lcd.setCursor(0, 0);
    lcd.print(row0);
  }
  {
    // row1 start
    String row1 = "TVi "; //reached 4
    row1 += (int)m_display_tvi; //reached 7
    while (row1.length() < 12)
    {
      row1 += " ";
    }
    row1 += "PIP ";
    dtostrf(m_display_pip, 4, 1, str_temp);
    sprintf(buffer, "%s", str_temp);
    row1 += buffer;
    lcd.setCursor(0, 1);
    lcd.print(row1);
  }
  {
    String row2 = "TVe "; //reached 4
    row2 += (int)m_display_tve; //reached 7
    while (row2.length() < 11)
    {
      row2 += " ";
    }
    row2 += "Plat ";
    dtostrf(m_display_plat, 4, 1, str_temp);
    sprintf(buffer, "%s", str_temp);
    row2 += buffer;
    lcd.setCursor(0, 2);
    lcd.print(row2);
  }
  {
    String row3 = "FiO2 ";
    row3 += (int)sensor_data[SENSOR_O2];
    row3 += "%";
    //Serial.println((PS_ReadSensorValueX10(O2)) / 10);
    while (row3.length() < 11)
    {
      row3 += " ";
    }
    row3 += "PEEP ";
    dtostrf(m_display_peep, 4, 1, str_temp);
    sprintf(buffer, "%s", str_temp);
    row3 += buffer;
    lcd.setCursor(0, 3);
    lcd.print(row3);
  }
}

#endif
static char str_temp[6];
static char buffer[6];
static char row[30] = "";

static bool blink = true;
static unsigned long last_o2update = 0;

void displayManager::displayRunTime(float *sensor_data)
{

  if ((true == _refreshRunTimeDisplay) 
      || (true == refreshfullscreen_inhale) 
      || (true == refreshfullscreen_exhale && (millis() > exhale_refresh_timeout))) {

    blink = true;

    row[0] = '\0';
    if (true == _refreshRunTimeDisplay)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      _refreshRunTimeDisplay = false;
    }
    {
      // row0 start
      row[0] = '\0';
      sprintf(row, "TV  %3d RR %2d IE 1:%1d", params[E_TV].value_curr_mem, params[E_BPM].value_curr_mem, params[E_IER].value_curr_mem);
      lcd.setCursor(0, 0);
      lcd.print(row);
    }
    {
      row[0] = '\0';
      dtostrf(m_display_pip, 4, 1, str_temp);
      sprintf(buffer, "%s", str_temp);
      sprintf(row, "%3d ",(int)m_display_tvi);
      lcd.setCursor(0, 1);
      lcd.print("TVi");
      if (tviErr > 0)
      {
        digitalWrite(BUZZER_PIN, blink);
        if(blink)
          lcd.write(DP_UP_TR);
        else
          lcd.write(DP_EM_UP_TR);
      } else if (tviErr < 0) {
        digitalWrite(BUZZER_PIN, blink);
        if(blink)
          lcd.write(DP_DW_TR);
        else
          lcd.write(DP_EM_DN_TR);
      } else {
        digitalWrite(BUZZER_PIN, LOW);
        lcd.print(" ");
      }
        lcd.print(row);
        lcd.setCursor(8,1);
        lcd.print("PIP ");
        if (pipErr > 0)
         {
           digitalWrite(BUZZER_PIN, blink);
           if(blink)
             lcd.write(DP_UP_TR);
           else
             lcd.write(DP_EM_UP_TR);
         } else if (pipErr < 0) {
           digitalWrite(BUZZER_PIN, blink);
           if(blink)
             lcd.write(DP_DW_TR);
           else
             lcd.write(DP_EM_DN_TR);
         } else {
           digitalWrite(BUZZER_PIN, LOW);
           lcd.print(" ");
         }
        sprintf(row, "%s", buffer);
        lcd.print(row);
      }
    //3rd row
    {
      row[0] = '\0';
      dtostrf(m_display_plat, 4, 1, str_temp);
      sprintf(buffer, "%s", str_temp);
      sprintf(row, " %3d Plat %s", (int)m_display_tve, buffer);
      lcd.setCursor(0, 2);
      lcd.print("TVe");
      lcd.print(row);
    }
    {
      row[0] = '\0';
      dtostrf(m_display_peep, 4, 1, str_temp);
      sprintf(buffer, "%s", str_temp);
      sprintf(row, "%2d%% ", (int)sensor_data[SENSOR_O2]);
      lcd.setCursor(0, 3);
      lcd.write(DP_FI);
      lcd.print("O2");
      if ((int)sensor_data[SENSOR_O2] > 1.1*params[E_FiO2].value_curr_mem)
      {
        if(blink)
          lcd.write(DP_UP_TR);
        else
          lcd.write(DP_EM_UP_TR);
      } else if ((int)sensor_data[SENSOR_O2] < 0.9*params[E_FiO2].value_curr_mem) {
      if(blink)
          lcd.write(DP_DW_TR);
        else
          lcd.write(DP_EM_DN_TR);
      } else {
        lcd.print(" ");
      }
      lcd.print(row);

      lcd.setCursor(8, 3);
      sprintf(row, "%s", buffer);
      lcd.print("PEEP");
      if (peepErr > 0)
      {
        digitalWrite(BUZZER_PIN, blink);
        if(blink) lcd.write(DP_UP_TR);
        else lcd.write(DP_EM_UP_TR);
      } else if (peepErr < 0) {
        digitalWrite(BUZZER_PIN, blink);
        if(blink)
          lcd.write(DP_DW_TR);
        else
          lcd.write(DP_EM_DN_TR);
      } else {
        digitalWrite(BUZZER_PIN, LOW);
        lcd.print(" ");
      }
      lcd.print(row);
    }
    lcd.setCursor(19,3);
    if (true == machineOn) {
      if (blink)
        lcd.print("R");
      else
        lcd.print(" ");
    } else {
        lcd.print("S");
    }
    refreshfullscreen_inhale = false;
    refreshfullscreen_exhale = false;
  } else if( last_o2update < millis()) {
     last_o2update = millis() + 500;
     row[0] = '\0';
     sprintf(row, "%2d%% ", (int)sensor_data[SENSOR_O2]);
     lcd.setCursor(3, 3);
     if ((int)sensor_data[SENSOR_O2] > 1.1*params[E_FiO2].value_curr_mem)
     {
       if(blink)
         lcd.write(DP_UP_TR);
       else
         lcd.write(DP_EM_UP_TR);
     } else if ((int)sensor_data[SENSOR_O2] < 0.9*params[E_FiO2].value_curr_mem) {
     if(blink)
         lcd.write(DP_DW_TR);
       else
         lcd.write(DP_EM_DN_TR);
     } else {
       lcd.print(" ");
     }
     lcd.print(row);
     blink = false;
     digitalWrite(BUZZER_PIN, LOW);
  }
}


void displayManager::displayManagerSetup() {
  // set to edit menu state and trigger state machine
    _dpState = STATUS_MENU_TO_EDIT_MENU;
    volatile STATE dpStateTemp;
  do {
      dpStateTemp = _dpState;
      stateMachine();
      if (dpStateTemp != _dpState) {
        Serial.print("(setup)dpState Change from : ");
        Serial.print(dpStatusString(dpStateTemp));
        Serial.print("-> ");
        Serial.println(dpStatusString(_dpState));
      }
   } while(EDIT_MENU_TO_STATUS_MENU != _dpState);
  // at the end of the setup move to show status menu
  _dpState = STATUS_MENU;
  _refreshRunTimeDisplay = true;
  _editSeletIndicator = 0;
  _editScrollIndex = 0;
}

void displayManager::displayManagerloop(float *sensor_data, sensorManager &sM) 
{
  RT_Events_T eRTState = RT_NONE;
  m_sM = &sM;
  volatile STATE dpStateTemp = _dpState;
  if (STATUS_MENU == _dpState) {
    displayRunTime(sensor_data);
    eRTState = encoderScanUnblocked();
  }
  
  if ((eRTState == RT_BT_PRESS) || (STATUS_MENU != _dpState))
  {
      if ((dpStateTemp != _dpState)  || (eRTState == RT_BT_PRESS)) {
      Serial.print("before SM: dpState Changed from : ");
      Serial.print(dpStatusString(dpStateTemp));
      Serial.print("-> ");
      Serial.println(dpStatusString(_dpState));
    }
    if (STATUS_MENU == _dpState) {
      _dpState = STATUS_MENU_TO_EDIT_MENU;
    }
    stateMachine();
    if (EDIT_MENU_TO_STATUS_MENU == _dpState) {
        _dpState = STATUS_MENU;
        _editSeletIndicator = 0;
        _editScrollIndex = 0;
        _refreshRunTimeDisplay = true;
      }

    if ((dpStateTemp != _dpState)  || (eRTState == RT_BT_PRESS)){
      VENT_DEBUG_INFO("State Change from", dpStatusString(dpStateTemp));
	  VENT_DEBUG_INFO("State Change to", _dpState);
    }
  }
}

void displayManager::errorDisplay(ErrorDef_T errorState)
{
  switch (errorState)
  {
    case (ERR_OXY):
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("       ERROR");
        lcd.setCursor(0, 1);
        lcd.print( OXY_FAILURE_MSG1);
        lcd.setCursor(0, 2);
        lcd.print( OXY_FAILURE_MSG2);
      }
      break;
    case (ERR_PATIENT_DISCONN):
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("       ERROR");
        lcd.setCursor(0, 1);
        lcd.print( PATIENT_DISCONN_MSG1);
        lcd.setCursor(0, 2);
        lcd.print( PATIENT_DISCONN_MSG2);
      }
      break;
    case (ERR_PEEP):
      break;
    case (ERR_BVM):
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("       ERROR");
        lcd.setCursor(0, 1);
        lcd.print( BVM_FAILURE_MSG1);
        lcd.setCursor(0, 2);
        lcd.print( BVM_FAILURE_MSG2);
      }
      break;
    case (ERR_TV):
      break;
    default:
      break;
  }
  
  digitalWrite(BUZZER_PIN, blink);
   bvmFailure = false;
}

