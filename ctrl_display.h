
/**************************************************************************/
/*!
    @file     ctrl_display.h

    @brief     This Module contains 

    @author   Tworks
     @{
*/
/**************************************************************************/



#define MAX_CTRL_PARAMS 6  /*!< Total number of control parameters  */

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
#define CANC_FLAG_CHOSEN "<CANCEL>"       /*!< defines the String for CANC_FLAG_CHOSEN  */


static const int mode_loop_delays[] = {100, 100, 100, 100};    /*!< loop delays  */
static const int mode_timeouts[] = {0, 0, 500, 0};    /*!< mode timeout delays */
#define EMPTY_TWENTY_STR "                    "        /*!< Twenty space Str  */
#define EMPTY_FIVE_STR "     "                         /*!< Empty five  space Str  */

/*!< mode_headers array difines all the modes */
static const String mode_headers[] = {"PRESS SELECT TO EDIT", EMPTY_TWENTY_STR, EMPTY_TWENTY_STR, "PRESS SELECT TO SAVE"};

/** @struct  ctrl_parameter_t
 *  @brief   Structure describes each control parameter 
 *
 *  
 */
struct ctrl_parameter_t {
  unsigned short index;    /*!< index variable stores the index according to the ctrl_parameter_t params array variables*/
  String parm_name;        /*!< It describes the control parameter name */
  int readPortNum;         /*!< It describes the pin number associated with control parameter*/
  int min_val;             /*!< It describes the minimum value associated with control parameter */
  int max_val;             /*!< It describes the maximum value associated with control parameter */
  String units;            /*!< It describes the units associated with control parameter */
  int incr;                /*!< It describes the incrementation factor associated with control parameter */
  int value_curr_mem;      /*!< It describes the current memory value associated with control parameter */
  int value_new_pot;       /*!< It describes the current pot value associated with control parameter */
};


typedef enum
{
  TIDAL_VOL=0,
  BPM,
  PEAK_PRES,
  FIO2_PERC,
  IE_RATIO,
  PEEP_PRES
};
/*!< default values assigned according to the ctrl_parameter_t Structure variables for tidal volume  */
const ctrl_parameter_t tidl_volu = {0, "TV  ", TIDAL_VOLUME_PIN,
                                    150, 750,
                                    "ml   ", 50,
                                    0, 0
                                   };
/*!< default values assigned according to the ctrl_parameter_t Structure variables for BPM */
const ctrl_parameter_t resp_rate =    {1, "BPM ", RR_PIN,
                                       2, 35,
                                       "BPM  ", 1,
                                       0, 0
                                      };
/*!< default values assigned according to the ctrl_parameter_t Structure variables for PeakPressure */
const ctrl_parameter_t peak_press =   {2, "PIP ", PMAX_PIN,
                                       29, 59,
                                       UNIT_CMH2O, 1,
                                       0, 0
                                      };
/*!< default values assigned according to the ctrl_parameter_t Structure variables for Fio2 */                                      
const ctrl_parameter_t fio2_perc =    {3, "FiO2", FiO2_PIN,
                                       20, 100,
                                       "%    ", 20,
                                       0, 0
                                      };
/*!< default values assigned according to the ctrl_parameter_t Structure variables for IER Ratio */                                      
const ctrl_parameter_t inex_rati =    {4, "IER ", DISP_ENC_CLK, //READ THROUGH ENCODER
                                       1, 3,
                                       "ratio", 1,
                                       0, 0
                                      };
/*!< default values assigned according to the ctrl_parameter_t Structure variables for PeeP */                                      
const ctrl_parameter_t peep_pres =    {5, "PEEP", DISP_ENC_CLK, //READ THROUGH ENCODER
                                       5, 20,
                                       UNIT_CMH2O, 5,
                                       0, 0
                                      };
/*!< Array contains all the control parameter values  */
static ctrl_parameter_t params[] = {tidl_volu, resp_rate, peak_press, fio2_perc, inex_rati, peep_pres};



/**@}*/
