
#include "entity.h"
#include "vector.h"
#include "component.h"
#include "vector_math.h"
#include "input.h"
#include "setup.h"
#include "update.h"
#include "render_front.h"

static const float friction_ground = 10.0f;
static const float acceleration_gravity = -40.0f;


/*
==================
==================
*/
void systems_::collision_response_::entity_vs_null(

	const collision_output_& collision_output,
	archetype_data_& archetype_data,
	particle_manager_& particle_manager_
) {

	// empty
	//printf_s("UNDEFINED COLLISON REACTION \n");
}

/*
==================
==================
*/
void systems_::collision_response_::apply_displacement(

	const collision_output_& collision_output,
	archetype_data_& archetype_data,
	particle_manager_& particle_manager
)
{

	float3_ collidee_displacement;
	{
		__int32 i_archetype = collision_output.collidee.i_archetype;
		__int32 i_entity = collision_output.collidee.i_entity;
		__int32 component_id = component_id_::MOVE;
		component_::move_* move = (component_::move_*)Find_Archetype_Component(i_archetype, component_id, archetype_data);
		collidee_displacement = move[i_entity].displacement;
	}

	__int32 i_archetype = collision_output.collider.i_archetype;
	__int32 i_entity = collision_output.collider.i_entity;
	__int32 component_id = component_id_::MOVE;
	component_::move_* move = (component_::move_*)Find_Archetype_Component(i_archetype, component_id, archetype_data);
	move[i_entity].displacement += collidee_displacement;
}

/*
==================
==================
*/
void systems_::collision_response_::particle_vs_entity(

	const collision_output_& collision_output,
	archetype_data_& archetype_data,
	particle_manager_& particle_manager

) {

	__int32 i_archetype = collision_output.collider.i_archetype;
	__int32 i_entity = collision_output.collider.i_entity;
	__int32 component_id = component_id_::MOVE;
	component_::move_* move = (component_::move_*)Find_Archetype_Component(i_archetype, component_id, archetype_data);
	bool is_top_contact = collision_output.collidee.normal.y > 0;

	move[i_entity].velocity.y -= is_top_contact ? 40.0f : 0.0f;
}

/*
==================
==================
*/
void systems_::collision_response_::projectile_vs_player(

	const collision_output_& collision_output,
	archetype_data_& archetype_data,
	particle_manager_& particle_manager

) {

	__int32 i_archetype = collision_output.collidee.i_archetype;
	__int32 i_entity = collision_output.collidee.i_entity;
	{
		component_::move_* move = (component_::move_*)Find_Archetype_Component(i_archetype, component_id_::MOVE, archetype_data);
		component_::mass_* mass = (component_::mass_*)Find_Archetype_Component(i_archetype, component_id_::MASS, archetype_data);
		//move[i_entity].velocity += collision_output.collider.velocity * (20.0f / mass[i_entity].value);
		move[i_entity].velocity += collision_output.collider.velocity;
	}
	{
		component_::animation_driver_* animation_driver = (component_::animation_driver_*)Find_Archetype_Component(i_archetype, component_id_::ANIMATION_DRIVER, archetype_data);
		animation_driver[i_entity].trigger_id = animation_data_::id_::PAIN;
	}
}

/*
==================
==================
*/
void systems_::collision_response_::projectile_vs_mob(

	const collision_output_& collision_output,
	archetype_data_& archetype_data,
	particle_manager_& particle_manager

) {

	__int32 type_id;
	{
		__int32 i_archetype = collision_output.collider.i_archetype;
		__int32 i_entity = collision_output.collider.i_entity;
		component_::projectile_id_* projectile_id = (component_::projectile_id_*)Find_Archetype_Component(i_archetype, component_id_::PROJECTILE_ID, archetype_data);
		type_id = projectile_id[i_entity].type_id;
	}

	__int32 i_archetype = collision_output.collidee.i_archetype;
	__int32 i_entity = collision_output.collidee.i_entity;
	component_::animation_driver_* animation_driver = (component_::animation_driver_*)Find_Archetype_Component(i_archetype, component_id_::ANIMATION_DRIVER, archetype_data);
	component_::effect_* effect = (component_::effect_*)Find_Archetype_Component(i_archetype, component_id_::EFFECT, archetype_data);

	animation_driver[i_entity].trigger_id = animation_data_::id_::PAIN;

	effect[i_entity].trigger_id = (type_id == component_::item_::type_::KILL) ? type_id : effect[i_entity].trigger_id;
	effect[i_entity].trigger_id = (type_id == component_::item_::type_::PETRIFY) ? type_id : effect[i_entity].trigger_id;
	effect[i_entity].trigger_id = (type_id == component_::item_::type_::SHRINK) ? type_id : effect[i_entity].trigger_id;
}

/*
==================
==================
*/
void systems_::collision_response_::projectile_vs_monster(

	const collision_output_& collision_output,
	archetype_data_& archetype_data,
	particle_manager_& particle_manager

) {

	__int32 type_id;
	{
		__int32 i_archetype = collision_output.collider.i_archetype;
		__int32 i_entity = collision_output.collider.i_entity;
		component_::projectile_id_* projectile_id = (component_::projectile_id_*)Find_Archetype_Component(i_archetype, component_id_::PROJECTILE_ID, archetype_data);
		type_id = projectile_id[i_entity].type_id;
	}

	__int32 i_archetype = collision_output.collidee.i_archetype;
	__int32 i_entity = collision_output.collidee.i_entity;
	//component_::move_* move = (component_::move_*)Find_Archetype_Component(i_archetype, component_id_::MOVE, archetype_data);
	//component_::mass_* mass = (component_::mass_*)Find_Archetype_Component(i_archetype, component_id_::MASS, archetype_data);
	component_::effect_* effect = (component_::effect_*)Find_Archetype_Component(i_archetype, component_id_::EFFECT, archetype_data);
	component_::behaviour_* behaviour = (component_::behaviour_*)Find_Archetype_Component(i_archetype, component_id_::BEHAVIOUR, archetype_data);
	component_::health_* health = (component_::health_*)Find_Archetype_Component(i_archetype, component_id_::HEALTH, archetype_data);

	behaviour[i_entity].trigger_id = component_::behaviour_::id_::PAIN;
	health[i_entity].hp--;

	effect[i_entity].trigger_id = (type_id == component_::item_::type_::KILL) ? type_id : effect[i_entity].trigger_id;
	effect[i_entity].trigger_id = (type_id == component_::item_::type_::PETRIFY) ? type_id : effect[i_entity].trigger_id;
	effect[i_entity].trigger_id = (type_id == component_::item_::type_::SHRINK) ? type_id : effect[i_entity].trigger_id;

	//float impact_modifier = 0.0f;
	//float impact_force = 200.0f / mass[i_entity].value;
	//impact_modifier = (type_id == component_::item_::type_::PUSH) ? impact_force : impact_modifier;
	//impact_modifier = (type_id == component_::item_::type_::PULL) ? -impact_force : impact_modifier;
	//move[i_entity].velocity += collision_output.collider.velocity * impact_modifier;
}

