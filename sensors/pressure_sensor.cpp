/**************************************************************************/
/*!
    @file     pressure_sensor.cpp

    @brief 	  Pressure sensor module

    @author   Tworks

	@defgroup VentilatorModule

    Module to read and measure pressure/flow rate 
    from the pressure sensors
	@{
*/
/**************************************************************************/

#include "pressure_sensor.h"
#include "ads1115_utils.h"

/*
 * Macros to enable the sensor functionalities
 */
#define MPXV7002DP  1
#define SPYRO_DLITE 1
#define MPX5010     1

/*
 * Sensor specific configurations
 */
#ifdef MPX5010
#define MPX5010_VS              (5.0)
#define MPX5010_VFSS            (0.0)
#define MPX5010_ACCURACY        (0.05 * MPX5010_VFSS)
#define MPX5010_ZERO_READING    2000
#define MPX5010_ERROR_THRESHOLD 20
#endif

#ifdef MPXV7002DP
#define MPXV7002DP_VS           (5.0)
#define MPXV7002DP_VFSS         (4.0)
#define MPXV7002DP_ACCURACY     (0.06250)
#define MPX7002DP_ZERO_READING  22000 // for Vs 5.0 -> 2.75v(@zero pressure) * ADS115_MULTIPLIER = 22000
#define MPX7002DP_ERROR_THRESHOLD 20
#endif

/*
 * Pressure sensors configurations
 */
#define SPYRO_KSYSTEM           110 // Ksystem assumed for spyro
#define FLOWRATE_MIN_THRESHOLD  4.0
#define CALIBRATION_COUNT       5

int _debug = 0;

/*
 * Initialization routine to setup the sensors
 * Calibrate the sensors for errors
 */
int pressure_sensor::init() {
  int err = 0;
  
  delay(20); //delay of 20ms for sensors to come up and respond

  // Initialize the data
  this->m_data.current_data.pressure = 0.0;
  this->m_data.previous_data.pressure = 0.0;
  if(m_dp == 1) {
	this->m_data.actual_at_zero = MPX7002DP_ZERO_READING;
	this->m_data.error_threshold = MPX7002DP_ERROR_THRESHOLD;
  } else {
	this->m_data.actual_at_zero = MPX5010_ZERO_READING;
	this->m_data.error_threshold = MPX5010_ERROR_THRESHOLD;
  }
  // Calibrate the sensors
  err = sensor_zero_calibration();
  if(err) {
    Serial.println("ERROR: Initializing pressure sensors");
    return err;
  }
  return 0;
}

/*
 * Function to be called from timer interrupt to read
 * and update the samples in local data structures
 */
void pressure_sensor::update_sensor_data(void) {
	m_sample_index = m_sample_index + 1;
	if(m_sample_index >= MAX_SENSOR_SAMPLES) {
		m_sample_index = 0;
	}
	if(m_dp == 1) {
		this->m_data.current_data.flowvolume += get_spyro_volume_MPX7002DP();
		this->samples[m_sample_index] = this->m_data.current_data.flowvolume;
	} else {
		this->m_data.current_data.pressure = get_pressure_MPX5010();
		this->samples[m_sample_index] = this->m_data.current_data.pressure;
	}
}

/*
 * Function to reset the local data structures
 */
void pressure_sensor::reset_sensor_data(void) {
	for(int index = 0; index < MAX_SENSOR_SAMPLES; index++) {
		this->samples[index] = 99.99;
	}
	if(m_dp == 1) {
		this->m_data.current_data.flowvolume = this->m_data.current_data.flowvolume = 0;
	} else {
		this->m_data.current_data.pressure = this->m_data.current_data.pressure = 0;
	}
}

/*
 * Function to read stored sensor data from the
 * local data structres
 */
float pressure_sensor::read_sensor_data() {
	if(m_dp == 1) {
		this->m_data.previous_data.flowvolume = this->m_data.current_data.flowvolume;
		return this->m_data.previous_data.flowvolume;
	} else {
		this->m_data.previous_data.pressure = this->m_data.current_data.pressure;
		return this->m_data.previous_data.pressure;
	}
}

#ifdef MPX5010
/*
 * Function to return the pressure from the sensor
 * vout = vs(0.09P + 0.04) + 5%VFSS
 * P = (vout - (0.005*VFSS) - (Vs*0.04))/(VS * 0.09)
 */
