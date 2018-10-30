#include "environment.h"
#include "master.h"
#include "component.h"
#include "input.h"
#include "vector_math.h"

static const float close_speed_g = 4.0f;

/*
==================
==================
*/
void systems_::environment_::update_clouds(

	void* parameters, __int32 i_thread

)
{

	parameters_::environment_::update_clouds_* func_parameters = (parameters_::environment_::update_clouds_*)parameters;
	const matrix& m_rotate_in = *func_parameters->m_rotate;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ fetch;
	fetch.n_components = 3;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::BASE;
	fetch.component_ids[1] = component_id_::COLOUR;
	fetch.component_ids[2] = component_id_::CLOUD;

	Populate_Fetch_Table(archetype_data, fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
		component_::colour_* colour = (component_::colour_*)fetch.table[1][i_archetype_index];
		component_::cloud_* cloud = (component_::cloud_*)fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			matrix_ m_rotate;
			store_u(m_rotate_in[X], m_rotate[X].f);
			store_u(m_rotate_in[Y], m_rotate[Y].f);
			store_u(m_rotate_in[Z], m_rotate[Z].f);
			store_u(m_rotate_in[W], m_rotate[W].f);

			float3_ temp;
			Vector_X_Matrix(cloud[i_entity].seed_scale, m_rotate, temp);
			float scale_factor = temp.x;
			scale_factor = max(0.0f, scale_factor);
			scale_factor = min(1.0f, scale_factor);

			for (__int32 i = X; i < W; i++) {

				base[i_entity].scale.f[i] = cloud[i_entity].base_scale.f[i] * scale_factor;
				colour[i_entity].colour.f[i] = cloud[i_entity].base_colour.f[i] * scale_factor;
			}
		}
	}
}


/*
==================
==================
*/
void systems_::environment_::update_lava(

	void* parameters, __int32 i_thread

)

{
	parameters_::environment_::update_lava_* func_parameters = (parameters_::environment_::update_lava_*)parameters;
	const matrix& m_rotate_in = *func_parameters->m_rotate;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	const float scale = 10.0f;

	component_fetch_ fetch;
	fetch.n_components = 3;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::BASE;
	fetch.component_ids[1] = component_id_::COLOUR;
	fetch.component_ids[2] = component_id_::LAVA;

	Populate_Fetch_Table(archetype_data, fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
		component_::colour_* colour = (component_::colour_*)fetch.table[1][i_archetype_index];
		component_::lava_* lava = (component_::lava_*)fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			matrix_ m_rotate;
			store_u(m_rotate_in[X], m_rotate[X].f);
			store_u(m_rotate_in[Y], m_rotate[Y].f);
			store_u(m_rotate_in[Z], m_rotate[Z].f);
			store_u(m_rotate_in[W], m_rotate[W].f);

			float3_ temp;
			Vector_X_Matrix(lava[i_entity].base_distance, m_rotate, temp);
			float unit_distance = temp.x;
			unit_distance = max(-1.0f, unit_distance);
			unit_distance = min(1.0f, unit_distance);
			float distance = unit_distance * scale;
			base[i_entity].position_fixed.y = lava[i_entity].origin.y + (__int32)(distance * fixed_scale_real);

			for (__int32 i = X; i < W; i++) {

				colour[i_entity].colour.f[i] = lava[i_entity].base_colour.f[i] * unit_distance;
			}
		}
	}
}


/*
==================
==================
*/
void systems_::environment_::update_doors(

	void* parameters, __int32 i_thread

)
{
	parameters_::environment_::update_doors_* func_parameters = (parameters_::environment_::update_doors_*)parameters;
	const timer_& timer = *func_parameters->timer;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ fetch;
	fetch.n_components = 3;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::BASE;
	fetch.component_ids[1] = component_id_::DOOR;
	fetch.component_ids[2] = component_id_::TRIGGER;

	Populate_Fetch_Table(archetype_data, fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
		component_::door_* door = (component_::door_*)fetch.table[1][i_archetype_index];
		component_::trigger_* trigger = (component_::trigger_*)fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			//float move_speed = blend(-close_speed_g, close_speed_g, trigger[i_entity].is_triggered);
			float move_speed = trigger[i_entity].is_triggered ? -close_speed_g : close_speed_g;

			door[i_entity].interval += timer.delta_time * move_speed;
			door[i_entity].interval = max(door[i_entity].interval, 0.0f);
			door[i_entity].interval = min(door[i_entity].interval, 1.0f);

			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				__int32 delta = (__int32)(door[i_entity].move.f[i_axis] * door[i_entity].interval * fixed_scale_real);
				base[i_entity].position_fixed.i[i_axis] = door[i_entity].position_default.i[i_axis] + delta;
			}

			bool is_moving = (door[i_entity].interval > 0.0f) && (door[i_entity].interval < 1.0f);
			//__int32 new_state = blend_int(component_::door_::state_::MOVE, component_::door_::state_::STATIC, is_moving);
			__int32 new_state = is_moving ? component_::door_::state_::MOVE : component_::door_::state_::STATIC;
			bool is_state_change = new_state != door[i_entity].state;
			//door[i_entity].state_trigger = blend_int(new_state, component_::door_::state_::NULL_, is_state_change);
			door[i_entity].state_trigger = is_state_change ? new_state : component_::door_::state_::NULL_;
			door[i_entity].state = new_state;
		}
	}
}


