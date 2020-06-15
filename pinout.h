
/**************************************************************************/
/*!
    @file     pinout.h

    @brief     This Module define all the pin configurations  for Arduino 2650 and ADS1115

    @author   Tworks
    @{
*/
/**************************************************************************/

//Display
#define DISPLAY_1_PIN       29       /*!< Display pin 1 is attached to pin 29*/ 
#define DISPLAY_2_PIN       28       /*!< Display pin 2 is attached to pin 28*/  
#define DISPLAY_3_PIN       27       /*!< Display pin 3 is attached to pin 27*/
#define DISPLAY_4_PIN       26       /*!< Display pin 4 is attached to pin 26*/
#define DISPLAY_RS_PIN       24      /*!< Display RS pin  is attached to pin 24*/
#define DISPLAY_EN_PIN       25      /*!< Display EN pin  is attached to pin 25*/

//Control Pots
#define TIDAL_VOLUME_PIN    A0       /*!< tidal volume pot is attached to pin A0 */
#define RR_PIN              A1       /*!< BPM or RR pot is attached to pin A1 */
#define PMAX_PIN            A2       /*!< Peak Pressure pot is attached to pin A2*/
#define FiO2_PIN            A3       /*!< Fio2 pot is attached to pin A3*/

#define AUTO_MODE      10            /*!< Auto mode  is attached to pin 10*/
#define ASSISTED_MODE  11            /*!< Assisted mode  is attached to pin 10*/

//I2C
#define I2C_SCL_PIN         21       /*!< I2C SCL  is attached to pin 21 */
#define I2C_SDA_PIN         20       /*!< I2C SDA  is attached to pin 20 */
#define BUZZER_PIN          3        /*!< Buzzer is attached to pin 3 */
#define ADS115_INT_PIN      6        /*!< ADS115 ALRT is attached to pin 6 */
                                     /*!< ADS115 ADDR is attached to pin ground */
#define RESET_SWITCH        13       /*!< Reset Switch is attached to pin 13 */



#define DISP_ENC_SW         4       /*!< Rotatory Encoder's SW   is attached to pin 18 */
#define DISP_ENC_CLK        2       /*!< Rotatory Encoder's CLK  is attached to pin 19 */
#define DISP_ENC_DT         3       /*!< Rotatory Encoder's DT  is attached to pin 30 */

#define POWER_SUPPLY_FAILURE   40    /*!< Power Supply Failure  is attached to pin 40 */
#define GAS_SUPPLY_FAILURE     41    /*!< Gas Supply Failure  is attached to pin 41 */
#define MECH_FAILSAFE_VALVE    42    /*!< Mech Failsafe Valve  is attached to pin 42 */


/**@}*/