/*
==================
==================
*/
void systems_::collision_response_::entity_vs_platform(

	const collision_output_& collision_output,
	archetype_data_& archetype_data,
	particle_manager_& particle_manager

) {
	float mass_modifier;
	{

		__int32 i_archetype = collision_output.collider.i_archetype;
		__int32 i_entity = collision_output.collider.i_entity;
		__int32 component_id = component_id_::MASS;
		component_::mass_* mass = (component_::mass_*)Find_Archetype_Component(i_archetype, component_id, archetype_data);
		mass_modifier = mass[i_entity].value;
	}

	__int32 i_archetype = collision_output.collidee.i_archetype;
	__int32 i_entity = collision_output.collidee.i_entity;
	__int32 component_id = component_id_::PLATFORM;
	component_::platform_* platform = (component_::platform_*)Find_Archetype_Component(i_archetype, component_id, archetype_data);

	platform[i_entity].contact_mass = mass_modifier;

	//printf_s("MASS: %f \n", platform[i_entity].contact_mass);

}

/*
==================
==================
*/
void systems_::collision_response_::entity_vs_bounce_pad(

	const collision_output_& collision_output,
	archetype_data_& archetype_data,
	particle_manager_& particle_manager

) {
	bool is_on;
	{

		__int32 i_archetype = collision_output.collidee.i_archetype;
		__int32 i_entity = collision_output.collidee.i_entity;
		__int32 component_id = component_id_::POWER;
		component_::power_* power = (component_::power_*)Find_Archetype_Component(i_archetype, component_id, archetype_data);
		is_on = power[i_entity].is_on;
	}

	__int32 i_archetype = collision_output.collider.i_archetype;
	__int32 i_entity = collision_output.collider.i_entity;
	__int32 component_id = component_id_::MOVE;
	component_::move_* move = (component_::move_*)Find_Archetype_Component(i_archetype, component_id, archetype_data);

	float velocity = is_on ? 500.0f : 0.0f;

	move[i_entity].velocity.x = (float)collision_output.collidee.normal.x * r_fixed_scale_real * velocity;
	move[i_entity].velocity.y = (float)collision_output.collidee.normal.y * r_fixed_scale_real * velocity;
	move[i_entity].velocity.z = (float)collision_output.collidee.normal.z * r_fixed_scale_real * velocity;
}

/*
==================
==================
*/
void systems_::collision_response_::entity_vs_switch(

	const collision_output_& collision_output,
	archetype_data_& archetype_data,
	particle_manager_& particle_manager

) {

	__int32 i_archetype = collision_output.collidee.i_archetype;
	__int32 i_entity = collision_output.collidee.i_entity;
	__int32 component_id = component_id_::SWITCH;
	component_::switch_* switch_ = (component_::switch_*)Find_Archetype_Component(i_archetype, component_id, archetype_data);
	switch_[i_entity].is_on = true;

	{
		float3_ collidee_displacement;
		{
			__int32 i_archetype = collision_output.collidee.i_archetype;
			__int32 i_entity = collision_output.collidee.i_entity;
			__int32 component_id = component_id_::MOVE;
			component_::move_* move = (component_::move_*)Find_Archetype_Component(i_archetype, component_id, archetype_data);
			collidee_displacement = move[i_entity].displacement;
		}

		__int32 i_archetype = collision_output.collider.i_archetype;
		__int32 i_entity = collision_output.collider.i_entity;
		__int32 component_id = component_id_::MOVE;
		component_::move_* move = (component_::move_*)Find_Archetype_Component(i_archetype, component_id, archetype_data);
		move[i_entity].displacement += collidee_displacement;
	}

}

/*
==================
==================
*/
void systems_::collision_response_::entity_vs_button(

	const collision_output_& collision_output,
	archetype_data_& archetype_data,
	particle_manager_& particle_manager

) {

	__int32 i_archetype = collision_output.collidee.i_archetype;
	__int32 i_entity = collision_output.collidee.i_entity;
	__int32 component_id = component_id_::SWITCH;
	component_::switch_* switch_ = (component_::switch_*)Find_Archetype_Component(i_archetype, component_id, archetype_data);
	switch_[i_entity].is_on = true;
}

