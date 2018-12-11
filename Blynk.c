// -------------------------------------------------------------------
// File name: Blynk.c
// Description: This code is used to bridge the TM4C123 board and the Blynk Application
//              via the ESP8266 WiFi board
// Author: Mark McDermott and Andrew Lynch (Arduino source)
// Converted to EE445L style Jonathan Valvano
// Orig gen date: May 21, 2018
// Last update: Sept 20, 2018
//
// Download latest Blynk library here:
//   https://github.com/blynkkk/blynk-library/releases/latest
//
//  Blynk is a platform with iOS and Android apps to control
//  Arduino, Raspberry Pi and the likes over the Internet.
//  You can easily build graphic interfaces for all your
//  projects by simply dragging and dropping widgets.
//
//   Downloads, docs, tutorials: http://www.blynk.cc
//   Sketch generator:           http://examples.blynk.cc
//   Blynk community:            http://community.blynk.cc
//
//------------------------------------------------------------------------------

// TM4C123       ESP8266-ESP01 (2 by 4 header)
// PE5 (U5TX) to Pin 1 (Rx)
// PE4 (U5RX) to Pin 5 (TX)
// PE3 output debugging
// PE2 nc
// PE1 output    Pin 7 Reset
// PE0 input     Pin 3 Rdy IO2
//               Pin 2 IO0, 10k pullup to 3.3V  
//               Pin 8 Vcc, 3.3V (separate supply from LaunchPad 
// Gnd           Pin 4 Gnd  
// Place a 4.7uF tantalum and 0.1 ceramic next to ESP8266 3.3V power pin
// Use LM2937-3.3 and two 4.7 uF capacitors to convert USB +5V to 3.3V for the ESP8266
// http://www.ti.com/lit/ds/symlink/lm2937-3.3.pdf
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "tm4c123gh6pm.h"
#include "ST7735.h"
#include "PLL.h"
#include "UART0.h"
#include "esp8266.h"
#include "Timers.h"
#include "update.h"


void EnableInterrupts(void);    // Defined in startup.s
void DisableInterrupts(void);   // Defined in startup.s
void WaitForInterrupt(void);    // Defined in startup.s

static uint32_t desiredSpeed = 0;  // VP1
static int32_t KP1 = 0;           // VP2
static int32_t KP2 = 1;           // VP3
static int32_t KI1 = 0;           // VP4
static int32_t KI2 = 1;           // VP5
static uint32_t tau = 1000;           // VP6
static uint32_t curSpeed = 0;  // VP74
static uint32_t lastSpeed = 0;
// These 6 variables contain the most recent Blynk to TM4C123 message
// Blynk to TM4C123 uses VP0 to VP15
char serial_buf[64];
char Pin_Number[2]   = "99";       // Initialize to invalid pin number
char Pin_Integer[8]  = "0000";     //
char Pin_Float[8]    = "0.0000";   //
uint32_t pin_num; 
uint32_t pin_int;
 