/*
==================
==================
*/
void systems_::environment_::update_platforms(


	void* parameters, __int32 i_thread

)
{
	parameters_::environment_::update_platforms_* func_parameters = (parameters_::environment_::update_platforms_*)parameters;
	const timer_& timer = *func_parameters->timer;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ component_fetch;
	component_fetch.n_components = 6;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::BASE;
	component_fetch.component_ids[1] = component_id_::MOVE;
	component_fetch.component_ids[2] = component_id_::PLATFORM;
	component_fetch.component_ids[3] = component_id_::POWER;
	component_fetch.component_ids[4] = component_id_::ATTACHED;
	component_fetch.component_ids[5] = component_id_::TEXTURE_BLEND;

	Populate_Fetch_Table(archetype_data, component_fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)component_fetch.table[0][i_archetype_index];
		component_::move_* move = (component_::move_*)component_fetch.table[1][i_archetype_index];
		component_::platform_* platform = (component_::platform_*)component_fetch.table[2][i_archetype_index];
		component_::power_* power = (component_::power_*)component_fetch.table[3][i_archetype_index];
		component_::attached_* attached = (component_::attached_*)component_fetch.table[4][i_archetype_index];
		component_::texture_blend_* texture_blend = (component_::texture_blend_*)component_fetch.table[5][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			const float force_down = -0.2f;

			{
				bool is_large_mass = platform[i_entity].contact_mass > 40.0f;		// shambler default mass
				platform[i_entity].contact_mass = 0.0f;
				platform[i_entity].intervals.f[Y] += power[i_entity].is_on && is_large_mass ? force_down * timer.delta_time : 0.0f;

				for (__int32 i_axis = X; i_axis < W; i_axis++) {
					platform[i_entity].intervals.f[i_axis] += power[i_entity].is_on ? platform[i_entity].speeds.f[i_axis] * timer.delta_time : 0.0f;
					platform[i_entity].intervals.f[i_axis] = min(platform[i_entity].intervals.f[i_axis], 1.0f);
					platform[i_entity].intervals.f[i_axis] = max(platform[i_entity].intervals.f[i_axis], 0.0f);
				}

				const __int32 i_axis_travel = platform[i_entity].axis_travel;
				bool is_limit = (platform[i_entity].intervals.f[i_axis_travel] == 1.0f) || (platform[i_entity].intervals.f[i_axis_travel] == 0.0f);
				platform[i_entity].speeds.f[i_axis_travel] *= is_limit ? -1.0f : 1.0f;

				int3_ position;
				{
					for (__int32 i_axis = X; i_axis < W; i_axis++) {
						__int64 interval = (__int32)(platform[i_entity].intervals.f[i_axis] * fixed_scale_real);
						__int64 delta = platform[i_entity].endpoints[1].i[i_axis] - platform[i_entity].endpoints[0].i[i_axis];
						__int64 increment = (interval * delta) / fixed_scale;
						position.i[i_axis] = platform[i_entity].endpoints[0].i[i_axis] + (__int32)increment;
					}
				}

				for (__int32 i_axis = X; i_axis < W; i_axis++) {
					move[i_entity].displacement.f[i_axis] = (float)(position.i[i_axis] - platform[i_entity].prev_position.i[i_axis]) * r_fixed_scale_real;
				}
				platform[i_entity].prev_position = position;

			}


			platform[i_entity].blend.t_interval += timer.delta_time * platform[i_entity].blend.blend_speed;
			platform[i_entity].blend.t_interval = min(1.0f, platform[i_entity].blend.t_interval);
			bool is_at_limit = platform[i_entity].blend.t_interval == 1.0f;
			//platform[i_entity].blend.t_interval = blend(-1.0f, platform[i_entity].blend.t_interval, is_at_limit);
			platform[i_entity].blend.t_interval = is_at_limit ? -1.0f : platform[i_entity].blend.t_interval;
			float t_smooth = Smooth_Step(0.0f, 1.0f, abs(platform[i_entity].blend.t_interval));

			texture_blend[i_entity].interval = t_smooth;

			//{
			//	if (attached[i_entity].is_attachment) {

			//		__int32 i_archetype_attached = attached[i_entity].i_archetype;
			//		__int32 i_entity_attached = attached[i_entity].i_entity;
			//		__int32 component_id_base = component_id_::BASE;
			//		component_::base_* base_attached = (component_::base_*)Find_Archetype_Component(i_archetype_attached, component_id_base, archetype_data);
			//		__int32 component_id_move = component_id_::MOVE;
			//		component_::move_* move_attached = (component_::move_*)Find_Archetype_Component(i_archetype_attached, component_id_move, archetype_data);
			//		move_attached[i_entity_attached].displacement.x = move[i_entity].displacement.x;
			//		move_attached[i_entity_attached].displacement.y = move[i_entity].displacement.y;
			//		move_attached[i_entity_attached].displacement.z = move[i_entity].displacement.z;

			//	}
			//}
		}
	}
}

