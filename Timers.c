// Timer1.c
// Runs on LM4F120/TM4C123
// Use TIMER1 in 32-bit periodic mode to request interrupts at a periodic rate
// Daniel Valvano
// May 5, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
  Program 7.5, example 7.6

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
#include <stdint.h>
#include "tm4c123gh6pm.h"

void (*PeriodicTask)(void);   // user function - Timer1
void (*PeriodicTask2)(void);   // user function - Timer2
void (*PeriodicTask3)(void);   // user function - Timer3

// ***************** TIMER1_Init ****************
// Activate TIMER1 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (1/clockfreq)
// Outputs: none
/*void Timer1_Init(void(*task)(void), uint32_t period){
    SYSCTL_RCGCTIMER_R |= 0x02;   // 0) activate TIMER1
    PeriodicTask = task;          // user function
    TIMER1_CTL_R = 0x00000000;    // 1) disable TIMER1A during setup
    TIMER1_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
    TIMER1_TAMR_R = 0x00000001;   // 3) configure for one-shot mode, default down-count settings
    TIMER1_TAILR_R = period-1;    // 4) reload value
    TIMER1_TAPR_R = 0;            // 5) bus clock resolution
    TIMER1_ICR_R = 0x00000001;    // 6) clear TIMER1A timeout flag
    TIMER1_IMR_R = 0x00000001;    // 7) arm timeout interrupt
    NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFF00FF)|0x00008000; // 8) priority 4
    // interrupts enabled in the main program after all devices initialized
    // vector number 37, interrupt number 21
    NVIC_EN0_R = 1<<21;           // 9) enable IRQ 21 in NVIC
}

void Timer1A_Handler(void){
    TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER1A timeout
    (*PeriodicTask)();                // execute user task
}*/

// ***************** Timer2_Init ****************
// Activate Timer2 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (1/clockfreq)
// Outputs: none
void Timer2_Init(void(*task)(void), uint32_t period){
  SYSCTL_RCGCTIMER_R |= 0x04;   // 0) activate timer2
  PeriodicTask2 = task;         // user function
  TIMER2_CTL_R = 0x00000000;    // 1) disable timer2A during setup
  TIMER2_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER2_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER2_TAILR_R = period-1;    // 4) reload value
  TIMER2_TAPR_R = 0;            // 5) bus clock resolution
  TIMER2_ICR_R = 0x00000001;    // 6) clear timer2A timeout flag
  TIMER2_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI5_R = (NVIC_PRI5_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 39, interrupt number 23
  NVIC_EN0_R = 1<<23;           // 9) enable IRQ 23 in NVIC
  TIMER2_CTL_R = 0x00000001;    // 10) enable timer2A
}

void Timer2A_Handler(void){
  TIMER2_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER2A timeout
  (*PeriodicTask2)();               // execute user task
}

// ***************** Timer3_Init ****************
// Activate Timer3 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (1/clockfreq)
// Outputs: none
void Timer3_Init(void(*task)(void), uint32_t period){
  SYSCTL_RCGCTIMER_R |= 0x08;   // 0) activate TIMER3
  PeriodicTask3 = task;         // user function
  TIMER3_CTL_R = 0x00000000;    // 1) disable TIMER3A during setup
  TIMER3_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER3_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER3_TAILR_R = period-1;    // 4) reload value
  TIMER3_TAPR_R = 0;            // 5) bus clock resolution
  TIMER3_ICR_R = 0x00000001;    // 6) clear TIMER3A timeout flag
  TIMER3_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI8_R = (NVIC_PRI8_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 51, interrupt number 35
  NVIC_EN1_R = 1<<(35-32);      // 9) enable IRQ 35 in NVIC
  TIMER3_CTL_R = 0x00000001;    // 10) enable TIMER3A
}

void Timer3A_Handler(void){
  TIMER3_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER3A timeout
  (*PeriodicTask3)();               // execute user task
}