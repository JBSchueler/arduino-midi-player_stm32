/*
 *	Midi2Wave
 *
 *	Implementation
 */


#include "midi2wave.h"

short ptr;

void setupMidi() {
	for (uint8_t i=0; i<MAX_NOTE; ++i)
		key_vels[i] = 0;
	for (uint8_t i=0; i<KEYBUF_SIZE; ++i)
		active_keys[i] = 0;
	ptr = 0;
}

void renderWaveBuffer() {
	note_count = 0;
	// Update active key buffer with new keys
	// Preserve original index if the key is not released
	for (uint8_t i = 0; i < KEYBUF_SIZE; ++i)
		if (key_vels[active_keys[i]]) {
			key_vels[active_keys[i]] |= 32; // the 6th bit is used as a "key-not-released" flag
			++note_count;
		}
		else {
			active_keys[i] = 0; // Clear keys that are released
		}

	for (int i=MAX_NOTE-1; i>=0; --i) // Goes down: make sure the most significant note gets played
		if ( key_vels[i] && !(key_vels[i]&32) ) {
			for (char j=0; j<KEYBUF_SIZE; j++)
				if (!active_keys[j])
				{
					active_keys[j] = i;
					++note_count;
					break;
				}
		}

	for (uint8_t i = 0; i < KEYBUF_SIZE; ++i)
		if (active_keys[i])
			key_vels[active_keys[i]] &= 31; // Clear out the 6th bit "key-not-released" flag
}

void loadNextEvent() {
	if (ptr >= SONG_LEN)
	{
		// Restart in 3s
		setupMidi();
		event_length = 3000;
		return;
	}


  uint16_t noteParam = pgm_read_word_near(params+ptr);
  uint16_t new_length = (noteParam>>4)*TEMPO;
  key_vels[NOTE_NUMBER(ptr)-1] = noteParam&0x0F;

	++ptr;
	if (new_length == 0)
		loadNextEvent();
	else {
		renderWaveBuffer();
		event_length = new_length;
	}
}
