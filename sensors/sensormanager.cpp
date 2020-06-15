/**************************************************************************/
/*!
    @file     sensors.cpp

    @brief 	  Base class for all sensor types

    @author   Tworks

	@defgroup VentilatorModule

    All sensor interfaces are available in this
	file.
	@{
*/
/**************************************************************************/
#include "sensormanager.h"
#include "pressure_sensor.cpp"
#include "O2_sensor.cpp"
#include "ads1115_utils.c"
#include "./../libraries/MsTimer2/MsTimer2.cpp"

#define MAX_PS2_SAMPLES 		10
#define THRESHOLD_COMPARE_INDEX 	2
#define DIP_THRESHOLD 			15 //better to be lower than PEEP

sensorManager sM;

static void sensorManager::capture_sensor_data();


//#define UNIT_TEST 1

#ifdef UNIT_TEST 
void setup() {
  Serial.begin(115200);
  pinMode(ACCUMULATOR_RESET_PIN, INPUT_PULLUP);
  sensors_init();
}
#endif

#ifdef UNIT_TEST
void loop() {
  float pressure0 = 0.0, volume0 = 0.0;
  float pressure1 = 0.0, volume1 = 0.0;
  if(digitalRead(ACCUMULATOR_RESET_PIN) == 0) {
    pressure0 = read_sensor_data(SENSOR_PRESSURE_A0);
    pressure1 = read_sensor_data(SENSOR_PRESSURE_A1);
    volume0 = read_sensor_data(SENSOR_DP_A0);
    volume1 = read_sensor_data(SENSOR_DP_A1);
    Serial.print("P0");
    Serial.print(" ");
    Serial.print(pressure0);
    Serial.print(", P1");
    Serial.print(" ");
    Serial.print(pressure1);
    Serial.print(", V0");
    Serial.print(" ");
    Serial.print(volume0, 6);
    Serial.print(", V1")
    Serial.print(" ");
    Serial.println(volume1, 6);
  }
  interrupts();
  delay(1);
}
#endif

/*
 * Function to read last few samples collected/measured
 */
int sensor::read_sensor_samples(float *samples, int sample_count) {
	int index = 0, sample_index = 0;
	if(samples == NULL || sample_count > MAX_SENSOR_SAMPLES) {
		Serial.println("ERROR: Sample is null or sample_count > MAX_SENSOR_SAMPLES");
		return ERROR_BAD_PARAM;
	}
	for(index = 0, sample_index = m_sample_index; index < MAX_SENSOR_SAMPLES && index < sample_count; index++, sample_index++) {
		if(sample_index >= MAX_SENSOR_SAMPLES) {
			sample_index = 0;
		}
		samples[index] = this->samples[sample_index];
	}
	return index;
}

/*
 * Function to initialize ads1115 board
 */
int ads1115_init() {
  ads.begin();
	ads.setGain(GAIN_ONE);
  ads1.begin();
	ads1.setGain(GAIN_ONE);
	return 0;
}


int no_of_sensorsenabled(unsigned int n) {
  int count=0;
  while(n!=0){
  if(n & 1 == 1){ //if current bit 1
    count++;//increase count
  }
    n=n>>1;//right shift
  }
  return count;
}

//#define FORCE_ENABLE_SENSOR 1
/*
 * Function to initialize all modules in sensors
 */
int sensorManager::init() {
  int err = 0;
  int en_sensors = 0;
  err += ads1115_init();
  err += _pS1.init();
  err += _pS2.init();
  err += _dpS1.init();
  err += _dpS2.init();
  err += _o2S.init();

  Serial.print("init configured ADC_CONVERSIONTIME_PERSENSOR: ");
  Serial.println(ADC_CONVERSIONTIME_PERSENSOR);

  //sM.enable_sensor(PRESSURE_A0 | DP_A0 | O2 | PRESSURE_A1 | DP_A1);
  return err;
}

void sensorManager::startTimer() {
  int en_sensors = no_of_sensorsenabled(_enabled_sensors);
  if (en_sensors) {
    unsigned long temp = _timervalueMs;
    _timervalueMs = ADC_CONVERSIONTIME_PERSENSOR*en_sensors + MINREQUIRED_DISPLAYREFRESH_TIME;
    if (temp != _timervalueMs) {
      MsTimer2::stop();
      MsTimer2::set(_timervalueMs, capture_sensor_data);
      MsTimer2::start();
      Serial.print("started timer with :");
      Serial.println(_timervalueMs);
      _dpS1.data_aquisitiontime(_timervalueMs);
      _dpS2.data_aquisitiontime(_timervalueMs);
    }
  }
}
/*
 * Function to enable specific sensors
 * Takes a parameter of different sensor combinations
 */
void sensorManager::enable_sensor(unsigned int flag) {
  if((_enabled_sensors & PRESSURE_A0) && !(flag & PRESSURE_A0)) {
    _pS1.reset_sensor_data();
  }
  if((_enabled_sensors & PRESSURE_A1) && !(flag & PRESSURE_A1)) {
    _pS2.reset_sensor_data();
  }
  if((_enabled_sensors & DP_A0) && !(flag & DP_A0)) {
    _dpS1.reset_sensor_data();
  }
  if((_enabled_sensors & DP_A1) && !(flag & DP_A1)) {
    _dpS2.reset_sensor_data();
  }
  if((_enabled_sensors & O2) && !(flag & O2)) {
    _o2S.reset_sensor_data();
  }
  _enabled_sensors = flag;
  startTimer();
}

unsigned int sensorManager::get_enable_sensors() {
  return _enabled_sensors;
}


/*
 * Wrapper API to get specific sensor readings
 */
