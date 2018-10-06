
#pragma once

#include "master.h"
#include "collide.h"

//======================================================================

struct command_buffer_;
struct command_buffer_handler_;
struct archetype_data_;
struct timer_;

struct sound_id_ {

	enum {

		AMBIENCE_WIND,
		SHAMBLER_DEATH,
		ROCKET_FIRE,
		DOOR,
		TINK,
		BOUNCE,
		TELEPORT,
		EXPLODE,
		JUMP,
		SHAMBLER_IDLE,
		SHAMBLER_CHARGE,
		SCRAG_DEATH,
		SCRAG_CHARGE,
		SCRAG_IDLE,
		NULL_SOUND,
		ITEM_PICKUP,
		ITEM_SPAWN,
		PLAYER_PAIN,
		HUM,
		COUNT,

	};
};

struct direct_sound_ {

	enum {

		NUM_CHANNELS = 2,
		NUM_BYTES_PER_SAMPLE = 4,
		NUM_SAMPLES_PER_SECOND = 11025,

		NUM_BITS_PER_SAMPLE = NUM_BYTES_PER_SAMPLE * 8,
		NUM_BYTES_PER_FRAME = NUM_CHANNELS * NUM_BYTES_PER_SAMPLE,
		AVERAGE_BYTES_PER_SECOND = NUM_SAMPLES_PER_SECOND * NUM_BYTES_PER_FRAME,
		NUM_SECONDS_BUFFER = 3,
		BUFFER_SIZE_BYTES = AVERAGE_BYTES_PER_SECOND * NUM_SECONDS_BUFFER,
	};

	LPDIRECTSOUNDBUFFER back_buffer;
	LPDIRECTSOUND8 DS_interface;
	DWORD previous_play_cursor;
};

struct sound_buffer_ {

	enum {

		NUM_SECONDS_BUFFER = 1,
		BUFFER_SIZE_BYTES = direct_sound_::AVERAGE_BYTES_PER_SECOND * NUM_SECONDS_BUFFER,
		BUFFER_SIZE_REAL = BUFFER_SIZE_BYTES / 4,
	};

	CACHE_ALIGN float samples[BUFFER_SIZE_REAL];

};

#pragma pack(push, 1)
struct RIFF_chunk_ {

	__int8 chunk_id[4];
	__int32 chunk_size;
	__int8 wave_id[4];
};

struct format_chunk_ {

	__int8 chunk_id[4];
	__int32 chunk_size;
	__int16 format_code;
	__int16 n_channels;
	__int32 n_samples_per_second;
	__int32 n_avg_bytes_per_second;
	__int16 n_block_align;
	__int16 bits_per_sample;
	//__int16 cb_size;
};

struct data_chunk_ {

	__int8 chunk_id[4];
	__int32 chunk_size;
};
#pragma pack(pop)

struct wav_data_ {

	RIFF_chunk_ RIFF_chunk;
	format_chunk_ format_chunk;
	data_chunk_ data_chunk;
	__int32 n_samples;
	unsigned __int8* samples_byte;
};

struct sound_mixer_ {

	enum {

		//NULL_INDEX = -1,
		MAX_SOUNDS = 64,
		//I_ACTIVE_SOUNDS_HEADER = MAX_SOUNDS,
		//I_INACTIVE_SOUNDS_HEADER,
	};

	struct sound_ {

		//__int32 i_next;

		__int32 i_wav;
		__int32 source_id;
		__int32 current_sample;
		bool is_looped;
		float volume[2];
		int3_ position;
	};
	struct attenuation_ {

		float near_plane;
		float far_plane;
		float max_spatial_volume;
		float min_spatial_volume;
	};

	__int32 n_active;
	__int32 n_inactive;
	__int32 i_active[MAX_SOUNDS];
	__int32 i_inactive[MAX_SOUNDS];
	sound_ sounds[MAX_SOUNDS];

	attenuation_ attenuation;
};


struct sound_event_ {

	__int32 sound_id;
	__int32 source_id;
	__int32 is_valid;
};

