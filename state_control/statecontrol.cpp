#include "statecontrol.h"
#include "../sensors/sensors.h"
#include "../debug.h"

bool bSendInitCommand = false;
bool minPressureForMaskOn = false;
bool bSendPeakHighDetected = false;
bool bSendPeepLowDetected = false;
bool bBreathDetectedFlag = false;

// Control statemachine gloabl variable
ControlStatesDef_T geCtrlState = CTRL_DO_NOTHING;
ControlStatesDef_T geCtrlPrevState  = CTRL_DO_NOTHING;

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
int  peepErr = 0;
int  tviErr = 0;
int  pipErr = 0;

bool refreshfullscreen_inhale = true;
bool refreshfullscreen_exhale = true;
unsigned long exhale_refresh_timeout = 0;


bool bvmFailure = false;
unsigned long int breathCount = 0;

int Ctrl_send_packet(int cmdIndex) 
{
	Serial3.print(commands[cmdIndex]);
	return 0;
}

int Ctrl_send_packet(String name, int value) 
{
  VENT_DEBUG_FUNC_START();
  
	String param = "";
	String command = "";
	if (name == tidl_volu.parm_name) {
	   param = PARAM1;
	} else if (name == inex_rati.parm_name) {
       param = PARAM5;
    } else if (name == resp_rate.parm_name) {
	   param = PARAM2;
	} else {
	   VENT_DEBUG_ERROR("ERROR: Trying to send invalid packet", -1);
	   return -1;
	}
    command = Ctrl_CreateCommand(param, value);
    Serial3.print(command);
	VENT_DEBUG_INFO ("Command", command);
	
    VENT_DEBUG_FUNC_END();
	return 0;
}

