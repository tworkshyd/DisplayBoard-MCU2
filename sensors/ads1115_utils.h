/**************************************************************************/
/*!
    @file     ads1115_utils.h

    @brief 	  ADS utility functions

    @author   Tworks

	@defgroup ADS1115UtilsModule

    Contains the utility functions to interface with ADS board
	@{
*/
/**************************************************************************/
#ifndef __ADS1115_UTILS_H__
#define __ADS1115_UTILS_H__

#include "./../libraries/Adafruit_ADS1X15/Adafruit_ADS1015.h"

#ifdef __cplusplus
extern "C" {
#endif

// calculated on board and on average it is taking 4ms per sample
const unsigned int ADC_CONVERSTION_TIME = 4;
const unsigned int MAX_SAMPLE_COUNT = 1;

/**************************************************************************/
/*!

    @brief  Function to read the sensor samples and average
	
	@param ads ads board used for reading samples
	
	@param channel channel in ads board for reading samples

	@param vout vout read from the sensors as digital value

    @return returns 0 for success and error code for other errors
*/
/**************************************************************************/
int ADS1115_ReadAvgSamplesOverI2C(Adafruit_ADS1115 *ads, int channel, float *vout);
/**************************************************************************/
/*!

    @brief  Function to return average error corrected samples
	
	@param ads ads board used for reading samples
	
	@param channel channel in ads board for reading samples

	@param vout vout read from the sensors as digital value

    @return returns 0 for success and error code for other errors
*/
/**************************************************************************/
int ADS1115_ReadVoltageOverI2C(Adafruit_ADS1115 *ads, int channel, float *vout);
/**************************************************************************/
/*!

    @brief  Function to read the O2 sensor samples and average
	
	@param ads ads board used for reading samples
	
	@param channel channel in ads board for reading samples
	
	@param error error correction needed for the samples

	@param error error correction needed for the samples

	@param vout vout read from the sensors as digital value

    @return returns 0 for success and error code for other errors
*/
/**************************************************************************/
int ADC_ReadVolageOnATMega2560(Adafruit_ADS1115 *ads, int channel, int error, float *vout);

#ifdef __cplusplus
}
#endif

#endif /*__ADS1115_UTILS_H__*/

/**@}*/