// ----------------------------------- TM4C_to_Blynk ------------------------------
// Send data to the Blynk App
// It uses Virtual Pin numbers between 70 and 99
// so that the ESP8266 knows to forward the data to the Blynk App
void TM4C_to_Blynk(uint32_t pin,uint32_t value){
  if((pin < 70)||(pin > 99)){
    return; // ignore illegal requests
  }
// your account will be temporarily halted if you send too much data
  ESP8266_OutUDec(pin);       // Send the Virtual Pin #
  ESP8266_OutChar(',');
  ESP8266_OutUDec(value);      // Send the cur value
  ESP8266_OutChar(',');
  ESP8266_OutString("0.0\n");  // Null value not used in this example
}
 
 
// -------------------------   Blynk_to_TM4C  -----------------------------------
// This routine receives the Blynk Virtual Pin data via the ESP8266 and parses the
// data and feeds the commands to the TM4C.
void Blynk_to_TM4C(void){int j; char data;
// Check to see if a there is data in the RXD buffer
  if(ESP8266_GetMessage(serial_buf)){  // returns false if no message
    // Read the data from the UART0
#ifdef DEBUG1
    j = 0;
    do{
      data = serial_buf[j];
      UART_OutChar(data);        // Debug only
      j++;
    }while(data != '\n');
    UART_OutChar('\r');        
#endif
           
// Rip the 3 fields out of the CSV data. The sequence of data from the 8266 is:
// Pin #, Integer Value, Float Value.
    strcpy(Pin_Number, strtok(serial_buf, ","));
    strcpy(Pin_Integer, strtok(NULL, ","));       // Integer value that is determined by the Blynk App
    strcpy(Pin_Float, strtok(NULL, ","));         // Not used
    pin_num = atoi(Pin_Number);     // Need to convert ASCII to integer
    pin_int = atoi(Pin_Integer);  
    switch(pin_num)  {
    // ---------------------------- VP #0 ----------------------------------------
        case 0x0:
            desiredSpeed = pin_int*10;
        #ifdef DEBUG3
            Output_Color(ST7735_CYAN);
            ST7735_OutString("Rcv VP1 data=");
            ST7735_OutUDec(desiredSpeed);
            ST7735_OutChar('\n');
        #endif
            break;
    // ---------------------------- VP #1 ----------------------------------------
        case 0x1:
            KP1 = pin_int;
        #ifdef DEBUG3
            Output_Color(ST7735_CYAN);
            ST7735_OutString("Rcv VP2 data=");
            ST7735_OutUDec(KP1);
            ST7735_OutChar('\n');
        #endif
            break;
    // ---------------------------- VP #2 ----------------------------------------
        case 0x2:
            KP2 = pin_int;
        #ifdef DEBUG3
            Output_Color(ST7735_CYAN);
            ST7735_OutString("Rcv VP3 data=");
            ST7735_OutUDec(KP2);
            ST7735_OutChar('\n');
        #endif
            break;
    // ---------------------------- VP #3 ----------------------------------------
        case 0x3:
            KI1 = pin_int;
        #ifdef DEBUG3
            Output_Color(ST7735_CYAN);
            ST7735_OutString("Rcv VP4 data=");
            ST7735_OutUDec(KI1);
            ST7735_OutChar('\n');
        #endif
            break;
    // ---------------------------- VP #4 ----------------------------------------
        case 0x4:
            KI2 = pin_int;
        #ifdef DEBUG3
            Output_Color(ST7735_CYAN);
            ST7735_OutString("Rcv VP5 data=");
            ST7735_OutUDec(KI2);
            ST7735_OutChar('\n');
        #endif
            break;
    // ---------------------------- VP #5 ----------------------------------------
        case 0x5:
            tau = pin_int;
        #ifdef DEBUG3
            Output_Color(ST7735_CYAN);
            ST7735_OutString("Rcv VP6 data=");
            ST7735_OutUDec(tau);
            ST7735_OutChar('\n');
        #endif
            break;


    }
    updateCommands(desiredSpeed, KP1, KP2, KI1, KI2, tau);
    // Parse incoming data
#ifdef DEBUG1
    UART_OutString(" Pin_Number = ");
    UART_OutString(Pin_Number);
    UART_OutString("   Pin_Integer = ");
    UART_OutString(Pin_Integer);
    UART_OutString("   Pin_Float = ");
    UART_OutString(Pin_Float);
    UART_OutString("\n\r");
#endif
  }  
}

void SendInformation(void){
// your account will be temporarily halted if you send too much data
  curSpeed = getCurSpeed();
  if(curSpeed != lastSpeed){
    TM4C_to_Blynk(74, curSpeed);  // VP74
#ifdef DEBUG3
    Output_Color(ST7735_WHITE);
    ST7735_OutString("Send 74 data=");
    ST7735_OutUDec(curSpeed);
    ST7735_OutChar('\n');
#endif
  }
  lastSpeed = curSpeed;
}

  
void blynkInit(void){
  //PLL_Init(Bus80MHz);   // Bus clock at 80 MHz
  DisableInterrupts();  // Disable interrupts until finished with inits
#ifdef DEBUG3
  //Output_Init();        // initialize ST7735
  ST7735_OutString("EE445L Lab 4D\nBlynk example\n");
#endif
#ifdef DEBUG1
  //UART_Init(5);         // Enable Debug Serial Port
  UART_OutString("\n\rEE445L Lab 4D\n\rBlynk example");
#endif
  ESP8266_Init();       // Enable ESP8266 Serial Port
  ESP8266_Reset();      // Reset the WiFi module
  ESP8266_SetupWiFi();  // Setup communications to Blynk Server  
  
  Timer2_Init(&Blynk_to_TM4C,800000); 
  // check for receive data from Blynk App every 10ms

  Timer3_Init(&SendInformation,40000000); 
  // Send data back to Blynk App every 1/2 second
  EnableInterrupts();

  while(1) {
    WaitForInterrupt(); // low power mode
  }
}


