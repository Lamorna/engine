
#pragma once

#include "master.h"

//======================================================================

struct timer_;
struct behaviour_manager_;
struct archetype_data_;
struct model_manager_;

struct systems_::monster_ {

	static void check_agro_radius(void* parameters, __int32 i_thread);
	static void face_target(void* parameters, __int32 i_thread);
	static void update_velocity(void* parameters, __int32 i_thread);
	static void process_effects(void* parameters, __int32 i_thread);
	static void process_health (void* parameters, __int32 i_thread);
	static void fire_projectile(void* parameters, __int32 i_thread);
	static void update_fly(void* parameters, __int32 i_thread);
	static void update_patrol_points(void* parameters, __int32 i_thread);
	static void update_behaviour(void* parameters, __int32 i_thread);
};

struct parameters_::monster_ {

	struct check_agro_radius_ {

		const behaviour_manager_* behaviour_manager;
		archetype_data_* archetype_data;
	};
	struct face_target_ {

		const timer_* timer;
		const behaviour_manager_* behaviour_manager;
		archetype_data_* archetype_data;
	};
	struct update_velocity_ {

		const timer_* timer;
		const behaviour_manager_* behaviour_manager;
		archetype_data_* archetype_data;
	};
	struct process_effects_ {

		const timer_* timer;
		archetype_data_* archetype_data;
	};
	struct process_health_ {

		archetype_data_* archetype_data;
	};
	struct fire_projectile_ {

		archetype_data_* archetype_data;
		model_manager_* model_manager;
	};
	struct update_fly_ {

		archetype_data_* archetype_data;
	};
	struct update_patrol_points_ {

		archetype_data_* archetype_data;
	};
	struct update_behaviour_ {

		const timer_* timer;
		behaviour_manager_* behaviour_manager;
		archetype_data_* archetype_data;
	};

	check_agro_radius_ check_agro_radius;
	face_target_ face_target;
	update_velocity_ update_velocity;
	process_effects_ process_effects;
	process_health_ process_health;
	fire_projectile_ fire_projectile;
	update_fly_ update_fly;
	update_patrol_points_ update_patrol_points;
	update_behaviour_ update_behaviour;

};

