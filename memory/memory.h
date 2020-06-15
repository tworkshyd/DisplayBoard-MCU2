
/**************************************************************************/
/*!
    @file     hbad_memory.h

    @brief      This module contains base addresses and range  for oxygen calibration, ps1, ps2, dps1, dps2, methods to store and retrieve.

    @author   Tworks
    
  @defgroup MemoryModule  MemoryModule
   
     MemoryModule contains base addresses and range  for oxygen calibration, ps1, ps2, dps1, dps2, methods to store and retrieve
  
  @{
*/
/**************************************************************************/



#include <extEEPROM.h>


/*! It is Default Data Write Addr< */
#define EEPROM_DEFAULT_DATA_WRITE_ADDR 0xA //10

/*! EEPROM Oxygen CALIB ADDR  */
#define EEPROM_O2_CALIB_ADDR 0xC //12 - 22
#define NUM_OF_SAMPLES_O2 5

/*! EEPROM PG1 CALIB ADDR  */
#define EEPROM_PG1_CALIB_ADDR 0x28 //40 - 78
#define NUM_OF_SAMPLES_PS1 19

/*!EEPROM PS2 CALIB ADDR  */
#define EEPROM_PS2_CALIB_ADDR  0
#define NUM_OF_SAMPLES_PS2 0

/*!EEPROM DPS1 CALIB ADDR  */
#define EEPROM_DPS1_CALIB_ADDR  0
#define NUM_OF_SAMPLES_DPS1 0

/*!EEPROM DPS2 CALIB ADDR  */
#define EEPROM_DPS2_CALIB_ADDR  0
#define NUM_OF_SAMPLES_DPS2 0

/*!EEPROM I2C Address  */
#define EEPROM_I2C_ADDR 0x50 //80
/*!< */
#define EEPROM_BASE_ADDR 0xC8
#define GUARD_VALUE 0x4
#define EEPROM_CALIBRATION_STORE_ADDR (EEPROM_BASE_ADDR+ (MAX_CTRL_PARAMS*2) + GUARD_VALUE)

//const uint32_t totalKBytes = 64;

/*!device size, number of devices, page size< */
extEEPROM hbad_mem(kbits_256, 1, 32, EEPROM_I2C_ADDR);

/**************************************************************************/
/*!

    @brief Function to store specific the caliberated parameter.

    @param sensor_e sensor describes which sensor to be read.

    @param data returns the sensor data read from sensor

    @return 0 for success and error code for any failures
*/
/**************************************************************************/
void storeParam(ctrl_parameter_t param);

/**************************************************************************/
/*!

    @brief Function to get specific the caliberated parameter.

    @param sensor_e sensor describes which sensor to be read.

    @param data returns the sensor data read from sensor

    @return 0 for success and error code for any failures
*/
/**************************************************************************/
void retrieveParam(ctrl_parameter_t param);

/**************************************************************************/
/*!

    @brief Function to get all the caliberated parameter.

    @param sensor_e sensor describes which sensor to be read.

    @param data returns the sensor data read from sensor

    @return 0 for success and error code for any failures
*/
/**************************************************************************/
void getAllParamsFromMem();

/**************************************************************************/
/*!

    @brief Function to store the caliberated parameter  on specific address with specified data.

    @param sensor_e sensor describes which sensor to be read.

    @param data returns the sensor data read from sensor

    @return 0 for success and error code for any failures
*/
/**************************************************************************/
void storeCalibParam(int storeAddress, int data);

/**************************************************************************/
/*!

    @brief Function to retrieve the caliberated Parameter based on Address.

    @param sensor_e sensor describes which sensor to be read.

    @param data returns the sensor data read from sensor

    @return 0 for success and error code for any failures
*/
/**************************************************************************/
int retrieveCalibParam(int address);



/**@}*/