/*
==================
==================
*/
void systems_::collision_response_::entity_vs_trapdoor(

	const collision_output_& collision_output,
	archetype_data_& archetype_data,
	particle_manager_& particle_manager

) {

	int3_ position;
	int3_ extent;
	{
		__int32 i_archetype = collision_output.collidee.i_archetype;
		__int32 i_entity = collision_output.collidee.i_entity;

		component_fetch_ fetch;
		fetch.n_components = 3;
		fetch.n_excludes = 0;
		fetch.component_ids[0] = component_id_::BASE;
		fetch.component_ids[1] = component_id_::COLLIDEE_STATIC;
		fetch.component_ids[2] = component_id_::TRAP_DOOR_ID;

		Populate_Fetch_Table(archetype_data, fetch);

		const __int32 i_archetype_index = 0;
		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
		component_::collidee_static_* collidee_static = (component_::collidee_static_*)fetch.table[1][i_archetype_index];

		position = base[i_entity].position_fixed;
		extent = collidee_static[i_entity].extent;

		Clear_Component_Bit(i_archetype, i_entity, component_id_::COLLIDEE_STATIC, archetype_data);
		Clear_Component_Bit(i_archetype, i_entity, component_id_::DRAW, archetype_data);
	}

	__int32 id_begin = particle_manager.emitter[particle_manager.i_emitter].start_id;
	//__int32 id_end = id_begin + particle_manager.particle_emitters[particle_manager.i_next_emitter].n_particles;
	particle_manager.i_emitter = (particle_manager.i_emitter + 1) % particle_manager_::NUM_EMITTERS;

	component_fetch_ fetch;
	fetch.n_components = 5;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::BASE;
	fetch.component_ids[1] = component_id_::MOVE;
	fetch.component_ids[2] = component_id_::COLOUR;
	fetch.component_ids[3] = component_id_::COLLIDER;
	fetch.component_ids[4] = component_id_::PARTICLE;

	Populate_Fetch_Table(archetype_data, fetch);

	const __int32 i_archetype_index = 0;
	component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
	component_::move_* move = (component_::move_*)fetch.table[1][i_archetype_index];
	component_::colour_* colour = (component_::colour_*)fetch.table[2][i_archetype_index];
	component_::collider_* collider = (component_::collider_*)fetch.table[3][i_archetype_index];
	component_::particle_* particle = (component_::particle_*)fetch.table[4][i_archetype_index];

	__int32 i_entity = id_begin;

	for (__int32 i_particle = 0; i_particle < particle_manager_::NUM_PARTICLES_PER_EMITTER; i_particle++) {

		base[i_entity].position_fixed.x = position.x + __int32(particle_manager.trapdoor.position[i_particle].x * fixed_scale_real);
		base[i_entity].position_fixed.y = position.y + __int32(particle_manager.trapdoor.position[i_particle].y * fixed_scale_real);
		base[i_entity].position_fixed.z = position.z + __int32(particle_manager.trapdoor.position[i_particle].z * fixed_scale_real);

		base[i_entity].scale = particle_manager.trapdoor.scale;
		move[i_entity].velocity = particle_manager.trapdoor.velocity[i_particle];
		colour[i_entity].colour = particle_manager.trapdoor.colour;

		i_entity++;
	}
}

/*
==================
==================
*/
void systems_::collision_response_::entity_vs_teleporter(

	const collision_output_& collision_output,
	archetype_data_& archetype_data,
	particle_manager_& particle_manager

) {

	int3_ teleport_location;
	{
		__int32 i_archetype = collision_output.collidee.i_archetype;
		__int32 i_entity = collision_output.collidee.i_entity;
		__int32 component_id = component_id_::TELEPORTER;
		component_::teleporter_* teleporter = (component_::teleporter_*)Find_Archetype_Component(i_archetype, component_id, archetype_data);
		teleport_location = teleporter[i_entity].destination;
	}

	__int32 i_archetype = collision_output.collider.i_archetype;
	__int32 i_entity = collision_output.collider.i_entity;
	__int32 component_id = component_id_::BASE;
	component_::base_* base = (component_::base_*)Find_Archetype_Component(i_archetype, component_id, archetype_data);
	base[i_entity].position_fixed = teleport_location;
}

/*
========================================================================
========================================================================
*/
void systems_::entity_::emit_particles(

	void* parameters, __int32 i_thread
)
{
	static const float3_ scales[] = {

		{ 1.5f, 1.5f, 1.5f },
		{ 2.0f, 2.0f, 2.0f },
		{ 2.5f, 2.5f, 2.5f },
		{ 3.0f, 3.0f, 3.0f },
	};

	parameters_::entity_::emit_particles_* func_parameters = (parameters_::entity_::emit_particles_*)parameters;
	const collision_manager_& collision_manager = *func_parameters->collision_manager;
	particle_manager_& particle_manager = *func_parameters->particle_manager;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ fetch;
	{
		fetch.n_components = 4;
		fetch.n_excludes = 0;
		fetch.component_ids[0] = component_id_::BASE;
		fetch.component_ids[1] = component_id_::MOVE;
		fetch.component_ids[2] = component_id_::COLOUR;
		fetch.component_ids[3] = component_id_::PARTICLE;

		Populate_Fetch_Table(archetype_data, fetch);
	}

	for (__int32 i_thread = 0; i_thread < func_parameters->n_threads; i_thread++) {

		const thread_collision_& thread_collision = collision_manager.thread_collisions[i_thread];

		for (__int32 i_collider = 0; i_collider < thread_collision.n_colliders; i_collider++) {

			const collision_output_& collision_output = thread_collision.collision_output[i_collider];

			bool is_collider_projectile = collision_output.collider.entity_type == colliding_type_::PROJECTILES;
			bool is_collidee_player = collision_output.collidee.entity_type == colliding_type_::PLAYER;
			bool is_collidee_monster = collision_output.collidee.entity_type == colliding_type_::MONSTER;
			bool is_collidee_mob = collision_output.collidee.entity_type == colliding_type_::MOB;
			bool is_emitter_event = is_collider_projectile && (is_collidee_player || is_collidee_monster || is_collidee_mob);

			__int32 id_begin = particle_manager.emitter[particle_manager.i_emitter].start_id;
			__int32 id_end = id_begin + particle_manager.emitter[particle_manager.i_emitter].n_particles;
			particle_manager.i_emitter = (particle_manager.i_emitter + is_emitter_event) % particle_manager_::NUM_EMITTERS;

			if (is_emitter_event) {

				float3_ colour_collider;
				{
					__int32 i_archetype = collision_output.collider.i_archetype;
					__int32 i_entity = collision_output.collider.i_entity;
					__int32 component_id = component_id_::COLOUR;
					component_::colour_* colour = (component_::colour_*)Find_Archetype_Component(i_archetype, component_id, archetype_data);
					colour_collider = colour[i_entity].colour;
				}
				{
					const __int32 i_archetype_index = 0;
					component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
					component_::move_* move = (component_::move_*)fetch.table[1][i_archetype_index];
					component_::colour_* colour = (component_::colour_*)fetch.table[2][i_archetype_index];
					component_::particle_* particle = (component_::particle_*)fetch.table[3][i_archetype_index];

					__int32 i_particle = 0;
					for (__int32 i_entity = id_begin; i_entity < id_end; i_entity++) {

						base[i_entity].position_fixed.x = collision_output.collision_point.x + __int32(particle_manager.impact.position[i_particle].x * fixed_scale_real);
						base[i_entity].position_fixed.y = collision_output.collision_point.y + __int32(particle_manager.impact.position[i_particle].y * fixed_scale_real);
						base[i_entity].position_fixed.z = collision_output.collision_point.z + __int32(particle_manager.impact.position[i_particle].z * fixed_scale_real);

						//base[i_entity].position_fixed = collision_output.collision_point;
						base[i_entity].scale = particle_manager.impact.scale;

						move[i_entity].velocity = particle_manager.impact.velocity[i_particle];
						colour[i_entity].colour = colour_collider;
						i_particle++;
					}
				}
			}
		}
	}
}

