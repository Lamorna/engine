#include "sound.h"
#include "windows.h"
#include "component.h"
#include "command.h"
#include "setup.h"
#include "memory.h"


enum {

	LEFT_CHANNEL,
	RIGHT_CHANNEL,
};


//======================================================================


/*
==================
==================
*/
void Initialise_Sound(

	direct_sound_& direct_sound,
	sound_buffer_& sound_buffer,
	sound_mixer_& sound_mixer

) {

	// ----------------------------------------------------------------------------------------------------------
	// initialise direct sound
	// ----------------------------------------------------------------------------------------------------------
	{
		HRESULT sound_handle = DirectSoundCreate8(
			NULL,
			&direct_sound.DS_interface,
			NULL
		);

		if (sound_handle == DS_OK) {
			printf_s("SOUND INTERFACE INITIALISED \n");
		}
		else {
			printf_s("SOUND INTERFACE FAILED \n");
			exit(0);
		}

		DIRECT_SOUND_Set_Cooperative_Level(direct_sound.DS_interface);

		DSBUFFERDESC buffer_descriptor_32;
		WAVEFORMATEXTENSIBLE wave_format;
		memset(&buffer_descriptor_32, 0, sizeof(DSBUFFERDESC));
		memset(&wave_format, 0, sizeof(WAVEFORMATEXTENSIBLE));

		{
			wave_format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
			wave_format.Format.nChannels = direct_sound_::NUM_CHANNELS;
			wave_format.Format.nSamplesPerSec = direct_sound_::NUM_SAMPLES_PER_SECOND;
			wave_format.Format.wBitsPerSample = direct_sound_::NUM_BITS_PER_SAMPLE;
			wave_format.Format.nAvgBytesPerSec = direct_sound_::AVERAGE_BYTES_PER_SECOND;
			wave_format.Format.nBlockAlign = direct_sound_::NUM_BYTES_PER_FRAME;
			wave_format.Format.cbSize = 22;

			wave_format.Samples.wValidBitsPerSample = direct_sound_::NUM_BITS_PER_SAMPLE;
			wave_format.Samples.wSamplesPerBlock = direct_sound_::NUM_CHANNELS;
			wave_format.Samples.wReserved = 0;

			wave_format.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
			wave_format.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;

			buffer_descriptor_32.dwSize = sizeof(buffer_descriptor_32);
			buffer_descriptor_32.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
			buffer_descriptor_32.dwBufferBytes = direct_sound_::BUFFER_SIZE_BYTES;;
			buffer_descriptor_32.dwReserved = 0;
			buffer_descriptor_32.lpwfxFormat = &wave_format.Format;
			buffer_descriptor_32.guid3DAlgorithm = DS3DALG_DEFAULT;
		}

		HRESULT buffer_handle = direct_sound.DS_interface->CreateSoundBuffer(
			&buffer_descriptor_32,
			&direct_sound.back_buffer,
			NULL
		);

		if (buffer_handle == DS_OK) {
			printf("SECONDARY BUFFER CREATED \n");
		}
		else {
			printf("SECONDARY BUFFER NOT CREATED \n");
			if (buffer_handle == DSERR_INVALIDPARAM) {
				printf("DSERR_INVALIDPARAM \n");
			}
			exit(0);
		}
	}
	// ----------------------------------------------------------------------------------------------------------
	// initialise sound mixer
	// ----------------------------------------------------------------------------------------------------------
	{
		for (__int32 i_sounds = 0; i_sounds < sound_mixer_::MAX_SOUNDS; i_sounds++) {

			sound_mixer.sounds[i_sounds].is_looped = false;
			sound_mixer.sounds[i_sounds].volume[LEFT_CHANNEL] = 1.0f;
			sound_mixer.sounds[i_sounds].volume[RIGHT_CHANNEL] = 1.0f;
		}

		sound_mixer.attenuation.far_plane = 600.0f;
		sound_mixer.attenuation.near_plane = 100.0f;
		sound_mixer.attenuation.max_spatial_volume = 1.0f;
		sound_mixer.attenuation.min_spatial_volume = 0.1f;

		{
			sound_mixer.n_active = 0;
			sound_mixer.n_inactive = sound_mixer_::MAX_SOUNDS;

			for (__int32 i_sound = 0; i_sound < sound_mixer_::MAX_SOUNDS; i_sound++) {
				sound_mixer.i_active[i_sound] = 0;
				sound_mixer.i_inactive[i_sound] = (sound_mixer_::MAX_SOUNDS - 1) - i_sound;
			}
		}
	}
	// ----------------------------------------------------------------------------------------------------------
	// add ambient sound track
	// ----------------------------------------------------------------------------------------------------------
	{
		{
			// add active sound
			sound_mixer.i_active[sound_mixer.n_active++] = sound_mixer.i_inactive[--sound_mixer.n_inactive];
			const __int32 i_sound = sound_mixer.i_active[sound_mixer.n_active - 1];

			const __int32 i_wav = sound_id_::AMBIENCE_WIND;
			sound_mixer.sounds[i_sound].i_wav = i_wav;
			sound_mixer.sounds[i_sound].current_sample = 0;
			sound_mixer.sounds[i_sound].is_looped = true;
			sound_mixer.sounds[i_sound].volume[LEFT_CHANNEL] = 0.8f;
			sound_mixer.sounds[i_sound].volume[RIGHT_CHANNEL] = 0.8f;
			sound_mixer.sounds[i_sound].source_id = sound_triggers_::source_id_::AMBIENCE;
		}
	}
	// ----------------------------------------------------------------------------------------------------------
	// clear sound buffer
	// ----------------------------------------------------------------------------------------------------------
	{
		memset(sound_buffer.samples, 0, sizeof(sound_buffer.samples));
	}
	// ----------------------------------------------------------------------------------------------------------
	// start playing
	// ----------------------------------------------------------------------------------------------------------
	{
		direct_sound.previous_play_cursor = 0;

		HRESULT play_handle = direct_sound.back_buffer->Play(0, 0, DSBPLAY_LOOPING);

		if (play_handle != DS_OK) {
			printf_s("CANNOT PLAY PRIMARY SOUND BUFFER \n");
			exit(0);
		}

	}
	// ----------------------------------------------------------------------------------------------------------
}

