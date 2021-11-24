// g++ -Wall -D__MACOSX_CORE__ -o silent_night_with_miku silent_night_with_miku.cpp RtMidi.cpp -framework CoreMIDI -framework CoreAudio -framework CoreFoundation
#include <unistd.h>
#include "RtMidi.h"
#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )

// #define INFINIT_LOOP

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
#define ROM_RESET 0x0A, 0x07, 0x01, 0x01

// Packet Length
#define SIZE_SYSEX_BEGIN 5
#define SIZE_LYRICS_HEADER (SIZE_SYSEX_BEGIN + 2)
#define SIZE_SYSEX_END 1

// Pitch
#define PITCH_R (0xffff)	// 休符
#define PITCH_C (PITCH_D - 1024)
#define PITCH_D (PITCH_E - 1024)
#define PITCH_E (PITCH_F - 512)
#define PITCH_F (PITCH_FS - 512)
#define PITCH_FS (8191 - 1024 * 4)
#define PITCH_G (PITCH_FS + 512)
#define PITCH_A (PITCH_G + 1024)
#define PITCH_B (PITCH_A + 1024)
#define PITCH_CO (PITCH_B + 512)
#define PITCH_DO (PITCH_CO + 1024)
#define PITCH_EO (PITCH_DO + 1024)
#define PITCH_FO (PITCH_EO + 512)

// Note time
#define NOTE_8 (300 * 1)			 // 八分音符
#define NOTE_4 (NOTE_8 << 1)		 // 四分音符
#define NOTE_4P (NOTE_4 + (NOTE_4 >> 1)) // 符点四分音符
#define NOTE_2 (NOTE_4 << 1)		 // 二分音符
#define NOTE_2P (NOTE_2 + (NOTE_2 >> 1)) // 符点二分音符

#define SEND_PITCH_COMMAND(pitch) { pitch_bend_cmd [8] = ((pitch & 0xff80) >> 7); \
		pitch_bend_cmd [9] = (pitch & 0x007f);							\
		midiout->sendMessage( pitch_bend_cmd, sizeof (pitch_bend_cmd) ); }

typedef struct {
	unsigned int pitch;
	int time;
} S_NOTE;

S_NOTE notes_holynight[] = {
	{PITCH_CO, NOTE_4}, {PITCH_G, NOTE_4}, {PITCH_E, NOTE_4}, {PITCH_G, NOTE_4P}, {PITCH_F, NOTE_8}, {PITCH_D, NOTE_4}, {PITCH_C, NOTE_2P}, {PITCH_C, NOTE_2P},
	{PITCH_G, NOTE_4P}, {PITCH_A, NOTE_8}, {PITCH_G, NOTE_4}, {PITCH_E, NOTE_2P}, {PITCH_G, NOTE_4P}, {PITCH_A, NOTE_8}, {PITCH_G, NOTE_4}, {PITCH_E, NOTE_2}, {PITCH_R, NOTE_4},
	{PITCH_DO, NOTE_2}, {PITCH_DO, NOTE_4}, {PITCH_B, NOTE_2P}, {PITCH_CO, NOTE_2}, {PITCH_CO, NOTE_4}, {PITCH_G, NOTE_2}, {PITCH_R, NOTE_4},
	{PITCH_A, NOTE_2}, {PITCH_A, NOTE_4}, {PITCH_CO, NOTE_4P}, {PITCH_B, NOTE_8}, {PITCH_A, NOTE_4}, {PITCH_G, NOTE_4P}, {PITCH_A, NOTE_8}, {PITCH_G, NOTE_4}, {PITCH_E, NOTE_2}, {PITCH_R, NOTE_4},
	{PITCH_A, NOTE_2}, {PITCH_A, NOTE_4}, {PITCH_CO, NOTE_4P}, {PITCH_B, NOTE_8}, {PITCH_A, NOTE_4}, {PITCH_G, NOTE_4P}, {PITCH_A, NOTE_8}, {PITCH_G, NOTE_4}, {PITCH_E, NOTE_2}, {PITCH_R, NOTE_4},
	{PITCH_DO, NOTE_2}, {PITCH_DO, NOTE_4}, {PITCH_FO, NOTE_4P}, {PITCH_DO, NOTE_8}, {PITCH_B, NOTE_4}, {PITCH_CO, NOTE_2P}, {PITCH_EO, NOTE_2}, {PITCH_R, NOTE_4},
	{PITCH_CO, NOTE_4}, {PITCH_G, NOTE_4}, {PITCH_E, NOTE_4}, {PITCH_G, NOTE_4P}, {PITCH_F, NOTE_8}, {PITCH_D, NOTE_4}, {PITCH_C, NOTE_2P}, {PITCH_R, NOTE_4},
};

