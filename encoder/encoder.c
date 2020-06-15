//#include <Wire.h>
#include <pins_arduino.h>
//#include "../pinout.h";

// Keep track of last rotary value
int lastCount = 100;

// Updated by the ISR (Interrupt Service Routine)
volatile int virtualPosition = 100;

// ------------------------------------------------------------------
// INTERRUPT     INTERRUPT     INTERRUPT     INTERRUPT     INTERRUPT
// ------------------------------------------------------------------

volatile int counter = 0; 
volatile static unsigned long last_interrupt_time = 0;

volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile int encoderPos = 0; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
volatile int oldEncPos = 0; //stores the last encoder position value so we can compare to the current reading and see if it has changed (so we know when to print to the serial monitor)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent

int getEncoderPos() {
    if(oldEncPos != encoderPos) {
      Serial.print("ENC: ");
      Serial.println(encoderPos);
      oldEncPos = encoderPos;
  }
    return encoderPos;
}

#ifndef OLD_BOARD
void isrEncoderClk(){
  //Serial.println("isrEncoderClk :");
  //PD2
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if(reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos --; //decrement the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void isrEncoderDt(){
  //Serial.println("isrEncoderDt :");
  //PD3
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos ++; //increment the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

#else
/*PIN is configured for high*/
void isrEncoderClk() {
  cli(); //stop interrupts happening before we read pin values
  reading = PINE & 0x30; // read all eight pin values then strip away all but pinA and pinB's values
  if(reading == B00110000 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos --; //decrement the encoder's position count
    if(encoderPos <=0) encoderPos = 200; 
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00010000) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts

}

void isrEncoderDt() {
  cli(); //stop interrupts happening before we read pin values
  reading = PINE & 0x30; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00110000 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos ++; //increment the encoder's position count
    if(encoderPos > 200) encoderPos = 1; 
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00100000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

#endif

bool switch_position_changed = false;

void isr_processSwitch() {
  //Serial.println("Button pressed :");
  switch_position_changed = true;
}

RT_Events_T encoderScanIsr() {
  RT_Events_T retVal = RT_NONE;
  counter = getEncoderPos();
  if (lastCount != counter) {
    if (lastCount < counter )
        retVal = RT_INC;
    else
        retVal = RT_DEC;
  }
  lastCount = counter;
  return retVal;
}

void isr_processStartEdit() {
  static unsigned long lastSwitchTime = 0;
  unsigned long switchTime = millis();
  //Serial.print("Button pressed :");
  if ((switchTime - lastSwitchTime) < DBNC_INTVL_SW) {
    return;
  }
  lastSwitchTime = switchTime;
}


#if 1
unsigned long lastButtonPress = 0;
int currentStateCLK;
int lastStateCLK;
boolean no_input = true;
int btnState;

RT_Events_T encoderScanUnblocked()
{
  RT_Events_T eRTState = RT_NONE;
    // Read the current state of CLK
   eRTState = encoderScanIsr();
   if ((eRTState == RT_INC) || (eRTState == RT_DEC))
    no_input = false;
  // Read the button state
  int btnState = digitalRead(DISP_ENC_SW);
  //Serial.print("btnState ");
  //Serial.println(btnState);

  //If we detect LOW signal, button is pressed
  if (btnState == LOW) 
  {
    //if 50ms have passed since last LOW pulse, it means that the
    //button has been pressed, released and pressed again
    if ((millis() - lastButtonPress > 50) && switch_position_changed) {
      eRTState = RT_BT_PRESS;
      switch_position_changed = false;
      no_input = false;
    }

    // Remember last button press event
    lastButtonPress = millis();
  }
  if (eRTState != RT_NONE)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1);
    digitalWrite(BUZZER_PIN, LOW);
    //Serial.print("encoderScanUnblocked ");
    //Serial.println(eRTState);
  }
  return eRTState;
}

RT_Events_T Encoder_Scan(void)
{
  RT_Events_T eRTState = RT_NONE;
  no_input = true;
  while(no_input)
  {
    eRTState = encoderScanUnblocked();
  }
  return(eRTState);
}
#endif

