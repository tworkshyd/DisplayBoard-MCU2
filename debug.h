/**************************************************************************/
/*!
    @file    debug.h

    @brief    This file defines and includes all of the debug related utilities

    @author   Ritesh
  
  @date   6/10/2020 Initial revision
    @{
*/
/**************************************************************************/

/*-----------------------------------------------*/
//Debug macros

#define VENT_DEBUG_LEVEL_HIGH           3
#define VENT_DEBUG_LEVEL_INFO           2
#define VENT_DEBUG_LEVEL_ERR            1
#define VENT_DEBUG_LEVEL_NONE           0


#define VENT_DEBUG_LEVEL        VENT_DEBUG_LEVEL_ERR

#if VENT_DEBUG_LEVEL == VENT_DEBUG_LEVEL_HIGH

  #define VENT_DEBUG_FUNC_START()     \
    Serial.print(__func__);    \
    Serial.print(" Start...");    \
    Serial.println() 

  #define VENT_DEBUG_FUNC_END()     \
    Serial.print(" ...End ");    \
    Serial.print(__func__);    \
    Serial.println() 

  #define VENT_DEBUG_INFO(message, result) VENT_DEBUG_ERROR(message, result)

  #define VENT_DEBUG_ERROR(message, err)     \
    Serial.print(__func__);    \
    Serial.print(": ");    \
    Serial.print(message);    \
    Serial.println(err) 

#elif VENT_DEBUG_LEVEL == VENT_DEBUG_LEVEL_INFO

  #define VENT_DEBUG_FUNC_START()
  #define VENT_DEBUG_FUNC_END()
  #define VENT_DEBUG_INFO(message, result) VENT_DEBUG_ERROR(message, result)
  #define VENT_DEBUG_ERROR(message, err)     \
    Serial.print(__func__);    \
    Serial.print(": ");    \
    Serial.print(message);    \
    Serial.println(err)

#elif VENT_DEBUG_LEVEL == VENT_DEBUG_LEVEL_ERR

  #define VENT_DEBUG_FUNC_START()
  #define VENT_DEBUG_FUNC_END()
  #define VENT_DEBUG_INFO(message, result)
  #define VENT_DEBUG_ERROR(message, err)     \
    Serial.print(__func__);    \
    Serial.print(": ");    \
    Serial.print(message);    \
    Serial.println(err)

#else 
  #define VENT_DEBUG_FUNC_START()
  #define VENT_DEBUG_FUNC_END()
  #define VENT_DEBUG_ERROR(message, err)
  #define VENT_DEBUG_INFO(message, result)
#endif 

/*-----------------------------------------------*/

/**@}*/
