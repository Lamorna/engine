#pragma once

#include "master.h"
#include "threads.h"

struct frame_jobs_ {

	enum {

		MAX_JOB_BATCHES = 1024,
	};

	__int32 n_jobs;
	job_ jobs[MAX_JOB_BATCHES];

};

struct model_manager_;
struct model_token_manager_;
struct user_input_;
struct timer_;
struct sound_system_;
struct display_;
struct game_matrices_;
struct behaviour_manager_;
struct animation_manager_;
struct way_point_manager_;
struct command_buffer_handler_;
struct systems_::collision_response_;
struct particle_manager_;
struct collision_manager_;
struct lightmap_manager_;
struct component_data_;
struct frame_jobs_;
struct thread_pool_;
struct user_interface_;
struct grid_;

void Load_Frame_Jobs(

	model_token_manager_&,
	model_manager_&,
	user_input_&,
	timer_&,
	sound_system_&,
	display_&,
	game_matrices_&,
	behaviour_manager_&,
	animation_manager_&,
	way_point_manager_&,
	command_buffer_handler_&,
	systems_::collision_response_&,
	particle_manager_&,
	collision_manager_&,
	lightmap_manager_&,
	component_data_&,
	frame_jobs_&,
	thread_pool_&,
	user_interface_&,
	grid_&
);


