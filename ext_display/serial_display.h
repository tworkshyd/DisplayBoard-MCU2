
/**************************************************************************/
/*!
    @file     Control_StateMachine.h

    @brief     This Module contains 

    @author   Tworks
    
  @defgroup VentilatorModule  VentilatorModule

    Module to initialize and deinitialize the ventilator
  @{
*/
/**************************************************************************/

#define TOTAL_PACKET_LEN		40

class ext_display {
	private:
		unsigned int m_flag;
		/*Counter acting as an context for the data*/
		unsigned int m_counter;
		/*Buffer for sending the data over Serial to Graphics display  module */
		unsigned char m_display_data[TOTAL_PACKET_LEN];
	public:
		int init();
		int enable();
		int disable();
		int setdata(char *data, unsigned int data_size);
		int show();
};