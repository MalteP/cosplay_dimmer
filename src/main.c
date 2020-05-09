// #############################################################################
// #                        --- Cosplay dimmer ---                             #
// #############################################################################
// # main.c - Main program                                                     #
// #############################################################################
// #              Version: 1.0 - Compiler: PlatformIO / AVR-GCC                #
// #     (c) 2020 by Malte Pöggel - www.MALTEPOEGGEL.de - malte@poeggel.de     #
// #############################################################################
// #  This program is free software; you can redistribute it and/or modify it  #
// #   under the terms of the GNU General Public License as published by the   #
// #        Free Software Foundation; either version 3 of the License,         #
// #                  or (at your option) any later version.                   #
// #                                                                           #
// #      This program is distributed in the hope that it will be useful,      #
// #      but WITHOUT ANY WARRANTY; without even the implied warranty of       #
// #           MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.            #
// #           See the GNU General Public License for more details.            #
// #                                                                           #
// #  You should have received a copy of the GNU General Public License along  #
// #      with this program; if not, see <http://www.gnu.org/licenses/>.       #
// #############################################################################

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "main.h"

int main( void )
 {
  // Init ports
  DDRB &= ~(1<<PB0);	// Switch input
  PORTB |= (1<<PB0);	// Enable internal pullup
  DDRB |= (1<<PB1);		// LED output (OC0B)

  // Setup timer
  TCCR0A |= (1<<COM0B1) | (1<<WGM00);	// Clear OC0B on Compare Match, PWM Phase Correct
  TCCR0B |= (1<<CS00);					// Clk/1
  OCR0B = 0;

  // Brigntness values
  int pwm_value = 0;
  const int pwm_values[] = {255, 128, 64, 32, 0};

  // Main loop
  while(1)
   {
    OCR0B = pwm_values[pwm_value];
    // Deep sleep on PWM value 0 and button not pressed
    sleep(pwm_values[pwm_value]==0&&!keystatus());
    // Check switch
    if(keydown())
     {
      // Loop through PWM value array
      if(++pwm_value>=(sizeof(pwm_values)/sizeof(pwm_values[0])))
       {
        pwm_value=0;
       }
     }
   }
 }

// Read key, negative logic
int keystatus( void )
 {
  return !(PINB & (1<<PB0));
 }

// Function to check if button / reed switch is pressed
int keydown( void )
 {
  int state = keystatus();
  static int debounce = 0;			// Counter for key debouncing
  if(debounce==0)
   {
    if(state) // Key down
     {
      debounce = 255; // Set debouncing time
      return 1;
     }
   } else {
    if(!state) // Key up
     {
      _delay_us(500);
      --debounce;
     }
   }
  return 0;
 }

// If LED is switched off, put AVR into deep sleep to save power (after some delay)
// Status flag must be set to 1 for given time bevor sleep mode is entered
void sleep( int status )
 {
  static int delay = 0;

  // Sleep mode delayed by 128ms, else interrupt might trigger immediately
  if(status==0)
   {
    delay = 255;
    return;
   } 
  if(delay>0)
   {
    _delay_us(500);
    --delay;
    return;
   }

  // Enable port change interrupt on PCINT0 (PB0)
  GIMSK |= (1<<PCIE);
  PCMSK |= (1<<PCINT0);

  // Enable global interrupts and go to sleep
  sei();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_mode();
  cli();

  // Disable PCINT
  GIMSK &= ~(1<<PCIE);
 }

// We'll wake up after deep sleep mode in this ISR
ISR(PCINT0_vect)
 {
 }
