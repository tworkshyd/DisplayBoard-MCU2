
/**************************************************************************/
/*!
    @file     lcd.h

    @brief     This Module contains all the configurations and Display details 

    @author   Tworks
    @{

*/
/**************************************************************************/

#pragma once
#include <LiquidCrystal.h>
#include <string.h>


#define LCD_LENGTH_CHAR 20                /*!< Length of the LCD in terms of Char*/
#define LCD_HEIGHT_CHAR 4                 /*!< Height of the LCD in terms of Char*/
#define NAME1_DISPLAY_POS 0               /*!< position where name1 to be displayed*/
#define VALUE1_DISPLAY_POS 5              /*!< position where value1 to be displayed*/
#define NAME2_DISPLAY_POS 11              /*!< position where name2 to be displayed*/
#define VALUE2_DISPLAY_POS 14             /*!< position where value2 to be displayed*/
#define PAR_SEP_CHAR ' '                  /*!< space assigned for parameter seperation*/
#define POWER_FAILURE_MSG1 "POWER FAILURE"
#define POWER_FAILURE_MSG2 "Check Power Supply"
#define PATIENT_DISCONN_MSG1 "PATIENT DISCONNECTED"
#define PATIENT_DISCONN_MSG2 "Check Breath Circuit"
#define PEEP_MSG1 "PEEP NOT ACHIEVED"
#define PEEP_MSG2 "Check Breath Circuit"
#define AIR_SUPPLY_MSG1 "AIR SUPPLY FAILURE"
#define AIR_SUPPLY_MSG2 "Check Breath Circuit"
#define TV_MSG1 "TV NOT ACHIEVED"
#define TV_MSG2 "Check Pressure Limit"
#define TV_MSG3 "BVM, Breath Circuit"
#define OXY_FAILURE_MSG1 "OXYGEN FAILURE"
#define OXY_FAILURE_MSG2 "Check Oxygen Supply"
#define BVM_FAILURE_MSG1 "BVM FAILURE"
#define BVM_FAILURE_MSG2 "Check BVM"




#define MAXCOLS  20
#define MAXROWS 4
#define MINGAP 2

char paddedValue[10];
LiquidCrystal lcd(DISPLAY_RS_PIN, DISPLAY_EN_PIN, DISPLAY_4_PIN, DISPLAY_3_PIN, DISPLAY_2_PIN, DISPLAY_1_PIN);

void lcdinsertword(String stringToInsert);

/**
 * @enum   ControlStatesDef_T
 * @brief   Following are the Control States during which Sensor data and user inputs are compared 
 * to generate specific commands required for that state.
 */
typedef enum
{
  NO_ERR = 0,
  ERR_OXY,
  ERR_BVM, 
  ERR_TV, 
  ERR_PEEP,
  ERR_PATIENT_DISCONN,          
}ErrorDef_T;


void cleanRow(unsigned short row) {
  lcd.setCursor(0, row);
  for (int i = 0; i < LCD_LENGTH_CHAR; i++) {
    lcd.print(" ");
  }
}


void cleanColRow(unsigned short col, unsigned short row) {
  lcd.setCursor(col, row);
  for (int i = col; i < LCD_LENGTH_CHAR; i++) {
    lcd.print(" ");
  }
}

void printPadded(int unpaddedNumber) {
  if (unpaddedNumber < 10) {
    lcd.print("  ");
  }else if (unpaddedNumber < 100) {
    lcd.print(" ");
  }
  lcd.print(unpaddedNumber);

}

const String emptystring ="                     ";
#ifdef DONT_USE
void lcdinsertword(String stringToInsert)
{
static String printing_row[] = {"","","",""};

static unsigned int currentRowNum;
static unsigned int currentColNum[]={0,0,0,0};
int status= -1;
 //tempstring = "";

  
if(currentColNum[currentRowNum]> 0 )//this means there is already a word that is left aligned. Now add a right aligned word
{
  if(stringToInsert.length() + currentColNum[currentRowNum] < MAXCOLS) //does the new word still fit in to this row?
  {
    printing_row[currentRowNum] =printing_row[currentRowNum].substring(0,currentColNum[currentRowNum]) + emptystring.substring(0,MAXCOLS-stringToInsert.length() - currentColNum[currentRowNum])+ stringToInsert; //insert the word to the right
    lcd.setCursor(0,currentRowNum);
    lcd.print(printing_row[currentRowNum]);
//    currentRowNum = (currentRowNum+1)%MAXROWS;
  }
  else
  {
    currentRowNum = (currentRowNum+1)%MAXROWS; //going to blindly overwrite next row
    printing_row[currentRowNum]= stringToInsert;
    currentColNum[currentRowNum]=stringToInsert.length();
    lcd.setCursor(0,currentRowNum);
    lcd.print(printing_row[currentRowNum]);   
       
  }
}
 else
  {
     printing_row[currentRowNum]= stringToInsert;
    currentColNum[currentRowNum]=stringToInsert.length();
   lcd.setCursor(0,currentRowNum);
    lcd.print(printing_row[currentRowNum]);
    
    
  
}

 Serial.println(printing_row[0]);
  Serial.println(printing_row[1]);
  Serial.println(printing_row[2]);
  Serial.println(printing_row[3]);
  
}
#endif

//String emptyString="aaaaaaaaaaaaaaaaaaaaaaa";
String emptyString="                       ";
void insertWord(int rowNum, int colNum, String insertString,unsigned int  minSize)
{
// Serial.println("*********);
//  Serial.println(colNum);
 //Serial.println(rowNum);
  
lcd.setCursor(colNum, rowNum);
lcd.print (insertString);
if(insertString.length() < minSize)
{
//  Serial.println(empty);
  lcd.print(emptyString.substring(0,minSize-insertString.length()));
//  lcd.setCursor(colNum+minSize-insertString.length(),rowNum);
}
  

}
/**@}*/
