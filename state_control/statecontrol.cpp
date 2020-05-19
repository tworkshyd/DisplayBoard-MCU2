#include "statecontrol.h"
#include "../sensors/sensors.h"

bool bSendInitCommand = false;
bool minPressureForMaskOn = false;
bool bSendPeakHighDetected = false;
bool bSendPeepLowDetected = false;
bool bBreathDetectedFlag = false;

// Control statemachine gloabl variable
ControlStatesDef_T geCtrlState = CTRL_INIT;
ControlStatesDef_T geCtrlPrevState  = CTRL_INIT;

/*!< Commands array contains static command values to be sent to ventilator master */

static const String commands[] =
{ "$VMST0000&",        /**< stop stepper motor*/
  "$VMST0001&",        /**< start stepper motor*/
  "$VMO20100&",        /**< oxygen solenoid cylinder line off*/
  "$VMO20101&",       /**< oxygen solenoid cylinder line on*/
  "$VMO20200&",       /**< oxygen solenoid hospital line off*/
  "$VMO20201&",       /**< oxygen solenoid hospital line on*/
  "$VMSV0100&",       /**< inhalation  solenoid off*/
  "$VMSV0101&",       /**< inhalation  solenoid on*/
  "$VMSV0200&",       /**< exhalation  solenoid off*/
  "$VMSV0201&",       /**< exhalation  solenoid on*/
  "$VMSV0300&",       /**< peak pressure relief off*/
  "$VMSV0301&",       /**< peak pressure relief on*/
  "$VMP1xxxx&",       /**< set tidal volume */
  "$VMP2xxxx&",       /**< set BPM */
  "$VMP3xxxx&",       /**< set peak pressure*/
  "$VMP4xxxx&",       /**< set Fio2*/   
  "$VMP5xxxx&",       /**< set inhalation exhalation ratio */
  "$VMPP0000&",       /**< slave entering parameter edit */
  "$VMPP1111&",       /**< slave completed parameter edit */
  "$VMIN0000&",       /**< initialize master*/
  "$VMIN0001&",       /**< initialize stepper module*/
  "$VMIN0002&",       /**< initialize stepper module*/
  "$VMIN0003&"        /**< initialize breath detection*/
};

String serial2_rxdata = "";

int Ctrl_send_packet(int cmdIndex) {
	Serial2.print(commands[cmdIndex]);
	return 0;
}

int Ctrl_send_packet(String name, int value) {
	String param = "";
	String command = "";
	if (name == tidl_volu.parm_name) {
	   param = PARAM1;
	} else if (name == inex_rati.parm_name) {
       param = PARAM5;
    } else if (name == resp_rate.parm_name) {
	   param = PARAM2;
	} else {
	   Serial.println("ERROR: Trying to send invalid packet");
	   return -1;
	}
    command = Ctrl_CreateCommand(param, value);
    Serial2.print(command);
	return 0;
}

void Ctrl_store_received_packet(String data) {
	serial2_rxdata = data;
}

void Ctrl_ProcessRxData(void) {
  String p1;
  String p2;
  String payload;
  String command;

  p1 = serial2_rxdata.substring(1, 3);
  p2 = serial2_rxdata.substring(3, 5);
  payload = serial2_rxdata.substring(5, 9);
  int value;

  if (p1 == VENTSLAVE) {
    if (p2 == SYNCH) {
      geCtrlState = payload.toInt();
    } else {
      int index;
      index =  payload.toInt();

      if (index < MAX_CTRL_PARAMS) {
        value = params[index].value_curr_mem;
        command = Ctrl_CreateCommand(p2, value);
        Serial2.print(command);
      }
    }
  }
}

/*
   Function to build the command to be sent to Ventilator Master
*/
String Ctrl_CreateCommand(String paramName, int value) {
  String command;
  char paddedValue[3];
  command = START_DELIM;
  command += VENT_MAST;
  command += paramName;
  sprintf(paddedValue, "%04d",
          value);
  command += paddedValue;
  command += END_DELIM;
  return command;
}

void Ctrl_Start() {
  geCtrlState = CTRL_INIT;
}

void Ctrl_Stop() {
  geCtrlState = CTRL_STOP;
}

bool Ctrl_StateMachine_Manager(int *sensor_data)
{
  bool stateChanged = false;
  switch (geCtrlState) {
    case CTRL_INIT:
    {
      Serial2.print(commands[INIT_MASTER]);
      geCtrlState = CTRL_DO_NOTHING;
    }
    break;
    case CTRL_EXPANSION_HOLD:
    case CTRL_COMPRESSION_HOLD:
    case CTRL_UNKNOWN_STATE:
	case CTRL_DO_NOTHING: // Always enter this during idle scenarios.
	break;
    case CTRL_COMPRESSION:
    {
      if ((minPressureForMaskOn == false) && ((sensor_data[SENSOR_PRESSURE_A0]) > MIN_PRESSURE_FOR_MASKON )) minPressureForMaskOn = true;
      /*When Peak Pressure Set in the UI is less than the sensor measured Peak PressureValue*/
      if ((sensor_data[SENSOR_PRESSURE_A0] > params[PEAK_PRES].value_curr_mem) && bSendPeakHighDetected == false) {
        bSendPeakHighDetected = true;
        Serial2.print(commands[INH_SOLE_OFF]);
      }
	  geCtrlState = CTRL_DO_NOTHING;
    }
    break;
    case CTRL_EXPANSION:
    {
      /*When Peak Pressure Set in the UI is less than the sensor measured Peak PressureValue*/
      if ((sensor_data[SENSOR_PRESSURE_A1] < params[PEEP_PRES].value_curr_mem) && bSendPeepLowDetected == false) {
        bSendPeepLowDetected = true;
        Serial2.print(commands[EXH_SOLE_OFF]);
        if ((digitalRead(AUTO_MODE))) {
          if (check_for_dip_in_pressure(SENSOR_DP_A2)) {
            Serial2.print(commands[INIT_BREATH_DET]);
          }
        }
      } else {
        if ((digitalRead(AUTO_MODE))) {
          if (check_for_dip_in_pressure(SENSOR_DP_A2)) {
		    Serial2.print(commands[INIT_BREATH_DET]);
          }
        }
      }
      geCtrlState = CTRL_DO_NOTHING;
    }
    break;
    case CTRL_INHALE_DETECTION:
    {
      if (bBreathDetectedFlag == false) {
        Serial.print(check_for_dip_in_pressure(SENSOR_DP_A2));
        if (check_for_dip_in_pressure(SENSOR_DP_A2)) {
          bBreathDetectedFlag = true;
          Serial2.print(commands[INIT_BREATH_DET]);
        }
      }
    }
    break;
    case CTRL_STOP:
    {
		Serial2.print(commands[STPR_STP]);
		geCtrlState = CTRL_DO_NOTHING;
    }
    break;
    default:
		Serial.println("ERROR: Unknown state received in State machine");
    break;
  }
  if (geCtrlPrevState != geCtrlState) {
    geCtrlPrevState = geCtrlState;
    bSendPeakHighDetected = false;
    bSendPeepLowDetected = false;
    bBreathDetectedFlag = false;
    if (geCtrlState == CTRL_COMPRESSION ) { minPressureForMaskOn = false; }
  }
}
