//Display
#define DISPLAY_1_PIN       A15   // Changed
#define DISPLAY_2_PIN       A14   // Changed
#define DISPLAY_3_PIN       A13   // Changed
#define DISPLAY_4_PIN       A12   // Changed
#define DISPLAY_RS_PIN       A6   // Changed
#define DISPLAY_EN_PIN       A7   // Changed


//Control Pots
#define TIDAL_VOLUME_PIN   A0 //A4      //Unchanged  5
#define RR_PIN             A1 //A2    //Unchanged
#define PMAX_PIN           A2 //A1    //Unchanged
#define FiO2_PIN           A4 // A0    //Unchanged


#define POT_5_PIN     A3    // New
#define OXYGEN_ANALOG_PIN   A3  //on extensiion header (Pin number 7)
#define ADS115_INT_PIN      A8 // Interrupt pin ALRT ..on extensiion header (Pin number 5)
#define ADS115_INT_PIN_1    A9 // Interrupt pin ALRT ..on extensiion header (Pin number 5)

// ADDR pin is connected to ground on extensiion header (Pin number 4,6,8,10,12,14,16,18,20)
#define ADS115_I2C_SCL_PIN         21   //Unchanged  on extensiion header (Pin number 1)
#define ADS115_I2C_SDA_PIN         20   // Unchanged on extensiion header (Pin number 3)

#define AUTO_MODE      10            /*!< Auto mode  is attached to pin 10*/
#define ASSISTED_MODE  11            /*!< Assisted mode  is attached to pin 10*/

//I2C
#define I2C_SCL_PIN         21   //Unchanged
#define I2C_SDA_PIN         20   // Unchanged

#define BUZZER_PIN          7

//Encoder Pins
#define DISP_ENC_SW          3   
#define DISP_ENC_CLK         19 
#define DISP_ENC_DT          18  

//Start Stop Button
#define RESET_SWITCH        2

// Serial0
#define TX_PIN               D1   // Unchanged
#define RX_PIN               D0   // Unchanged

// Serial3
#define TX_PIN               15   // Unchanged
#define RX_PIN               14   // Unchanged

// Serial RS485
#define TX_PIN_485               17   // Unchanged
#define RX_PIN_485               16   // Unchanged
#define RTS_PIN_485              6

#define POWER_SUPPLY_FAILURE   40    /*!< Power Supply Failure  is attached to pin 40 */
#define GAS_SUPPLY_FAILURE     41    /*!< Gas Supply Failure  is attached to pin 41 */
#define MECH_FAILSAFE_VALVE    42    /*!< Mech Failsafe Valve  is attached to pin 42 */

//#define ONBOARD_LED_PIN  13

#define O2_CYN_SWITCH 13
#define O2_HOSP_SWITCH 12

#define BoardToBoard Serial3
#define BoardToPlotter Serial
#define BoardToRaspberryPi  Serial2
 
/*
 * PinOut
 * With comments on changes from older one.
 */
