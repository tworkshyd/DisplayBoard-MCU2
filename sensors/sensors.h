/**************************************************************************/
/*!
    @file     sensors.h

    @brief 	  Base sensor class

    @author   Tworks

	@defgroup BaseSensorsModule

	Sensor module that interfaces with other modules.
	Allows other modules to enable/read and reset the
	sensors available.
	@{
*/
/**************************************************************************/
#ifndef __SENSORS_H__
#define __SENSORS_H__

//#include "./../libraries/MsTimer2/MsTimer2.h"
#include "./../libraries/Adafruit_ADS1X15/Adafruit_ADS1015.h"

/**
 * @def 	Defines 
 * @brief 	Defines used in this file
 */
#define SUCCESS						0
#define ERROR_OFFSET				(-1)
#define ERROR_UNKNOWN				(ERROR_OFFSET+1)
#define ERROR_I2C_TIMEOUT   		(ERROR_OFFSET+2)
#define ERROR_TIMER_INIT    		(ERROR_OFFSET+3)
#define ERROR_SENSOR_READ			(ERROR_OFFSET+4)
#define ERROR_BAD_PARAM				(ERROR_OFFSET+5)
#define ERROR_SENSOR_UNSUPPORTED	(ERROR_OFFSET+6)
#define ERROR_SENSOR_CALIBRATION	(ERROR_OFFSET+7)


#define SENSOR_DATA_PRECISION	100000
#define MAX_SENSOR_SAMPLES		5   /*!< maximum number of sensor samples to be taken */
#define ACCUMULATOR_RESET_PIN   3

#define ADC_CONVERSIONTIME_PERSENSOR    (MAX_SAMPLE_COUNT * ADC_CONVERSTION_TIME)       /*!< Timer for reading the sensor data */
#define MINREQUIRED_DISPLAYREFRESH_TIME 50
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
  sensor_data_t previous_data;   /*!< Previous recorded sensor data */
  sensor_data_t current_data;     /*!< Current sensor data */
} sensor_t;

/**
 * @enum   sensor_e
 * @brief   Identifier for all sensors present in the system
 */
typedef enum {
  SENSOR_PRESSURE_A0 = 0, /**< peak pressure sensor MPX5010 */
  SENSOR_PRESSURE_A1, /**< peep pressure sensor MPX5010 */  
  SENSOR_DP_A0,       /**< differential pressure sensor for Peak pressure MPXV7002DP */  
  SENSOR_DP_A1,       /**< differential pressure sensor for peep pressure MPXV7002DP */
  SENSOR_O2,          /**< oxygen sensor */
  MAX_SENSORS
} sensor_e;

/**
 * @enum   sensor_flags_e flags to enable different sensors
 * @brief   Following are the flags to enable/disable specific sensors
 */
typedef enum {
  PRESSURE_A0 = 1, /**< flag to select  peak pressure sensor */
  PRESSURE_A1 = 2, /**< flag to select peep pressure sensor */
  DP_A0 = 4,       /**< flag to select differential pressure sensor for Peak pressure */
  DP_A1 = 8,       /**< flag to select differential pressure sensor for Peek pressure*/
  O2 = 16          /**< flag to select oxygen sensor */
} sensor_flags_e;


/**************************************************************************/
/*!
    @brief  Base class for all sensors
*/
/**************************************************************************/
class sensor {
public:
	int m_error;						/*!< Stores the last error in the sensor */
	sensor_t m_data; 					/*!< Sensor calibration data and sensor readings stored here */
	float samples[MAX_SENSOR_SAMPLES];  /*!< recent sensor readings stored here */
	int m_sample_index;                 /*!< index pointing to the lastest sensor reading */
public:
    /**
	 *   @brief  Constructor for sensors
	 *           Initializes all sensor variables
	 *   @param  none
	 **/
	sensor() { m_data = {{0},{0}}; m_sample_index = 0; m_error = 0; };
    /**
	 *   @brief  Function to read the sensor samples
	 *   @param samples - Sensor readings
	 *   @param sample_count - number of sensor readings
	 *   @return average of all sensor samples
	 **/
	bool check_for_dip();
    /**
	 *   @brief  setter function for m_error
	 *   @param err error code
	 *   @return none
	 **/
	void set_error(int err) { m_error = err; }
    /**
	 *   @brief  getter function for m_error
	 *   @param none
	 *   @return error code stored in m_error
	 **/
	int get_error(void)  { return m_error; }
    /**
	 *   @brief  Function to initialize the sensors
	 *           Pure virtual function, child class should implement this
	 *   @param  none
	 *   @return 0 on success and -1 on error
	 **/
	virtual int init(void) = 0;
        /**
	 *   @brief  Function to read sensor data
	 *			 Pure virtual function, child class should implement this
	 *   @param  none
	 *   @return Returns the readings from sensor as float
	 **/
	virtual float read_sensor_data(void) = 0;
        /**
	 *   @brief  Function to reset sensor data
	 *           Pure virtual function, child class should implement this
	 *   @param  none
	 *   @return None

	 **/
	virtual void reset_sensor_data(void) = 0;
        /**
	 *   @brief  Function to update sensor data (called in timer interrupt)
	 *           Pure virtual function, child class should implement this
	 *   @param  none
	 *   @return None
	 **/
	virtual void capture_and_store(void) = 0;

  int read_rawvoltage() {
    return m_raw_voltage;
  }

  float read_sensorpressure() {
    return m_value;
  }
  /**
   *   @brief  Calibrate the sensor
   *   @param  None
   *   @return returns 0 on success and -1 on failure as integer
   **/
  virtual int sensor_zero_calibration(void) = 0;


protected:
  int m_raw_voltage;
  float m_value = 0.0;
  sensor_e m_sensor_id;

};
#endif /*__SENSORS_H__*/

/**@}*/