float pressure_sensor::get_pressure_MPX5010() {
  float pressure = 0.0;
  float vout = 0.0;
  int err = 0;

  err = ADS1115_ReadVoltageOverI2C(m_ads, m_adc_channel, m_data.actual_at_zero, m_data.error_at_zero, &vout);
  if(ERROR_I2C_TIMEOUT == err) {
	  Serial.println("ERROR: Sensor read I2C timeout failure\n");
	  this->set_error(ERROR_SENSOR_READ);
	  return 0.0;
  }
  pressure = ((vout - (MPX5010_ACCURACY) - (MPX5010_VS * 0.04))/(MPX5010_VS * 0.09));
  // Error correction on the pressure, based on the H2O calibration
  pressure = ((pressure - 0.07)/0.09075);
  if(_debug) {
    Serial.print("C");
    Serial.print(" ");
    Serial.print(m_adc_channel);
    Serial.print(", V");
    Serial.print(" ");
    Serial.print(vout, 4);
    Serial.print(" ");
    Serial.print(", P");
    Serial.print(" ");
    Serial.println(pressure, 4);
  }
  return pressure;
}
#endif

/*
 * Function to calculate sensor errors during boot
 */
int pressure_sensor::sensor_zero_calibration() {
  float sample = 0.0;
  float avg = 0.0;
  int err = 0;

  for(int index = 0; index < CALIBRATION_COUNT; index++) {
    err = ADS1115_ReadAvgSamplesOverI2C(m_ads, m_adc_channel, &sample);
    if(err) {
	  Serial.println("ERROR: Sensor calibration failure\n");
	  return ERROR_SENSOR_CALIBRATION;
    }
	avg += sample;
    delay(10);
  }
  this->m_data.error_at_zero = ((avg/CALIBRATION_COUNT) - this->m_data.actual_at_zero);
  
  if(abs(this->m_data.error_at_zero) > this->m_data.error_threshold) {
	  Serial.println("ERROR: calibration error is more than the threshold value");
	  return ERROR_SENSOR_CALIBRATION;
  }
  if(_debug) {
    Serial.print("ZE");
    Serial.print(" ");
    Serial.println(this->m_data.error_at_zero);
  }
  return 0;
}

#ifdef MPXV7002DP
/*
 * Function to get the flow rate of spyro
 */
float pressure_sensor::get_spyro_volume_MPX7002DP() {
  float vout = 0.0;
  float pressure = 0.0;
  float flowrate = 0.0, accflow = 0.0;
  int err = 0;

  err = ADS1115_ReadVoltageOverI2C(m_ads, m_adc_channel, m_data.actual_at_zero, m_data.error_at_zero, &vout);
  if(ERROR_I2C_TIMEOUT == err) {
	  Serial.println("ERROR: Sensor read failure\n");
	  this->set_error(ERROR_SENSOR_READ);
	  return 0.0;
  }
  pressure = get_pressure_MPXV7002DP(vout);
  flowrate = get_flowrate_spyro(pressure);
  if(flowrate > FLOWRATE_MIN_THRESHOLD) {
    accflow = (((flowrate * 1000)/60000) * SENSOR_TIMER_IN_MS);
  }
  if(_debug) {
    Serial.print("C");
    Serial.print(" ");
    Serial.print(m_adc_channel);
    Serial.print(", V");
    Serial.print(" ");
    Serial.print(vout * 1000, 6);
    Serial.print(", P");
    Serial.print(" ");
    Serial.print(pressure, 6);
    Serial.print(", F");
    Serial.print(" ");
    Serial.print(flowrate, 6);
    Serial.print(", AF");
    Serial.print(" ");
    Serial.println(accflow, 6);
  }
  return accflow;
}

/*
 * Vout = Vs (0.2P + 0.5) +- accuracy%VFSS
 * P = (Vout - accuracy*VFSS/100 - (Vs * 0.5))/(0.2 * Vs)
 */
float pressure_sensor::get_pressure_MPXV7002DP(float vout) {
  static float lastPressure = 0.0;
  float tmppressure = 0.0;
  float pressure = 0.0;
  float correction = (MPXV7002DP_ACCURACY * MPXV7002DP_VFSS);
  pressure = (vout - correction - (MPXV7002DP_VS * 0.5))/(0.2 * MPXV7002DP_VS);
  tmppressure = pressure = ((lastPressure * 0.2) + (pressure * 0.8));
  lastPressure = pressure;
  if (tmppressure < 0)
      return tmppressure;
  else 
      return 0;
}
#endif

#ifdef SPYRO_DLITE
/*
 * Flowrate = Ksystem * sqrt(pressuredifference)
 * API to return flow rate in liters per minute
 */
float pressure_sensor::get_flowrate_spyro(float pressure) {
  float flowrate = SPYRO_KSYSTEM * sqrt(abs(pressure));
  if(pressure > 0)
    return 0;
  return flowrate;
}
#endif

