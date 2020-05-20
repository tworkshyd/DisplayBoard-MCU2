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
#include "sensors.h"
#include "pressure_sensor.h"
#include "O2_sensor.h"
#include "pressure_sensor.cpp"
#include "O2_sensor.cpp"
#include "ads1115_utils.c"
#include "./../3plibs/MsTimer2/MsTimer2.cpp"

#define MAX_PS2_SAMPLES 		10
#define THRESHOLD_COMPARE_INDEX 	2
#define DIP_THRESHOLD 			15 //better to be lower than PEEP
#define ADS115_INT_PIN          	6


Adafruit_ADS1115 ads(ADS1015_ADDRESS, ADS115_INT_PIN);
pressure_sensor g_p1(&ads, 0), g_p2(&ads, 1);
dpressure_sensor g_dp1(&ads, 2), g_dp2(&ads, 3);
o2_sensor g_o2(NULL, OXYGEN_ANALOG_PIN);

unsigned int enabled_sensors = 0;

void read_and_update_sensor_data(void);

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
    volume0 = read_sensor_data(SENSOR_DP_A2);
    volume1 = read_sensor_data(SENSOR_DP_A3);
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
	return 0;
}

/*
 * Function to initialize all modules in sensors
 */
int sensors_init() {
	int err = 0;

	err += ads1115_init();
	err += g_p1.init();
	err += g_p2.init();
	err += g_dp1.init();
	err += g_dp2.init();
	err += g_o2.init();

    MsTimer2::set(120, read_and_update_sensor_data);
    MsTimer2::start();
	return err;
}

/*
 * Function to enable specific sensors
 * Takes a parameter of different sensor combinations
 */
void enable_sensor(unsigned int flag) {
  if((enabled_sensors & PRESSURE_A0) && !(flag & PRESSURE_A0)) {
	g_p1.reset_sensor_data();
  }
  if((enabled_sensors & PRESSURE_A1) && !(flag & PRESSURE_A1)) {
	g_p2.reset_sensor_data();
  }
  if((enabled_sensors & DP_A2) && !(flag & DP_A2)) {
	g_dp1.reset_sensor_data();
  }
  if((enabled_sensors & DP_A3) && !(flag & DP_A3)) {
	g_dp2.reset_sensor_data();
  }
  if((enabled_sensors & O2) && !(flag & O2)) {
	g_o2.reset_sensor_data();
  }
  enabled_sensors = flag;
}

/*
 * Wrapper API to get specific sensor readings
 */
int read_sensor_data(sensor_e s, float *data) {
  sensor *p_sensor = NULL;
  int err = 0;

  switch(s) {
    case SENSOR_DP_A2:
	  p_sensor = &g_dp1;
	break;
	case SENSOR_DP_A3:
	  p_sensor = &g_dp2;
	break;
	case SENSOR_PRESSURE_A0:
	  p_sensor = &g_p1;
	break;
	case SENSOR_PRESSURE_A1:
	  p_sensor = &g_p2;
	break;
	case SENSOR_O2:
	  p_sensor = &g_o2;
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
 * Function to aggregate or read sensor data
 * in an timer interrupt
 */
void read_and_update_sensor_data(void) {
//  long int starttime = millis();

  interrupts(); // Called to enable other interrupts.
  if(enabled_sensors & PRESSURE_A0) {
    g_p1.update_sensor_data();
  }
  if(enabled_sensors & PRESSURE_A1) {
    g_p2.update_sensor_data();
  }
  if(enabled_sensors & DP_A2) {
    g_dp1.update_sensor_data();
  }
  if(enabled_sensors & DP_A3) {
    g_dp2.update_sensor_data();
  }
  if(enabled_sensors & O2) {
    g_o2.update_sensor_data();
  }
//  Serial.print("T:");
//  Serial.println(millis()-starttime);
}

/*
 * Checks the previous pressure readings and find
 * if there are any dip in pressure
 * returns -1 for invlaid sensors, 0 for no dip
 * 1 for a dip in pressure
 */
int check_for_dip_in_pressure(sensor_e sensor) {
	float sensor_data[MAX_SENSOR_SAMPLES] = {99.99};
	int count = 0;

	if(sensor == SENSOR_PRESSURE_A0) {
		count = g_p1.read_sensor_samples(&sensor_data[0], MAX_SENSOR_SAMPLES);
	} else if(sensor == SENSOR_PRESSURE_A1) {
		count = g_p2.read_sensor_samples(&sensor_data[0], MAX_SENSOR_SAMPLES);
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
