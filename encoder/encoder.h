
/**
 * @enum   EnumType
 * @brief   An enum description.
 */
#pragma once

typedef enum 
{
  RT_INC=0,
  RT_DEC,
  RT_BT_PRESS,
  RT_NONE
}RT_Events_T;


void isrEncoderClk();
RT_Events_T encoderScanIsr();
RT_Events_T encoderScanUnblocked() ;
RT_Events_T Encoder_Scan(void);

