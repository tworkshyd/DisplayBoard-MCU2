/**************************************************************************/
/*!
    @file     sensors.h

    @brief 	  Base sensor class

    @author   Tworks

	@defgroup VentilatorModule

	Sensor module that interfaces with other modules.
	Allows other modules to enable/read and reset the
	sensors available.
	@{
*/
/**************************************************************************/
#ifndef __SENSORS_H__
#define __SENSORS_H__

#include "./../3plibs/MsTimer2/MsTimer2.h"
#include "./../3plibs/Adafruit_ADS1X15/Adafruit_ADS1015.h"

#define MAX_SENSOR_SAMPLES		5   /*!< maximum number of sensor samples to be taken */
#define I2C_TIMEOUT             120        /*!< I2C timeout value */
#define SENSOR_TIMER_IN_MS      60         /*!< Timer for reading the sensor data */

#define ACCUMULATOR_RESET_PIN   3
/**
 * @brief sensor_data_t where the sensor reading is stored
 */
typedef union {
    float flowvolume;   
    float pressure;    
    float O2;         
} sensor_data_t;     

/** @struct  sensor_t
 *  @brief   Structure contains sensor specific data.
 *           Calibration data, previous and current data read from sensors.
 */
typedef struct {
  float actual_at_zero;   /*!< Actual sensor data read currently*/
  float error_at_zero;    /*!< error part for the actual sensor data */
  sensor_data_t previous_data;   /*!< Previous recorded sensor data */
  sensor_data_t current_data;     /*!< Current sensor data */
} sensor_t;

/**
 * @enum   sensor_e
 * @brief   Following are flags to enable sensors 
 */
typedef enum {
  SENSOR_PRESSURE_A0, /**< peak pressure sensor MPX5010 */
  SENSOR_PRESSURE_A1, /**< peep pressure sensor MPX5010 */  
  SENSOR_DP_A2,       /**< differential pressure sensor for Peak pressure MPXV7002DP */  
  SENSOR_DP_A3,       /**< differential pressure sensor for Peek pressure MPXV7002DP */
  SENSOR_O2,          /**< oxygen sensor */
  MAX_SENSORS
} sensor_e;
/**
 * @enum   sensor flag
 * @brief   Following are the sensor flags.
 */
enum {
  PRESSURE_A0 = 1, /**< flag to select  peak pressure sensor */
  PRESSURE_A1 = 2, /**< flag to select peep pressure sensor */
  DP_A2 = 4,       /**< flag to select differential pressure sensor for Peak pressure */
  DP_A3 = 8,       /**< flag to select differential pressure sensor for Peek pressure*/
  O2 = 16          /**< flag to select oxygen sensor */
};

/**************************************************************************/
/*!
    @brief  Base class for all sensors
*/
/**************************************************************************/
class sensor {
public:
	sensor_t m_data; 					/*!< Sensor calibration data and sensor readings stored here */
	float samples[MAX_SENSOR_SAMPLES];  /*!< recent sensor readings stored here */
	int m_sample_index;                 /*!< index pointing to the lastest sensor reading */
public:
    /**
	 *   @brief  Constructor for sensors
	 *           Initializes all sensor variables
	 **/
	sensor() { m_data = {0}; m_sample_index = 0; };
        /**
	 *   @brief  Function to read the sensor samples
	 *   @param samples - Sensor readings
	 *   @param sample_count - number of sensor readings
	 *   @return <TBD description>
	 **/
	int read_sensor_samples(float *samples, int sample_count);
        /**
	 *   @brief  Function to initialize the sensors
	 *           Pure virtual function, child class should implement this
	 *   @return 0 on success and -1 on error
	 **/
	virtual int init() = 0;
        /**
	 *   @brief  Function to read sensor data
	 *			 Pure virtual function, child class should implement this
	 *   @return Returns the readings from sensor
	 **/
	virtual float read_sensor_data() = 0;
        /**
	 *   @brief  Function to reset sensor data
	 *           Pure virtual function, child class should implement this
	 *   @return None

	 **/
	virtual void reset_sensor_data() = 0;
        /**
	 *   @brief  Function to update sensor data (called in timer interrupt)
	 *           Pure virtual function, child class should implement this
	 *   @return None
	 **/
	virtual void update_sensor_data() = 0;
};

/**************************************************************************/
/*!

    @brief  Function to initialize all modules in sensors

    @return indicates 0 for SUCCESS and -1 for FAILURE
*/
/**************************************************************************/
int sensors_init();
/**************************************************************************/
/*!

    @brief   Function to enable specific sensors or Takes a parameter of different sensor combinations
 
    @param flag defines which sensor or a combination of sensors to be enabled

    @param source parameter to set the source of input

    @return indicates Non-Null for SUCCESS and Null for FAILURE
*/
/**************************************************************************/
void enable_sensor(unsigned int flag);
/**************************************************************************/
/*!

    @brief Wrapper API to get specific sensor readings.

    @param sensor_e sensor describes which sensor to be read.

    @return indicates Non-Null for SUCCESS and Null for FAILURE
*/
/**************************************************************************/
float read_sensor_data(sensor_e sensor);
/**************************************************************************/
/*!

    @brief  Function  Checks the previous pressure readings and find if there are any dip in pressure 
                    
    @param paranName paramter is used to specify the 

    @return -1 for invlaid sensors, 0 for no dip 1 for a dip in pressure
 
*/
/**************************************************************************/
int check_for_dip_in_pressure(sensor_e sensor);

#endif /*__SENSORS_H__*/
