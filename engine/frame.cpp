
#include "frame.h"
#include "component.h"
#include "sound.h"
#include "render_front.h"
#include "light_maps.h"
#include "collide.h"
#include "player.h"
#include "entity.h"
#include "items.h"
#include "command.h"
#include "environment.h"
#include "input.h"
#include "update.h"
#include "monster.h"
#include "setup.h"

parameters_::entity_ entity;
parameters_::camera_ camera;
parameters_::player_ player;
parameters_::environment_ environment;
parameters_::monster_ monster;
parameters_::collide_ collide;
parameters_::sound_ sound;
parameters_::item_ item;
parameters_::command_ command;
parameters_::render_ render;
parameters_::lightmap_ lightmap;

/*
==================
==================
*/
void Load_Frame_Jobs(

	model_token_manager_& model_token_manager,
	model_manager_& model_manager,
	user_input_& user_input,
	timer_& timer,
	sound_system_& sound_system,
	display_& display,
	game_matrices_& game_matrices,
	behaviour_manager_& behaviour_manager,
	animation_manager_& animation_manager,
	way_point_manager_& way_point_manager,
	command_buffer_handler_& command_buffer_handler,
	systems_::collision_response_& collision_response,
	particle_manager_& particle_manager,
	collision_manager_& collision_manager,
	lightmap_manager_& lightmap_manager,
	component_data_& component_data,
	frame_jobs_& frame_jobs,
	thread_pool_& thread_pool,
	user_interface_& user_interface,
	grid_& grid

) {



	// -------------------------------------------------------------------------------

	job_* jobs = frame_jobs.jobs;
	__int32& n_jobs = frame_jobs.n_jobs;
	n_jobs = 0;

	// -------------------------------------------------------------------------------
	struct job_group_ {

		struct id_ {

			enum {

				GAME_LOGIC_PRE_COLLIDE,
				COLLISION_DETECTION,
				COLLISION_RESPONSE,
				GAME_LOGIC_POST_COLLIDE,
				SOUND,
				COMMAND_BUFFER_POPULATE,
				RENDER_FRONT_END,
				LIGHTMAP_BVH,
				LIGHTMAP_GENERATION,
				LIGHTMAP_BUFFER_SWAP,
				LOCK_BACK_BUFFER,
				RENDER_BACK_END,
				RENDER_UI,
				RELEASE_BACK_BUFFER,
				COMMAND_BUFFER_SWAP,



				COUNT
			};
		};

		bool is_concurrent;
		__int32 i_first;
		__int32 n_jobs;
		__int32 n_permisions;
		__int32 i_permissions[job_::MAX_PERMISSIONs];
	};

	// -------------------------------------------------------------------------------
	//
	//		GAME LOGIC PRE COLLIDE
	//
	// -------------------------------------------------------------------------------

	component_fetch_ component_fetch;
	component_fetch.n_components = 3;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::BASE;
	component_fetch.component_ids[1] = component_id_::MOVE;
	component_fetch.component_ids[2] = component_id_::COLLIDER;

	Populate_Fetch_Table(component_data.archetype_data, component_fetch);

	const __int32 n_colliders_per_batch = 16;
	__int32 n_collider_jobs = component_fetch.n_entities / n_colliders_per_batch;
	n_collider_jobs += (component_fetch.n_entities % n_colliders_per_batch) > 0;

	// -------------------------------------------------------------------------------
	player.model_select.user_input = &user_input;
	player.model_select.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::player_::model_select;
	jobs[n_jobs].parameters = (void*)&player.model_select;
	n_jobs++;
	// -------------------------------------------------------------------------------
	player.impart_velocity.mouse = &user_input.mouse;
	player.impart_velocity.user_input = &user_input;
	player.impart_velocity.timer = &timer;
	player.impart_velocity.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::player_::impart_velocity;
	jobs[n_jobs].parameters = (void*)&player.impart_velocity;
	n_jobs++;
	// -------------------------------------------------------------------------------
	camera.camera_to_model.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::camera_::camera_to_model;
	jobs[n_jobs].parameters = (void*)&camera.camera_to_model;
	n_jobs++;
	// -------------------------------------------------------------------------------
	player.fire_projectile.user_input = &user_input;
	player.fire_projectile.timer = &timer;
	player.fire_projectile.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::player_::fire_projectile;
	jobs[n_jobs].parameters = (void*)&player.fire_projectile;
	n_jobs++;
	// -------------------------------------------------------------------------------
	environment.update_clouds.m_rotate = &game_matrices.m_rotate_y;
	environment.update_clouds.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::environment_::update_clouds;
	jobs[n_jobs].parameters = (void*)&environment.update_clouds;
	n_jobs++;
	// -------------------------------------------------------------------------------
	environment.update_lava.m_rotate = &game_matrices.m_rotate_y;
	environment.update_lava.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::environment_::update_lava;
	jobs[n_jobs].parameters = (void*)&environment.update_lava;
	n_jobs++;
	// -------------------------------------------------------------------------------
	monster.check_agro_radius.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::monster_::check_agro_radius;
	jobs[n_jobs].parameters = (void*)&monster.check_agro_radius;
	n_jobs++;
	// -------------------------------------------------------------------------------
	monster.update_patrol_points.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::monster_::update_patrol_points;
	jobs[n_jobs].parameters = (void*)&monster.update_patrol_points;
	n_jobs++;
	// -------------------------------------------------------------------------------
	monster.face_target.timer = &timer;
	monster.face_target.behaviour_manager = &behaviour_manager;
	monster.face_target.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::monster_::face_target;
	jobs[n_jobs].parameters = (void*)&monster.face_target;
	n_jobs++;
	// -------------------------------------------------------------------------------
	monster.update_velocity.timer = &timer;
	monster.update_velocity.behaviour_manager = &behaviour_manager;
	monster.update_velocity.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::monster_::update_velocity;
	jobs[n_jobs].parameters = (void*)&monster.update_velocity;
	n_jobs++;
	// -------------------------------------------------------------------------------
	monster.process_effects.timer = &timer;
	monster.process_effects.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::monster_::process_effects;
	jobs[n_jobs].parameters = (void*)&monster.process_effects;
	n_jobs++;
	// -------------------------------------------------------------------------------
	monster.process_health.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::monster_::process_health;
	jobs[n_jobs].parameters = (void*)&monster.process_health;
	n_jobs++;
	// -------------------------------------------------------------------------------
	monster.fire_projectile.archetype_data = &component_data.archetype_data;
	monster.fire_projectile.model_manager = &model_manager;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::monster_::fire_projectile;
	jobs[n_jobs].parameters = (void*)&monster.fire_projectile;
	n_jobs++;
	// -------------------------------------------------------------------------------
	monster.update_fly.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::monster_::update_fly;
	jobs[n_jobs].parameters = (void*)&monster.update_fly;
	n_jobs++;
	// -------------------------------------------------------------------------------
	environment.update_doors.timer = &timer;
	environment.update_doors.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::environment_::update_doors;
	jobs[n_jobs].parameters = (void*)&environment.update_doors;
	n_jobs++;
	// -------------------------------------------------------------------------------
	environment.update_platforms.timer = &timer;
	environment.update_platforms.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::environment_::update_platforms;
	jobs[n_jobs].parameters = (void*)&environment.update_platforms;
	n_jobs++;
	// -------------------------------------------------------------------------------
	environment.update_switches.timer = &timer;
	environment.update_switches.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::environment_::update_switches;
	jobs[n_jobs].parameters = (void*)&environment.update_switches;
	n_jobs++;
	// -------------------------------------------------------------------------------
	environment.update_buttons.timer = &timer;
	environment.update_buttons.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::environment_::update_buttons;
	jobs[n_jobs].parameters = (void*)&environment.update_buttons;
	n_jobs++;
	// -------------------------------------------------------------------------------
	entity.apply_gravity.timer = &timer;
	entity.apply_gravity.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::entity_::apply_gravity;
	jobs[n_jobs].parameters = (void*)&entity.apply_gravity;
	n_jobs++;
	// -------------------------------------------------------------------------------
	entity.update_displacement_main.timer = &timer;
	entity.update_displacement_main.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::entity_::update_displacement_main;
	jobs[n_jobs].parameters = (void*)&entity.update_displacement_main;
	n_jobs++;
	// -------------------------------------------------------------------------------
	entity.update_displacement_fly.timer = &timer;
	entity.update_displacement_fly.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::entity_::update_displacement_fly;
	jobs[n_jobs].parameters = (void*)&entity.update_displacement_fly;
	n_jobs++;
	// -------------------------------------------------------------------------------
	entity.update_global_matrices.timer = &timer;
	entity.update_global_matrices.game_matrices = &game_matrices;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
	jobs[n_jobs].function = systems_::entity_::update_global_matrices;
	jobs[n_jobs].parameters = (void*)&entity.update_global_matrices;
	n_jobs++;
	// -------------------------------------------------------------------------------
	//
	//		COLLISION DETECTION
	//
	// -------------------------------------------------------------------------------
	{
		collide.collision_BVH.archetype_data = &component_data.archetype_data;
		collide.collision_BVH.grid = &grid;

		jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
		jobs[n_jobs].function = systems_::collide_::collision_BVH;
		jobs[n_jobs].parameters = (void*)&collide.collision_BVH;
		n_jobs++;
	}
	{
		const __int32 i_start_job = n_jobs;
		__int32 i_collide_jobs = 0;
		for (__int32 i_colliders_batch = 0; i_colliders_batch < component_fetch.n_entities; i_colliders_batch += n_colliders_per_batch) {

			__int32 n_colliders = min(component_fetch.n_entities - i_colliders_batch, n_colliders_per_batch);

			collide.collision_detection[i_collide_jobs].i_begin = i_colliders_batch;
			collide.collision_detection[i_collide_jobs].n_colliders_per_job = n_colliders;
			collide.collision_detection[i_collide_jobs].archetype_data = &component_data.archetype_data;
			collide.collision_detection[i_collide_jobs].collision_manager = &collision_manager;
			collide.collision_detection[i_collide_jobs].grid = &grid;

			jobs[n_jobs].type = job_group_::id_::COLLISION_DETECTION;
			jobs[n_jobs].function = systems_::collide_::collision_detection;
			jobs[n_jobs].parameters = (void*)&collide.collision_detection[i_collide_jobs];

			i_collide_jobs++;
			n_jobs++;
		}
	}
	// -------------------------------------------------------------------------------
	//
	//		COLLISION RESPONSE
	//
	// -------------------------------------------------------------------------------
	entity.mark_entity_grounded.n_threads = thread_pool.n_threads;
	entity.mark_entity_grounded.collision_manager = &collision_manager;
	entity.mark_entity_grounded.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::COLLISION_RESPONSE;
	jobs[n_jobs].function = systems_::entity_::mark_entity_grounded;
	jobs[n_jobs].parameters = (void*)&entity.mark_entity_grounded;
	n_jobs++;
	// -------------------------------------------------------------------------------
	entity.process_response_table.n_threads = thread_pool.n_threads;
	entity.process_response_table.collision_manager = &collision_manager;
	entity.process_response_table.collision_response = &collision_response;
	entity.process_response_table.archetype_data = &component_data.archetype_data;
	entity.process_response_table.particle_manager = &particle_manager;

	jobs[n_jobs].type = job_group_::id_::COLLISION_RESPONSE;
	jobs[n_jobs].function = systems_::entity_::process_response_table;
	jobs[n_jobs].parameters = (void*)&entity.process_response_table;
	n_jobs++;
	// -------------------------------------------------------------------------------
	entity.emit_particles.n_threads = thread_pool.n_threads;
	entity.emit_particles.collision_manager = &collision_manager;
	entity.emit_particles.particle_manager = &particle_manager;
	entity.emit_particles.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::COLLISION_RESPONSE;
	jobs[n_jobs].function = systems_::entity_::emit_particles;
	jobs[n_jobs].parameters = (void*)&entity.emit_particles;
	n_jobs++;
	// -------------------------------------------------------------------------------
	//
	//		SOUND
	//
	// -------------------------------------------------------------------------------
	sound.play_sounds.timer = &timer;
	sound.play_sounds.archetype_data = &component_data.archetype_data;
	sound.play_sounds.command_buffer_handler = &command_buffer_handler;
	sound.play_sounds.wavs = sound_system.wavs;
	sound.play_sounds.direct_sound = &sound_system.direct_sound;
	sound.play_sounds.sound_mixer = &sound_system.sound_mixer;
	sound.play_sounds.sound_buffer = &sound_system.sound_buffer;

	jobs[n_jobs].type = job_group_::id_::SOUND;
	jobs[n_jobs].function = systems_::sound_::play_sounds;
	jobs[n_jobs].parameters = (void*)&sound.play_sounds;
	n_jobs++;
	// -------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------

	//
	//		GAME LOGIC POST COLLIDER
	//
	// -------------------------------------------------------------------------------
	entity.update_position.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_POST_COLLIDE;
	jobs[n_jobs].function = systems_::entity_::update_position;
	jobs[n_jobs].parameters = (void*)&entity.update_position;
	n_jobs++;
	// -------------------------------------------------------------------------------
	entity.out_of_bounds.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_POST_COLLIDE;
	jobs[n_jobs].function = systems_::entity_::out_of_bounds;
	jobs[n_jobs].parameters = (void*)&entity.out_of_bounds;
	n_jobs++;
	// -------------------------------------------------------------------------------
	entity.out_of_bounds_respawn.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_POST_COLLIDE;
	jobs[n_jobs].function = systems_::entity_::out_of_bounds_respawn;
	jobs[n_jobs].parameters = (void*)&entity.out_of_bounds_respawn;
	n_jobs++;
	// -------------------------------------------------------------------------------
	item.process_triggers.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_POST_COLLIDE;
	jobs[n_jobs].function = systems_::item_::process_triggers;
	jobs[n_jobs].parameters = (void*)&item.process_triggers;
	n_jobs++;
	// -------------------------------------------------------------------------------
	item.update_ammo.timer = &timer;
	item.update_ammo.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_POST_COLLIDE;
	jobs[n_jobs].function = systems_::item_::update_ammo;
	jobs[n_jobs].parameters = (void*)&item.update_ammo;
	n_jobs++;
	// -------------------------------------------------------------------------------
	monster.update_behaviour.timer = &timer;
	monster.update_behaviour.behaviour_manager = &behaviour_manager;
	monster.update_behaviour.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_POST_COLLIDE;
	jobs[n_jobs].function = systems_::monster_::update_behaviour;
	jobs[n_jobs].parameters = (void*)&monster.update_behaviour;
	n_jobs++;
	// -------------------------------------------------------------------------------
	player.update_animation_driver.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_POST_COLLIDE;
	jobs[n_jobs].function = systems_::player_::update_animation_driver;
	jobs[n_jobs].parameters = (void*)&player.update_animation_driver;
	n_jobs++;
	// -------------------------------------------------------------------------------
	entity.update_animation.timer = &timer;
	entity.update_animation.animation_manager = &animation_manager;
	entity.update_animation.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_POST_COLLIDE;
	jobs[n_jobs].function = systems_::entity_::update_animation;
	jobs[n_jobs].parameters = (void*)&entity.update_animation;
	n_jobs++;
	// -------------------------------------------------------------------------------
	entity.broadcast_switch_state.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_POST_COLLIDE;
	jobs[n_jobs].function = systems_::entity_::broadcast_switch_state;
	jobs[n_jobs].parameters = (void*)&entity.broadcast_switch_state;
	n_jobs++;
	// -------------------------------------------------------------------------------
	entity.update_power_effects.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_POST_COLLIDE;
	jobs[n_jobs].function = systems_::entity_::update_power_effects;
	jobs[n_jobs].parameters = (void*)&entity.update_power_effects;
	n_jobs++;
	// -------------------------------------------------------------------------------
	entity.update_model_space_transform.timer = &timer;
	entity.update_model_space_transform.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_POST_COLLIDE;
	jobs[n_jobs].function = systems_::entity_::update_model_space_transform;
	jobs[n_jobs].parameters = (void*)&entity.update_model_space_transform;
	n_jobs++;
	// -------------------------------------------------------------------------------
	entity.update_colour_space_transform.timer = &timer;
	entity.update_colour_space_transform.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_POST_COLLIDE;
	jobs[n_jobs].function = systems_::entity_::update_colour_space_transform;
	jobs[n_jobs].parameters = (void*)&entity.update_colour_space_transform;
	n_jobs++;
	// -------------------------------------------------------------------------------
	entity.update_texture_space_transform.timer = &timer;
	entity.update_texture_space_transform.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_POST_COLLIDE;
	jobs[n_jobs].function = systems_::entity_::update_texture_space_transform;
	jobs[n_jobs].parameters = (void*)&entity.update_texture_space_transform;
	n_jobs++;
	// -------------------------------------------------------------------------------
	camera.compute_camera_matrix.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_POST_COLLIDE;
	jobs[n_jobs].function = systems_::camera_::compute_camera_matrix;
	jobs[n_jobs].parameters = (void*)&camera.compute_camera_matrix;
	n_jobs++;
	// -------------------------------------------------------------------------------
	entity.bbox_vs_view_volume.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::GAME_LOGIC_POST_COLLIDE;
	jobs[n_jobs].function = systems_::entity_::bbox_vs_view_volume;
	jobs[n_jobs].parameters = (void*)&entity.bbox_vs_view_volume;
	n_jobs++;

	// -------------------------------------------------------------------------------
	//
	//		COMMAND BUFFER GENERATION
	//
	// -------------------------------------------------------------------------------
	command.write_camera_matrix.archetype_data = &component_data.archetype_data;
	command.write_camera_matrix.command_buffer_handler = &command_buffer_handler;
	command.write_camera_matrix.timer = &timer;

	jobs[n_jobs].type = job_group_::id_::COMMAND_BUFFER_POPULATE;
	jobs[n_jobs].function = systems_::command_::write_camera_matrix;
	jobs[n_jobs].parameters = (void*)&command.write_camera_matrix;
	n_jobs++;
	// -------------------------------------------------------------------------------
	command.write_colour_space.archetype_data = &component_data.archetype_data;
	command.write_colour_space.command_buffer_handler = &command_buffer_handler;

	jobs[n_jobs].type = job_group_::id_::COMMAND_BUFFER_POPULATE;
	jobs[n_jobs].function = systems_::command_::write_colour_space;
	jobs[n_jobs].parameters = (void*)&command.write_colour_space;
	n_jobs++;
	// -------------------------------------------------------------------------------
	command.write_colour.archetype_data = &component_data.archetype_data;
	command.write_colour.command_buffer_handler = &command_buffer_handler;

	jobs[n_jobs].type = job_group_::id_::COMMAND_BUFFER_POPULATE;
	jobs[n_jobs].function = systems_::command_::write_colour;
	jobs[n_jobs].parameters = (void*)&command.write_colour;
	n_jobs++;
	// -------------------------------------------------------------------------------
	command.write_texture_space.archetype_data = &component_data.archetype_data;
	command.write_texture_space.command_buffer_handler = &command_buffer_handler;

	jobs[n_jobs].type = job_group_::id_::COMMAND_BUFFER_POPULATE;
	jobs[n_jobs].function = systems_::command_::write_texture_space;
	jobs[n_jobs].parameters = (void*)&command.write_texture_space;
	n_jobs++;
	// -------------------------------------------------------------------------------
	command.write_blend_interval.archetype_data = &component_data.archetype_data;
	command.write_blend_interval.command_buffer_handler = &command_buffer_handler;

	jobs[n_jobs].type = job_group_::id_::COMMAND_BUFFER_POPULATE;
	jobs[n_jobs].function = systems_::command_::write_blend_interval;
	jobs[n_jobs].parameters = (void*)&command.write_blend_interval;
	n_jobs++;
	// -------------------------------------------------------------------------------
	command.write_animated.archetype_data = &component_data.archetype_data;
	command.write_animated.command_buffer_handler = &command_buffer_handler;

	jobs[n_jobs].type = job_group_::id_::COMMAND_BUFFER_POPULATE;
	jobs[n_jobs].function = systems_::command_::write_animated;
	jobs[n_jobs].parameters = (void*)&command.write_animated;
	n_jobs++;
	// -------------------------------------------------------------------------------
	command.write_model_space.archetype_data = &component_data.archetype_data;
	command.write_model_space.command_buffer_handler = &command_buffer_handler;

	jobs[n_jobs].type = job_group_::id_::COMMAND_BUFFER_POPULATE;
	jobs[n_jobs].function = systems_::command_::write_model_space;
	jobs[n_jobs].parameters = (void*)&command.write_model_space;
	n_jobs++;
	// -------------------------------------------------------------------------------
	command.write_static_model.archetype_data = &component_data.archetype_data;
	command.write_static_model.command_buffer_handler = &command_buffer_handler;

	jobs[n_jobs].type = job_group_::id_::COMMAND_BUFFER_POPULATE;
	jobs[n_jobs].function = systems_::command_::write_static_model;
	jobs[n_jobs].parameters = (void*)&command.write_static_model;
	n_jobs++;
	// -------------------------------------------------------------------------------
	command.write_small_model.archetype_data = &component_data.archetype_data;
	command.write_small_model.command_buffer_handler = &command_buffer_handler;

	jobs[n_jobs].type = job_group_::id_::COMMAND_BUFFER_POPULATE;
	jobs[n_jobs].function = systems_::command_::write_small_model;
	jobs[n_jobs].parameters = (void*)&command.write_small_model;
	n_jobs++;
	// -------------------------------------------------------------------------------
	command.write_light_sources.archetype_data = &component_data.archetype_data;
	command.write_light_sources.command_buffer_handler = &command_buffer_handler;

	jobs[n_jobs].type = job_group_::id_::COMMAND_BUFFER_POPULATE;
	jobs[n_jobs].function = systems_::command_::write_light_sources;
	jobs[n_jobs].parameters = (void*)&command.write_light_sources;
	n_jobs++;
	// -------------------------------------------------------------------------------
	command.bin_vertex_lights.command_buffer_handler = &command_buffer_handler;

	jobs[n_jobs].type = job_group_::id_::COMMAND_BUFFER_POPULATE;
	jobs[n_jobs].function = systems_::command_::bin_vertex_lights;
	jobs[n_jobs].parameters = (void*)&command.bin_vertex_lights;
	n_jobs++;
	// -------------------------------------------------------------------------------
	sound.process_event_table.collision_manager = &collision_manager;
	sound.process_event_table.command_buffer_handler = &command_buffer_handler;
	sound.process_event_table.sound_event_table = sound_system.sound_event_table;
	sound.process_event_table.n_threads = thread_pool.n_threads;

	jobs[n_jobs].type = job_group_::id_::COMMAND_BUFFER_POPULATE;
	jobs[n_jobs].function = systems_::sound_::process_event_table;
	jobs[n_jobs].parameters = (void*)&sound.process_event_table;
	n_jobs++;
	// -------------------------------------------------------------------------------
	sound.player_triggers.command_buffer_handler = &command_buffer_handler;
	sound.player_triggers.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::COMMAND_BUFFER_POPULATE;
	jobs[n_jobs].function = systems_::sound_::player_triggers;
	jobs[n_jobs].parameters = (void*)&sound.player_triggers;
	n_jobs++;
	// -------------------------------------------------------------------------------
	sound.door_triggers.command_buffer_handler = &command_buffer_handler;
	sound.door_triggers.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::COMMAND_BUFFER_POPULATE;
	jobs[n_jobs].function = systems_::sound_::door_triggers;
	jobs[n_jobs].parameters = (void*)&sound.door_triggers;
	n_jobs++;
	// -------------------------------------------------------------------------------
	sound.item_triggers.command_buffer_handler = &command_buffer_handler;
	sound.item_triggers.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::COMMAND_BUFFER_POPULATE;
	jobs[n_jobs].function = systems_::sound_::item_triggers;
	jobs[n_jobs].parameters = (void*)&sound.item_triggers;
	n_jobs++;
	// -------------------------------------------------------------------------------
	sound.switch_triggers.command_buffer_handler = &command_buffer_handler;
	sound.switch_triggers.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::COMMAND_BUFFER_POPULATE;
	jobs[n_jobs].function = systems_::sound_::switch_triggers;
	jobs[n_jobs].parameters = (void*)&sound.switch_triggers;
	n_jobs++;
	// -------------------------------------------------------------------------------
	sound.monster_triggers.command_buffer_handler = &command_buffer_handler;
	sound.monster_triggers.archetype_data = &component_data.archetype_data;

	jobs[n_jobs].type = job_group_::id_::COMMAND_BUFFER_POPULATE;
	jobs[n_jobs].function = systems_::sound_::monster_triggers;
	jobs[n_jobs].parameters = (void*)&sound.monster_triggers;
	n_jobs++;
	// -------------------------------------------------------------------------------
	command.command_buffer_swap.command_buffer_handler = &command_buffer_handler;

	jobs[n_jobs].type = job_group_::id_::COMMAND_BUFFER_SWAP;
	jobs[n_jobs].function = systems_::command_::command_buffer_swap;
	jobs[n_jobs].parameters = (void*)&command.command_buffer_swap;
	n_jobs++;
	// -------------------------------------------------------------------------------
	//
	//		FRONT END RENDERER
	//
	// -------------------------------------------------------------------------------
	for (__int32 i_draw_call = 0; i_draw_call < draw_call_::id_::COUNT; i_draw_call++) {

		render.process_draw_call[i_draw_call].i_draw_call = i_draw_call;
		render.process_draw_call[i_draw_call].model_manager = &model_manager;
		render.process_draw_call[i_draw_call].command_buffer_handler = &command_buffer_handler;
		render.process_draw_call[i_draw_call].display = &display;

		jobs[n_jobs].type = job_group_::id_::RENDER_FRONT_END;
		jobs[n_jobs].function = Render_Draw_Call_PRECALL;
		jobs[n_jobs].parameters = (void*)&render.process_draw_call[i_draw_call];
		n_jobs++;
	}
	// -------------------------------------------------------------------------------
	//
	//		LIGHTMAP BVH
	//
	// -------------------------------------------------------------------------------
	{
		lightmap.process_BVH.command_buffer_handler = &command_buffer_handler;
		lightmap.process_BVH.lightmap_manager = &lightmap_manager;
		lightmap.process_BVH.grid = &grid;

		jobs[n_jobs].type = job_group_::id_::LIGHTMAP_BVH;
		jobs[n_jobs].parameters = (void*)&lightmap.process_BVH;
		jobs[n_jobs].function = systems_::lightmap_::process_BVH;
		n_jobs++;
	}
	// -------------------------------------------------------------------------------
	//
	//		LIGHTMAP GENERATION
	//
	// -------------------------------------------------------------------------------
	{
		component_fetch_ fetch;
		fetch.n_components = 2;
		fetch.n_excludes = 0;
		fetch.component_ids[0] = component_id_::BASE;
		fetch.component_ids[1] = component_id_::MAP_COLLIDE;

		Populate_Fetch_Table(component_data.archetype_data, fetch);

		const __int32 n_per_batch = fetch.n_entities / thread_pool.n_threads;
		__int32 i_job_local = 0;
		//for (__int32 i_batch = 0; i_batch < fetch.n_entities; i_batch += n_per_batch) {
		for (__int32 i_batch = 0; i_batch < lightmap_manager_::bvh_::MAX_LIT_MODELS; i_batch++) {

			__int32 n_entities = min(fetch.n_entities - i_batch, n_per_batch);

			lightmap.process_lightmaps[i_job_local].i_lit_model_index = i_batch;
			lightmap.process_lightmaps[i_job_local].command_buffer_handler = &command_buffer_handler;
			lightmap.process_lightmaps[i_job_local].lightmap_manager = &lightmap_manager;
			lightmap.process_lightmaps[i_job_local].model_manager = &model_manager;
			lightmap.process_lightmaps[i_job_local].model_spotlight = &model_manager.model[model_::id_::SPOTLIGHT];

			jobs[n_jobs].type = job_group_::id_::LIGHTMAP_GENERATION;
			jobs[n_jobs].parameters = (void*)&lightmap.process_lightmaps[i_job_local];
			jobs[n_jobs].function = systems_::lightmap_::process_lightmaps;

			i_job_local++;
			n_jobs++;
		}
	}
	// -------------------------------------------------------------------------------
	//
	//		LIGHTMAP BUFFER SWAP
	//
	// -------------------------------------------------------------------------------
	{
		lightmap.fade_lightmaps.timer = &timer;
		lightmap.fade_lightmaps.lightmap_manager= &lightmap_manager;
		lightmap.fade_lightmaps.model_map = &model_manager.model[model_::id_::MAP];

		jobs[n_jobs].type = job_group_::id_::LIGHTMAP_BUFFER_SWAP;
		jobs[n_jobs].function = systems_::lightmap_::fade_lightmaps;
		jobs[n_jobs].parameters = (void*)&lightmap.fade_lightmaps;
		n_jobs++;
	}
	{
		lightmap.buffer_swap.n_threads = thread_pool.n_threads;
		lightmap.buffer_swap.lightmap_manager = &lightmap_manager;
		lightmap.buffer_swap.model_manager = &model_manager;

		jobs[n_jobs].type = job_group_::id_::LIGHTMAP_BUFFER_SWAP;
		jobs[n_jobs].function = systems_::lightmap_::buffer_swap;
		jobs[n_jobs].parameters = (void*)&lightmap.buffer_swap;
		n_jobs++;
	}
	// -------------------------------------------------------------------------------
	//
	//		LOCK BACK BUFFER
	//
	// -------------------------------------------------------------------------------
	{
		render.lock_back_buffer.display = &display;

		jobs[n_jobs].type = job_group_::id_::LOCK_BACK_BUFFER;
		jobs[n_jobs].function = systems_::render_::lock_back_buffer;
		jobs[n_jobs].parameters = (void*)&render.lock_back_buffer;
		n_jobs++;
	}
	// -------------------------------------------------------------------------------
	//
	//		RENDERER BACK END
	//
	// -------------------------------------------------------------------------------
	{
		__int32 i_job_local = 0;
		for (__int32 i_bin = 0; i_bin < parameters_::render_::process_screen_bins_::NUM_JOBS; i_bin++) {

			render.process_screen_bins[i_job_local].command_buffer_handler = &command_buffer_handler;
			render.process_screen_bins[i_job_local].model_manager = &model_manager;
			render.process_screen_bins[i_job_local].display = &display;
			render.process_screen_bins[i_job_local].i_bin = i_bin;
			render.process_screen_bins[i_job_local].n_threads = thread_pool.n_threads;

			jobs[n_jobs].type = job_group_::id_::RENDER_BACK_END;;
			jobs[n_jobs].parameters = (void*)&render.process_screen_bins[i_job_local];
			jobs[n_jobs].function = systems_::render_::process_screen_bins;

			i_job_local++;
			n_jobs++;
		}
	}
	// -------------------------------------------------------------------------------
	//
	//		RENDER UI
	//
	// -------------------------------------------------------------------------------
	{
		render.render_UI.display = &display;
		render.render_UI.user_interface = &user_interface;

		jobs[n_jobs].type = job_group_::id_::RENDER_UI;
		jobs[n_jobs].function = systems_::render_::render_UI;
		jobs[n_jobs].parameters = (void*)&render.render_UI;
		n_jobs++;
	}
	// -------------------------------------------------------------------------------
	//
	//		RELEASE BACK BUFFER
	//
	// -------------------------------------------------------------------------------
	{
		render.release_back_buffer.display = &display;

		jobs[n_jobs].type = job_group_::id_::RELEASE_BACK_BUFFER;
		jobs[n_jobs].function = systems_::render_::release_back_buffer;
		jobs[n_jobs].parameters = (void*)&render.release_back_buffer;
		n_jobs++;
	}



	// -------------------------------------------------------------------------------
	//
	//		PROCESS FRAME JOBS
	//
	// -------------------------------------------------------------------------------

	job_group_ job_groups[job_group_::id_::COUNT];

	{
		const __int32 i_job_group = job_group_::id_::GAME_LOGIC_PRE_COLLIDE;
		job_groups[i_job_group].is_concurrent = false;
		job_groups[i_job_group].n_permisions = 1;
		job_groups[i_job_group].i_permissions[0] = job_group_::id_::COLLISION_DETECTION;
	}
	{
		const __int32 i_job_group = job_group_::id_::COLLISION_DETECTION;
		job_groups[i_job_group].is_concurrent = true;
		job_groups[i_job_group].n_permisions = 1;
		job_groups[i_job_group].i_permissions[0] = job_group_::id_::COLLISION_RESPONSE;
	}
	{
		const __int32 i_job_group = job_group_::id_::COLLISION_RESPONSE;
		job_groups[i_job_group].is_concurrent = false;
		job_groups[i_job_group].n_permisions = 1;
		job_groups[i_job_group].i_permissions[0] = job_group_::id_::GAME_LOGIC_POST_COLLIDE;
	}
	{
		const __int32 i_job_group = job_group_::id_::GAME_LOGIC_POST_COLLIDE;
		job_groups[i_job_group].is_concurrent = false;
		job_groups[i_job_group].n_permisions = 1;
		job_groups[i_job_group].i_permissions[0] = job_group_::id_::COMMAND_BUFFER_POPULATE;
	}
	{
		const __int32 i_job_group = job_group_::id_::COMMAND_BUFFER_POPULATE;
		job_groups[i_job_group].is_concurrent = false;
		job_groups[i_job_group].n_permisions = 1;
		job_groups[i_job_group].i_permissions[0] = job_group_::id_::COMMAND_BUFFER_SWAP;
	}
	{
		const __int32 i_job_group = job_group_::id_::COMMAND_BUFFER_SWAP;
		job_groups[i_job_group].is_concurrent = false;
		job_groups[i_job_group].n_permisions = 0;
	}
	// -------------------------------------------------------------------------------
	{
		const __int32 i_job_group = job_group_::id_::SOUND;
		job_groups[i_job_group].is_concurrent = false;
		job_groups[i_job_group].n_permisions = 1;
		job_groups[i_job_group].i_permissions[0] = job_group_::id_::COMMAND_BUFFER_SWAP;
	}
	// -------------------------------------------------------------------------------
	{
		const __int32 i_job_group = job_group_::id_::RENDER_FRONT_END;
		job_groups[i_job_group].is_concurrent = true;
		job_groups[i_job_group].n_permisions = 1;
		job_groups[i_job_group].i_permissions[0] = job_group_::id_::LOCK_BACK_BUFFER;
	}
	{
		const __int32 i_job_group = job_group_::id_::LOCK_BACK_BUFFER;
		job_groups[i_job_group].is_concurrent = false;
		job_groups[i_job_group].n_permisions = 1;
		job_groups[i_job_group].i_permissions[0] = job_group_::id_::RENDER_BACK_END;
	}
	{
		const __int32 i_job_group = job_group_::id_::RENDER_BACK_END;
		job_groups[i_job_group].is_concurrent = true;
		job_groups[i_job_group].n_permisions = 1;
		job_groups[i_job_group].i_permissions[0] = job_group_::id_::RENDER_UI;
	}
	{
		const __int32 i_job_group = job_group_::id_::RENDER_UI;
		job_groups[i_job_group].is_concurrent = false;
		job_groups[i_job_group].n_permisions = 1;
		job_groups[i_job_group].i_permissions[0] = job_group_::id_::RELEASE_BACK_BUFFER;
	}
	{
		const __int32 i_job_group = job_group_::id_::RELEASE_BACK_BUFFER;
		job_groups[i_job_group].is_concurrent = false;
		job_groups[i_job_group].n_permisions = 2;
		job_groups[i_job_group].i_permissions[0] = job_group_::id_::LIGHTMAP_BUFFER_SWAP;
		job_groups[i_job_group].i_permissions[1] = job_group_::id_::COMMAND_BUFFER_SWAP;
	}
	// -------------------------------------------------------------------------------
	{
		const __int32 i_job_group = job_group_::id_::LIGHTMAP_BVH;
		job_groups[i_job_group].is_concurrent = false;
		job_groups[i_job_group].n_permisions = 1;
		job_groups[i_job_group].i_permissions[0] = job_group_::id_::LIGHTMAP_GENERATION;
	}
	{
		const __int32 i_job_group = job_group_::id_::LIGHTMAP_GENERATION;
		job_groups[i_job_group].is_concurrent = true;
		job_groups[i_job_group].n_permisions = 1;
		job_groups[i_job_group].i_permissions[0] = job_group_::id_::LIGHTMAP_BUFFER_SWAP;
	}
	{
		const __int32 i_job_group = job_group_::id_::LIGHTMAP_BUFFER_SWAP;
		job_groups[i_job_group].is_concurrent = false;
		job_groups[i_job_group].n_permisions = 0;
	}
	// -------------------------------------------------------------------------------
	{
		for (__int32 i_job = 0; i_job < n_jobs; i_job++) {

			jobs[i_job].id = i_job;
			jobs[i_job].n_permisions = 0;
		}
	}
	// -------------------------------------------------------------------------------
	{
		for (__int32 i_job_group = 0; i_job_group < job_group_::id_::COUNT; i_job_group++) {

			const __int32 group_id = i_job_group;
			job_groups[i_job_group].i_first = -1;
			job_groups[i_job_group].n_jobs = 0;

			for (__int32 i_job = 0; i_job < n_jobs; i_job++) {

				bool is_group = jobs[i_job].type == group_id;
				bool is_first = (job_groups[i_job_group].i_first == -1) && is_group;
				job_groups[i_job_group].i_first = (is_first ? i_job : job_groups[i_job_group].i_first);
				job_groups[i_job_group].n_jobs += is_group;
			}

			//printf_s("i first: %i | n_jobs: %i \n", job_groups[i_job_group].i_first, job_groups[i_job_group].n_jobs);
		}
	}
	// -------------------------------------------------------------------------------
	{
		for (__int32 i_job_group = 0; i_job_group < job_group_::id_::COUNT; i_job_group++) {

			const __int32 i_first = job_groups[i_job_group].i_first;
			const __int32 i_last = i_first + job_groups[i_job_group].n_jobs - 1;

			if (job_groups[i_job_group].is_concurrent) {

				for (__int32 i_job = i_first; i_job <= i_last; i_job++) {

					for (__int32 i_permission = 0; i_permission < job_groups[i_job_group].n_permisions; i_permission++) {

						const __int32 i_permit = job_groups[i_job_group].i_permissions[i_permission];
						jobs[i_job].i_permissions[jobs[i_job].n_permisions][0] = job_groups[i_permit].i_first;
						jobs[i_job].i_permissions[jobs[i_job].n_permisions][1] = (job_groups[i_permit].is_concurrent ? job_groups[i_permit].n_jobs : 1);
						jobs[i_job].n_permisions++;
					}
				}
			}
			else {

				for (__int32 i_job = i_first; i_job < i_last; i_job++) {

					jobs[i_job].n_permisions = 1;
					jobs[i_job].i_permissions[0][0] = i_job + 1;
					jobs[i_job].i_permissions[0][1] = 1;
				}

				for (__int32 i_permission = 0; i_permission < job_groups[i_job_group].n_permisions; i_permission++) {

					const __int32 i_permit = job_groups[i_job_group].i_permissions[i_permission];
					jobs[i_last].i_permissions[jobs[i_last].n_permisions][0] = job_groups[i_permit].i_first;
					jobs[i_last].i_permissions[jobs[i_last].n_permisions][1] = (job_groups[i_permit].is_concurrent ? job_groups[i_permit].n_jobs : 1);
					jobs[i_last].n_permisions++;
				}
			}
		}
	}
}