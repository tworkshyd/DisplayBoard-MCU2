
/**************************************************************************/
/*!
    @file     hbad_memory.h

    @brief      This module contains base addresses and range  for oxygen calibration, ps1, ps2, dps1, dps2, methods to store and retrieve.

    @author   Tworks
    
  @defgroup MemoryModule  MemoryModule

  
  @{
*/
/**************************************************************************/



#include <extEEPROM.h>


/*! It is Default Data Write Addr< */
#define EEPROM_DEFAULT_DATA_WRITE_ADDR 0xA //10

/*! EEPROM Oxygen CALIB ADDR < */
#define EEPROM_O2_CALIB_ADDR 0xC //12 - 22
#define NUM_OF_SAMPLES_O2 5

/*! EEPROM PG1 CALIB ADDR < */
#define EEPROM_PG1_CALIB_ADDR 0x28 //40 - 78
#define NUM_OF_SAMPLES_PS1 19

/*!EEPROM PS2 CALIB ADDR < */
#define EEPROM_PS2_CALIB_ADDR  0
#define NUM_OF_SAMPLES_PS2 0

/*!EEPROM DPS1 CALIB ADDR < */
#define EEPROM_DPS1_CALIB_ADDR  0
#define NUM_OF_SAMPLES_DPS1 0

/*!EEPROM DPS2 CALIB ADDR < */
#define EEPROM_DPS2_CALIB_ADDR  0
#define NUM_OF_SAMPLES_DPS2 0

/*!EEPROM I2C Address < */
#define EEPROM_I2C_ADDR 0x50 //80
/*!< */
#define EEPROM_BASE_ADDR 0xC8

//const uint32_t totalKBytes = 64;

/*!device size, number of devices, page size< */
extEEPROM hbad_mem(kbits_256, 1, 32, EEPROM_I2C_ADDR);

void storeParam(ctrl_parameter_t param) {
  Serial.println("Saving");
  byte dataToStore[2] = {param.value_curr_mem >> 8, param.value_curr_mem};
  int storeAddress = EEPROM_BASE_ADDR + (2 * (param.index));
  hbad_mem.write(storeAddress, dataToStore, 2);
  Serial.print("Stored");
  Serial.println(param.value_curr_mem);
}

void retrieveParam(ctrl_parameter_t param) {
  int storeAddress = EEPROM_BASE_ADDR + (2 * (param.index));
  byte retrievedData[2];
  hbad_mem.read(storeAddress, retrievedData, 2);
  params[param.index].value_curr_mem = (retrievedData[0]<<8) + retrievedData[1];
  Serial.print("Got");
  Serial.println(param.value_curr_mem);
}

void getAllParamsFromMem() {
  for (int i = 0; i < MAX_CTRL_PARAMS; i++) {
    retrieveParam(params[i]);
    Serial.print("Mem\t");
    Serial.print(i);
    Serial.print("\t");
    Serial.println(params[i].value_curr_mem);
  }
}
  
  
void storeCalibParam(int storeAddress, int data) {
  Serial.println("Saving calib");
  byte dataToStore[2] = {data >> 8, data};
  hbad_mem.write(storeAddress, dataToStore, 2);
}

int retrieveCalibParam(int address) {
  byte retrievedData[2];
  hbad_mem.read(address, retrievedData, 2);
  return ((int)(retrievedData[0] << 8) + (int) retrievedData[1]);
}



/**@}*/