struct sound_triggers_ {

	__m128 camera_position;

	enum {
		MAX_TRIGGERS = sound_mixer_::MAX_SOUNDS,
	};

	struct source_id_ {

		enum {

			DEFAULT,
			AMBIENCE = DEFAULT + 100,
			PLAYER = AMBIENCE + 100,
			PLAYER_WEAPON = PLAYER + 1,
			PROJECTILE = PLAYER + 100,
			BOUNCE_PAD = PROJECTILE + 100,
			TELEPORTER = BOUNCE_PAD + 100,
			ITEM = TELEPORTER + 100,
			BUTTON = ITEM + 100,
			DOOR = BUTTON + 100,
			PLATFORM = DOOR + 100,
			MONSTER = PLATFORM + 100,
		};
	};

	__int32 n_triggers;
	__int32 i_wavs[MAX_TRIGGERS];
	__int32 source_ids[MAX_TRIGGERS];
	int3_ positions[MAX_TRIGGERS];

};

//struct sound_memory_ {
//
//	enum {
//
//		CHUNK_SIZE = 1 << 19
//	};
//
//	unsigned __int8* chunk_ptr;
//	unsigned __int8 chunk[CHUNK_SIZE];
//};

struct sound_system_ {

	direct_sound_ direct_sound;
	sound_buffer_ sound_buffer;
	wav_data_ wavs[sound_id_::COUNT];
	sound_mixer_ sound_mixer;
	//sound_memory_ sound_memory;
	sound_event_ sound_event_table[colliding_type_::COUNT][colliding_type_::COUNT];

};


struct systems_::sound_ {


	static void process_event_table(void*, __int32);
	static void player_triggers(void*, __int32);
	static void door_triggers(void*, __int32);
	static void item_triggers(void*, __int32);
	static void switch_triggers(void*, __int32);
	static void monster_triggers(void*, __int32);
	static void play_sounds(void*, __int32);
};

struct parameters_::sound_ {

	struct process_event_table_ {

		__int32 n_threads;
		collision_manager_* collision_manager;
		sound_event_ (*sound_event_table)[colliding_type_::COUNT];
		command_buffer_handler_* command_buffer_handler;
	};
	struct player_triggers_ {

		command_buffer_handler_* command_buffer_handler;
		archetype_data_* archetype_data;
	};
	struct door_triggers_ {

		command_buffer_handler_* command_buffer_handler;
		archetype_data_* archetype_data;
	};
	struct item_triggers_ {

		command_buffer_handler_* command_buffer_handler;
		archetype_data_* archetype_data;
	};
	struct switch_triggers_ {

		command_buffer_handler_* command_buffer_handler;
		archetype_data_* archetype_data;
	};
	struct monster_triggers_ {

		command_buffer_handler_* command_buffer_handler;
		archetype_data_* archetype_data;
	};
	struct play_sounds_ {

		const timer_* timer;
		const archetype_data_* archetype_data;
		const command_buffer_handler_* command_buffer_handler;
		const wav_data_* wavs;
		direct_sound_* direct_sound;
		sound_mixer_* sound_mixer;
		sound_buffer_* sound_buffer;
	};

	process_event_table_ process_event_table;
	player_triggers_ player_triggers;
	door_triggers_ door_triggers;
	item_triggers_ item_triggers;
	switch_triggers_ switch_triggers;
	monster_triggers_ monster_triggers;
	play_sounds_ play_sounds;
};


//======================================================================

struct timer_;
struct memory_chunk_;

//======================================================================

void Initialise_Sound(
	
	direct_sound_&,
	sound_buffer_&,
	sound_mixer_&

);

void Load_Sounds(memory_chunk_&, wav_data_[sound_id_::COUNT]);

void Play_Sounds(

	const timer_&,
	const archetype_data_&,
	const sound_triggers_&,
	const wav_data_[sound_id_::COUNT],
	direct_sound_&,
	sound_mixer_&,
	sound_buffer_&

);

void Shutdown_Sound(direct_sound_&);
