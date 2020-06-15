/**************************************************************************/
/*!
    @file     O2_sensor.cpp

    @brief 	  O2 sensor module

    @author   Tworks

	@defgroup VentilatorModule

    Module to read and measure O2 % from the sensor
	@{
*/
/**************************************************************************/

#include "O2_sensor.h"

#define AVOID_EEPROM 			0
#define NUM_OF_SAMPLES_O2		5
#define EEPROM_O2_CALIB_ADDR	0xC

int const x_samples[NUM_OF_SAMPLES_O2] = {0, 216, 280, 400, 1000};
int const yO2VoltX1000[NUM_OF_SAMPLES_O2] = {377, 1088, 1750, 2110, 4812};

void write_to_eeprom(unsigned int numOfIntWrites, int * addr, int *val);

/*
 * Function to initialize the O2 sensors
 */
int o2_sensor::init() {
	int err = 0;
	err += store_default_o2_calibration_data();
	err += sensor_zero_calibration();
	return err;
}

/*
 * Stores the default calibration data for O2
 * sensor
 */
int o2_sensor::store_default_o2_calibration_data() {
  // put your setup code here, to run once:
  int write_to_mem = 0;
  //TODO: Bharath: Fill the right value here.
  int *addr_write_to_mem = EEPROM_O2_CALIB_ADDR;

#if AVOID_EEPROM
  write_to_mem = 1;
#else
  write_to_mem = retrieveCalibParam(addr_write_to_mem);
#endif
  
  if (write_to_mem) {
      write_to_eeprom(NUM_OF_SAMPLES_O2,
          EEPROM_O2_CALIB_ADDR,
          yO2VoltX1000);
  }
    
#if AVOID_EEPROM
    *(addr_write_to_mem) = 0;
#else
    storeCalibParam(addr_write_to_mem, false);
#endif
	return 0;
}

void write_to_eeprom(unsigned int numOfIntWrites, int * addr, int *val) {
  unsigned int index;
  if ((numOfIntWrites == 0)||
      (addr == NULL)) {
    return;
  }
  for (index=0; index<numOfIntWrites; index++) {
#if AVOID_EEPROM
    *addr = *val;
#else
    storeCalibParam(addr,*val);
#endif
    addr++;
    val++;
  }
}


/*
 * main function to calibrate and get m (slope) and c (constant) after curve fitting
 * read y data from eeprom
 * convert from byte to float
 * use in algo to calc m and c values.
 */
int o2_sensor::sensor_zero_calibration()
{
  float x = 0, sigmaX = 0, sigmaY = 0, sigmaXX = 0, sigmaXY = 0, denominator = 0, y = 0;
  int value = 0;
  int *eeprom_addr = EEPROM_O2_CALIB_ADDR;
  
  for (int index = 0; index < NUM_OF_SAMPLES_O2; index++) {
#if AVOID_EEPROM
    y = ((float)(*addr))/1000;
#else
    value = retrieveCalibParam(eeprom_addr);
    y = ((float)value)/1000;
#endif
    eeprom_addr += 1;
    
    x = (float(x_samples[index]))/10;
    sigmaX += x;
    sigmaY += y;
    sigmaXX += x * x;
    sigmaXY += x * y;
  }
  denominator = (NUM_OF_SAMPLES_O2 * sigmaXX) - (sigmaX*sigmaX);
  if (denominator != 0) {
    m_slope = ((NUM_OF_SAMPLES_O2 * sigmaXY) - (sigmaX*sigmaY)) / denominator;
    m_const = ((sigmaY*sigmaXX) - (sigmaX*sigmaXY)) / denominator;
  } else {
    m_slope = 0;
    m_const = 0;
    Serial.println("Error: O2 calibration failed!!");
    return ERROR_SENSOR_CALIBRATION;
  }
}

/*
 * Function to read stored sensor data from the
 * local data structres
 */
float o2_sensor::read_sensor_data() {
  //return 20;
	this->m_data.previous_data.O2 = this->m_data.current_data.O2;
  //Serial.print("o2 read_sensor_data :");
  //Serial.println(this->m_data.previous_data.O2);

	return this->m_data.previous_data.O2;
}

/*
 * Function to reset the local data structures
 */
void o2_sensor::reset_sensor_data() {
	this->m_data.current_data.O2 = this->m_data.previous_data.O2 = 0;
}

/*
 * Function to be called from timer interrupt to read
 * and update the samples in local data structures
 */
void o2_sensor::capture_and_store(){
  float o2_value = 0.0;
  float vout = 0.0;
  int err = 0;
#if 0  
  err = ADC_ReadVolageOnATMega2560(m_ads, m_adc_channel, m_data.error_at_zero, &vout);
  if(err) {
    Serial.print("ERROR: Sensor read I2C timeout failure for:SENSOR_O2");
	  this->set_error(ERROR_SENSOR_READ);
	  return;
  } else {
	  this->set_error(SUCCESS);
  }
  if (m_slope != 0) {
	  this->m_data.current_data.O2 = (vout - m_const) / m_slope;
  }
#else
 err = ADS1115_ReadVoltageOverI2C(m_ads, m_adc_channel, m_data.actual_at_zero, m_data.error_at_zero, &vout);
  if(ERROR_I2C_TIMEOUT == err) {
    Serial.print("ERROR: Sensor read I2C timeout failure for:");
    Serial.println(sensorId2String(m_sensor_id));
    this->set_error(ERROR_SENSOR_READ);
    return 0.0;
  } else {
     this->set_error(SUCCESS);
  }
  m_raw_voltage = vout*1000;
  o2_value = ((vout ) +0.2034897959) /0.05099489796;
#if DEBUG_PRESSURE_SENSOR
  if ((millis() - m_lasO2UpdatedTime) > SENSOR_DISPLAY_REFRESH_TIME)
  {  
    m_lasO2UpdatedTime = millis();
    Serial.print("sensorType->");
    Serial.print(sensorId2String(m_sensor_id));
    Serial.print("::"); 
    Serial.print("C");
    Serial.print(" ");
    Serial.print(m_adc_channel);
    Serial.print(", V");
    Serial.print(" ");
    Serial.print(vout, 4);
    Serial.print(" ");
    Serial.print(", O2");
    Serial.print(" ");
    Serial.print((o2_value), 4);
    Serial.print(", raw");
    Serial.print(" ");
    Serial.println((m_raw_voltage ), 4);
  }
#endif
#endif
  this->m_data.current_data.O2 = o2_value ;
}
