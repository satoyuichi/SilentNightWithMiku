// g++ -Wall -D__MACOSX_CORE__ -o midiprobe midi_test.cpp RtMidi.cpp -framework CoreMIDI -framework CoreAudio -framework CoreFoundation
// midiout.cpp
#include <iostream>
#include <cstdlib>
#include "RtMidi.h"
#include <unistd.h>
#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )

// Packets
#define NSX1_GOKAN_ON 0xF0, 0x43, 0x79, 0x09, 0x11, 0x0D, 0x0A, 0x06, 0x01, 0x00, 0xF7
#define NSX1_GOKAN_OFF_MIDI_RESET 0xF0, 0x43, 0x79, 0x09, 0x11, 0x0D, 0x0A, 0x06, 0x00, 0x00, 0x0A, 0x07, 0x00, 0x00, 0xF7

#define SYSEX_BEGIN 0xF0, 0x43, 0x79, 0x09, 0x11
#define SYSEX_END 0xF7

#define DIRECT_COMMAND_HEADER SYSEX_BEGIN, 0x0D
#define KASHI_POS_INC 0x09, 0x01, 0x00, 0x00
#define NOTE_ON 0x08, 0x09, 0x00, 0x00
#define NOTE_OFF 0x08, 0x08, 0x00, 0x00
#define REVOICE 0x08, 0x01, 0x00, 0x00
#define MOJI_SET_KASHI 0x09, 0x05, 0x00, 0x00
#define PITCH_BEND 0x08, 0x09, 0x00, 0x00

#define MIDI_RESET 0x0A, 0x07, 0x00, 0x00

// Pitch
#define PITCH_R (0xffff)
#define PITCH_C (PITCH_D - 1024)
#define PITCH_D (PITCH_E - 1024)
#define PITCH_E (PITCH_F - 512)
#define PITCH_F (PITCH_FS - 512)
#define PITCH_FS (8191)
#define PITCH_G (PITCH_FS + 512)
#define PITCH_A (PITCH_G + 1024)
#define PITCH_B (PITCH_A + 1024)
#define PITCH_CO (PITCH_B + 512)
#define PITCH_DO (PITCH_CO + 1024)

// Note time
#define NOTE_8 (800 >> 1)
#define NOTE_4 (NOTE_8 << 1)
#define NOTE_4P (NOTE_4 + (NOTE_4 >> 1))
#define NOTE_2 (NOTE_4 << 1)
#define NOTE_2P (NOTE_2 + (NOTE_2 >> 1))

// Packet Length
#define SIZE_SYSEX_BEGIN 5
#define SIZE_LYRICS_HEADER (SIZE_SYSEX_BEGIN + 2)
#define SIZE_SYSEX_END 1

#define SEND_PITCH_COMMAND(pitch) { pitch_bend_cmd [8] = ((pitch & 0xff80) >> 7); \
				   pitch_bend_cmd [9] = (pitch & 0x007f); \
				   midiout->sendMessage( pitch_bend_cmd, sizeof (pitch_bend_cmd) ); }

typedef struct {
     unsigned int pitch;
     int time;
} S_NOTE;

S_NOTE notes[] = {
     {PITCH_CO, NOTE_4}, {PITCH_G, NOTE_4}, {PITCH_E, NOTE_4}, {PITCH_G, NOTE_4P}, {PITCH_F, NOTE_8}, {PITCH_D, NOTE_4}, {PITCH_C, NOTE_2P}, {PITCH_C, NOTE_2P},
     {PITCH_G, NOTE_4P}, {PITCH_A, NOTE_8}, {PITCH_G, NOTE_4}, {PITCH_E, NOTE_2P}, {PITCH_G, NOTE_4P}, {PITCH_A, NOTE_8}, {PITCH_G, NOTE_4}, {PITCH_E, NOTE_2}, {PITCH_R, NOTE_4},
     {PITCH_DO, NOTE_2}, {PITCH_DO, NOTE_4}, {PITCH_B, NOTE_2P}, {PITCH_CO, NOTE_2}, {PITCH_CO, NOTE_4}, {PITCH_G, NOTE_2}, {PITCH_R, NOTE_4},
     {PITCH_A, NOTE_2}, {PITCH_A, NOTE_4}, {PITCH_CO, NOTE_4P}, {PITCH_B, NOTE_8}, {PITCH_A, NOTE_4}, {PITCH_G, NOTE_4P}, {PITCH_A, NOTE_8}, {PITCH_G, NOTE_4}, {PITCH_E, NOTE_2}, {PITCH_R, NOTE_4},
     {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_R, NOTE_4},
     {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_R, NOTE_4},
     {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_F, NOTE_8}, {PITCH_R, NOTE_4},
};

