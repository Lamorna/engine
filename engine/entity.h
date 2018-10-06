#pragma once

#include "master.h"
#include "collide.h"

struct systems_;
struct timer_;
struct archetype_data_;
struct game_matrices_;
struct particle_manager_;
struct animation_manager_;
struct collision_output_;
struct collision_manager_;

struct systems_::entity_ {

	static void update_displacement_main(void*, __int32);
	static void apply_gravity(void*, __int32);
	static void update_displacement_fly(void*, __int32);
	static void mark_entity_grounded(void*, __int32);
	static void process_response_table(void*, __int32);
	static void emit_particles(void*, __int32);
	static void update_global_matrices(void*, __int32);
	static void update_position(void*, __int32);
	static void out_of_bounds(void*, __int32);
	static void out_of_bounds_respawn(void*, __int32);
	static void update_animation(void*, __int32);
	static void update_power_effects(void*, __int32);
	static void update_model_space_transform(void*, __int32);
	static void update_colour_space_transform(void*, __int32);
	static void update_texture_space_transform(void*, __int32);
	static void bbox_vs_view_volume(void*, __int32);
	static void broadcast_switch_state(void*, __int32);
};

struct systems_::collision_response_ {

	static void entity_vs_null(const collision_output_&, archetype_data_&, particle_manager_&);
	static void apply_displacement(const collision_output_&, archetype_data_&, particle_manager_&);
	static void projectile_vs_player(const collision_output_&, archetype_data_&, particle_manager_&);
	static void projectile_vs_monster(const collision_output_&, archetype_data_&, particle_manager_&);
	static void projectile_vs_mob(const collision_output_&, archetype_data_&, particle_manager_&);
	static void entity_vs_bounce_pad(const collision_output_&, archetype_data_&, particle_manager_&);
	static void entity_vs_platform(const collision_output_&, archetype_data_&, particle_manager_&);
	static void entity_vs_teleporter(const collision_output_&, archetype_data_&, particle_manager_&);
	static void entity_vs_trapdoor(const collision_output_&, archetype_data_&, particle_manager_&);
	static void entity_vs_switch(const collision_output_&, archetype_data_&, particle_manager_&);
	static void entity_vs_button(const collision_output_&, archetype_data_&, particle_manager_&);
	static void particle_vs_entity(const collision_output_&, archetype_data_&, particle_manager_&);

	enum {
		MAX_REACTIONS = 4,
	};

	__int32 n_entries[colliding_type_::COUNT][colliding_type_::COUNT];

	void(*Entity_Reaction[colliding_type_::COUNT][colliding_type_::COUNT][MAX_REACTIONS])(

		const collision_output_&,
		archetype_data_&,
		particle_manager_&

		);
};

struct parameters_::entity_ {

	struct update_displacement_main_ {

		const timer_* timer;
		archetype_data_* archetype_data;
	};
	struct apply_gravity_ {

		const timer_* timer;
		archetype_data_* archetype_data;
	};
	struct update_displacement_fly_ {

		const timer_* timer;
		archetype_data_* archetype_data;
	};
	struct mark_entity_grounded_ {

		__int32 n_threads;
		collision_manager_* collision_manager;
		archetype_data_* archetype_data;
	};
	struct process_response_table_ {

		__int32 n_threads;
		const collision_manager_* collision_manager;
		const systems_::collision_response_* collision_response;
		archetype_data_* archetype_data;
		particle_manager_* particle_manager;
	};
	struct emit_particles_ {

		__int32 n_threads;
		const collision_manager_* collision_manager;
		particle_manager_* particle_manager;
		archetype_data_* archetype_data;
	};
	struct update_global_matrices_ {

		const timer_* timer;
		game_matrices_* game_matrices;
	};
	struct update_position_ {

		archetype_data_* archetype_data;

	};
	struct out_of_bounds_ {

		archetype_data_* archetype_data;

	};
	struct out_of_bounds_respawn_ {

		archetype_data_* archetype_data;

	};
	struct update_animation_ {

		const timer_* timer;
		const animation_manager_* animation_manager;
		archetype_data_* archetype_data;
	};
	struct update_power_effects_ {

		archetype_data_* archetype_data;
	};
	struct update_model_space_transform_ {

		const timer_* timer;
		archetype_data_* archetype_data;
	};
	struct update_colour_space_transform_ {

		const timer_* timer;
		archetype_data_* archetype_data;
	};
	struct update_texture_space_transform_ {

		const timer_* timer;
		archetype_data_* archetype_data;
	};
	struct bbox_vs_view_volume_ {

		archetype_data_* archetype_data;
	};
	struct broadcast_switch_state_ {

		archetype_data_* archetype_data;
	};

	apply_gravity_ apply_gravity;
	update_displacement_main_ update_displacement_main;
	update_displacement_fly_ update_displacement_fly;
	mark_entity_grounded_ mark_entity_grounded;
	process_response_table_ process_response_table;
	emit_particles_ emit_particles;
	update_global_matrices_ update_global_matrices;
	update_position_ update_position;
	out_of_bounds_ out_of_bounds;
	out_of_bounds_respawn_ out_of_bounds_respawn;
	update_animation_ update_animation;
	update_power_effects_ update_power_effects;
	update_model_space_transform_ update_model_space_transform;
	update_colour_space_transform_ update_colour_space_transform;
	update_texture_space_transform_ update_texture_space_transform;
	bbox_vs_view_volume_ bbox_vs_view_volume;
	broadcast_switch_state_ broadcast_switch_state;
};