S_NOTE notes_farewell [] = {
	{PITCH_C, NOTE_4},
	{PITCH_F, NOTE_4P}, {PITCH_F, NOTE_8}, {PITCH_F, NOTE_4}, {PITCH_A, NOTE_4},
	{PITCH_G, NOTE_4P}, {PITCH_F, NOTE_8}, {PITCH_G, NOTE_4}, {PITCH_A, NOTE_4},
	{PITCH_F, NOTE_4}, {PITCH_F, NOTE_4}, {PITCH_A, NOTE_4}, {PITCH_CO, NOTE_4},
	{PITCH_DO, NOTE_2P}, {PITCH_R, NOTE_4},

	{PITCH_DO, NOTE_4},
	{PITCH_CO, NOTE_4P}, {PITCH_A, NOTE_8}, {PITCH_A, NOTE_4}, {PITCH_F, NOTE_4},
	{PITCH_G, NOTE_4P}, {PITCH_F, NOTE_8}, {PITCH_G, NOTE_4}, {PITCH_A, NOTE_4},
	{PITCH_F, NOTE_4P}, {PITCH_D, NOTE_8}, {PITCH_D, NOTE_4}, {PITCH_C, NOTE_4},
	{PITCH_F, NOTE_2P}, {PITCH_R, NOTE_4},

	{PITCH_DO, NOTE_4},
	{PITCH_CO, NOTE_4P}, {PITCH_A, NOTE_8}, {PITCH_A, NOTE_4}, {PITCH_F, NOTE_4},
	{PITCH_G, NOTE_4P}, {PITCH_F, NOTE_8}, {PITCH_G, NOTE_4}, {PITCH_DO, NOTE_4},
	{PITCH_CO, NOTE_4P}, {PITCH_A, NOTE_8}, {PITCH_A, NOTE_4}, {PITCH_CO, NOTE_4},
	{PITCH_DO, NOTE_2P}, {PITCH_R, NOTE_4},

	{PITCH_DO, NOTE_4},
	{PITCH_CO, NOTE_4P}, {PITCH_A, NOTE_8}, {PITCH_A, NOTE_4}, {PITCH_F, NOTE_4},
	{PITCH_G, NOTE_4P}, {PITCH_F, NOTE_8}, {PITCH_G, NOTE_4}, {PITCH_A, NOTE_4},
	{PITCH_F, NOTE_4P}, {PITCH_D, NOTE_8}, {PITCH_D, NOTE_4}, {PITCH_C, NOTE_4},
	{PITCH_F, NOTE_2P}, {PITCH_R, NOTE_4},
};

// Commands
unsigned char pitch_bend_cmd[] = {
	DIRECT_COMMAND_HEADER,
	PITCH_BEND,
	SYSEX_END
};
unsigned char rom_reset_cmd[] = {
	DIRECT_COMMAND_HEADER,
	ROM_RESET,
	SYSEX_END
};
unsigned char gokan_on_cmd[] = {
	NSX1_GOKAN_ON
};
unsigned char gokan_off_cmd[] = {
	NSX1_GOKAN_OFF_MIDI_RESET
};
unsigned char note_on_cmd[] = {
	DIRECT_COMMAND_HEADER,
	MOJI_SET_KASHI,
	NOTE_ON,
	SYSEX_END
};
unsigned char note_on_and_next_cmd[] = {
	DIRECT_COMMAND_HEADER,
	MOJI_SET_KASHI,
	NOTE_ON,
	KASHI_POS_INC,
	SYSEX_END
};
unsigned char revoice_and_next_cmd[] = {
	DIRECT_COMMAND_HEADER,
	MOJI_SET_KASHI,
	REVOICE,
	KASHI_POS_INC,
	SYSEX_END
};
unsigned char revoice_cmd[] = {
	DIRECT_COMMAND_HEADER,
	MOJI_SET_KASHI,
	REVOICE,
	SYSEX_END
};
unsigned char note_off_cmd[] = {
	DIRECT_COMMAND_HEADER,
	NOTE_OFF,
	SYSEX_END
};

