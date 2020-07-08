#include "memory.h"
#include "../debug.h"



void store_sensor_data_long(int storeAddress, long int data)
{
  byte dataToStore[4] = {byte(data >> 24), byte(data >> 16), byte(data >> 8), byte(data)};

  VENT_DEBUG_FUNC_START();
  
  hbad_mem.write(storeAddress, dataToStore, 4);
    VENT_DEBUG_INFO("Address to store", storeAddress);
	VENT_DEBUG_INFO("Store calib data [0]", dataToStore[0]);
	VENT_DEBUG_INFO("Store calib data [1]", dataToStore[1]);
	VENT_DEBUG_INFO("Store calib data [2]", dataToStore[2]);
	VENT_DEBUG_INFO("Store calib data [3]", dataToStore[3]);
  VENT_DEBUG_INFO("Stored Sensor data", data);
  VENT_DEBUG_FUNC_END();
}

long int retrieve_sensor_data_long(int readAddress)
{
  byte retrievedData[4] = {0};
  long int data = 0x00000000;
  
  VENT_DEBUG_FUNC_START();
  
  hbad_mem.read(readAddress, retrievedData, sizeof(long int));
    VENT_DEBUG_INFO("Address to read", readAddress);
	VENT_DEBUG_INFO("Retrive calib data [0]", retrievedData[0]);
	VENT_DEBUG_INFO("Retrive calib data [1]", retrievedData[1]);
	VENT_DEBUG_INFO("Retrive calib data [2]", retrievedData[2]);
	VENT_DEBUG_INFO("Retrive calib data [3]", retrievedData[3]);
  data = (data | retrievedData[0]) << 8;
  data = (data | retrievedData[1]) << 8;
  data = (data | retrievedData[2]) << 8;
  data = (data | retrievedData[3]);
  VENT_DEBUG_INFO("sensor data retireved", data);
  VENT_DEBUG_FUNC_END();
  
  return data;
}

void storeParam(ctrl_parameter_t param) 
{
  byte dataToStore[2] = {byte(param.value_curr_mem >> 8), byte(param.value_curr_mem)};
  int storeAddress = EEPROM_BASE_ADDR + (2 * (param.index));

  VENT_DEBUG_FUNC_START();
  
  hbad_mem.write(storeAddress, dataToStore, 2);

  VENT_DEBUG_INFO("sStored Param", param.value_curr_mem);
  VENT_DEBUG_FUNC_END();
}

void retrieveParam(ctrl_parameter_t param)
{
  int storeAddress = EEPROM_BASE_ADDR + (2 * (param.index));
  byte retrievedData[2];
  
  VENT_DEBUG_FUNC_START();
  
  hbad_mem.read(storeAddress, retrievedData, 2);
  int value_curr_mem = (retrievedData[0]<<8) + retrievedData[1];
  if (250 != value_curr_mem) {
   params[param.index].value_curr_mem = value_curr_mem;
  } else {
   VENT_DEBUG_INFO("Read default and store for next cycle", 0);
   storeParam(param);
  }
  
  VENT_DEBUG_INFO("Param retireved", param.value_curr_mem);
  VENT_DEBUG_FUNC_END();
}

void getAllParamsFromMem()
{
  VENT_DEBUG_FUNC_START();
  
  for (int i = 0; i < MAX_CTRL_PARAMS; i++) {
    retrieveParam(params[i]);
	VENT_DEBUG_INFO("Parameter Read", params[i].value_curr_mem);
  }
  
  VENT_DEBUG_FUNC_END();
}
  
  
void storeCalibParam(int storeAddress, int data)
{
  VENT_DEBUG_FUNC_START();
  
  byte dataToStore[2] = {byte(data >> 8), byte(data)};
  hbad_mem.write(storeAddress, dataToStore, 2);
  
  VENT_DEBUG_FUNC_END();
}

int retrieveCalibParam(int address)
{
  VENT_DEBUG_FUNC_START(); 
  
  byte retrievedData[2];
  hbad_mem.read(address, retrievedData, 2);
  
  VENT_DEBUG_FUNC_END();
  return ((int)(retrievedData[0] << 8) + (int) retrievedData[1]);
}

 //Below function performs read/write functionality in the EEPROM
 //It take the following arguments 
 // Address = The address where read/write functionality has to be performed 
 // *data = the pointer to the data, will be filled in in case of read and will be read from in case of write
 // size = size of data pointed by *data
 // ops = operation select (defined by enum EEPROM_READ, EEPROM_WRITE)

int eeprom_ext_rw(unsigned int address, char *data, unsigned int size, eeprom_ops ops)
{
    int err = 0;
    unsigned int i=0;
    
	VENT_DEBUG_FUNC_START(); 	
	
    if ((data == NULL) || (size == 0) || (size > EEPROM_MAX_SIZE)) 
    {
        err = -1;
        goto err_ret;
    }
    
    if (ops == EEPROM_READ)
    {
        for (i=0; i<size; i++)
        {
            *data = EEPROM.read(address);
            data++;
			address++;
        }
    }
    else if (ops == EEPROM_WRITE)
    {
        for (i=0; i<size; i++)
        {
            EEPROM.write(address, *data);
            data++;
			address++;
        }
    }
    else
    {
        err = -1;
		VENT_DEBUG_ERROR("Invalid Input", err);
    }
	
err_ret:
    VENT_DEBUG_FUNC_END();
    return err;
}


