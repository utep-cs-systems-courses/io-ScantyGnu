//Alternate LEDs from Off, Green, and Red
#include <msp430.h>
#include "libTimer.h"
#include "led.h"

int count = 0;

int main(void) {
  P1DIR |= LEDS;
  P1OUT |= LED_GREEN;
  P1OUT |= LED_RED;

  configureClocks();		/* setup master oscillator, CPU & peripheral clocks */
  enableWDTInterrupts();	/* enable periodic interrupt */
  
  or_sr(0x18);		/* CPU off, GIE on */
}


void
__interrupt_vec(WDT_VECTOR) WDT()	/* 250 interrupts/sec */
{
  count ++;
  if (count >= 3){
    P1OUT ^= LED_RED;
    count = 0;
  }
  P1OUT |= LED_GREEN;
} 

