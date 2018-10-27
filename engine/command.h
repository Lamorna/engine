#pragma once

#include "master.h"
#include "render_front.h"
#include "sound.h"

struct grid_;
struct timer_;

struct fog_effect_ {

	float timer;
};

struct command_buffer_ {

	draw_call_ draw_calls[draw_call_::id_::COUNT];

	struct light_ {

		enum {

			MAX_LIGHTS = 256,
		};

		int3_ position;
		float3_ colour;
		float intensity;
	};

	__int32 n_vertex_lights;
	__int32 n_lightmap_lights;
	light_ vertex_lights[light_::MAX_LIGHTS];
	light_ lightmap_lights[light_::MAX_LIGHTS];
	int3_ position_camera;
	matrix_ m_clip_space;
	vertex_light_manager_ vertex_light_manager;
	sound_triggers_ sound_triggers;

	float fog_effect_timer;
	const model_* model;

	__int32 ui_ammo_counter;
	__int32 ui_ammo_type;
};

struct command_buffer_handler_ {

	__int32 i_read;
	__int32 i_write;
	command_buffer_ command_buffers[2];
	fog_effect_ fog_effect;
};

struct systems_::command_ {

	static void write_camera_matrix(void*, __int32);
	static void write_colour_space(void*, __int32);
	static void write_colour(void*, __int32);
	static void write_texture_space(void*, __int32);
	static void write_blend_interval(void*, __int32);
	static void write_animated(void*, __int32);
	static void write_model_space(void*, __int32);
	static void write_static_model(void*, __int32);
	static void write_small_model(void*, __int32);
	static void write_light_sources(void*, __int32);
	static void bin_vertex_lights(void*, __int32);
	static void command_buffer_swap(void*, __int32);
};

struct parameters_::command_ {

	struct write_camera_matrix_ {

		archetype_data_* archetype_data;
		command_buffer_handler_* command_buffer_handler;
		timer_* timer;
	};
	struct write_colour_space_ {

		archetype_data_* archetype_data;
		command_buffer_handler_* command_buffer_handler;
	};
	struct write_colour_ {

		archetype_data_* archetype_data;
		command_buffer_handler_* command_buffer_handler;
	};
	struct write_texture_space_ {

		archetype_data_* archetype_data;
		command_buffer_handler_* command_buffer_handler;
	};
	struct write_blend_interval_ {

		archetype_data_* archetype_data;
		command_buffer_handler_* command_buffer_handler;
	};
	struct write_animated_ {

		archetype_data_* archetype_data;
		command_buffer_handler_* command_buffer_handler;
	};
	struct write_model_space_ {

		archetype_data_* archetype_data;
		command_buffer_handler_* command_buffer_handler;
	};
	struct write_static_model_ {

		archetype_data_* archetype_data;
		command_buffer_handler_* command_buffer_handler;
	};
	struct write_small_model_ {

		archetype_data_* archetype_data;
		command_buffer_handler_* command_buffer_handler;
	};
	struct write_light_sources_ {

		archetype_data_* archetype_data;
		command_buffer_handler_* command_buffer_handler;
	};
	struct bin_vertex_lights_ {

		command_buffer_handler_* command_buffer_handler;
	};
	struct command_buffer_swap_ {

		command_buffer_handler_* command_buffer_handler;
	};

	write_camera_matrix_ write_camera_matrix;
	write_colour_space_ write_colour_space;
	write_colour_ write_colour;
	write_texture_space_ write_texture_space;
	write_blend_interval_ write_blend_interval;
	write_animated_ write_animated;
	write_model_space_ write_model_space;
	write_static_model_ write_static_model;
	write_small_model_ write_small_model;
	write_light_sources_ write_light_sources;
	bin_vertex_lights_ bin_vertex_lights;
	command_buffer_swap_ command_buffer_swap;
};
