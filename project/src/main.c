#include <msp430.h>
#include "libTimer.h"

#define LED_RED BIT6
#define LED_GREEN BIT0
#define LEDS (BIT0 | BIT6)

#define SW1 BIT0 // button at P2.0
#define SW2 BIT1 // button at P2.1
#define SW3 BIT2 // button at P2.2
#define SW4 BIT3 // button at P2.3
#define SWITCHES (SW1 | SW2 | SW3 | SW4)

void switch_init() {
  P2REN |= SWITCHES;/* enables resistors for switches */
  P2IE |= SWITCHES;/* enable interrupts from switches */
  P2OUT |= SWITCHES;/* pull-ups for switches */
  P2DIR &= ~SWITCHES;/* set switches' bits for input */
}

void led_init() {
  P1DIR |= LEDS;
  P1OUT &= ~LEDS;/* leds initially off */
}

void buzzer_init(){
  timerAUpmode();
  P2SEL2 &= ~(BIT6 | BIT7);
  P2SEL &= ~BIT7;
  P2SEL |= BIT6;
  P2DIR = BIT6;
  CCR0 = 0;
  CCR1 = 0;
}

void wdt_init() {
  configureClocks();/* setup master oscillator, CPU & peripheral clocks */
  enableWDTInterrupts();/* enable periodic interrupt */
}

void main(void) {
  switch_init();
  led_init();
  wdt_init();
  buzzer_init();
  or_sr(0x18);  // CPU off, GIE on
}

static int blinkState = 0;
static int songState = 0;
static int time = 1; // from 1 to 2048

// alternates when used to flash silly pattern
static int sillyBlink = 0;
// indicates state of wave pattern
static int waveState = 0;
static int wavePeriod = 64;

void move_blink(int mov){
  P1OUT &= ~LEDS; // Turn leds off
  blinkState += mov; // Move state machine
  sillyBlink = 0; // reset blink states
  waveState = 0;
  wavePeriod = 64; 
  if (blinkState >= 4) {
    blinkState = 0;
  }
  else if (blinkState < 0) {
    blinkState = 3;
  }
}

void move_song(int mov){
  songState += mov;
  CCR0 = 0;
  CCR1 = 0;
  if (songState >= 4){
    songState = 0;
  }
  if (songState < 0 ){
    songState = 4;
  }
}

void blink(){
  switch (blinkState) {
  case 0: // Silly blink state
    if (time % 256 == 0){
      if (sillyBlink){ // toggle green
	if (P1OUT & LED_RED){ // only if red is on
	  if (P1OUT & LED_GREEN){
	    P1OUT &= ~LED_GREEN;
	  }
	  else{
	    P1OUT |= LED_GREEN;
	  }
	}
      }
      else { // toggle red always
	if (P1OUT & LED_RED){
	  P1OUT &= ~LED_RED;
	}
	else{
	  P1OUT |= LED_RED;
	}
      }
      if (sillyBlink){
	sillyBlink = 0;
      }
      else{
	sillyBlink = 1;
      }
    }
    break;
    
  case 1: // Wave Pattern
    switch (waveState){
    case 0: //Green lights up
      if (time % wavePeriod == 0){
	P1OUT |= LED_GREEN;
	if (time % 64 == 0){
	  wavePeriod /= 2;
	}

	if (wavePeriod == 1){
	  wavePeriod = 128;
	  waveState = 1;
	}
      }
      else{
	P1OUT &= ~LED_GREEN;
      }
      P1OUT | LED_GREEN;
      break;
    case 1: // Red Lights up
      if (time % wavePeriod == 0){
	P1OUT |= LED_RED;
	if (time % 64 == 0){
	  wavePeriod /= 2;
	}
	
	if (wavePeriod == 1){
	  waveState = 2;
	}
      }
      else{
	P1OUT &= ~LED_RED;
      }
      break;
    case 2: // Green powers off
      if (time % wavePeriod == 0){
	P1OUT |= LED_GREEN;
	if (time % 64 == 0){
	  wavePeriod *= 2;
	}
	
	if (wavePeriod == 128){
	  P1OUT &= ~LED_GREEN;
	  wavePeriod = 1;
	  waveState = 3;
	}
      }
      else{
	P1OUT &= ~LED_GREEN;
      }
      break;
    case 3: // Red powers off
      if (time % wavePeriod == 0){
	P1OUT |= LED_RED;
	if (time % 64 == 0){
	  wavePeriod *= 2;
	}

	if (wavePeriod == 128){
	  P1OUT &= ~LED_RED;
	  wavePeriod = 128;
	  waveState = 0;
	}
      }
      else{
	P1OUT &= ~LED_RED;
      }
      break;
    default:
      P1OUT |= LEDS;
    }
    break;
    
  case 2:
    P1OUT |= LED_GREEN;
    break;
  case 3:
    P1OUT |= LED_RED;
    break;
  default: // Blink red led if there is an error
    if (P1OUT & LED_RED){
      P1OUT &= ~LED_RED;
    }
    else {
      P1OUT |= LED_RED;
    }
    break;
  }
}

void buzz(){
  switch (songState){
  case 0:
    CCR0 = CCR0 - 100;
    CCR1 = CCR0 >> 1;
    if (CCR0 <= 0){
      CCR0 = 2000;
      CCR1 >> 1;
    }
    break;
  case 1:
    CCR0 = CCR0 + 50;
    CCR1 = CCR0 >> 1;
    if (CCR0 > 2000){
      CCR0 = 0;
      CCR1 = 0;
    }
    break;
  case 2:
    CCR0 = 100;
    CCR1 = CCR0 >> 1;
    break;
  case 3:
    CCR0 = 1000;
    CCR1 = CCR0 >> 1;
    break;
  default:
    CCR0 = 0;
    CCR1 = 0;
    break;
  }
}

void __interrupt_vec(PORT2_VECTOR) Port_2() {
  if (P2IFG & SW1 & P2IN){
    P2IFG &= ~SW1;
    move_blink(1);
  }
  else if (P2IFG & SW2 & P2IN){
    P2IFG &= ~SW2;
    move_blink(-1);
  }
  else if (P2IFG & SW3 & P2IN){
    P2IFG &= ~SW3;
    move_song(1);
  }
  else if (P2IFG & SW4 & P2IN){
    P2IFG &= ~SW4;
    move_song(-1);
  }
  P1IES |= (P1IN & SWITCHES);
  P1IES &= (P1IN | ~SWITCHES);
}

void __interrupt_vec(WDT_VECTOR) WDT() {
  time++;
  if (time >= 2048){
    time = 1;
  }
  blink();
  buzz();
}