/*
==================
==================
*/
void systems_::environment_::update_switches(

	void* parameters, __int32 i_thread

)
{
	parameters_::environment_::update_switches_* func_parameters = (parameters_::environment_::update_switches_*)parameters;
	const timer_& timer = *func_parameters->timer;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ fetch;
	fetch.n_components = 5;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::BASE;
	fetch.component_ids[1] = component_id_::MOVE;
	fetch.component_ids[2] = component_id_::COLOUR;
	fetch.component_ids[3] = component_id_::PLATE;
	fetch.component_ids[4] = component_id_::SWITCH;

	Populate_Fetch_Table(archetype_data, fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
		component_::move_* move = (component_::move_*)fetch.table[1][i_archetype_index];
		component_::colour_* colour = (component_::colour_*)fetch.table[2][i_archetype_index];
		component_::plate_* plate = (component_::plate_*)fetch.table[3][i_archetype_index];
		component_::switch_* switch_ = (component_::switch_*)fetch.table[4][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			float t_increment = switch_[i_entity].is_on ? timer.delta_time : -timer.delta_time;
			plate[i_entity].t_interval += t_increment;
			plate[i_entity].t_interval = max(plate[i_entity].t_interval, 0.0f);
			plate[i_entity].t_interval = min(plate[i_entity].t_interval, 1.0f);
			t_increment = (plate[i_entity].t_interval == 0.0f) || (plate[i_entity].t_interval == 1.0f) ? 0.0f : t_increment;

			//__int64 t_int = (__int64)(abs(plate[i_entity].t_interval) * fixed_scale_real);
			__int64 t_int = (__int64)(t_increment * fixed_scale_real);
			int3_ position;
			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				__int64 delta = plate[i_entity].positions[1].i[i_axis] - plate[i_entity].positions[0].i[i_axis];
				__int64 temp = (delta * t_int) / fixed_scale;
				//position.i[i_axis] = plate[i_entity].positions[0].i[i_axis] + (__int32)temp;
				position.i[i_axis] = (__int32)temp;
			}

			//printf_s("POSITION: %i, %i, %i \n", base[i_entity].position_fixed.x, base[i_entity].position_fixed.y, base[i_entity].position_fixed.z);
			//printf_s("DISPLACMENT: %f, %f, %f \n", move[i_entity].displacement.x, move[i_entity].displacement.y, move[i_entity].displacement.z);


			base[i_entity].position_fixed.x += position.x;
			base[i_entity].position_fixed.y += position.y;
			base[i_entity].position_fixed.z += position.z;

			colour[i_entity].colour = plate[i_entity].colours[0] + ((plate[i_entity].colours[1] - plate[i_entity].colours[0]) * plate[i_entity].t_interval);
			plate[i_entity].sound_trigger = switch_[i_entity].is_on != switch_[i_entity].was_on;
			switch_[i_entity].was_on = switch_[i_entity].is_on;
			switch_[i_entity].is_on = switch_[i_entity].is_hold ? false : switch_[i_entity].is_on;
		}
	}
}

/*
==================
==================
*/
void systems_::environment_::update_buttons(

	void* parameters, __int32 i_thread

)
{
	parameters_::environment_::update_buttons_* func_parameters = (parameters_::environment_::update_buttons_*)parameters;
	const timer_& timer = *func_parameters->timer;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ fetch;
	fetch.n_components = 3;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::COLOUR;
	fetch.component_ids[1] = component_id_::BUTTON;
	fetch.component_ids[2] = component_id_::SWITCH;

	Populate_Fetch_Table(archetype_data, fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::colour_* colour = (component_::colour_*)fetch.table[0][i_archetype_index];
		component_::button_* button = (component_::button_*)fetch.table[1][i_archetype_index];
		component_::switch_* switch_ = (component_::switch_*)fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			colour[i_entity].colour = switch_[i_entity].is_on ? button[i_entity].colours[0] : button[i_entity].colours[1];
			button[i_entity].sound_trigger = switch_[i_entity].is_on != switch_[i_entity].was_on;
			switch_[i_entity].was_on = switch_[i_entity].is_on;
			switch_[i_entity].is_on = switch_[i_entity].is_hold ? false : switch_[i_entity].is_on;
		}
	}
}