unsigned char lyrics_holynight[] = {
	SYSEX_BEGIN, 0x0A, 0x00,
	0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,		// わわわわわわわ
	0x06, 0x01, 0x6e, 0x20, 0x09, 0x43, 0x6e, 0x71, // きよしこのよる
	0x4b, 0x20, 0x77, 0x48, 0x05, 0x70,				// ほしはひかり
	0x46, 0x3C, 0x42, 0x03, 0x43, 0x65, 0x01, 0x07, 0x77, // はつねのミクは
	0x63, 0x4E, 0x42, 0x03, 0x43, 0x3F, 0x00, 0x05, 0x40, // まぶねのなかに
	0x42, 0x66, 0x70, 0x01, 0x29, 0x68, 0x04,		// ねむりたもう
	0x01, 0x01, 0x2D, 0x6C, 0x00, 0x17, 0x07, 0x02, // いとやすく
	SYSEX_END
};

unsigned char lyrics_farewell[] = {
	SYSEX_BEGIN, 0x0A, 0x00,
	0x4b, 0x29, 0x71, 0x43, 0x48, 0x05, 0x00, 0x70, 0x64, 0x32, 0x43, 0x6d, 0x02, 0x06, // ほたるのひかあり、まどのゆうき、
	0x49, 0x65, 0x6e, 0x66, 0x3c, 0x06, 0x01, 0x48, 0x05, 0x15, 0x42, 0x3c, 0x02, 0x3c, // ふみよむつきいひ、かさねつうつ、
	0x01, 0x3c, 0x20, 0x05, 0x2d, 0x20, 0x01, 0x68, 0x17, 0x0b, 0x01, 0x43, 0x2d, 0x7a, // いつしかとしいも、すぎいのとを、
	0x00, 0x08, 0x2c, 0x1e, 0x08, 0x15, 0x00, 0x77, 0x77, 0x05, 0x72, 0x6d, 0x02, 0x07, // あけてぞけさあは、わかれゆうく。
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
	midiout->sendMessage( lyrics_farewell, sizeof( lyrics_farewell) );

#ifdef INFINIT_LOOP
	while (1) {
#endif  // INFINIT_LOOP
		midiout->sendMessage( note_on_and_next_cmd, sizeof( revoice_and_next_cmd ) );
		for (int i = 0, l = 7; i < sizeof (notes_farewell) / sizeof (notes_farewell[0]); i++ ) {
			if (notes_farewell[i].pitch != PITCH_R) {
				SEND_PITCH_COMMAND(notes_farewell[i].pitch);
			}
			else {
				midiout->sendMessage( note_off_cmd, sizeof( note_off_cmd ) );
			}
			SLEEP(notes_farewell[i].time);

			fprintf (stderr, "[%2d] lyrics: 0x%02x, pitch: 0x%x, time: %d, pitch: 0x%02x, 0x%02x\n",
					 i, lyrics_farewell[l], notes_farewell[i].pitch, notes_farewell[i].time, pitch_bend_cmd[8], pitch_bend_cmd[9]);

			if (notes_farewell[i].pitch != PITCH_R) {
				midiout->sendMessage( revoice_and_next_cmd, sizeof( revoice_and_next_cmd ) );
				l++;
			}
		}
		SLEEP(NOTE_2P);
#ifdef INFINIT_LOOP
	}
#endif  // INFINIT_LOOP

	midiout->sendMessage( note_off_cmd, sizeof( note_off_cmd ) );
	midiout->sendMessage( gokan_off_cmd, sizeof( gokan_off_cmd ) );

	// Clean up
 cleanup:
	delete midiout;
	return 0;
}
