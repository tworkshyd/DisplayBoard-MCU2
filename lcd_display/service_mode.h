
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
#ifndef SVCMODEH
#define SVCMODEH
#include "lcd.h"
#include "../encoder/encoder.h"
#include "ctrl_display.h"


#define SERIAL_PRINTS 0

/**
 * @enum   EnumType
 * @brief   An enum description.
 */
typedef enum{
  mainMenuE,
  subMenu1E,
  subMenu2E,
  subMenu3E,
  MAX_MENUS,
}menuIndex;

/**
 * @enum   EnumType
 * @brief   An enum description.
 */
typedef enum{
  MENU_LEVEL_0,
  MENU_LEVEL_1,
  MAX_LEVEL,
}menuLevelT;


typedef struct {
  char * menuName;
  char ** menu;
  int menuLength;
  menuLevelT menuLevel;
  void (* functionPtr)();
}menuItemsT;

void Diagnostics_Mode(void);
RT_Events_T Encoder_Scan(void);

menuIndex currentMenuIdx = mainMenuE;
int currentMenuLevel = 0;
int CursorLine = 0;
int DisplayFirstLine = 0;
int seletIndicator = 1; // can be 1,2,3
int scrollIndex = 0;


#define MAIN_MENU_LENGTH 4
#define SUB_MENU1_LENGTH 6
#define SUB_MENU2_LENGTH 2
#define SUB_MENU3_LENGTH 4




char* mainMenu[MAIN_MENU_LENGTH] = {" exit diag mode", " O2-Calib"," Check ADS1115"," Read All"};
char* subMenu1[SUB_MENU1_LENGTH] = {" go back", " O2 0%"," O2 21.6%"," O2 28%", " O2 40%"," O2 100%"};
char* subMenu2[SUB_MENU2_LENGTH] = {" go back", " validate ADC"};
char* subMenu3[SUB_MENU3_LENGTH] = {" go back", "Sensors Voltage"};

void diagO2Sensor(void);
void diagAds1115(void);
void sensorstatus(void);
void diagSolStatus(void);
void setup_service_mode ();
void print_menu_common( menuIndex menuIdx);

menuItemsT menuItems[MAX_MENUS] = 
{
  [mainMenuE] = {"Main menu ", mainMenu, MAIN_MENU_LENGTH, MENU_LEVEL_0, NULL},
  [subMenu1E] = {"SubMenu1 O2 ", subMenu1, SUB_MENU1_LENGTH, MENU_LEVEL_1, &diagO2Sensor},
  [subMenu2E] = {"SubMenu2 PS ", subMenu2, SUB_MENU2_LENGTH, MENU_LEVEL_1, &diagAds1115},  
  [subMenu3E] = {"SubMenu3 SNS ", subMenu3, SUB_MENU3_LENGTH, MENU_LEVEL_1, &sensorstatus}
};


void setup_service_mode ()
{
//  Serial.begin(9600);
  digitalWrite (DISP_ENC_DT, HIGH);     // enable pull-ups
  digitalWrite (DISP_ENC_CLK, HIGH);
  digitalWrite (DISP_ENC_SW, HIGH); 
//
//
//  lcd.begin (20,4);
}  // end of setup
#endif