/*
========================================================================
========================================================================
*/
void systems_::entity_::update_global_matrices(

	void* parameters, __int32 i_thread
)
{
	parameters_::entity_::update_global_matrices_* func_parameters = (parameters_::entity_::update_global_matrices_*)parameters;
	const timer_& timer = *func_parameters->timer;
	game_matrices_& game_matrices = *func_parameters->game_matrices;


	static const __m128 Colour_Pulse_Velocity = set(0.0f, 0.0f, 0.0f, 50.0f);
	static const __m128 Ring_Rotation_Velocity = set(0.0f, 0.0f, 0.0f, 50.0f);
	static const __m128 Totem_Rotation_Velocity = set(0.0f, 0.0f, 0.0f, 100.0f);

	static __m128 axis_angle = set(0.0f, 1.0f, 0.0f, 0.0f);
	static __m128 ring_axis_angle = set(1.0f, 0.0f, 0.0f, 0.0f);
	static __m128 totem_axis_angle = set(0.0f, 1.0f, 0.0f, 0.0f);

	{
		__m128 time_elapsed = broadcast(load_s(timer.delta_time));
		axis_angle += Colour_Pulse_Velocity * time_elapsed;
		ring_axis_angle += (Ring_Rotation_Velocity * time_elapsed);
		totem_axis_angle += (Totem_Rotation_Velocity * time_elapsed);

		matrix quaternion_m;
		quaternion_m[X] = Axis_Angle_To_Quaternion(axis_angle);
		quaternion_m[Y] = Axis_Angle_To_Quaternion(ring_axis_angle);
		quaternion_m[Z] = Axis_Angle_To_Quaternion(totem_axis_angle);
		matrix out[4];
		Quaternion_To_Matrix(quaternion_m, out);

		for (__int32 i = 0; i < 4; i++) {

			game_matrices.m_rotate_z[i] = out[Y][i];
			game_matrices.m_rotate_y[i] = out[Z][i];
		}
	}
}

/*
========================================================================
========================================================================
*/
void systems_::entity_::mark_entity_grounded(

	void* parameters, __int32 i_thread
)
{
	parameters_::entity_::mark_entity_grounded_* func_parameters = (parameters_::entity_::mark_entity_grounded_*)parameters;
	collision_manager_& collision_manager = *func_parameters->collision_manager;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	for (__int32 i_thread = 0; i_thread < func_parameters->n_threads; i_thread++) {

		thread_collision_& thread_collision = collision_manager.thread_collisions[i_thread];

		for (__int32 i_collider = 0; i_collider < thread_collision.n_colliders; i_collider++) {

			collision_output_& collision_output = thread_collision.collision_output[i_collider];
			bool is_ground_contact = collision_manager.is_ground[collision_output.collidee.entity_type] && (collision_output.collidee.normal.y > 0);

			if (is_ground_contact) {

				__int32 i_archetype = collision_output.collider.i_archetype;
				__int32 i_entity = collision_output.collider.i_entity;
				__int32 component_id = component_id_::COLLIDER;
				component_::collider_* collider = (component_::collider_*)Find_Archetype_Component(i_archetype, component_id, archetype_data);
				collider[i_entity].is_ground_contact = true;
			}
		}
	}
}
/*
========================================================================
========================================================================
*/
void systems_::entity_::process_response_table(

	void* parameters, __int32 i_thread
)
{
	parameters_::entity_::process_response_table_* func_parameters = (parameters_::entity_::process_response_table_*)parameters;
	const collision_manager_& collision_manager = *func_parameters->collision_manager;
	const systems_::collision_response_& collision_response = *func_parameters->collision_response;
	particle_manager_& particle_manager = *func_parameters->particle_manager;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	for (__int32 i_thread = 0; i_thread < func_parameters->n_threads; i_thread++) {

		const thread_collision_& thread_collision = collision_manager.thread_collisions[i_thread];

		for (__int32 i_collider = 0; i_collider < thread_collision.n_colliders; i_collider++) {

			const collision_output_& collision_output = thread_collision.collision_output[i_collider];

			const __int32 n_entries = collision_response.n_entries[collision_output.collider.entity_type][collision_output.collidee.entity_type];
			for (__int32 i_entry = 0; i_entry < n_entries; i_entry++) {

				collision_response.Entity_Reaction[collision_output.collider.entity_type][collision_output.collidee.entity_type][i_entry](

					collision_output,
					archetype_data,
					particle_manager
					);

			}
		}
	}
}

/*
==================
==================
*/
void systems_::entity_::apply_gravity(

	void* parameters, __int32 i_thread
)
{
	parameters_::entity_::apply_gravity_* func_parameters = (parameters_::entity_::apply_gravity_*)parameters;
	const timer_& timer = *func_parameters->timer;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ fetch;
	fetch.n_components = 3;
	fetch.component_ids[0] = component_id_::MOVE;
	fetch.component_ids[1] = component_id_::COLLIDER;
	fetch.component_ids[2] = component_id_::MASS;
	fetch.n_excludes = 1;
	fetch.exclude_ids[0] = component_id_::FLY;

	Populate_Fetch_Table(archetype_data, fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::move_* move = (component_::move_*)fetch.table[0][i_archetype_index];
		component_::collider_* collider = (component_::collider_*)fetch.table[1][i_archetype_index];
		component_::mass_* mass = (component_::mass_*)fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			float3_ horizontal_velocity = move[i_entity].velocity;
			horizontal_velocity.y = 0.0f;

			float3_ horizontal_direction;
			float speed = Vector_Normalise_Magnitude(horizontal_velocity, horizontal_direction);
			float new_friction = (mass[i_entity].value * speed * 0.2f) + 80.0f;
			//float friction = blend(friction_ground, 0.0f, collider[i_entity].is_ground_contact);
			float friction = blend(new_friction, 0.0f, collider[i_entity].is_ground_contact);
			speed -= friction * timer.delta_time;
			speed = max(speed, 0.0f);
			move[i_entity].velocity.x = horizontal_direction.x * speed;
			move[i_entity].velocity.z = horizontal_direction.z * speed;

			move[i_entity].velocity.y += acceleration_gravity * mass[i_entity].value * timer.delta_time;

		}
	}
}

