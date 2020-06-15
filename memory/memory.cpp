#include "memory.h"



void storeParam(ctrl_parameter_t param) {
  Serial.println("Saving");
  byte dataToStore[2] = {param.value_curr_mem >> 8, param.value_curr_mem};
  int storeAddress = EEPROM_BASE_ADDR + (2 * (param.index));
  hbad_mem.write(storeAddress, dataToStore, 2);
  Serial.print("Stored ");
  Serial.println(param.value_curr_mem);
}

void retrieveParam(ctrl_parameter_t param) {
  int storeAddress = EEPROM_BASE_ADDR + (2 * (param.index));
  byte retrievedData[2];
  hbad_mem.read(storeAddress, retrievedData, 2);
  int value_curr_mem = (retrievedData[0]<<8) + retrievedData[1];
  if (250 != value_curr_mem) {
   params[param.index].value_curr_mem = value_curr_mem;
  } else {
   Serial.println("using default hardcoded values and storing for next cycle");
   storeParam(param);
  }
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
