
#pragma once

#include "vector.h"

//======================================================================

struct game_matrices_ {

	__m128 m_rotate_z[4];
	__m128 m_rotate_y[4];
};

//======================================================================

struct model_manager_;
struct user_input_;
struct timer_;
struct thread_pool_;
struct sound_triggers_;
struct entity_manager_;
struct component_data_;
struct component_dispatch_;
struct component_buffer_;
struct display_;
struct draw_call_;
struct collision_manager_;
struct behaviour_manager_;
struct animation_manager_;
struct way_point_manager_;
struct archetype_data_;
struct frame_jobs_;
struct command_buffer_;
struct vertex_light_manager_;



void Bin_Vertex_Lights(

	const command_buffer_&,
	vertex_light_manager_&
);

void Render_STATIC_SMALL(

	const __int32,
	const __int32,
	const model_manager_&,
	command_buffer_&,
	display_&
);

void Render_Static_Model(

	const __int32,
	const __int32,
	const model_manager_&,
	command_buffer_&,
	display_&
);

void Render_Animated_Model(

	const __int32,
	const __int32,
	const model_manager_&,
	command_buffer_&,
	display_&
);

void Render_Rotated_Model(

	const __int32,
	const __int32,
	const model_manager_&,
	command_buffer_&,
	display_&
);

void Render_Draw_Call_PRECALL(void*, __int32);

