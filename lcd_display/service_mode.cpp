#include "service_mode.h"
void loop__ ()
{
     //Diagnostics_Mode();
}  

int Menu_Sel=0;

boolean continue_diag_mode = true;

void print_menu_common( menuIndex menuIdx)
{
  int numOfRowsToWrite=0;
  String strOnLine234 = ">";
  #if SERIAL_PRINTS
//  Serial.println("in print_menu_common menu, slect, scroll");
//  Serial.println(menuIdx);
//  Serial.println(seletIndicator);
//  Serial.println(scrollIndex);
  #endif
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(menuItems[menuIdx].menuName);
  #if SERIAL_PRINTS
//  Serial.println(menuItems[menuIdx].menuName);
  #endif
  numOfRowsToWrite = min(LCD_HEIGHT_CHAR-1, menuItems[menuIdx].menuLength);
  for (int i=0; i < numOfRowsToWrite; i++)//menuItems[menuIdx].menuLength; i++)
  {
    lcd.setCursor(0,i+1);
    if (seletIndicator == i+1)
    {
      strOnLine234 = ">";
    }
    else
    {
      strOnLine234 = " ";
    }
      strOnLine234 += menuItems[menuIdx].menu[scrollIndex + i];
      lcd.print (strOnLine234);
      #if SERIAL_PRINTS
//      Serial.println(strOnLine234);
      #endif
  }
  lcd.setCursor(0,(CursorLine-scrollIndex)+1);
}


void move_up()
{
  if (menuItems[currentMenuIdx].menuLength <= LCD_HEIGHT_CHAR-1) //menu length starts with 1
  {
    scrollIndex = 0;
  }
  seletIndicator++;
  /*
   * check wrap around of select indicator
   */
  if ((seletIndicator > menuItems[currentMenuIdx].menuLength)||
      (seletIndicator > LCD_HEIGHT_CHAR-1))
  {
    seletIndicator=min(LCD_HEIGHT_CHAR-1,menuItems[currentMenuIdx].menuLength);
    if (seletIndicator == LCD_HEIGHT_CHAR-1)
    {
      scrollIndex++;
    }
    if ((scrollIndex + seletIndicator) > menuItems[currentMenuIdx].menuLength)
    {
      scrollIndex = menuItems[currentMenuIdx].menuLength - seletIndicator;
    }
  }
  print_menu_common (currentMenuIdx);
}

void move_down()
{
  if (menuItems[currentMenuIdx].menuLength <= LCD_HEIGHT_CHAR-1) //menu length starts with 1
  {
    scrollIndex = 0;
  }
  seletIndicator--;
  /*
   * check wrap around of select indicator
   */
  if (seletIndicator == 0 )
  {
    seletIndicator=1;
    if (scrollIndex>0)
    {
      scrollIndex--;
    }
  }
  print_menu_common (currentMenuIdx);
}

void selection()
{

  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("You selected:");
  lcd.setCursor(0,2);
  lcd.print(menuItems[currentMenuIdx].menu[seletIndicator + scrollIndex -1 ]);
  delay(2000);
  lcd.clear();

  if ((seletIndicator == 1) &&
      (scrollIndex == 0))
  {
    if (currentMenuIdx != mainMenuE)
    {
      currentMenuLevel = MENU_LEVEL_0; 
      currentMenuIdx = mainMenuE;
    }
    else
    {
      continue_diag_mode = false;
    }
  }
  else
  {
    currentMenuLevel++;
    
    if (currentMenuLevel >= MAX_LEVEL )
    {
      if (menuItems[currentMenuIdx].functionPtr != NULL)
      {
        (menuItems[currentMenuIdx].functionPtr)();
      }
    }
    currentMenuIdx = seletIndicator + scrollIndex-1;
    if (currentMenuLevel >= MAX_LEVEL )
    {
      currentMenuLevel = MENU_LEVEL_0; 
      currentMenuIdx = mainMenuE;
    }
  }
  seletIndicator=1;
  scrollIndex=0;
  print_menu_common(currentMenuIdx);
}



void Diagnostics_Mode(void)
{
//  Serial.println("in Diagnostics_Mode");
  while(continue_diag_mode)
  {
    RT_Events_T eRTState = RT_NONE;
    if (!Menu_Sel)
    {
      print_menu_common(mainMenuE);
      Encoder_Scan();
      Menu_Sel++;
    }
    eRTState = Encoder_Scan();
    #if SERIAL_PRINTS
//    Serial.print("in Diagnostics_Mode");Serial.println(eRTState);
    #endif
    switch(eRTState)
    {
      case RT_INC:
         move_up();
         break;
      case   RT_DEC:
         move_down();
         break;
      case   RT_BT_PRESS:
         selection();
         break;
    }
  }
}

int initSelect = 0;
void move_up_init()
{
  lcd.setCursor(0,1);
  lcd.print(">>Edit parameters");
  lcd.setCursor(0,2);
  lcd.print("  Enter Diagnostics");
  initSelect = 1;
}

void move_down_init()
{
  lcd.setCursor(0,1);
  lcd.print("  Edit parameters");
  lcd.setCursor(0,2);
  lcd.print(">>Enter Diagnostics");
  initSelect = 2;
}

void selection_init(displayManager &dM)
{
  if  (initSelect == 1)
  {
     dM.displayManagerSetup();
  }
  else if (initSelect == 2)
  {
    Diagnostics_Mode();
  }
  initSelect = 0;
}

void displayInitialScreen(displayManager &dM)
{
  boolean continueLoop = true;
  int wait = 299;
  RT_Events_T eRTState = RT_NONE;
  encoderScanUnblocked();
  encoderScanUnblocked();
  encoderScanUnblocked();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Rotate encoder");
  lcd.setCursor(0,1);
  lcd.print("  Edit parameters");
  lcd.setCursor(0,2);
  lcd.print("  Enter Diagnostics");
  while( wait>0 )
  {
    eRTState = encoderScanUnblocked();
    if (eRTState != RT_NONE)
    {
      break;
    }
    lcd.setCursor(0,3);
    lcd.print(wait/100);
    delay (10);
    wait--;
  }
  if (eRTState != RT_NONE)
  {
    while (continueLoop)
    {
      eRTState = Encoder_Scan();
      #if SERIAL_PRINTS
//      Serial.print("in displayInitialScreen input received");Serial.println(eRTState);
      #endif
      switch(eRTState)
      {
        case RT_INC:
           move_up_init();
           break;
        case   RT_DEC:
           move_down_init();
           break;
        case   RT_BT_PRESS:
           selection_init(dM);
           continueLoop = false;
           break;
      }
    }
  }
//  Serial.println("exited displayInitialScreen");
}