int sensorManager::read_sensor_data(sensor_e s, float *data) {
  sensor *p_sensor = NULL;
  int err = 0;

  switch(s) {
    case SENSOR_PRESSURE_A0:
	  p_sensor = &_pS1;
	break;
	case SENSOR_PRESSURE_A1:
	  p_sensor = &_pS2;
	break;
	case SENSOR_DP_A0:
	  p_sensor = &_dpS1;
	break;
	case SENSOR_DP_A1:
	  p_sensor = &_dpS2;
	break;
	case SENSOR_O2:
	  p_sensor = &_o2S;
	break;
	default:
	  Serial.print(s);
	  Serial.println(" ERROR: Invalid sensor read request");
	  return 0.0;
	break;
  }
  err = p_sensor->get_error();
  if(err) {
	  Serial.print("ERROR: Sensor read error for ");
	  Serial.println(s);
	  return err;
  }
  *data = p_sensor->read_sensor_data();
  return SUCCESS;
}

/*
 * Checks the previous pressure readings and find
 * if there are any dip in pressure
 * returns -1 for invlaid sensors, 0 for no dip
 * 1 for a dip in pressure
 */
int sensorManager::check_for_dip_in_pressure(sensor_e sensor) {
	float sensor_data[MAX_SENSOR_SAMPLES] = {99.99};
	int count = 0;

	if(sensor == SENSOR_PRESSURE_A0) {
		count = _pS1.read_sensor_samples(&sensor_data[0], MAX_SENSOR_SAMPLES);
	} else if(sensor == SENSOR_PRESSURE_A1) {
		count = _pS2.read_sensor_samples(&sensor_data[0], MAX_SENSOR_SAMPLES);
	} else {
		return ERROR_SENSOR_UNSUPPORTED;
	}
	for(int index = 1; index < count; index++) {
		float diff = (sensor_data[index] - sensor_data[index - 1]);
		if(diff < 0) {
			if(abs(diff) > DIP_THRESHOLD) {
				return 1;
			}
		}
	}
	return 0;
}

/*
 * Function to aggregate or read sensor data
 * in an timer interrupt
 */
void sensorManager::capture_sensor_data(void) {
  interrupts(); // Called to enable other interrupts.
  long int starttime = millis();
  if(sM._enabled_sensors & PRESSURE_A0) {
    sM._pS1.capture_and_store();
  }
  if(sM._enabled_sensors & PRESSURE_A1) {
    sM._pS2.capture_and_store();
  }
  if(sM._enabled_sensors & DP_A0) {
    sM._dpS1.capture_and_store();
  }
  if(sM._enabled_sensors & DP_A1) {
    sM._dpS2.capture_and_store();
  }
  if(sM._enabled_sensors & O2) {
    sM._o2S.capture_and_store();
  }
#if 0
  Serial.print("T:");
  Serial.println(millis()-starttime);
#endif
}

int sensorManager::start_calibration(void) {
  sensor *p_sensor = NULL;
  int err = ERROR_UNKNOWN;

  for (int idx = 0; idx < MAX_SENSORS; idx++) {
    p_sensor = NULL;
    switch(idx) {
      case SENSOR_PRESSURE_A0:
        p_sensor = &_pS1;
        break;
      case SENSOR_PRESSURE_A1:
        p_sensor = &_pS2;
        break;
      case SENSOR_DP_A0:
        p_sensor = &_dpS1;
        break;
      case SENSOR_DP_A1:
        p_sensor = &_dpS2;
        break;
#if 0
      case SENSOR_O2:
        p_sensor = &_o2S;
      break;
#endif
    }
    if (NULL != p_sensor)
      err = p_sensor->sensor_zero_calibration();
#if 0
    if ( SUCCESS != err) 
      return err;
#endif
  }
  return SUCCESS;
}

int sensorManager::read_sensor_rawvoltage(sensor_e s) {
  sensor *p_sensor = NULL;
  int err = 0;

  switch(s) {
    case SENSOR_PRESSURE_A0:
	  p_sensor = &_pS1;
	break;
	case SENSOR_PRESSURE_A1:
	  p_sensor = &_pS2;
	break;
	case SENSOR_DP_A0:
	  p_sensor = &_dpS1;
	break;
	case SENSOR_DP_A1:
	  p_sensor = &_dpS2;
	break;
	case SENSOR_O2:
	  p_sensor = &_o2S;
	break;
	default:
	  Serial.print(s);
	  Serial.println(" ERROR: Invalid sensor read request");
	  return 0.0;
	break;
  }
  err = p_sensor->get_error();
  if(err) {
	  Serial.print("ERROR: Sensor read error for ");
	  Serial.println(s);
	  return -1;
  }
  return p_sensor->read_rawvoltage();
}

float sensorManager::read_sensor_pressurevalues(sensor_e s) {
  sensor *p_sensor = NULL;
  int err = 0;

  switch(s) {
    case SENSOR_PRESSURE_A0:
	  p_sensor = &_pS1;
	break;
	case SENSOR_PRESSURE_A1:
	  p_sensor = &_pS2;
	break;
	case SENSOR_DP_A0:
	  p_sensor = &_dpS1;
	break;
	case SENSOR_DP_A1:
	  p_sensor = &_dpS2;
	break;
	case SENSOR_O2:
	  p_sensor = &_o2S;
	break;
	default:
	  Serial.print(s);
	  Serial.println(" ERROR: Invalid sensor read request");
	  return 0.0;
	break;
  }
  err = p_sensor->get_error();
  if(err) {
	  Serial.print("ERROR: Sensor read error for ");
	  Serial.println(s);
	  return -1;
  }
  return p_sensor->read_sensorpressure();
}


