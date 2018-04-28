/*
 *  Arduino MIDI Player
 *
 *  Setup Arduino and use timer2 to synthesize and output sine wave
 *
 *  2016 by ilufang
 *  2018 by jbschueler
 *       optimized
 *       STM32 support (pwm pin PB0)
 */

/*
 * Part of this file contains code modified/referenced from
 * http://interface.khm.de/index.php/lab/interfaces-advanced/arduino-dds-sinewave-generator/
 *
 * DDS Sine Generator mit ATMEGA 168
 * Timer2 generates the  31250 KHz Clock Interrupt
 *
 * KHM 2009 /  Martin Nawrath
 * Kunsthochschule fuer Medien Koeln
 * Academy of Media Arts Cologne
 */

#include "avr/pgmspace.h"

#include "midi2wave.h"

typedef union phaccu_u
{
  uint16_t ulong;
  uint8_t  ubyte[2];
} phaccu_u;

// variables in interrupt service
uint32_t millisnow = 0;  // timing counter in milliseconds
uint32_t millisprev = 0; // timing counter in milliseconds

volatile phaccu_u phaccu[KEYBUF_SIZE]; // phase accumulator

volatile uint32_t tword_m[KEYBUF_SIZE]; // DDS tuning word m

static int Spkr = PB0;    
HardwareTimer pwmtimer(3);

void setup()
{
  setupMidi();

  // initialize DDS tuning word
  for ( uint8_t i=0; i<KEYBUF_SIZE; i++ )
  {
    tword_m[i]=0;
  }

  PWM_Init();
  Serial.begin(250000);
}

void loop()
{

  while(true) {
    millisnow = millis();
    if ( (millisnow-millisprev) > event_length) { // wait for the next midi event
      millisprev=millisnow;

      TIMER2_BASE->CR1 &= ~TIMER_CR1_CEN;      // ENABLE Timer counter
      loadNextEvent();
      // calculate new DDS tuning word

      for (uint8_t i=0; i<KEYBUF_SIZE; i++ )
      {
        uint8_t key = active_keys[i];
        tword_m[i]=PIANOINC(key);
        if (!tword_m[i]) phaccu[i].ulong = 0;
        if ((key>>4)<10) Serial.print( " " );
        Serial.print( key>>4 );
        Serial.print( ":" );
        if ((key&0xF)<10) Serial.print( " " );
        Serial.print( key&0xF );
        Serial.print( ",\t" );
      }
      Serial.println();
      TIMER2_BASE->CR1  |= TIMER_CR1_CEN;      // ENABLE Timer counter
    }
  }
}

/************************************************************************
Name        : PWM_Init()
Description : Initializes PWM hardware and associated GPIO ports
              Enables clocks for peripherals and configure modes
Parameters  : None
Return_val  : None
************************************************************************/
void PWM_Init( void )
{
  Timer3.attachCompare1Interrupt( irq_handler_timer3 ); // <-- this will set all the needed clokcks and int vectors
  pwmtimer.setPrescaleFactor(9);
  pwmtimer.setCount(0);
  pwmtimer.setOverflow(255);
  pinMode(Spkr, PWM);
  pwmWritefast(Spkr, 128);
}

static void pwmWritefast(uint8 pin, uint16 duty_cycle)
{
   timer_set_compare(PIN_MAP[pin].timer_device, PIN_MAP[pin].timer_channel, duty_cycle);
}

/*
 * Timer2 Interrupt Service
 *
 * Running at 31250 KHz = 32uSec
 * this is the timebase REFCLOCK for the DDS generator
 * FOUT = (M (REFCLK)) / (2 exp 32)
 * runtime : 8 microseconds ( inclusive push and pop)
 */
//=============================================================================
// TIM2 Interrupt Handler
//=============================================================================
void irq_handler_timer3( void )
{

    uint32_t phaccu_all=0;

    for ( uint8_t i=0; i<KEYBUF_SIZE; i++ )
    {
      phaccu[i].ulong += tword_m[i];
      phaccu_all += sine[phaccu[i].ubyte[1]];
    }

    // Write to PWM
#if KEYBUF_SIZE == 1
    pwmWritefast(Spkr, phaccu_all);
#elif KEYBUF_SIZE == 2
    pwmWritefast(Spkr, phaccu_all>>1);
#elif KEYBUF_SIZE == 4
    pwmWritefast(Spkr, phaccu_all>>2);
#elif KEYBUF_SIZE == 8
    pwmWritefast(Spkr, phaccu_all>>3);
#elif KEYBUF_SIZE == 16
    pwmWritefast(Spkr, phaccu_all>>4);
#else
    pwmWritefast(Spkr, phaccu_all/KEYBUF_SIZE);
#endif

}