/*
==================
==================
*/
void systems_::entity_::update_displacement_main(

	void* parameters, __int32 i_thread
)
{
	parameters_::entity_::update_displacement_main_* func_parameters = (parameters_::entity_::update_displacement_main_*)parameters;
	timer_ const& timer = *func_parameters->timer;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ fetch;
	fetch.n_components = 2;
	fetch.component_ids[0] = component_id_::MOVE;
	fetch.component_ids[1] = component_id_::COLLIDER;
	fetch.n_excludes = 1;
	fetch.exclude_ids[0] = component_id_::FLY;

	Populate_Fetch_Table(archetype_data, fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::move_* move = (component_::move_*)fetch.table[0][i_archetype_index];
		component_::collider_* collider = (component_::collider_*)fetch.table[1][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			//bool is_grounded = collider[i_entity].is_ground_contact;
			{
				//float3_ horizontal_velocity = move[i_entity].velocity;
				//horizontal_velocity.y = 0.0f;
				//const float friction_modifier = 1.0f;

				//float3_ horizontal_direction;
				//float speed = Vector_Normalise_Magnitude(horizontal_velocity, horizontal_direction);
				//float friction = blend(friction_ground, 0.0f, is_grounded);
				//speed -= friction * friction_modifier * timer.delta_time;
				//float max_speed = blend(move[i_entity].max_speed, speed, is_grounded);
				//speed = max(speed, 0.0f);
				//move[i_entity].velocity.x = horizontal_direction.x * speed;
				//move[i_entity].velocity.z = horizontal_direction.z * speed;

				move[i_entity].displacement = move[i_entity].velocity * timer.delta_time;

				//move[i_entity].displacement.x = move[i_entity].velocity.x * timer.delta_time;
				//move[i_entity].displacement.z = move[i_entity].velocity.z * timer.delta_time;
			}
			//{
			//	move[i_entity].velocity.y += acceleration_gravity * move[i_entity].gravity_modifier * timer.delta_time;
			//	move[i_entity].displacement.y = move[i_entity].velocity.y * timer.delta_time;

			//}
		}
	}
}



/*
==================
==================
*/
void systems_::entity_::update_displacement_fly(

	void* parameters, __int32 i_thread
)
{
	parameters_::entity_::update_displacement_fly_* func_parameters = (parameters_::entity_::update_displacement_fly_*)parameters;
	const timer_& timer = *func_parameters->timer;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ component_fetch;
	component_fetch.n_components = 3;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::MOVE;
	component_fetch.component_ids[1] = component_id_::COLLIDER;
	component_fetch.component_ids[2] = component_id_::FLY;

	Populate_Fetch_Table(archetype_data, component_fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::move_* move = (component_::move_*)component_fetch.table[0][i_archetype_index];
		component_::collider_* collider = (component_::collider_*)component_fetch.table[1][i_archetype_index];
		component_::fly_* fly = (component_::fly_*)component_fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			//float3_ horizontal_velocity = move[i_entity].velocity;
			//horizontal_velocity.y = 0.0f;

			//float3_ horizontal_direction;
			//float speed = Vector_Normalise_Magnitude(horizontal_velocity, horizontal_direction);
			//speed -= friction_ground * move[i_entity].friction_modifier * timer.delta_time;
			//speed = min(speed, move[i_entity].max_speed);
			//speed = max(speed, 0.0f);

			//move[i_entity].velocity.x = horizontal_direction.x * speed;
			//move[i_entity].velocity.z = horizontal_direction.z * speed;


			//move[i_entity].displacement = move[i_entity].velocity * timer.delta_time;
		}
	}
}