/*
==================
==================
*/
void Load_Sounds(
	
	memory_chunk_& memory_chunk,
	wav_data_ wavs[sound_id_::COUNT]

) {

	const char *file_names[sound_id_::COUNT] = {

		"sounds/ambience/wind.wav",
		"sounds/shambler/death.wav",
		"sounds/weapon/shoot.wav",
		"sounds/misc/door.wav",
		"sounds/weapon/ricochet.wav",
		"sounds/misc/bounce.wav",
		"sounds/misc/teleport.wav",
		"sounds/weapon/explode.wav",
		"sounds/player/jump.wav",
		"sounds/shambler/idle.wav",
		"sounds/shambler/charge.wav",
		"sounds/scrag/death.wav",
		"sounds/scrag/charge.wav",
		"sounds/scrag/idle.wav",
		"sounds/misc/null.wav",
		"sounds/items/pickup.wav",
		"sounds/items/spawn.wav",
		"sounds/player/pain.wav",
		"sounds/ambience/hum.wav",
	};

	const __int32 n_wavs = sound_id_::COUNT;

	for (__int32 i_wav = 0; i_wav < n_wavs; i_wav++) {

		FILE *file_handle = NULL;
		fopen_s(&file_handle, file_names[i_wav], "rb");
		if (file_handle == NULL) {
			printf_s("FAILED to open %s \n", file_names[i_wav]);
			exit(0);
		}
		else {
			printf_s("opened %s \n", file_names[i_wav]);
		}

		enum {

			OFFSET_RIFF,
			OFFSET_WAVE,
			OFFSET_FORMAT,
			OFFSET_DATA,

			MAX_CHARS = 8,
		};

		unsigned __int8 file_signatures[][MAX_CHARS] = {

			"RIFF",
			"WAVE",
			"fmt",
			"data",
		};

		const __int32 n_signatures = sizeof(file_signatures) / sizeof(file_signatures[0]);
		__int32 signature_offsets[n_signatures];

		{
			for (__int32 i_signature = 0; i_signature < n_signatures; i_signature++) {

				unsigned __int8 ch = ' ';
				__int32 n_chars = 0;
				while ((ch != '\0') && (n_chars < MAX_CHARS)) {
					ch = file_signatures[i_signature][n_chars];
					n_chars++;
				}
				n_chars--; // don't count nul terminator

				const __int32 i_last_char = n_chars - 1;
				const unsigned __int32 bit_mask = (0x1 << n_chars) - 1;
				unsigned __int8 char_buffer[MAX_CHARS];
				unsigned __int32 result = 0x0;
				__int32 read_char = ' ';
				signature_offsets[i_signature] = 0;

				fseek(file_handle, 0, SEEK_SET);	// reset file pointer to start

				while ((read_char != EOF) && (result != bit_mask)) {

					for (__int32 i_char = 0; i_char < n_chars - 1; i_char++) {
						char_buffer[i_char] = char_buffer[i_char + 1];
					}
					read_char = fgetc(file_handle);
					char_buffer[i_last_char] = read_char;

					result = 0x0;
					for (__int32 i_char = 0; i_char < n_chars; i_char++) {
						result |= (char_buffer[i_char] == file_signatures[i_signature][i_char]) << i_char;
					}
					signature_offsets[i_signature]++;
				}
				signature_offsets[i_signature] -= n_chars;
			}
		}

		fseek(file_handle, 0, SEEK_SET);	// reset file pointer to start

		{
			const __int32 byte_offset = signature_offsets[OFFSET_RIFF];
			fseek(file_handle, byte_offset, SEEK_SET);
			fread(&wavs[i_wav].RIFF_chunk, sizeof(RIFF_chunk_), 1, file_handle);
		}
		{
			const __int32 byte_offset = signature_offsets[OFFSET_FORMAT];
			fseek(file_handle, byte_offset, SEEK_SET);
			fread(&wavs[i_wav].format_chunk, sizeof(format_chunk_), 1, file_handle);
		}
		{
			const __int32 byte_offset = signature_offsets[OFFSET_DATA];
			fseek(file_handle, byte_offset, SEEK_SET);
			fread(&wavs[i_wav].data_chunk, sizeof(data_chunk_), 1, file_handle);


			wavs[i_wav].samples_byte = (unsigned __int8*)memory_chunk.chunk_ptr;


			fread(wavs[i_wav].samples_byte, sizeof(unsigned __int8), wavs[i_wav].data_chunk.chunk_size, file_handle);
		}

		wavs[i_wav].n_samples = wavs[i_wav].data_chunk.chunk_size;
		memory_chunk.chunk_ptr = wavs[i_wav].samples_byte + wavs[i_wav].data_chunk.chunk_size;

		fclose(file_handle);

		//RIFF_chunk& riff_chunk = wavs[i_wav].RIFF_chunk;
		//format_chunk& format_ch = wavs[i_wav].format_chunk;
		//data_chunk& data_ch = wavs[i_wav].data_chunk;
		//printf_s("CHUNK ID: %c%c%c%c \n", riff_chunk.chunk_id[0], riff_chunk.chunk_id[1], riff_chunk.chunk_id[2], riff_chunk.chunk_id[3]);
		//printf_s("CHUNK SIZE: %i \n", riff_chunk.chunk_size);
		//printf_s("WAVE ID: %c%c%c%c \n", riff_chunk.wave_id[0], riff_chunk.wave_id[1], riff_chunk.wave_id[2], riff_chunk.wave_id[3]);
		//printf_s("FORMAT CHUNK ID: %c%c%c%c \n", format_ch.chunk_id[0], format_ch.chunk_id[1], format_ch.chunk_id[2], format_ch.chunk_id[3]);
		//printf_s("FORMAT CHUNK SIZE: %i \n", format_ch.chunk_size);
		//printf_s("FORMAT CODE: %i \n", format_ch.format_code);
		//printf_s("N_CHANNELS: %i \n", format_ch.n_channels);
		//printf_s("N_SAMPLES_PER_SECOND: %i \n", format_ch.n_samples_per_second);
		//printf_s("N_AVG_BYTES_PER_SECOND: %i \n", format_ch.n_avg_bytes_per_second);
		//printf_s("N_BLOCK_ALIGN: %i \n", format_ch.n_block_align);
		//printf_s("BITS_PER_SAMPLE: %i \n", format_ch.bits_per_sample);
		//printf_s("DATA CHUNK ID: %c%c%c%c \n", data_ch.chunk_id[0], data_ch.chunk_id[1], data_ch.chunk_id[2], data_ch.chunk_id[3]);
		//printf_s("DATA CHUNK SIZE: %i \n", data_ch.chunk_size);
		//printf_s("\n");
	}
}


