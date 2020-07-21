/**************************************************************************/
/*!
    @file     ads1115_utils.c

    @brief 	  Utility functions to read the samples from I2C

    @author   Tworks

	@defgroup VentilatorModule

    Utility functions to handle ADS1115, functions to read 
	analog samples and convert them digital
	@{
*/
/**************************************************************************/

#include "ads1115_utils.h"
#include "./../libraries/Adafruit_ADS1X15/Adafruit_ADS1015.cpp"
/*
 * I2C ADC configurations
 */
static unsigned int I2C_TIMEOUT = 6;
#ifdef UNIT_TEST
#define ADS115_MULTIPLIER       (0.000125)
#else
#define ADS115_MULTIPLIER       (0.000125)
#endif

//#define DEBUG_ADS 0
const float  O2SensMultiplier = 0.00488;

float ADC_ApplyAvgFilter(int *SampleBuf, int SampleCount, float Multiplier);

/*
 * Function to average the samples from sensor
 */
float get_sample_average(int *samples, int sample_count) {
  float avg = 0.0;
  for(int index = 0; index < sample_count; index++) {
    avg += samples[index];
  }
  return (avg/sample_count);
}

int ADS1115_ReadAvgSamplesOverI2C(Adafruit_ADS1115 *ads, int channel, float *vout) {
  int samples[MAX_SAMPLE_COUNT] = {0};
  float PressSensorVolts = 0.0;
  unsigned long int timeout;

#ifdef DEBUG_ADS
  unsigned long int ctimeout = millis();
   VENT_DEBUG_INFO("ADC:S", MAX_SAMPLE_COUNT);
#endif

  for(unsigned int i=0; i<MAX_SAMPLE_COUNT; i++) {
    timeout = millis();
    ads->readADC_SingleEnded(channel);
    while ((digitalRead(ads->m_intPin)!=LOW) &&
           ((millis()-timeout) < I2C_TIMEOUT)) {
    }
    if((millis()-timeout) >= I2C_TIMEOUT) 
	{
      Serial.print("ERROR: I2C timed out, please check connection i2c address:");
      Serial.print(ads->m_i2cAddress);
      Serial.print(", ads->m_intPin:");
      Serial.print(ads->m_intPin);
      Serial.print(", timeout value:");
      Serial.println(I2C_TIMEOUT);
      *vout = 0;
      return ERROR_I2C_TIMEOUT;
    } 
    samples[i] = ads->readADC_ConvertedSample();
  } 
  PressSensorVolts = get_sample_average(samples, MAX_SAMPLE_COUNT);
#ifdef DEBUG_ADS
  Serial.print("ADC:E t:");
  Serial.print(millis()-ctimeout);
  Serial.print(", mV= ");
  Serial.println(PressSensorVolts) ;
#endif
  *vout = PressSensorVolts;
  return 0;
}

int ADS1115_ReadVoltageOverI2C(Adafruit_ADS1115 *ads, int channel, float *value) {
  float PressSensorVolts = 0.0;
  int err = ADS1115_ReadAvgSamplesOverI2C(ads, channel, &PressSensorVolts);
  if (err) {
	  *value = 0;
	  return err;
  }
  *value = (PressSensorVolts * ADS115_MULTIPLIER);
  return 0;
}