/*
==================
==================
*/
void systems_::entity_::bbox_vs_view_volume(

	void* parameters, __int32 i_thread
) {

	parameters_::entity_::bbox_vs_view_volume_* func_parameters = (parameters_::entity_::bbox_vs_view_volume_*)parameters;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	__int32 i_entity_camera;
	__int32 i_archetype_camera;
	component_::camera_* camera;
	component_::base_* base_camera;
	{

		component_fetch_ component_fetch;
		component_fetch.n_components = 2;
		component_fetch.n_excludes = 0;
		component_fetch.component_ids[0] = component_id_::BASE;
		component_fetch.component_ids[1] = component_id_::CAMERA;

		Populate_Fetch_Table(archetype_data, component_fetch);

		i_archetype_camera = component_fetch.i_archetypes[0];

		i_entity_camera = Return_Camera_Component(archetype_data, &camera);
		base_camera = (component_::base_*)component_fetch.table[0][0];
		camera = (component_::camera_*)component_fetch.table[1][0];
	}

	component_fetch_ component_fetch;
	component_fetch.n_components = 3;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::BASE;
	component_fetch.component_ids[1] = component_id_::BOUNDING_BOX;
	component_fetch.component_ids[2] = component_id_::DRAW;

	Populate_Fetch_Table(archetype_data, component_fetch);

	enum {
		NUM_VERTICES_BOX = 8,
	};

	static const float3_ unit_cube[NUM_VERTICES_BOX] = {

		{ 1.0f,  1.0f,  1.0f },
	{ -1.0f,  1.0f,  1.0f },
	{ 1.0f,  1.0f, -1.0f },
	{ -1.0f,  1.0f, -1.0f },
	{ 1.0f, -1.0f,  1.0f },
	{ -1.0f, -1.0f,  1.0f },
	{ 1.0f, -1.0f, -1.0f },
	{ -1.0f, -1.0f, -1.0f },
	};

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)component_fetch.table[0][i_archetype_index];
		component_::bounding_box_* bounding_box = (component_::bounding_box_*)component_fetch.table[1][i_archetype_index];
		component_::draw_* draw_ = (component_::draw_*)component_fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			//if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			int3_ position_fixed;
			position_fixed.x = base[i_entity].position_fixed.x - base_camera[i_entity_camera].position_fixed.x;
			position_fixed.y = base[i_entity].position_fixed.y - base_camera[i_entity_camera].position_fixed.y;
			position_fixed.z = base[i_entity].position_fixed.z - base_camera[i_entity_camera].position_fixed.z;

			//position_fixed.x = bounding_box[i_entity].centre.x - base_camera[i_entity_camera].position_fixed.x;
			//position_fixed.y = bounding_box[i_entity].centre.y - base_camera[i_entity_camera].position_fixed.y;
			//position_fixed.z = bounding_box[i_entity].centre.z - base_camera[i_entity_camera].position_fixed.z;

			float3_ position;
			position.x = float(position_fixed.x) * r_fixed_scale_real;
			position.y = float(position_fixed.y) * r_fixed_scale_real;
			position.z = float(position_fixed.z) * r_fixed_scale_real;

			float3_ extent;
			extent.x = float(bounding_box[i_entity].extent.x) * r_fixed_scale_real;
			extent.y = float(bounding_box[i_entity].extent.y) * r_fixed_scale_real;
			extent.z = float(bounding_box[i_entity].extent.z) * r_fixed_scale_real;

			float4_ vertices[NUM_VERTICES_BOX];
			for (__int32 i_vertex = 0; i_vertex < NUM_VERTICES_BOX; i_vertex++) {

				float3_ temp = (unit_cube[i_vertex] * extent) + position;
				Vector_X_Matrix(temp, camera[i_entity_camera].m_clip_space, vertices[i_vertex]);
			}

			unsigned __int32 bb_mask = 0xffff;
			for (__int32 i_vertex = 0; i_vertex < NUM_VERTICES_BOX; i_vertex++) {

				const float w_value = vertices[i_vertex].w;
				const float w_value_neg = -vertices[i_vertex].w;
				unsigned __int32 vertex_mask = 0x0;

				vertex_mask |= (vertices[i_vertex].x < w_value_neg) << PLANE_LEFT;
				vertex_mask |= (vertices[i_vertex].x > w_value) << PLANE_RIGHT;
				vertex_mask |= (vertices[i_vertex].y < w_value_neg) << PLANE_TOP;
				vertex_mask |= (vertices[i_vertex].y > w_value) << PLANE_BOTTOM;
				vertex_mask |= (vertices[i_vertex].z < w_value_neg) << PLANE_NEAR;
				vertex_mask |= (vertices[i_vertex].z > w_value) << PLANE_FAR;

				bb_mask &= vertex_mask;
			}

			Set_Component_Bit(i_archetype, i_entity, component_id_::DRAW, archetype_data);

			bool is_culled = bb_mask != 0x0;

			if (is_culled) {

				Clear_Component_Bit(i_archetype, i_entity, component_id_::DRAW, archetype_data);

				//printf_s("culled: %i \n", i_entity);
			}
		}
	}

	// TEMP HACK FOR CAMERA
	Toggle_Component_Bit(i_archetype_camera, i_entity_camera, component_id_::DRAW, archetype_data);
}

/*
==================
==================
*/
void systems_::entity_::update_position(

	void* parameters, __int32 i_thread
) {

	parameters_::entity_::update_position_* func_parameters = (parameters_::entity_::update_position_*)parameters;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ component_fetch;
	component_fetch.n_components = 2;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::BASE;
	component_fetch.component_ids[1] = component_id_::MOVE;

	Populate_Fetch_Table(archetype_data, component_fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)component_fetch.table[0][i_archetype_index];
		component_::move_* move = (component_::move_*)component_fetch.table[1][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			base[i_entity].position_fixed.x += (__int32)(move[i_entity].displacement.x * fixed_scale_real);
			base[i_entity].position_fixed.y += (__int32)(move[i_entity].displacement.y * fixed_scale_real);
			base[i_entity].position_fixed.z += (__int32)(move[i_entity].displacement.z * fixed_scale_real);
		}
	}
}

/*
==================
==================
*/
void systems_::entity_::out_of_bounds(

	void* parameters, __int32 i_thread
) {

	parameters_::entity_::out_of_bounds_* func_parameters = (parameters_::entity_::out_of_bounds_*)parameters;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	const __int32 fall_limit = -500 * fixed_scale;

	component_fetch_ component_fetch;
	component_fetch.n_components = 2;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::BASE;
	component_fetch.component_ids[1] = component_id_::MOVE;

	Populate_Fetch_Table(archetype_data, component_fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)component_fetch.table[0][i_archetype_index];
		component_::move_* move = (component_::move_*)component_fetch.table[1][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			bool is_out_bounds = base[i_entity].position_fixed.y < fall_limit;
			base[i_entity].position_fixed.y = max(base[i_entity].position_fixed.y, fall_limit);
			move[i_entity].velocity *= blend(0.0f, 1.0f, is_out_bounds);
		}
	}
}

/*
==================
==================
*/
void systems_::entity_::out_of_bounds_respawn(

	void* parameters, __int32 i_thread
) {

	parameters_::entity_::out_of_bounds_respawn_* func_parameters = (parameters_::entity_::out_of_bounds_respawn_*)parameters;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	const __int32 fall_limit = -400 * fixed_scale;

	component_fetch_ component_fetch;
	component_fetch.n_components = 3;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::BASE;
	component_fetch.component_ids[1] = component_id_::MOVE;
	component_fetch.component_ids[2] = component_id_::SPAWN;

	Populate_Fetch_Table(archetype_data, component_fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)component_fetch.table[0][i_archetype_index];
		component_::move_* move = (component_::move_*)component_fetch.table[1][i_archetype_index];
		component_::spawn_* spawn = (component_::spawn_*)component_fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			bool is_out_bounds = base[i_entity].position_fixed.y < fall_limit;

			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				base[i_entity].position_fixed.i[i_axis] = blend_int(spawn[i_entity].position.i[i_axis], base[i_entity].position_fixed.i[i_axis], is_out_bounds);
			}

			move[i_entity].velocity *= blend(0.0f, 1.0f, is_out_bounds);
		}
	}
}


