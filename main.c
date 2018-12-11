/* 
	lab10 
	main.c
				*/

#include <stdint.h>
#include "tm4c123gh6pm.h"

#include "Timers.h"
#include "PLL.h"
#include "InputCapture.h"
#include "Blynk.h"
#include "esp8266.h"
#include "SysTick.h"
#include "PWM.h"
#include "ST7735.h"
#include "UART0.h"

void DisableInterrupts(void);
void EnableInterrupts(void);
void PortF_Init(void);
int getPID(int Error);
uint32_t desiredSpeed=0, tau=1000, curSpeed=0;
int32_t KP1=0, KP2=1, KI1=0, KI2=1;

/*preset to run at at start*/
//uint32_t desiredSpeed=600, tau=1000, curSpeed=2140;
//int32_t KP1=0, KP2=100, KI1=400, KI2=140;

int static count = 0;

int32_t Error, U, Up, Ui;

// Input capture
uint32_t period;
uint32_t static first;

int EDump[1000];
int dumpI = 0;

void SendData(void) {
    TM4C_to_Blynk(74, curSpeed/10);
}

void updateCommands(uint32_t speed, int32_t kp1, int32_t kp2, int32_t ki1, int32_t ki2, uint32_t t) {
    desiredSpeed = speed;
    KP1 = kp1;
    KP2 = kp2;
    KI1 = ki1;
    KI2 = ki2;
    tau = t;
}

void Timer1A_Handler(void) {
    GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R^0x04;
    GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R^0x04;
    TIMER1_ICR_R = TIMER_ICR_CAECINT;// acknowledge timer1A capture match
    int current = TIMER1_TAR_R;
    if(first < current)
        first += 0xFFFFFF;
    period = (first - current) & 0x00FFFFFF;
    first = current;
    if(count < 3 && period > 200000) {
        curSpeed = ((uint64_t)200000000)/period;   // Current speed is in units of 0.1 rps (0.1 resolution fixed-point)
    }
    count = 0;
    GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R^0x04;
}

uint32_t getCurSpeed() {
  return curSpeed;
}

int main(void){
  SYSCTL_RCGCGPIO_R |= 0x20;        // activate port F
  DisableInterrupts();
  PLL_Init(Bus80MHz);               // set system clock to 50 MHz
  // Initialize UART for debugging (terminal)
  UART_Init(1);

  // Setup ESP8266, using ST7735 Init as delay
  ESP8266_Init();       // Enable ESP8266 Serial Port
  ST7735_InitR(INITR_REDTAB);
  ESP8266_Reset();      // Reset the WiFi module
  ESP8266_SetupWiFi();  // Setup communications to Blynk Server

  Error = 0;
  MotorControlInit(Error);              // initialize Motor Control
  SysTick_Init();                   // Initialize SysTick
  InputCaptureInit();  // Initialize input capture

  // send data every 0.5s
  Timer2_Init(&SendData,40000000);
	PortF_Init();
  EnableInterrupts();
  ST7735_PlotClear(0,159);
	
//	while(1){
//	 PWM0B_Duty(Error);
//	}
	
  while(1){
    SysTick_Wait100us(10000 / (tau * 10));    // wait time depending on tau
    GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R^0x04; // toggle PF2
    Blynk_to_TM4C();
    count++;
    if(count >= 3) {
        curSpeed = 0;
    }

		
    Error = (desiredSpeed) - (curSpeed);// * 39990 / ((int)600);
		U = getPID(Error);
		
    PWM0B_Duty(U);

    //Print data to LCD
    ST7735_SetTextColor(ST7735_WHITE);
    ST7735_DrawString(15, 0, "   ", ST7735_WHITE);
    ST7735_DrawString(0, 0, "PI Loop  RPS = ", ST7735_WHITE);
		ST7735_SetCursor(15, 0);
    ST7735_OutUDec(desiredSpeed/10);
    ST7735_DrawString(4, 1, "       ", ST7735_WHITE);
    ST7735_DrawString(0, 1, "P = ", ST7735_WHITE);
    ST7735_SetCursor(4, 1);
    ST7735_OutUDec(Up);
    ST7735_DrawString(4, 2, "       ", ST7735_WHITE);
    ST7735_DrawString(0, 2, "I = ", ST7735_WHITE);
    ST7735_SetCursor(4, 2);
    ST7735_OutUDec(Ui);

    //Plot next speeds
    ST7735_PlotPoints((desiredSpeed * 159)/ 600, (curSpeed * 159)/600);
    ST7735_PlotNextErase();
  }
}

int getPID(int Error){
			EDump[dumpI] = Error;
			dumpI = (dumpI + 1) % 1000;
			int sum = 0;
			for(int i = 0; i < 1000; i++) {
					int x = EDump[i];
					if(x < 0)
							x = x * -1;
					sum += x;
			}
			Up = (KP1  * Error) / KP2;   //Proportional
			Ui += (KI1 * Error) / (KI2); //Integral
			if(Ui > 39990)
					Ui = 39990;
			if(Ui < 0)
					Ui = 0;
			U = Up + Ui;
			if(U > 39990)
					U = 39990;
			if(U < 0)
					U = 0;
			return U;
}
	
void PortF_Init(void){
	first = 0x00FFFFFF;               // Set initial value of first edge
  GPIO_PORTF_DIR_R |= 0x04;         // make PF2 out (built-in blue LED)
  GPIO_PORTF_AFSEL_R &= ~0x04;      // disable alt funct on PF2
  GPIO_PORTF_DEN_R |= 0x04;         // enable digital I/O on PF2
                                    // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF0FF)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;           // disable analog functionality on PF
}
