#ifndef __ADS1115_UTILS_H__
#define __ADS1115_UTILS_H__

#include "./../libraries/Adafruit_ADS1X15/Adafruit_ADS1015.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************/
/*!

    @brief  Function to read the sensor samples and average
	
	@param ads - ads board used for reading samples
	
	@param channel - channel in ads board for reading samples

    @return returns the average of all samples read
*/
/**************************************************************************/
int ADS1115_ReadAvgSamplesOverI2C(Adafruit_ADS1115 *ads, int channel);
/**************************************************************************/
/*!

    @brief  Function to return average error corrected samples
	
	@param ads - ads board used for reading samples
	
	@param channel - channel in ads board for reading samples
	
	@param base - actual value expected from sensor @ zero activity
	
	@param error - (base - actual) returned from sensor @ zero activity

    @return returns the average of all samples read
*/
/**************************************************************************/
float ADS1115_ReadVoltageOverI2C(Adafruit_ADS1115 *ads, int Channel, int base, int error);
/**************************************************************************/
/*!

    @brief  Function to read the O2 sensor samples and average
	
	@param ads - ads board used for reading samples
	
	@param channel - channel in ads board for reading samples
	
	@param error - error correction needed for the samples

    @return returns the average of all samples read
*/
/**************************************************************************/
float ADC_ReadVolageOnATMega2560(Adafruit_ADS1115 *ads, int Channel, int error);

#ifdef __cplusplus
}
#endif

#endif /*__ADS1115_UTILS_H__*/