int ADC_ReadVolageOnATMega2560(Adafruit_ADS1115 *ads, int channel, int correction, float *vout) {
  int ADCSampleBuff[MAX_SAMPLE_COUNT] = {0};
  float OxygenSensorVolts = 0.0;
  
  (void)correction;
#if AVCC_DYNAMIC
  int Vref = 0;
  float DynacmicO2SensMult = 0.0;
  Vref = getVrefVoltage();
  DynacmicO2SensMult = Vref;
  DynacmicO2SensMult /= 1024;
  DynacmicO2SensMult /= 1000; //Coversion of mV to V
#endif

#ifdef DEBUG_ADS
  unsigned long int ctimeout = millis();
  Serial.print("ADC:MS C#");
  Serial.println(MAX_SAMPLE_COUNT);
#endif

  if(ads == NULL) {
	  for (unsigned int i = 0; i < MAX_SAMPLE_COUNT; i++) {
  		ADCSampleBuff[i] = analogRead(channel);
	  }
  } else {
	  unsigned long int timeout = 0;
	  for(unsigned int i=0; i<MAX_SAMPLE_COUNT; i++) {
		timeout = millis();
		ads->readADC_SingleEnded(channel);
		while ((digitalRead(ads->m_intPin)!=LOW) &&
			   ((millis()-timeout) < I2C_TIMEOUT)) {
		}
		if((millis()-timeout) >= I2C_TIMEOUT) {
		  Serial.println("ERROR: I2C timed out, please check connection.");
		  *vout = 0;
		  return ERROR_I2C_TIMEOUT;
		}
		ADCSampleBuff[i] = ads->readADC_ConvertedSample();
	  } 
  }

#if AVCC_DYNAMIC
  OxygenSensorVolts = ADC_ApplyAvgFilter(ADCSampleBuff, MAX_SAMPLE_COUNT, DynacmicO2SensMult);
#else
  OxygenSensorVolts = ADC_ApplyAvgFilter(ADCSampleBuff, MAX_SAMPLE_COUNT, O2SensMultiplier);
#endif

#ifdef DEBUG_ADS
    Serial.print("ADC:ME t:");
    Serial.print(millis()-ctimeout);
    Serial.print(", mV= ");
    Serial.println(OxygenSensorVolts);
#endif

  *vout = OxygenSensorVolts;
  return 0;
}
#if AVCC_DYNAMIC
int getVrefVoltage(void)
{
  int ADCSample[MAX_SAMPLE_COUNT] = {0};
  int results, ADCCount;
  const long InternalReferenceVoltage = 1100L;  // Adust this value to your specific internal BG voltage x1000
  // REFS1 REFS0          --> 0 1, AVcc internal ref.
  // MUX3 MUX2 MUX1 MUX0  --> 1 1110 1.1V (VBG)
  ADMUX = (0 << REFS1) | (1 << REFS0) | (0 << ADLAR) | (1 << MUX4) | (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0);

  ADCSRA |= ADEN;
  delay(5);
  for (unsigned int i = 0; i < MAX_SAMPLE_COUNT; i++)
  {
    // Start a conversion
    ADCSRA |= _BV( ADSC );
    // Wait for it to complete
    while ( ( (ADCSRA & (1 << ADSC)) != 0 ) );
    ADCSample[i] = ADC;
  }
  ADCCount = ADC_GetMedian(ADCSample, MAX_SAMPLE_COUNT);
  // Scale the value
  results = ((InternalReferenceVoltage * 1024L) / ADCCount);
  ADCSRA &= (~ADEN);
  return results;
}
#endif

#if MEDIAN_ADC_FILTER
float ADC_ApplyAvgFilter(int *SampleBuf, int SampleCount, float Multiplier) {
  return (ADC_GetMedian(SampleBuf,SampleCount) * Multiplier);
}
#else
float ADC_ApplyAvgFilter(int *SampleBuf, int SampleCount, float Multiplier) {
  int AvgSampleCount;
  float Avg10Samples;
  float SumValue = 0.0, ADCThresH = 0.0, ADCThresL = 0.0;
  float SensorVolts = 0.0;
  float ADCAnalogValues[SampleCount] = {0.0};

  for (int i = 0; i < SampleCount; i++) {
    ADCAnalogValues[i] = SampleBuf[i] * Multiplier;
    SumValue += ADCAnalogValues[i];
  }

  Avg10Samples = SumValue / SampleCount;
  ADCThresH = Avg10Samples + Multiplier;
  ADCThresL = Avg10Samples - Multiplier;

  AvgSampleCount = 0;
  SumValue = 0.0;
  for (int i = 0; i < SampleCount; i++) {
    if ((ADCAnalogValues[i] >= ADCThresL) && (ADCAnalogValues[i] <= ADCThresH)) {
      AvgSampleCount++;
      SumValue += ADCAnalogValues[i];
    }
    if (AvgSampleCount == 0) {
      SensorVolts = Avg10Samples;
    } else {
      SensorVolts = SumValue / AvgSampleCount;
    }
  }
  return (SensorVolts);
}
#endif
#if (MEDIAN_ADC_FILTER | AVCC_DYNAMIC)
int ADC_GetMedian(int *SampleBuf, int len)
{
  int i = 0, j = 0, a = 0;
  long int median = 0;
  //sort the samples 
  for (i = 0; i < len; ++i) {
    for (j = i + 1; j < len; ++j) {
      if (SampleBuf[i] > SampleBuf[j]) {
        a =  SampleBuf[i];
        SampleBuf[i] = SampleBuf[j];
        SampleBuf[j] = a;
      }
    }
  }
  i = len/2;
  if((len%2)==0) {
    median = ((long int)SampleBuf[i-1]+(long int)SampleBuf[i])/2;
  } else {
    median = SampleBuf[i];
  }
  return((int)median);
}
#endif