void Ctrl_store_received_packet(String data) {
	serial2_rxdata = data;
}
float plat = 0;
void Ctrl_ProcessRxData(void) {
  String p1;
  String p2;
  String payload;
  String command;
  long int state; 
  

  p1 = serial2_rxdata.substring(1, 3);
  p2 = serial2_rxdata.substring(3, 5);
  payload = serial2_rxdata.substring(5, 9);
  int value;

  if (p1 == VENTSLAVE) {
    if (p2 == SYNCH) {
	  state = payload.toInt();
	  if ((ControlStatesDef_T(state)) >= CTRL_UNKNOWN_STATE)
	  {
		  VENT_DEBUG_ERROR("Payload with Incorrect State", state);
	  }
	  else
	  {
		  geCtrlState = ControlStatesDef_T(state);
	  }
    } else if (p2=="O2") {
      if(0 == params[E_O2_INPUT].value_curr_mem) {
        Ctrl_send_packet(OXY_SOLE_CYL_ONN);
      } else {
        Ctrl_send_packet(OXY_SOLE_HOS_O2_ONN);
      }
    } else {
      int index;
      index =  payload.toInt();
      if (index < MAX_CTRL_PARAMS) {
        value = params[index].value_curr_mem;
        command = Ctrl_CreateCommand(p2, value);
        Serial3.print(command);
        Serial.println(command);
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
  sprintf(paddedValue, "%04d", value);
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
float pmax = 0;

void Ctrl_StateMachine_Manager(const float *sensor_data, sensorManager &sM, displayManager &dM)
{

  VENT_DEBUG_FUNC_START ();
 
  switch (geCtrlState) {
    case CTRL_INIT:
    {
      Serial3.print(commands[INIT_MASTER]);
      Serial.println(commands[INIT_MASTER]);
      geCtrlState = CTRL_DO_NOTHING;
	  //enable_sensor(0);
     }
    break;
    case CTRL_EXPANSION_HOLD:
    case CTRL_COMPRESSION_HOLD:
    case CTRL_UNKNOWN_STATE:
	case CTRL_DO_NOTHING: // Always enter this during idle scenarios.
	break;
    case CTRL_COMPRESSION:
    {
               
      if (geCtrlPrevState != geCtrlState) {
                      peepErr = 0;
          pipErr = 0;
         dM.setDisplayParam(DISPLAY_TVE, sensor_data[SENSOR_DP_A1]);
         dM.setDisplayParam(DISPLAY_PEEP, sensor_data[SENSOR_PRESSURE_A1]); 
         if(sensor_data[SENSOR_PRESSURE_A1]  <  params[E_PEEP].value_curr_mem){
            peepErr = -1;
          }
         if(sensor_data[SENSOR_PRESSURE_A1]  >  2 * params[E_PEEP].value_curr_mem){
            peepErr = 1;
          } 
         sM.enable_sensor(PRESSURE_A0 | DP_A0 | O2);
         refreshfullscreen_inhale = true;
      }

        pmax = sensor_data[SENSOR_PRESSURE_A0];
      if ((minPressureForMaskOn == false) && ((sensor_data[SENSOR_PRESSURE_A0]) > MIN_PRESSURE_FOR_MASKON )) minPressureForMaskOn = true;
      /*When Peak Pressure Set in the UI is less than the sensor measured Peak PressureValue*/
      if ((sensor_data[SENSOR_PRESSURE_A0] > params[E_PIP].value_curr_mem) && bSendPeakHighDetected == false) {
        bSendPeakHighDetected = true;
        Serial3.print(commands[INH_SOLE_OFF]);
        pipErr = 1;

      }
   //geCtrlState = CTRL_DO_NOTHING;
    }
    break;
    case CTRL_EXPANSION:
    {
      if (geCtrlPrevState != geCtrlState) {
        VENT_DEBUG_INFO("SC :EX ", sensor_data[SENSOR_DP_A1]);
 
 
        tviErr = 0;
        dM.setDisplayParam(DISPLAY_PIP,pmax);       
        dM.setDisplayParam(DISPLAY_PLAT,sensor_data[SENSOR_PRESSURE_A1]);
        dM.setDisplayParam(DISPLAY_TVI,sensor_data[SENSOR_DP_A0]);
        if(sensor_data[SENSOR_DP_A0] * 1.085 < 100){
          bvmFailure = true;
        }
        if((sensor_data[SENSOR_DP_A0] <params[E_TV].value_curr_mem * 0.85) ) {
          tviErr = -1;
        }
        if((sensor_data[SENSOR_DP_A0] > params[E_TV].value_curr_mem * 1.15)) {
          tviErr = 1;
        }
        sM.enable_sensor(PRESSURE_A1 | DP_A1 | O2);
        breathCount++;
        refreshfullscreen_exhale = true;
        exhale_refresh_timeout = millis() + 500;
      }
      /*When the sensor measured Peek PressureValue is less than peek pressure set in the UI*/
      if ((sensor_data[SENSOR_PRESSURE_A1] < params[E_PEEP].value_curr_mem) && bSendPeepLowDetected == false) {
        bSendPeepLowDetected = true;
        //Serial3.print(commands[EXH_SOLE_OFF]);
        if ((digitalRead(AUTO_MODE))) {
          if (sM.check_for_dip_in_pressure(SENSOR_DP_A0)) {
            //Serial3.print(commands[INIT_BREATH_DET]);
          }
        }
      } else {
        if ((digitalRead(AUTO_MODE))) {
          if (sM.check_for_dip_in_pressure(SENSOR_DP_A0)) {
		   // Serial3.print(commands[INIT_BREATH_DET]);
          }
        }
      }
      //geCtrlState = CTRL_DO_NOTHING;
    }
    break;
    case CTRL_INHALE_DETECTION:
    {
      if (bBreathDetectedFlag == false) {
        Serial.println(sM.check_for_dip_in_pressure(SENSOR_DP_A0));
        if (sM.check_for_dip_in_pressure(SENSOR_DP_A0)) {
          bBreathDetectedFlag = true;
          Serial3.print(commands[INIT_BREATH_DET]);
        }
      }
    }
    break;
    case CTRL_STOP:
    {
		Serial3.print(commands[STPR_STP]);
		geCtrlState = CTRL_DO_NOTHING;
   peepErr = 0;
   tviErr = 0;
   pipErr = 0;
    }
    break;
    default:
	   VENT_DEBUG_ERROR("ERROR: Unknown state received in State machine", 0);
    break;
  }
  if (geCtrlPrevState != geCtrlState) {
    geCtrlPrevState = geCtrlState;
    bSendPeakHighDetected = false;
    bSendPeepLowDetected = false;
    bBreathDetectedFlag = false;
    if (geCtrlState == CTRL_COMPRESSION ) { 
		minPressureForMaskOn = false; 
		
	}
	if (geCtrlState == CTRL_EXPANSION ) 
	plat = sensor_data[SENSOR_PRESSURE_A1];
  }
  VENT_DEBUG_FUNC_END();
}
