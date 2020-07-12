#pragma once

#include "sensors.h"
#include "pressure_sensor.h"
#include "O2_sensor.h"

//#define OPEN_BOARD 0
Adafruit_ADS1115 ads(ADS1015_ADDRESS, ADS115_INT_PIN);
Adafruit_ADS1115 ads1(ADS1015_ADDRESS_1, ADS115_INT_PIN_1);

class sensorManager {
public:
/**************************************************************************/
/*!

    @brief  Function to initialize all modules in sensors

	@param  none

    @return indicates 0 for SUCCESS and -1 for FAILURE
*/
/**************************************************************************/
int init(void);
/**************************************************************************/
/*!

    @brief   Function to enable specific sensors or Takes a parameter of different sensor combinations
 
    @param flag defines which sensor or a combination of sensors to be enabled

    @param source parameter to set the source of input

    @return none
*/
/**************************************************************************/
void enable_sensor(unsigned int flag);
/**************************************************************************/
/*!

    @brief Wrapper API to get specific sensor readings.

    @param sensor_e sensor describes which sensor to be read.

    @param data returns the sensor data read from sensor

    @return 0 for success and error code for any failures
*/
/**************************************************************************/
int read_sensor_data(sensor_e sensor, float *data);
/**************************************************************************/
/*!

    @brief  Function  Checks the previous pressure readings and find if there are any dip in pressure 
                    
    @param paranName paramter is used to specify the 

    @return -1 for invlaid sensors, 0 for no dip 1 for a dip in pressure
 
*/
/**************************************************************************/
int check_for_dip_in_pressure(sensor_e sensor);

int read_sensor_rawvoltage(sensor_e s);

float read_sensor_pressurevalues(sensor_e s);

int start_calibration(void);

unsigned int get_enable_sensors();

void capture_sensor_data();

private:
#ifdef OPEN_BOARD
  pressure_sensor _pS2 = pressure_sensor(&ads, 1, SENSOR_PRESSURE_A0);
  pressure_sensor _pS1 = pressure_sensor(&ads1, 1, SENSOR_PRESSURE_A1);
  dpressure_sensor _dpS1= dpressure_sensor(&ads, 0, SENSOR_DP_A0);
  dpressure_sensor _dpS2 = dpressure_sensor(&ads1, 0, SENSOR_DP_A1);
#else
  pressure_sensor _pS1 = pressure_sensor(&ads1, 0, SENSOR_PRESSURE_A0);
  pressure_sensor _pS2 = pressure_sensor(&ads, 0, SENSOR_PRESSURE_A1);
  dpressure_sensor _dpS1= dpressure_sensor(&ads, 1, SENSOR_DP_A0);
  dpressure_sensor _dpS2 = dpressure_sensor(&ads1, 1, SENSOR_DP_A1);
#endif

  o2_sensor _o2S = o2_sensor(&ads, 2);

  unsigned int _enabled_sensors = 0;


  unsigned long _timervalueMs = -1; // starting with -1
 

};
