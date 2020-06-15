
/**************************************************************************/
/*!
    @file     ctrl_display.h

    @brief     This Module contains 

    @author   Tworks
     @{
*/
/**************************************************************************/

#pragma once
#include "lcd.h"
#include "../encoder/encoder.h"
#include "../sensors/sensormanager.h"

//#define MAX_CTRL_PARAMS sizeof(params)/ sizeof(params[0])
#define DBNC_INTVL_SW 500                                           /*!< millisecs delay before switch debounce  */
#define DBNC_INTVL_ROT 100                                          /*!< millisecs delay before rotation debounce  */
#define MAX_IDLE_AFTER_SEL 10000000                                 /*!< maximum idle time after edit selection   */
#define POT_TURN_MAX_WAIT_MILLIS 2000                               /*!< maximum waiting time for turning pot  */
#define DISPLAY_MODE 0                                              /*!< value for Display mode is 0  */
#define EDIT_MODE 1                                                 /*!< value for edit mode is 1  */
#define PAR_SELECTED_MODE 2                                         /*!< value for param selection mode is 2  */
#define PAR_SAVE_MODE 3                                             /*!< value for param save mode is 3  */
#define POT_HIGH 1000                                               /*!< maximum pot high value  */

/*Let us start writing from the 16th memory location.
   This means Parameter 1 will be stored in locs with addr 16, 17, 18, 19
   and Parameter 1 will be stored in 20, 21, 22, 23 and so on
*/

#define PARAM_STOR_SIZE 4 //4 bytes for float data
#define PARAM_TYPE_UCFG 1
#define PARAM_TYPE_SENS 2
#define UNIT_CMH2O "cmH20"

#define SAVE_FLAG " SAVE "                 /*!< defines the String for SAVE_FLAG   */
#define SAVE_FLAG_CHOSEN "<SAVE>"         /*!< defines the String for SAVE_FLAG_CHOSEN  */
#define CANC_FLAG " CANCEL "              /*!< defines the String for CANC_FLAG  */
#define CANC_FLAG_CHOSEN "<CANCEL> "       /*!< defines the String for CANC_FLAG_CHOSEN  */
#define CALIB_FLAG "CALIBRATE"                 /*!< defines the String for SAVE_FLAG   */
#define CALIB_FLAG_CHOSEN "<CALIBRATE>"         /*!< defines the String for SAVE_FLAG_CHOSEN  */



static const int mode_loop_delays[] = {100, 100, 100, 100};    /*!< loop delays  */
static const int mode_timeouts[] = {0, 0, 500, 0};    /*!< mode timeout delays */
#define EMPTY_TWENTY_STR "                    "        /*!< Twenty space Str  */
#define EMPTY_FIVE_STR "     "                         /*!< Empty five  space Str  */

/*!< mode_headers array difines all the modes */
static const String mode_headers[] = {"PRESS SELECT TO EDIT", EMPTY_TWENTY_STR, EMPTY_TWENTY_STR, "PRESS SELECT TO SAVE"};


typedef enum {
  E_EXIT=0,
  E_TV,
  E_BPM,
  E_FiO2,
  E_IER,
  E_PEEP,
  E_PIP,
  E_O2_INPUT,
  SHOW_VOL,
  SHOW_PRESSURE,
  MAX_EDIT_MENU_ITEMS = 10
} eMainMenu;

#define MAX_CTRL_PARAMS 8  /*!< Total number of control parameters  */

const String mainEditMenu[MAX_EDIT_MENU_ITEMS] = { "EXIT EDIT MENU", "TV   : ", "RR   : ", "FiO2 : ", "IER  : ", "PEEP : ", "PIP  : ", "O2in : ", "Volt : ", "Pres : "};
//eMainMenu currentEditMenuIdx = MAX_EDIT_MENU_ITEMS;

/** @struct  ctrl_parameter_t
 *  @brief   Structure describes each control parameter 
 *
 *  
 */
struct ctrl_parameter_t {
  const eMainMenu index;    /*!< index variable stores the index according to the ctrl_parameter_t params array variables*/
  const String parm_name;        /*!< It describes the control parameter name */
  const int readPortNum;         /*!< It describes the pin number associated with control parameter*/
  const int min_val;             /*!< It describes the minimum value associated with control parameter */
  const int max_val;             /*!< It describes the maximum value associated with control parameter */
  const String units;            /*!< It describes the units associated with control parameter */
  int incr;                /*!< It describes the incrementation factor associated with control parameter */
  int value_curr_mem;      /*!< It describes the current memory value associated with control parameter */
  int value_new_pot;       /*!< It describes the current pot value associated with control parameter */
};

/*!< default values assigned according to the ctrl_parameter_t Structure variables for exit handler  */
const ctrl_parameter_t exit_menu = {E_EXIT, mainEditMenu[E_EXIT],  0,
                                       0, 0,
                                       "", 0,
                                       0, 0
                                      };

/*!< default values assigned according to the ctrl_parameter_t Structure variables for tidal volume  */
const ctrl_parameter_t tidl_volu = {E_TV, mainEditMenu[E_TV], TIDAL_VOLUME_PIN,
                                    200, 650,
                                    "ml   ", 50,
                                    350, 0
                                   };
/*!< default values assigned according to the ctrl_parameter_t Structure variables for BPM */
const ctrl_parameter_t resp_rate =    {E_BPM, mainEditMenu[E_BPM], RR_PIN,
                                       4, 39,
                                       "BPM  ", 1,
                                       10, 0
                                      };