/*
==================
==================
*/
void systems_::sound_::play_sounds(

	void* parameters, __int32 i_thread
)
{
	parameters_::sound_::play_sounds_* func_parameters = (parameters_::sound_::play_sounds_*)parameters;
	//const timer_& timer = *func_parameters->timer;
	//const archetype_data_& archetype_data = *func_parameters->archetype_data;
	const command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	const wav_data_* wavs = func_parameters->wavs;
	direct_sound_& direct_sound = *func_parameters->direct_sound;
	sound_mixer_& sound_mixer = *func_parameters->sound_mixer;
	sound_buffer_& sound_buffer = *func_parameters->sound_buffer;

	const command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_read];
	const sound_triggers_& sound_triggers = command_buffer.sound_triggers;

	const __int32 n_channels = direct_sound_::NUM_CHANNELS;
	const __int32 n_bytes_per_sample = direct_sound_::NUM_BYTES_PER_SAMPLE;
	const __int32 buffer_size_bytes = direct_sound_::BUFFER_SIZE_BYTES;
	//const __int32 buffer_size_real = direct_sound_::BUFFER_SIZE_BYTES / 4;

	// ----------------------------------------------------------------------------------------------------------
	// advance samples by play cursor delta
	// ----------------------------------------------------------------------------------------------------------
	{


		DWORD play_cursor;
		DWORD write_cursor;
		direct_sound.back_buffer->GetCurrentPosition(
			&play_cursor,
			&write_cursor
		);

		const __int32 delta_bytes = play_cursor - direct_sound.previous_play_cursor;
		const __int32 delta_bytes_wrapped = buffer_size_bytes + delta_bytes;
		const bool is_wrapped = delta_bytes < 0;
		//const __int32 n_bytes_write = blend_int(delta_bytes_wrapped, delta_bytes, is_wrapped);
		const __int32 n_bytes_write = is_wrapped ? delta_bytes_wrapped : delta_bytes;
		const __int32 n_frames = n_bytes_write / (n_channels * n_bytes_per_sample);

		for (__int32 i_active = 0; i_active < sound_mixer.n_active; i_active++) {

			const __int32 i_sound = sound_mixer.i_active[i_active];
			const __int32 i_wav = sound_mixer.sounds[i_sound].i_wav;
			sound_mixer.sounds[i_sound].current_sample += n_frames;
			__int32 clamped = min(sound_mixer.sounds[i_sound].current_sample, wavs[i_wav].n_samples);
			__int32 wrapped = sound_mixer.sounds[i_sound].current_sample % wavs[i_wav].n_samples;
			sound_mixer.sounds[i_sound].current_sample = (sound_mixer.sounds[i_sound].is_looped) ? wrapped : clamped;
		}

		direct_sound.previous_play_cursor = play_cursor;
	}
	// ----------------------------------------------------------------------------------------------------------
	// remove finished sounds from mixer
	// ----------------------------------------------------------------------------------------------------------
	{		

		for (__int32 i_active = 0; i_active < sound_mixer.n_active; i_active++) {

			const __int32 i_sound = sound_mixer.i_active[i_active];
			const __int32 i_wav = sound_mixer.sounds[i_sound].i_wav;
			const __int32 n_samples_remaining = wavs[i_wav].n_samples - sound_mixer.sounds[i_sound].current_sample;

			if (n_samples_remaining <= 0) {

				sound_mixer.i_inactive[sound_mixer.n_inactive++] = sound_mixer.i_active[i_active];
				sound_mixer.i_active[i_active] = sound_mixer.i_active[--sound_mixer.n_active];
			}
		}		
	}
	// ----------------------------------------------------------------------------------------------------------
	// add triggered sounds to mixer
	// ----------------------------------------------------------------------------------------------------------
	{
		for (__int32 i_trigger = 0; i_trigger < sound_triggers.n_triggers; i_trigger++) {

			__int32 i_current_sound = INVALID_RESULT;
			for (__int32 i_active = 0; i_active < sound_mixer.n_active; i_active++) {

				const __int32 i_sound = sound_mixer.i_active[i_active];
				bool is_match = sound_mixer.sounds[i_sound].source_id == sound_triggers.source_ids[i_trigger];
				//i_current_sound = blend_int(i_sound, i_current_sound, is_match);
				i_current_sound = is_match ? i_sound : i_current_sound;
			}
			bool is_new_sound = i_current_sound < 0;
			const __int32 i_new_sound = sound_mixer.i_inactive[sound_mixer.n_inactive - 1];
			//const __int32 i_sound = blend_int(i_new_sound, i_current_sound, is_new_sound);
			const __int32 i_sound = is_new_sound ? i_new_sound : i_current_sound;

			sound_mixer.sounds[i_sound].i_wav = sound_triggers.i_wavs[i_trigger];
			sound_mixer.sounds[i_sound].current_sample = 0;
			sound_mixer.sounds[i_sound].position = sound_triggers.positions[i_trigger];
			sound_mixer.sounds[i_sound].source_id = sound_triggers.source_ids[i_trigger];

			sound_mixer.i_active[sound_mixer.n_active] = i_sound;
			sound_mixer.n_active += is_new_sound;
			sound_mixer.n_inactive -= is_new_sound;
		}
		//printf_s("NUM ACTIVE SOUNDS: %i \n", sound_mixer.n_active);
		//printf_s("NUM INACTIVE SOUNDS: %i \n\n", sound_mixer.n_inactive);
	}
	// ----------------------------------------------------------------------------------------------------------
	// spatial & distance attenuation
	// ----------------------------------------------------------------------------------------------------------
	{
		// for spatial attenuation
		// x:	1.0 - 0.0 - (-1.0)
		// L:   1.0 - 0.5 - 0.0
		// R:   0.0 - 0.5 - 1.0

		// distance attenuation
		// dist		100 - 600
		// volume	1.0 - 0.0

		const __m128 zero = set_zero();
		const __m128 one = set_all(1.0f);

		const __m128 spatial_max = one;
		const __m128 spatial_min = -one;
		const __m128 volume_max_spatial = broadcast(load_s(sound_mixer.attenuation.max_spatial_volume));
		const __m128 volume_min_spatial = broadcast(load_s(sound_mixer.attenuation.min_spatial_volume));
		const __m128 delta_spacial = (volume_max_spatial - volume_min_spatial) / (spatial_max - spatial_min);

		const __m128 near_plane = broadcast(load_s(sound_mixer.attenuation.near_plane));
		const __m128 far_plane = broadcast(load_s(sound_mixer.attenuation.far_plane));
		const __m128 volume_max_distance = one;
		const __m128 volume_min_distance = zero;
		const __m128 delta_distance = (volume_max_distance - volume_min_distance) / (near_plane - far_plane);

		const __m128i source_ambience = broadcast(load_s(sound_triggers_::source_id_::AMBIENCE));

		for (__int32 i_active = 0; i_active < sound_mixer.n_active; i_active += 4) {

			__int32 n = min(sound_mixer.n_active - i_active, 4);

			float3_ gather_positions[4];
			float current_volume_4[2][4];
			__int32 source_id_4[4];
			__int32 i_sounds[4];
			for (__int32 i = 0; i < n; i++) {

				const __int32 i_sound = sound_mixer.i_active[i_active + i];
				i_sounds[i] = i_sound;
				source_id_4[i] = sound_mixer.sounds[i_sound].source_id;
				gather_positions[i].x = (float)(sound_mixer.sounds[i_sound].position.x - command_buffer.position_camera.x) * r_fixed_scale_real;
				gather_positions[i].y = (float)(sound_mixer.sounds[i_sound].position.y - command_buffer.position_camera.y) * r_fixed_scale_real;
				gather_positions[i].z = (float)(sound_mixer.sounds[i_sound].position.z - command_buffer.position_camera.z) * r_fixed_scale_real;
				current_volume_4[LEFT_CHANNEL][i] = sound_mixer.sounds[i_sound].volume[LEFT_CHANNEL];
				current_volume_4[RIGHT_CHANNEL][i] = sound_mixer.sounds[i_sound].volume[RIGHT_CHANNEL];
			}

			__m128 positions[4];
			for (__int32 i_active = 0; i_active < n; i_active++) {
				positions[i_active] = load_u(gather_positions[i_active].f);
			}
			Transpose(positions);

			positions[W] = set_all(1.0f);

			matrix camera_matrix;
			for (__int32 i = 0; i < 4; i++) {
				camera_matrix[i] = load_u(command_buffer.m_clip_space[i].f);
			}

			__m128 transposed_camera_matrix[4];
			Transpose(camera_matrix, transposed_camera_matrix);
			__m128 positions_camera_space[4];
			Matrix_X_Matrix(positions, transposed_camera_matrix, positions_camera_space);

			__m128 magnitude = set_zero();
			for (__int32 i = X; i < W; i++) {
				magnitude += positions_camera_space[i] * positions_camera_space[i];
			}
			magnitude = _mm_sqrt_ps(magnitude);
			__m128 magnitude_reciprocal = reciprocal(magnitude);

			__m128 normalised_positions[3];
			for (__int32 i = X; i < W; i++) {
				normalised_positions[i] = positions_camera_space[i] * magnitude_reciprocal;
			}
			__m128 volume_out[2];
			{
				__m128 left = volume_max_spatial + ((normalised_positions[X] - spatial_max) * delta_spacial);
				left = max_vec(left, volume_min_spatial);
				left = min_vec(left, volume_max_spatial);
				__m128 right = volume_max_spatial - left;

				// intentionally swapped here
				volume_out[LEFT_CHANNEL] = right * right;
				volume_out[RIGHT_CHANNEL] = left * left;
			}
			{
				__m128 volume = volume_max_distance + ((magnitude - near_plane) * delta_distance);
				volume = max_vec(volume, volume_min_distance);
				volume = min_vec(volume, volume_max_distance);
				volume_out[LEFT_CHANNEL] *= volume * volume;
				volume_out[RIGHT_CHANNEL] *= volume * volume;
			}

			__m128i source_id = load_u(source_id_4);
			__m128i is_attenuated = source_id != source_ambience;
			volume_out[LEFT_CHANNEL] = blend(volume_out[LEFT_CHANNEL], load_u(current_volume_4[LEFT_CHANNEL]), is_attenuated);
			volume_out[RIGHT_CHANNEL] = blend(volume_out[RIGHT_CHANNEL], load_u(current_volume_4[RIGHT_CHANNEL]), is_attenuated);

			float volume_out_4[2][4];
			store(volume_out[LEFT_CHANNEL], volume_out_4[LEFT_CHANNEL]);
			store(volume_out[RIGHT_CHANNEL], volume_out_4[RIGHT_CHANNEL]);

			for (__int32 i = 0; i < n; i++) {
				const __int32 i_sound = i_sounds[i];
				sound_mixer.sounds[i_sound].volume[LEFT_CHANNEL] = volume_out_4[LEFT_CHANNEL][i];
				sound_mixer.sounds[i_sound].volume[RIGHT_CHANNEL] = volume_out_4[RIGHT_CHANNEL][i];
			}
		}
	}
	// ----------------------------------------------------------------------------------------------------------
	// write to main buffer
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 n_frames_oversample = 300;
		const __int32 n_bytes_oversample = n_frames_oversample * n_channels * n_bytes_per_sample;

		// ----------------------------------------------------------------------------------------------------------
		{
			const __m128 max = set_all(1.0f);
			const __m128 min = -max;
			const __m128 shift = broadcast(load_s(256.0f / 2.0f));
			const __m128 scale = reciprocal(shift);
			const __m128i loop_inc = broadcast(load_s(4));
			const __m128i loop_initialise = set(0, 1, 2, 3);
			const __m128 zero = set_zero();

			const __int32 n_samples = n_frames_oversample * n_channels;
			for (__int32 i_sample = 0; i_sample < n_samples; i_sample += 4) {
				store(zero, sound_buffer.samples + i_sample);
			}

			for (__int32 i_active = 0; i_active < sound_mixer.n_active; i_active++) {

				const __int32 i_sound = sound_mixer.i_active[i_active];
				const __int32 i_wav = sound_mixer.sounds[i_sound].i_wav;
				const __int32 n_samples_remaining = wavs[i_wav].n_samples - sound_mixer.sounds[i_sound].current_sample;
				const bool is_ending = (n_samples_remaining < n_frames_oversample) && (!sound_mixer.sounds[i_sound].is_looped);
				//const __int32 n_samples_write = blend_int(n_samples_remaining, n_frames_oversample, is_ending);
				const __int32 n_samples_write = is_ending ? n_samples_remaining : n_frames_oversample;

				float* write_buffer = sound_buffer.samples;
				__int32 current_sample = sound_mixer.sounds[i_sound].current_sample;

				const __m128i is_looped_4 = broadcast(load_s(sound_mixer.sounds[i_sound].is_looped));
				const __m128 volume_left = broadcast(load_s(sound_mixer.sounds[i_sound].volume[LEFT_CHANNEL]));
				const __m128 volume_right = broadcast(load_s(sound_mixer.sounds[i_sound].volume[RIGHT_CHANNEL]));
				const __m128i n_samples_write_4 = broadcast(load_s(n_samples_write));

				__m128i loop_count = loop_initialise;

				for (__int32 i_sample = 0; i_sample < n_samples_write; i_sample += 4) {

					__int32 n = min(n_samples_write - i_sample, 4);

					__m128 sample_float;	// convert from UINT 8 bit PCM data to 32 bit float
					{
						unsigned __int32 samples[4];
						for (__int32 i = 0; i < n; i++) {
							samples[i] = wavs[i_wav].samples_byte[current_sample];
							current_sample++;
							current_sample %= wavs[i_wav].n_samples;
						}
						__m128i sample_byte = load_u(samples);
						sample_float = (convert_float(sample_byte) - shift) * scale;
						sample_float = min_vec(max, sample_float);
						sample_float = max_vec(min, sample_float);
					}
					__m128 frame_0_1;		// build frame & attenuate with volume
					__m128 frame_2_3;
					{
						sample_float &= loop_count < n_samples_write_4;
						__m128 sample_left = sample_float * volume_left;
						__m128 sample_right = sample_float * volume_right;
						frame_0_1 = interleave_lo(sample_left, sample_right);
						frame_2_3 = interleave_hi(sample_left, sample_right);
						frame_0_1 += load(write_buffer);
						frame_2_3 += load(write_buffer + 4);

					}
					store(frame_0_1, write_buffer);
					store(frame_2_3, write_buffer + 4);

					loop_count += loop_inc;
					write_buffer += 8;
				}
			}
		}
		// ----------------------------------------------------------------------------------------------------------
		{
			LPVOID audio_ptr_0;
			DWORD audio_bytes_0;
			LPVOID audio_ptr_1;
			DWORD audio_bytes_1;

			HRESULT lock_error = direct_sound.back_buffer->Lock(
				0,
				n_bytes_oversample,
				&audio_ptr_0,
				&audio_bytes_0,
				&audio_ptr_1,
				&audio_bytes_1,
				DSBLOCK_FROMWRITECURSOR
			);

			// ASSUME that write and play cursors, and the data blocks returned by
			// the lock are all aligned

			if (lock_error == DS_OK) {

				memcpy((float*)audio_ptr_0, sound_buffer.samples, audio_bytes_0);
				__int32 read_offset = audio_bytes_0 / direct_sound_::NUM_BYTES_PER_SAMPLE;
				memcpy((float*)audio_ptr_1, sound_buffer.samples + read_offset, audio_bytes_1);
			}

			direct_sound.back_buffer->Unlock(
				audio_ptr_0,
				audio_bytes_0,
				audio_ptr_1,
				audio_bytes_1
			);
		}
	}
}

