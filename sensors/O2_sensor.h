/**************************************************************************/
/*!
    @file     O2_sensor.h

    @brief 	  O2 sensor module

    @author   Tworks

	@defgroup VentilatorModule

    O2 sensor module - allows clients to read sensor readings
	sensor data is updated to local data structure in a timer
	in frequent intervals
	The same readings can be read or reset using utility APIs
	@{
*/
/**************************************************************************/
#ifndef __O2_SENSOR_H__
#define __O2_SENSOR_H__

#include "sensors.h"

/**************************************************************************/
/*!
    @brief  Class to handle O2 sensor, inherits base sensor class
*/
/**************************************************************************/
class o2_sensor : public sensor {
	protected:
		float m_slope;		/*!< Stores the slope measured from the sensor voltage at different water level  */
		float m_const;		/*!< Stores the const measured from the sensor voltage at different water level  */
		int m_adc_channel;	/*!< adc channel where the sensor is connected to */
		Adafruit_ADS1115 *m_ads;	/*!< ADS board where the sensor is connected to */
	protected:
		/**
		 *   @brief  Stores the default calibrated values to memory during boot
		 *   @return returns 0 on success and -1 on failure
		 **/
		int store_default_o2_calibration_data();
		/**
		 *   @brief  Calibrate the O2 sensor during boot
		 *   @return returns 0 on success and -1 on failure
		 **/
		int calibrate_o2_sensor();
	public:
		/**
		 *   @brief  Constructor for O2 sensors
		 *           Initializes O2 sensor variables
		 **/
		o2_sensor(Adafruit_ADS1115 *ads, int adc_channel) : sensor() { m_adc_channel = adc_channel; m_ads = ads; };
       /**
		 *   @brief  Function to initialize the O2 sensor
		 *   @return 0 on success and -1 on error
		 **/
		int init();
        /**
		 *   @brief  Function to read sensor data
		 *   @return Returns the readings from sensor
		 **/
		float read_sensor_data();
        /**
		 *   @brief  Function to reset sensor data
		 *   @return None
		 **/
		void reset_sensor_data();
        /**
		 *   @brief  Function to read and update sensor data in local data structures (called in timer interrupt)
		 *   @return None
		 **/
		void update_sensor_data();
};

#endif /*__O2_SENSOR_H__*/