/*
==================
==================
*/
void systems_::entity_::update_animation(

	void* parameters, __int32 i_thread
) {

	parameters_::entity_::update_animation_* func_parameters = (parameters_::entity_::update_animation_*)parameters;
	const timer_& timer = *func_parameters->timer;
	const animation_manager_& animation_manager = *func_parameters->animation_manager;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	//const static float frame_speed = 10.0f;

	component_fetch_ component_fetch;
	component_fetch.n_components = 1;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::ANIMATION;

	Populate_Fetch_Table(archetype_data, component_fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::animation_* animation = (component_::animation_*)component_fetch.table[0][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			float& frame_interval = animation[i_entity].frame_interval;
			frame_interval += animation[i_entity].frame_speed * animation[i_entity].frame_speed_modifier * timer.delta_time;
			animation[i_entity].is_frame_change = frame_interval > 1.0f;
			frame_interval = blend(frame_interval - 1.0f, frame_interval, animation[i_entity].is_frame_change);

			animation[i_entity].i_frames[component_::animation_::CURRENT_FRAME_LOW] = blend_int(animation[i_entity].i_frames[component_::animation_::CURRENT_FRAME_HI], animation[i_entity].i_frames[component_::animation_::CURRENT_FRAME_LOW], animation[i_entity].is_frame_change);
			animation[i_entity].i_frames[component_::animation_::CURRENT_FRAME_HI] = blend_int(animation[i_entity].i_frames[component_::animation_::NEXT_FRAME], animation[i_entity].i_frames[component_::animation_::CURRENT_FRAME_HI], animation[i_entity].is_frame_change);
			animation[i_entity].i_frames[component_::animation_::NEXT_FRAME] += animation[i_entity].is_frame_change;

			const animation_model_& animation_model = animation_manager.animation_model[animation[i_entity].model_id];
			const __int32 i_animation = animation[i_entity].i_current;
			const __int32 frame_limit = animation_model.animation_data[i_animation].i_start + animation_model.animation_data[i_animation].n_frames;

			animation[i_entity].is_end_animation = animation[i_entity].i_frames[component_::animation_::NEXT_FRAME] == frame_limit;
			animation[i_entity].i_frames[component_::animation_::NEXT_FRAME] = blend_int(animation_model.animation_data[i_animation].i_start, animation[i_entity].i_frames[component_::animation_::NEXT_FRAME], animation[i_entity].is_end_animation);

			{
				//const __int32 n_animations = animation_manager.animation_model[animation[i_entity].model_id].n_animations;
				__int32 i_animation_trigger = INVALID_RESULT;
				for (__int32 i_animation = 0; i_animation < animation_data_::id_::COUNT; i_animation++) {
					bool is_match = animation_manager.animation_model[animation[i_entity].model_id].animation_data[i_animation].id == animation[i_entity].trigger_id;
					i_animation_trigger = blend_int(i_animation, i_animation_trigger, is_match);
				}
				animation[i_entity].trigger_id = animation_data_::id_::NULL_;
				bool is_new_animation = i_animation_trigger != INVALID_RESULT;
				i_animation_trigger = blend_int(i_animation_trigger, 0, is_new_animation);
				animation[i_entity].i_current = blend_int(i_animation_trigger, animation[i_entity].i_current, is_new_animation);
				animation[i_entity].i_frames[component_::animation_::NEXT_FRAME] = blend_int(animation_model.animation_data[i_animation_trigger].i_start, animation[i_entity].i_frames[component_::animation_::NEXT_FRAME], is_new_animation);
			}
		}
	}
}

/*
==================
==================
*/
void systems_::entity_::broadcast_switch_state(

	void* parameters, __int32 i_thread
) {

	parameters_::entity_::broadcast_switch_state_* func_parameters = (parameters_::entity_::broadcast_switch_state_*)parameters;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ component_fetch;
	component_fetch.n_components = 1;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::SWITCH;

	Populate_Fetch_Table(archetype_data, component_fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		component_::switch_* switch_ = (component_::switch_*)component_fetch.table[0][i_archetype_index];

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];

		for (__int32 i_entity = 0; i_entity < archetype.n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			const __int32 i_archetype_target = switch_[i_entity].i_archetype_target;
			const __int32 i_entity_target = switch_[i_entity].i_entity_target;

			const __int32 component_id = component_id_::POWER;
			component_::power_* power = (component_::power_*)Find_Archetype_Component(i_archetype_target, component_id, archetype_data);
			power[i_entity_target].is_on = switch_[i_entity].is_on;

			//printf_s(" %i ,", power[i_entity_target].is_on);
			//printf_s("archtype: %i ,", i_archetype_target);
			//printf_s("entity: %i \n", i_entity_target);
		}
	}
}

/*
==================
==================
*/
void systems_::entity_::update_power_effects(

	void* parameters, __int32 i_thread
) {

	parameters_::entity_::update_power_effects_* func_parameters = (parameters_::entity_::update_power_effects_*)parameters;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ component_fetch;
	component_fetch.n_components = 3;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::POWER;
	component_fetch.component_ids[1] = component_id_::COLOUR_SPACE;
	component_fetch.component_ids[2] = component_id_::TEXTURE_SPACE;

	Populate_Fetch_Table(archetype_data, component_fetch);


	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		component_::power_* power = (component_::power_*)component_fetch.table[0][i_archetype_index];
		component_::colour_space_* colour_space = (component_::colour_space_*)component_fetch.table[1][i_archetype_index];
		component_::texture_space_* texture_space = (component_::texture_space_*)component_fetch.table[2][i_archetype_index];

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];

		for (__int32 i_entity = 0; i_entity < archetype.n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			colour_space[i_entity].q_add = power[i_entity].is_on ? power[i_entity].q_add_colour : power[i_entity].q_null;
			texture_space[i_entity].q_add = power[i_entity].is_on ? power[i_entity].q_add_texture : power[i_entity].q_null;

			//if (power[i_entity].is_on) {
			//	printf_s("PENIS");
			//	printf_s(" %f %f %f %f \n", colour_space[i_entity].q_add.x, colour_space[i_entity].q_add.y, colour_space[i_entity].q_add.z, colour_space[i_entity].q_add.w);
			//}
		}
	}
}

