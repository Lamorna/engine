

#include "entity.h"
#include "setup.h"
#include "light_maps.h"
#include "frame.h"
#include "input.h"
#include "update.h"
#include "obj_reader.h"
#include "quake.h"
#include "sound.h"
#include "windows.h"
#include "render_front.h"
#include "render_back.h"
#include "command.h"
#include "memory.h"




//======================================================================


struct game_data_ {


	user_interface_ user_interface;
	model_manager_	model_manager;
	lightmap_manager_ lightmap_manager;

	component_data_ component_data;
	animation_manager_ animation_manager;
	behaviour_manager_ behaviour_manager;
	way_point_manager_ way_point_manager;

	collision_manager_ collision_manager;

	model_token_manager_ model_token_manager;
	map_validate map_validate;

	display_ display;
	CACHE_ALIGN_PROPER visibility_data_ visibility_data[thread_pool_::MAX_WORKER_THREADS];
	game_matrices_ game_matrices;
	timer_ timer;
	user_input_ user_input;
	thread_pool_ thread_pool;
	sound_system_ sound_system;
	particle_manager_ particle_manager;

	grid_ grid;

	command_buffer_handler_ command_buffer_handler;
	frame_jobs_ frame_jobs;
	systems_::collision_response_ collision_response;
	memory_chunk_ memory_chunk;

};

game_data_ Game_Data;

//======================================================================


/*
==================
==================
*/
void Lamorna_Setup(

	game_data_& game_data,
	FILE *boot_log

)
{
	game_data.memory_chunk.chunk_ptr = game_data.memory_chunk.chunk;

	Get_System_Info(game_data.thread_pool);

	Initialise_Input(game_data.timer, game_data.user_input);

	Load_UI(game_data.user_interface, game_data.memory_chunk);

	Load_Map_Elements(game_data.model_token_manager);

	Load_External_Models(game_data.model_manager, game_data.memory_chunk);

	Hardcode_Cube_Template(

		game_data.model_manager.model[model_::id_::CUBE],
		game_data.model_manager.model[model_::id_::CUBE_BACKFACE],
		game_data.memory_chunk
	);

	Specialise_Cube_Template(game_data.model_manager, game_data.memory_chunk);

	Create_BVH(

		game_data.model_token_manager,
		game_data.lightmap_manager,
		game_data.model_manager,
		game_data.memory_chunk,
		game_data.grid
	);

	Load_Sounds(game_data.memory_chunk, game_data.sound_system.wavs);

	Initialise_Sound(game_data.sound_system.direct_sound, game_data.sound_system.sound_buffer, game_data.sound_system.sound_mixer);

	Initialise_Systems(

		game_data.sound_system.sound_event_table,
		game_data.animation_manager,
		game_data.behaviour_manager,
		game_data.collision_manager,
		game_data.way_point_manager,
		game_data.lightmap_manager,
		game_data.model_manager,
		game_data.component_data,
		game_data.model_token_manager,
		game_data.timer,
		game_data.particle_manager,
		game_data.command_buffer_handler,
		game_data.collision_response,
		game_data.grid,
		game_data.memory_chunk
	);

	Load_Frame_Jobs(

		game_data.model_token_manager,
		game_data.model_manager,
		game_data.user_input,
		game_data.timer,
		game_data.sound_system,
		game_data.display,
		game_data.game_matrices,
		game_data.behaviour_manager,
		game_data.animation_manager,
		game_data.way_point_manager,
		game_data.command_buffer_handler,
		game_data.collision_response,
		game_data.particle_manager,
		game_data.collision_manager,
		game_data.lightmap_manager,
		game_data.visibility_data,
		game_data.component_data,
		game_data.frame_jobs,
		game_data.thread_pool,
		game_data.user_interface,
		game_data.grid

	);

	{
		for (__int32 i_thread = 0; i_thread < thread_pool_::MAX_WORKER_THREADS; i_thread++) {
			Clear_Tile_Buffer(i_thread, game_data.display);
		}
	}

	const __int64 num_bytes_allocated = (char*)game_data.memory_chunk.chunk_ptr - (char*)game_data.memory_chunk.chunk;
	const __int64 num_bytes_unallocated = memory_chunk_::NUM_BYTES - num_bytes_allocated;
	printf_s("MEMORY REMAINING: %i \n", (__int32)num_bytes_unallocated);

	Thread_Pool_Initialise(game_data.thread_pool);

	printf_s("\n...WELCOME to Lamorna Engine! \n");
}

/*
==================
==================
*/
void Lamorna_Run(

	game_data_& game_data

) {

	const __int32 i_write = game_data.command_buffer_handler.i_write;

	for (__int32 i_draw_call = 0; i_draw_call < draw_call_::id_::COUNT; i_draw_call++) {
		game_data.command_buffer_handler.command_buffers[i_write].draw_calls[i_draw_call].n_models = 0;
	}

	Thread_Pool_Submit_Jobs(

		game_data.frame_jobs.n_jobs,
		game_data.frame_jobs.jobs,
		game_data.thread_pool
	);

}

/*
==================
==================
*/
void Lamorna_Shutdown(

	game_data_& game_data

) {

	Shutdown_Sound(game_data.sound_system.direct_sound);
	Thread_Pool_Shutdown(game_data.thread_pool);
	Windows_Shutdown(game_data.display);
}

/*
==================
==================
*/
__int32 __cdecl main()
{
	FILE* file_log;
	fopen_s(&file_log, "boot_log.txt", "w+");

	Initialise_Window(file_log, Game_Data.display);
	Lamorna_Setup(Game_Data, file_log);

	fclose(file_log);

	bool is_running = true;

	float fps_timer = 0.0f;
	__int32 n_frames = 0;
	Game_Data.user_interface.frame_rate = 0;

	//----------------------------------
	while (is_running) {

		__int32 n_steps = Update_Timer(Game_Data.timer);

		fps_timer += Game_Data.timer.real_delta_time;

		bool is_one_second = fps_timer >= 1.0f;
		fps_timer -= is_one_second ? 1.0f : 0.0f;
		Game_Data.user_interface.frame_rate = is_one_second ? n_frames : Game_Data.user_interface.frame_rate;
		n_frames = is_one_second ? 0 : n_frames;

		for (__int32 i_step = 0; i_step < n_steps; i_step++) {

			//Sleep(20);

			is_running = Windows_Messages(Game_Data.user_input);

			Lamorna_Run(Game_Data);

			Game_Data.timer.accumulator -= Game_Data.timer.fixed_step;
			Game_Data.timer.frame_count++;
			n_frames++;

		}
	}

	Lamorna_Shutdown(Game_Data);

	return 0;
}