/*
==================
==================
*/
void Shutdown_Sound(direct_sound_& direct_sound) {

	direct_sound.back_buffer->Stop();
	direct_sound.back_buffer->Release();
	direct_sound.DS_interface->Release();
}

/*
========================================================================
========================================================================
*/
void systems_::sound_::process_event_table(

	void* parameters, __int32 i_thread
)
{
	parameters_::sound_::process_event_table_* func_parameters = (parameters_::sound_::process_event_table_*)parameters;
	collision_manager_& collision_manager = *func_parameters->collision_manager;
	command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_write];
	sound_triggers_& sound_triggers = command_buffer.sound_triggers;
	sound_event_(*sound_event_table)[colliding_type_::COUNT] = func_parameters->sound_event_table;

	sound_triggers.n_triggers = 0;

	for (__int32 i_thread = 0; i_thread < func_parameters->n_threads; i_thread++) {

		thread_collision_& thread_collision = collision_manager.thread_collisions[i_thread];

		for (__int32 i_collider = 0; i_collider < thread_collision.n_colliders; i_collider++) {

			const collision_output_& collision_output = thread_collision.collision_output[i_collider];
			sound_event_& sound_event = sound_event_table[collision_output.collider.entity_type][collision_output.collidee.entity_type];

			sound_triggers.i_wavs[sound_triggers.n_triggers] = sound_event.sound_id;
			sound_triggers.source_ids[sound_triggers.n_triggers] = sound_event.source_id + collision_output.collider.entity_id;
			sound_triggers.positions[sound_triggers.n_triggers] = collision_output.collision_point;
			sound_triggers.n_triggers += sound_event.is_valid;
		}

		thread_collision.n_colliders = 0;
	}
}
/*
========================================================================
========================================================================
*/
void systems_::sound_::player_triggers(

	void* parameters, __int32 i_thread
)
{
	parameters_::sound_::player_triggers_* func_parameters = (parameters_::sound_::player_triggers_*)parameters;
	archetype_data_& archetype_data = *func_parameters->archetype_data;
	command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_write];
	sound_triggers_& sound_triggers = command_buffer.sound_triggers;

	component_fetch_ fetch;
	fetch.n_components = 5;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::BASE;
	fetch.component_ids[1] = component_id_::CAMERA;
	fetch.component_ids[2] = component_id_::WEAPON;
	fetch.component_ids[3] = component_id_::ANIMATION_DRIVER;
	fetch.component_ids[4] = component_id_::SOUND_TRIGGER;


	Populate_Fetch_Table(archetype_data, fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
		component_::camera_* camera = (component_::camera_*)fetch.table[1][i_archetype_index];
		component_::weapon_* weapon = (component_::weapon_*)fetch.table[2][i_archetype_index];
		component_::animation_driver_* animation_driver = (component_::animation_driver_*)fetch.table[3][i_archetype_index];
		component_::sound_trigger_* sound_trigger = (component_::sound_trigger_*)fetch.table[4][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			sound_triggers.i_wavs[sound_triggers.n_triggers] = sound_id_::ROCKET_FIRE;
			sound_triggers.source_ids[sound_triggers.n_triggers] = sound_triggers_::source_id_::PLAYER_WEAPON;
			sound_triggers.positions[sound_triggers.n_triggers] = base[i_entity].position_fixed;
			sound_triggers.n_triggers += weapon[i_entity].is_fired;

			sound_triggers.i_wavs[sound_triggers.n_triggers] = sound_id_::JUMP;
			sound_triggers.source_ids[sound_triggers.n_triggers] = sound_trigger[i_entity].source_id;
			sound_triggers.positions[sound_triggers.n_triggers] = base[i_entity].position_fixed;
			sound_triggers.n_triggers += camera[i_entity].is_jumped;

			sound_triggers.i_wavs[sound_triggers.n_triggers] = sound_id_::PLAYER_PAIN;
			sound_triggers.source_ids[sound_triggers.n_triggers] = sound_trigger[i_entity].source_id;
			sound_triggers.positions[sound_triggers.n_triggers] = base[i_entity].position_fixed;
			sound_triggers.n_triggers += animation_driver[i_entity].trigger_id == animation_data_::id_::PAIN;
		}
	}
}
/*
========================================================================
========================================================================
*/
void systems_::sound_::door_triggers(

	void* parameters, __int32 i_thread
)
{
	parameters_::sound_::door_triggers_* func_parameters = (parameters_::sound_::door_triggers_*)parameters;
	archetype_data_& archetype_data = *func_parameters->archetype_data;
	command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_write];
	sound_triggers_& sound_triggers = command_buffer.sound_triggers;

	component_fetch_ fetch;
	fetch.n_components = 3;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::BASE;
	fetch.component_ids[1] = component_id_::SOUND_TRIGGER;
	fetch.component_ids[2] = component_id_::DOOR;

	Populate_Fetch_Table(archetype_data, fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
		component_::sound_trigger_* sound_trigger = (component_::sound_trigger_*)fetch.table[1][i_archetype_index];
		component_::door_* door = (component_::door_*)fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			sound_triggers.i_wavs[sound_triggers.n_triggers] = sound_id_::DOOR;
			sound_triggers.source_ids[sound_triggers.n_triggers] = sound_trigger[i_entity].source_id;
			sound_triggers.positions[sound_triggers.n_triggers] = base[i_entity].position_fixed;
			sound_triggers.n_triggers += door[i_entity].state_trigger == component_::door_::state_::MOVE;

			sound_triggers.i_wavs[sound_triggers.n_triggers] = sound_id_::NULL_SOUND;
			sound_triggers.source_ids[sound_triggers.n_triggers] = sound_trigger[i_entity].source_id;
			sound_triggers.positions[sound_triggers.n_triggers] = base[i_entity].position_fixed;
			sound_triggers.n_triggers += door[i_entity].state_trigger == component_::door_::state_::STATIC;
		}
	}
}
/*
========================================================================
========================================================================
*/
void systems_::sound_::item_triggers(

	void* parameters, __int32 i_thread
)
{
	parameters_::sound_::item_triggers_* func_parameters = (parameters_::sound_::item_triggers_*)parameters;
	archetype_data_& archetype_data = *func_parameters->archetype_data;
	command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_write];
	sound_triggers_& sound_triggers = command_buffer.sound_triggers;

	component_fetch_ fetch;
	fetch.n_components = 3;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::BASE;
	fetch.component_ids[1] = component_id_::SOUND_TRIGGER;
	fetch.component_ids[2] = component_id_::ITEM;

	Populate_Fetch_Table(archetype_data, fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
		component_::sound_trigger_* sound_trigger = (component_::sound_trigger_*)fetch.table[1][i_archetype_index];
		component_::item_* item = (component_::item_*)fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			sound_triggers.i_wavs[sound_triggers.n_triggers] = sound_id_::ITEM_PICKUP;
			sound_triggers.source_ids[sound_triggers.n_triggers] = sound_trigger[i_entity].source_id;
			sound_triggers.positions[sound_triggers.n_triggers] = item[i_entity].spawn_position;
			sound_triggers.n_triggers += item[i_entity].state_trigger == component_::item_::state_::PICKED_UP;

			sound_triggers.i_wavs[sound_triggers.n_triggers] = sound_id_::ITEM_SPAWN;
			sound_triggers.source_ids[sound_triggers.n_triggers] = sound_trigger[i_entity].source_id;
			sound_triggers.positions[sound_triggers.n_triggers] = item[i_entity].spawn_position;
			sound_triggers.n_triggers += item[i_entity].state_trigger == component_::item_::state_::SPAWNED;
		}
	}
}
/*
========================================================================
========================================================================
*/
void systems_::sound_::switch_triggers(

	void* parameters, __int32 i_thread
)
{
	parameters_::sound_::item_triggers_* func_parameters = (parameters_::sound_::item_triggers_*)parameters;
	archetype_data_& archetype_data = *func_parameters->archetype_data;
	command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_write];
	sound_triggers_& sound_triggers = command_buffer.sound_triggers;

	component_fetch_ fetch;
	fetch.n_components = 3;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::BASE;
	fetch.component_ids[1] = component_id_::SOUND_TRIGGER;
	fetch.component_ids[2] = component_id_::PLATE;

	Populate_Fetch_Table(archetype_data, fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
		component_::sound_trigger_* sound_trigger = (component_::sound_trigger_*)fetch.table[1][i_archetype_index];
		component_::plate_* plate = (component_::plate_*)fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			sound_triggers.i_wavs[sound_triggers.n_triggers] = sound_id_::ITEM_PICKUP;
			sound_triggers.source_ids[sound_triggers.n_triggers] = sound_trigger[i_entity].source_id;
			sound_triggers.positions[sound_triggers.n_triggers] = base[i_entity].position_fixed;
			sound_triggers.n_triggers += plate[i_entity].sound_trigger != 0;

		}
	}
}
/*
========================================================================
========================================================================
*/
void systems_::sound_::monster_triggers(

	void* parameters, __int32 i_thread
)
{
	parameters_::sound_::monster_triggers_* func_parameters = (parameters_::sound_::monster_triggers_*)parameters;
	archetype_data_& archetype_data = *func_parameters->archetype_data;
	command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_write];
	sound_triggers_& sound_triggers = command_buffer.sound_triggers;

	component_fetch_ fetch;
	fetch.n_components = 3;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::BASE;
	fetch.component_ids[1] = component_id_::BEHAVIOUR;
	fetch.component_ids[2] = component_id_::SOUND_TRIGGER;

	Populate_Fetch_Table(archetype_data, fetch);

	//__int32 sound_id[2][3];
	//sound_id[0][0] = sound_id_::SHAMBLER_DEATH;
	//sound_id[1][0] = sound_id_::SCRAG_DEATH;

	//sound_id[0][1] = sound_id_::SHAMBLER_IDLE;
	//sound_id[1][1] = sound_id_::SCRAG_IDLE;

	//sound_id[0][2] = sound_id_::SHAMBLER_CHARGE;
	//sound_id[1][2] = sound_id_::SCRAG_CHARGE;

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
		component_::behaviour_* behaviour = (component_::behaviour_*)fetch.table[1][i_archetype_index];
		component_::sound_trigger_* sound_trigger = (component_::sound_trigger_*)fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			// SUCH A HACK!!!
			//const __int32 n_shamblers = 6;
			//const __int32 i_monster_type = (i_entity < n_shamblers) ? 0 : 1;

			sound_triggers.i_wavs[sound_triggers.n_triggers] = sound_id_::SHAMBLER_DEATH;
			sound_triggers.source_ids[sound_triggers.n_triggers] = sound_trigger[i_entity].source_id;
			sound_triggers.positions[sound_triggers.n_triggers] = base[i_entity].position_fixed;
			sound_triggers.n_triggers += behaviour[i_entity].state_trigger == component_::behaviour_::id_::DEATH;

			sound_triggers.i_wavs[sound_triggers.n_triggers] = sound_id_::SHAMBLER_IDLE;
			sound_triggers.source_ids[sound_triggers.n_triggers] = sound_trigger[i_entity].source_id;
			sound_triggers.positions[sound_triggers.n_triggers] = base[i_entity].position_fixed;
			sound_triggers.n_triggers += behaviour[i_entity].state_trigger == component_::behaviour_::id_::STAND;

			sound_triggers.i_wavs[sound_triggers.n_triggers] = sound_id_::SHAMBLER_CHARGE;
			sound_triggers.source_ids[sound_triggers.n_triggers] = sound_trigger[i_entity].source_id;
			sound_triggers.positions[sound_triggers.n_triggers] = base[i_entity].position_fixed;
			sound_triggers.n_triggers += behaviour[i_entity].state_trigger == component_::behaviour_::id_::ATTACK;
		}
	}
}