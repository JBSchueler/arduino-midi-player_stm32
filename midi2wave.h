/*
 *	Midi2Wave
 *
 *	Playback controller
 *
 *	Load MIDI events into the global variable
 */

#ifndef __MIDI2WAVE_H__
#define __MIDI2WAVE_H__

#include "sequence.h"

#define MAX_NOTE 128
#define KEYBUF_SIZE (TRACKS+1)

/*!brief
 * These are the tone increment values of a note of octave 10
 * lower nibble selects the note
 * higher nibble divides the increment to the correct octave
 */
volatile const uint16_t incrtone[16] = { 29528, 31284, 33144, 35115, 37203, 39415, 41759, 44242, 46873, 49660, 52613, 55741 }; // Tones of octave 10
#define PIANOINC(key) (key? ( incrtone[key&0x0F] >> (10-(key>>4)) ) : 0)

#define NOTE_NUMBER(index) pgm_read_byte_near(notes+index)
#define NOTE_DELAY(index) (pgm_read_word_near(params+index)>>4)
#define NOTE_VEL(index) (pgm_read_word_near(params+index)&15)

uint8_t volatile active_keys[KEYBUF_SIZE];
uint8_t volatile key_vels[MAX_NOTE];

volatile uint8_t note_count = 0;

// The current length in milliseconds
uint16_t event_length = 0;

// The primary key
uint8_t key = 61;

/*
 *	Setup
 *
 *	Initialize stuff
 */
void setupMidi();

/*
 *	Render wave buffer
 *
 *	Generate the wave buffer with current notes
 */
void renderWaveBuffer();

/*
 *	Load next event
 *
 *	Load the next midi note/chord
 *	Updates the next delay variable
 *	Updates LEDs
 */
void loadNextEvent();

#endif