/*!< default values assigned according to the ctrl_parameter_t Structure variables for Fio2 */
const ctrl_parameter_t fio2_perc =    {E_FiO2, mainEditMenu[E_FiO2], FiO2_PIN,
                                       15, 100,
                                       "%    ", 1,
                                       0, 0
                                      };


/*!< default values assigned according to the ctrl_parameter_t Structure variables for IER Ratio */
const ctrl_parameter_t inex_rati =    {E_IER, mainEditMenu[E_IER], DISP_ENC_CLK, //READ THROUGH ENCODER
                                       1, 3,
                                       "ratio", 1,
                                       2, 0
                                      };


/*!< default values assigned according to the ctrl_parameter_t Structure variables for PeeP */
const ctrl_parameter_t peep_pres =    {E_PEEP, mainEditMenu[E_PEEP], DISP_ENC_CLK, //READ THROUGH ENCODER
                                       0, 20,
                                       UNIT_CMH2O, 1,
                                       0, 0
                                      };

/*!< default values assigned according to the ctrl_parameter_t Structure variables for PeakPressure */
const ctrl_parameter_t peak_press =   {E_PIP, mainEditMenu[E_PIP], PMAX_PIN,
                                       29, 59,
                                       UNIT_CMH2O, 1,
                                       0, 0
                                      };
/*!< default values assigned according to the ctrl_parameter_t Structure variables for o2_input */
const ctrl_parameter_t o2_input   =    {E_O2_INPUT, mainEditMenu[E_O2_INPUT], 0,
                                       0, 0,
                                       "", 0,
                                       0, 0
                                      };

const ctrl_parameter_t show_voltage =  {SHOW_VOL, mainEditMenu[SHOW_VOL], 0,
                                       0, 0,
                                       "", 0,
                                       0, 0
                                      };

const ctrl_parameter_t show_pressure =  {SHOW_PRESSURE, mainEditMenu[SHOW_PRESSURE], 0,
                                       0, 0,
                                       "", 0,
                                       0, 0
                                      };


/*!< Array contains all the control parameter values  */
/*order should be same as in eMainMenu*/
static ctrl_parameter_t params[] = {exit_menu,tidl_volu, resp_rate, fio2_perc, inex_rati, peep_pres, peak_press, o2_input, show_voltage, show_pressure};

// global variables here
enum STATE {
  STATUS_MENU,
  STATUS_MENU_TO_EDIT_MENU,
  EDIT_MENU,
  EDIT_MENU_TO_SUB_EDIT_MENU,
  SUB_EDIT_MENU,
  SUB_EDIT_MENU_TO_EDIT_MENU,
  EDIT_MENU_TO_STATUS_MENU,
};

enum eDisplayPrm {
  DISPLAY_TVI,
  DISPLAY_TVE,
  DISPLAY_PEEP,
  DISPLAY_PIP,
  DISPLAY_PLAT
};

#define CYLINDER 0
#define HOSPITAL_LINE 1

class displayManager {
public:
  void displayManagerloop(float *sensor_data, sensorManager &sM);
  void displayManagerSetup();
  void errorDisplay(ErrorDef_T errorState);
  void setDisplayParam(eDisplayPrm param, float value);
  float getDisplayParam(eDisplayPrm param);

private:
  String dpStatusString(STATE dpState);
  void moveDownEdit();
  void moveUpEdit();
  int rectifyBoundaries(int value, int minimum, int maximum);
  int getCalibratedParamFromPot(ctrl_parameter_t param);
  int getCalibValue(int potValue, int paramIndex);
  void editModeEncoderInput(void);
  void handleItemUpdate(void);
  void getItemIndx(void);
  void stateMachine(void);  
  void displayRunTime(float *sensor_data);
  void drawEditMenu( void);
  void drawUpdateO2_InputMenu(RT_Events_T eRTState);
  void drawUpdatePEEPorIERMenu(RT_Events_T eRTState);
  void drawUpdateFiO2Menu(RT_Events_T eRTState);
  void drawDefaultItemUpdateMenu( RT_Events_T eRTState);
  void drawSensorvoltageMenu(RT_Events_T eRTState);
  void drawSensorValueMenu(RT_Events_T eRTState);

//variables from here
  volatile STATE _dpState = STATUS_MENU;
  boolean _refreshDisplay = true;
  volatile bool _bBack2EditMenu;
  volatile short _currItem = 1;
  int _currentSaveFlag = 1;
  volatile unsigned long _lastEditMenuTime = 0;
  volatile unsigned long _lastSubEditMenuTime = 0;
  /*default true , we need to do lcd.clear once moved out from setup*/
  bool _refreshRunTimeDisplay = true;
  bool _bEditItemSelected = false;
  byte _editSeletIndicator = 0;
  byte _editScrollIndex = 0;
  long unsigned _lastDisplayTime = 0;
  unsigned short _newIER = 1;
  unsigned short _newPeep = 5;  
  bool _bRefreshEditScreen = false;
  bool _o2LineSelect = CYLINDER;
  sensorManager *m_sM;
  float m_display_tve = 0;
  float m_display_tvi = 0;
  float m_display_peep = 0;
  float m_display_plat = 0;
  float m_display_pip = 0;
  unsigned int _o2_en_flag = 0;
};
/**@}*/