// Commands
unsigned char pitch_bend_cmd[] = {
     DIRECT_COMMAND_HEADER,
     PITCH_BEND,
     SYSEX_END
};
unsigned char gokan_on_cmd[] = {
     NSX1_GOKAN_ON
};
unsigned char gokan_off_cmd[] = {
     NSX1_GOKAN_OFF_MIDI_RESET
};
unsigned char note_on_cmd[] = { DIRECT_COMMAND_HEADER,
				MOJI_SET_KASHI,
				NOTE_ON,
				SYSEX_END
};
unsigned char revoice_and_next_cmd[] = { DIRECT_COMMAND_HEADER,
					 MOJI_SET_KASHI,
					 REVOICE,
					 KASHI_POS_INC,
					 SYSEX_END
};
unsigned char revoice_cmd[] = { DIRECT_COMMAND_HEADER,
				MOJI_SET_KASHI,
				REVOICE,
				SYSEX_END
};
unsigned char note_off_cmd[] = { DIRECT_COMMAND_HEADER,
				 NOTE_OFF,
//				 MIDI_RESET,
				 SYSEX_END
};
  
unsigned char lyrics[] = {
     SYSEX_BEGIN, 0x0A, 0x01,
     0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,	     // わわわわわわわ
     0x06, 0x01, 0x6e, 0x20, 0x09, 0x43, 0x6e, 0x71, // きよしこのよる
     0x4b, 0x20, 0x77, 0x48, 0x05, 0x70,	     // ほしはひかり
     0x17, 0x07, 0x01, 0x01, 0x43, 0x65, 0x01, 0x09, 0x77, // すくいのみこは
     0x64, 0x4E, 0x42, 0x43, 0x3F, 0x05, 0x40, // まぶねのなかに
     0x42, 0x66, 0x70, 0x29, 0x68, 0x02,       // ねむりたもう
     0x01, 0x2D, 0x6C, 0x17, 0x07,	       // いとやすく
     SYSEX_END
};

int main(int argc, char** argv)
{
  RtMidiOut *midiout = new RtMidiOut();

  // Check available ports.
  unsigned int nPorts = midiout->getPortCount();
  if ( nPorts == 0 ) {
    std::cout << "No ports available!\n";
    goto cleanup;
  }
  // Open first available port.
  midiout->openPort( 0 );

  midiout->sendMessage( gokan_on_cmd, sizeof( gokan_on_cmd ) );
  midiout->sendMessage( lyrics, sizeof( lyrics ) );

  midiout->sendMessage( note_on_cmd, sizeof( note_on_cmd ) );
//  for (int i = 0; i < sizeof (notes) / sizeof (notes[0]); i++ ) {
  for (int i = 0; i < 33; i++ ) {
       if (notes[i].pitch != PITCH_R) {
	    SEND_PITCH_COMMAND(notes[i].pitch);
       }
       SLEEP(notes[i].time);

       fprintf (stderr, "pitch: 0x%x, time: %d, pitch: 0x%02x, 0x%02x\n", notes[i].pitch, notes[i].time,
		pitch_bend_cmd[8], pitch_bend_cmd[9]);

       if (notes[i].pitch != PITCH_R) {
	    midiout->sendMessage( revoice_and_next_cmd, sizeof( revoice_and_next_cmd ) );
       }
  }
  SLEEP(500);

  midiout->sendMessage( note_off_cmd, sizeof( note_off_cmd ) );
  midiout->sendMessage( gokan_off_cmd, sizeof( gokan_off_cmd ) );

// Clean up
cleanup:
  delete midiout;
  return 0;
}
