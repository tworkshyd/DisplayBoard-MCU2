/**************************************************************************/
/*!
    @file     statecontrol.h

    @brief     Control StateMachine Module defines the state 

    @author   Tworks
    
  @defgroup StateControlModule  StateControlModule

    Module is a stateMachine where sensors data and UI parameters data is read to take appropriate decision in the respective state. 
  @{
*/
/**************************************************************************/

#ifndef __STATECONTROL_H__
#define __STATECONTROL_H__


          
#define STPR_STP 0                    /**< it gives the index value of the commands array  to  stop stepper motor*/
#define STPR_ON  1                    /**< it gives the index value of the commands array  to start stepper motor*/
#define OXY_SOLE_CYL_OFF 2            /**< it gives the index value of the commands array  to oxygen solenoid cylinder line off*/
#define OXY_SOLE_CYL_ONN  3           /**< it gives the index value of the commands array  to oxygen solenoid cylinder line on*/
#define OXY_SOLE_HOS_O2_OFF 4         /**< it gives the index value of the commands array  to oxygen solenoid hospital line off*/ 
#define OXY_SOLE_HOS_O2_ONN  5        /**< it gives the index value of the commands array  to oxygen solenoid hospital line on*/
#define INH_SOLE_OFF 6                /**< it gives the index value of the commands array  to inhalation  solenoid off*/
#define INH_SOLE_ONN  7               /**< it gives the index value of the commands array  to inhalation  solenoid on*/
#define EXH_SOLE_OFF 8                /**< it gives the index value of the commands array  to exhalation  solenoid off*/
#define EXH_SOLE_ONN  9               /**< it gives the index value of the commands array  to exhalation  solenoid on*/
#define PK_PR_REL_OFF 10              /**< it gives the index value of the commands array  to peak pressure relief off*/
#define PK_PR_REL_ONN  11             /**< it gives the index value of the commands array  to peak pressure relief on*/
#define SET_TID_VOL  12               /**< it gives the index value of the commands array  to set tidal volume */
#define SET_BPM  13                   /**< it gives the index value of the commands array  to set BPM */
#define SET_PK_PR 14                  /**< it gives the index value of the commands array  to set peak pressure*/
#define SET_FIO2  15                   /**<it gives the index value of the commands array  to  set Fio2*/                 
#define SET_IE_RATIO 16               /**< it gives the index value of the commands array  to set inhalation exhalation ratio */
#define SL_EN_PARM_EDT  17            /**< it gives the index value of the commands array  to slave entering parameter edit */
#define SL_COM_PARM_EDT 18            /**<it gives the index value of the commands array  to  slave completed parameter edit */
#define INIT_MASTER  19               /**<it gives the index value of the commands array  to initialize master*/
#define INIT_STPR_MOD 20              /**< it gives the index value of the commands array  to initialize stepper module*/
#define INIT_VALV_BLK  21              /**<it gives the index value of the commands array  to  initialize stepper module*/
#define INIT_BREATH_DET  22            /**<it gives the index value of the commands array  to  initialize breath detection*/

#define SYNCH "SY"       /**< Unknown State is used when request structure is valid but makes no sense semantically */
#define VENTSLAVE "VS"   /**< Unknown State is used when request structure is valid but makes no sense semantically */

#define START_DELIM '$'  /**< Start Delimeter for the Command Structure*/
#define END_DELIM '&'    /**< End Delimeter for the Command Structure*/
#define VENT_MAST "VM"   /**< String to Identify Whether the request is from Ventilator Master or not*/
#define CMD_PACKET_SIZE 10   /**< Command Packet Data Size*/

#define PARAM1 "P1"       /**< String is used to Identify which parameter needs  to be Sent.In this case it is 0 param from Params Arrays */
#define PARAM2 "P2"       /**< String is used to Identify which parameter needs  to be Sent.In this case it is 1 param from Params Arrays*/
#define PARAM5 "P5"       /**< String is used to Identify which parameter needs  to be Sent.In this case it is 4 param from Params Arrays*/

#define MIN_PRESSURE_FOR_MASKON  15

/**
 * @enum   ControlStatesDef_T
 * @brief   Following are the Control States during which Sensor data and user inputs are compared 
 * to generate specific commands required for that state.
 */
typedef enum
{
  CTRL_INIT=0,                /**< Initiation State*/
  CTRL_COMPRESSION,           /**< Compression Cycle Start State */
  CTRL_COMPRESSION_HOLD,      /**< Compression Cycle Hold State */
  CTRL_EXPANSION,             /**< Expansion Cycle Start State */
  CTRL_EXPANSION_HOLD,        /**< Expansion Cycle Hold State */
  CTRL_INHALE_DETECTION,      /**< Inhale Detection State */
  CTRL_STOP,                  /**<  Stop state  */
  CTRL_DO_NOTHING,            /**<  Idle State */
  CTRL_UNKNOWN_STATE,         /**< Unknown State is used when request structure is valid but makes no sense semantically */
}ControlStatesDef_T;


/**************************************************************************/
/*!

    @brief  Function to create control command 

    @param paramName parameter is used to create command using it. 

    @param value parameter defines the actual to be sent in the command

    @return indicates Non-Null for SUCCESS and Null for FAILURE
*/
/**************************************************************************/
String Ctrl_CreateCommand(String paramName, int value);
/**************************************************************************/
/*!

    @brief  Function to perform specific operation based on the Current State

    @return indicates true for SUCCESS and false for FAILURE
*/
/**************************************************************************/
bool Ctrl_StateMachine_Manager(void);

/**************************************************************************/
/*!

    @brief  Function to process the received data 
*/
/**************************************************************************/
void Ctrl_ProcessRxData(void);
/**************************************************************************/
/*!

    @brief  Function to store the bufferred data into the local variable

    @param data parameter is the bufferred data 

*/
/**************************************************************************/
void Ctrl_store_received_packet(String data);
/**************************************************************************/
/*!

    @brief  Function to send the command from commands array based on the index

    @param commandIndex parameter is used to index in the commands array


    @return indicates 0 for SUCCESS and -1 for FAILURE
*/
/**************************************************************************/
int  Ctrl_send_packet(int commandIndex);
/**************************************************************************/
/*!

    @brief  Function to send the command based on the parameter and its value 

    @param name parameter is used to send specific parameter  

    @param value parameter is the actual data to be sent

    @return indicates Non-Null for SUCCESS and Null for FAILURE
*/
/**************************************************************************/
int  Ctrl_send_packet(String name, int value);
/**************************************************************************/
/*!

    @brief  Function to change the state to INIT 

*/
/**************************************************************************/
void Ctrl_Start();
/**************************************************************************/
/*!

    @brief  Function to change the state to stop the stepper motor

*/
/**************************************************************************/
void Ctrl_Stop();

/**@}*/

#endif /* __STATECONTROL_H__ */
