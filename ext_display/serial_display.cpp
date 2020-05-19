#include "serial_display.h"

#define POLY 0x8408


unsigned short crc16(unsigned char *data_p, unsigned short length);

/*
 * Display initialization
 * Serial port initialized for communication
 */
int serial_display::init() {
	Serial3.begin(115200);
	return 0;
}	

void serial_display::enable() {
	m_flags = 1;
}

void serial_display::disable() {
	m_flags = 0;
}

/*
//                                      16   12   5
// this is the CCITT CRC 16 polynomial X  + X  + X  + 1.
// This works out to be 0x1021, but the way the algorithm works
// lets us use 0x8408 (the reverse of the bit pattern).  The high
// bit is always assumed to be set, thus we only use 16 bits to
// represent the 17 bit value.
*/

unsigned short serial_display::crc16(unsigned char *data_p, unsigned short length)
{
      unsigned char i;
      unsigned int data;
      unsigned int crc = 0xffff;

      if (length == 0)
            return (~crc);

      do
      {
            for (i=0, data=(unsigned int)0xff & *data_p++;
                 i < 8; 
                 i++, data >>= 1)
            {
                  if ((crc & 0x0001) ^ (data & 0x0001))
                        crc = (crc >> 1) ^ POLY;
                  else  crc >>= 1;
            }
      } while (--length);

      crc = ~crc;
      data = crc;
      crc = (crc << 8) | (data >> 8 & 0xff);

      return (crc);
}

/*
 * PACKET STUCTURE
 * <SDL><TID><FC><VALUES><EDL>
 * SDL; STart delimiter
 * EDL: End delimiter
 * TID: Target Identifier; VM for Ventilator Master. VS For Ventilator Slave
 * FC: Function Code
 */

void serial_display::set_data(char *data, unsigned int data_len) {
	int index = 0, rindex = 0;
	m_display_data_count = 0;
	if(data_len >= TOTAL_PACKET_LEN) {
		Serial.println("ERROR: packet length for external display is big"); 
		return -1;
	}
	for(index = 0; index < data_len; index++) {
		m_display_data[index] = data[index];
	}
	m_display_data_count = data_len;
	for(rindex = index; rindex < TOTAL_PACKET_LEN; rindex++) {
		m_display_data[rindex] = 0;
	}
}

void serial_display::append_data(char *data, unsigned int data_len) {
	int index = 0, rindex = 0;
	if((m_display_data_count + data_len) >= TOTAL_PACKET_LEN) {
		Serial.println("ERROR: packet length for external display is big"); 
		return -1;
	}
	for(index = m_display_data_count; index < m_display_data_count + data_len; index++) {
		m_display_data[index] = data[index];
	}
	this->m_display_data[0] = m_display_data_count = (m_display_data_count + data_len);
	for(rindex = index; rindex < TOTAL_PACKET_LEN; rindex++) {
		m_display_data[rindex] = 0;
	}
}

void serial_display::show() {
	unsigned short int crc16Val = 0;
	
	this->m_display_data[0] = m_display_data_count + 2;
    crc16Val = crc16(this->m_display_data, m_display_data_count);
    this->m_display_data[index++] = (unsigned char)(((crc16Val & 0xFF00) >> 8) & 0x00FF);
    this->m_display_data[index++] = (unsigned char)(crc16Val & 0x00FF);
    Serial3.write(this->m_display_data, index);
}
