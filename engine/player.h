
#pragma once

#include "master.h"

//======================================================================

struct mouse_;
struct component_data_;
struct component_buffer_;
struct player;
struct timer_;
struct user_input_;
struct archetype_data_;

struct systems_::player_ {


	static void model_select(void*, __int32);
	static void impart_velocity(void*, __int32);
	static void fire_projectile(void*, __int32);
	static void update_animation_driver(void*, __int32);
};

struct parameters_::player_ {

	struct model_select_ {

		const user_input_* user_input;
		archetype_data_* archetype_data;
	};
	struct impart_velocity_ {

		const mouse_* mouse;
		const user_input_* user_input;
		const timer_* timer;
		archetype_data_* archetype_data;
	};
	struct fire_projectile_ {

		const user_input_* user_input;
		const timer_* timer;
		archetype_data_* archetype_data;
	};
	struct update_animation_driver_ {

		archetype_data_* archetype_data;
	};

	model_select_ model_select;
	impart_velocity_ impart_velocity;
	fire_projectile_ fire_projectile;
	update_animation_driver_ update_animation_driver;
};

struct systems_::camera_ {

	static void camera_to_model(void*, __int32);
	static void compute_camera_matrix(void*, __int32);
};

struct parameters_::camera_ {

	struct camera_to_model_ {

		archetype_data_* archetype_data;
	};
	struct compute_camera_matrix_ {

		archetype_data_* archetype_data;
	};

	camera_to_model_ camera_to_model;
	compute_camera_matrix_ compute_camera_matrix;
};