/*
==================
==================
*/
void systems_::entity_::update_model_space_transform(

	void* parameters, __int32 i_thread
) {

	parameters_::entity_::update_model_space_transform_* func_parameters = (parameters_::entity_::update_model_space_transform_*)parameters;
	const timer_& timer = *func_parameters->timer;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ component_fetch;
	component_fetch.n_components = 2;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::MODEL_SPACE;
	component_fetch.component_ids[1] = component_id_::MODEL_SPACE_UPDATE;

	Populate_Fetch_Table(archetype_data, component_fetch);


	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		component_::model_space_* model_space = (component_::model_space_*)component_fetch.table[0][i_archetype_index];
		component_::model_space_update_* model_space_update = (component_::model_space_update_*)component_fetch.table[1][i_archetype_index];

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];

		for (__int32 i_entity = 0; i_entity < archetype.n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			float4_ q_current;
			for (__int32 i_axis = X; i_axis <= W; i_axis++) {
				q_current.f[i_axis] = model_space_update[i_entity].q_add.f[i_axis] * timer.delta_time;
			}

			Quaternion_X_Quaternion(q_current, model_space_update[i_entity].q_rotate, model_space_update[i_entity].q_rotate);
			Normalise_Quaternion(model_space_update[i_entity].q_rotate);
			Quaternion_To_Matrix(model_space_update[i_entity].q_rotate, model_space[i_entity].m_rotate);
		}
	}
}


/*
==================
==================
*/
void systems_::entity_::update_colour_space_transform(

	void* parameters, __int32 i_thread
) {

	parameters_::entity_::update_colour_space_transform_* func_parameters = (parameters_::entity_::update_colour_space_transform_*)parameters;
	const timer_& timer = *func_parameters->timer;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	const float3_ scale = { 1.0f, 1.0f, 1.0f };
	const float3_ displacement = { 0.0f, 0.0f, 1.0f };

	component_fetch_ component_fetch;
	component_fetch.n_components = 1;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::COLOUR_SPACE;

	Populate_Fetch_Table(archetype_data, component_fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::colour_space_* colour_space = (component_::colour_space_*)component_fetch.table[0][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			//printf_s(" %f %f %f %f \n", colour_space[i_entity].q_add.x, colour_space[i_entity].q_add.y, colour_space[i_entity].q_add.z, colour_space[i_entity].q_add.w);


			float4_ q_current;
			for (__int32 i_axis = X; i_axis <= W; i_axis++) {
				q_current.f[i_axis] = colour_space[i_entity].q_add.f[i_axis] * timer.delta_time;
			}
			Quaternion_X_Quaternion(q_current, colour_space[i_entity].q_rotate, colour_space[i_entity].q_rotate);
			Normalise_Quaternion(colour_space[i_entity].q_rotate);
			Quaternion_To_Matrix(colour_space[i_entity].q_rotate, colour_space[i_entity].m_rotate);

			colour_space[i_entity].m_rotate[X].x *= scale.x;
			colour_space[i_entity].m_rotate[Y].y *= scale.y;
			colour_space[i_entity].m_rotate[Z].z *= scale.z;

			colour_space[i_entity].m_rotate[W].x = displacement.x * scale.x;
			colour_space[i_entity].m_rotate[W].y = displacement.y * scale.y;
			colour_space[i_entity].m_rotate[W].z = displacement.z * scale.z;
			colour_space[i_entity].m_rotate[W].w = 1.0f;
		}
	}
}

/*
==================
==================
*/
void systems_::entity_::update_texture_space_transform(

	void* parameters, __int32 i_thread
) {

	parameters_::entity_::update_texture_space_transform_* func_parameters = (parameters_::entity_::update_texture_space_transform_*)parameters;
	const timer_& timer = *func_parameters->timer;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ component_fetch;
	component_fetch.n_components = 1;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::TEXTURE_SPACE;

	Populate_Fetch_Table(archetype_data, component_fetch);

	static __m128 period = set(0.0f, 0.5f, 0.0f, 0.0f);
	const __m128 half = broadcast(load_s(0.5f));

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::texture_space_* texture_space = (component_::texture_space_*)component_fetch.table[0][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			float4_ q_current;
			matrix_ temp;
			for (__int32 i_axis = X; i_axis <= W; i_axis++) {
				q_current.f[i_axis] = texture_space[i_entity].q_add.f[i_axis] * timer.delta_time;
			}
			Quaternion_X_Quaternion(q_current, texture_space[i_entity].q_rotate, texture_space[i_entity].q_rotate);
			Normalise_Quaternion(texture_space[i_entity].q_rotate);
			Quaternion_To_Matrix(texture_space[i_entity].q_rotate, temp);

			matrix m_rotate_z;
			m_rotate_z[X] = load_u(temp[X].f);
			m_rotate_z[Y] = load_u(temp[Y].f);
			m_rotate_z[Z] = load_u(temp[Z].f);
			m_rotate_z[W] = load_u(temp[W].f);

			const __m128 one = set_one();
			__m128 rotated_period = Vector_X_Matrix(period, m_rotate_z);
			matrix displacement;
			for (__int32 i = 0; i < 4; i++) {
				displacement[i] = rotated_period;
			}
			Transpose(displacement);
			displacement[Y] = one - abs(displacement[Y]);
			//displacement[Y] = abs(displacement[Y]);

			matrix tex_transform;
			Initialise(tex_transform);
			tex_transform[X] *= displacement[Y];
			tex_transform[Y] *= displacement[Y];
			tex_transform[W] |= ((one - displacement[Y]) * half) & (X_Mask | Y_Mask);

			store_u(tex_transform[X], texture_space[i_entity].m_rotate[X].f);
			store_u(tex_transform[Y], texture_space[i_entity].m_rotate[Y].f);
			store_u(tex_transform[Z], texture_space[i_entity].m_rotate[Z].f);
			store_u(tex_transform[W], texture_space[i_entity].m_rotate[W].f);
		}
	}
}




