#include "monster.h"
#include "vector.h"
#include "component.h"
#include "vector_math.h"
#include "input.h"
#include "setup.h"
#include "model.h"

static const __m128 Negate_Quaternion = set(-1.0f, -1.0f, -1.0f, 1.0f);

//======================================================================

/*
==================
==================
*/
void systems_::monster_::check_agro_radius(

	void* parameters, __int32 i_thread
)
{
	parameters_::monster_::check_agro_radius_* func_parameters = (parameters_::monster_::check_agro_radius_*)parameters;
	//const behaviour_manager_& behaviour_manager = *func_parameters->behaviour_manager;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	int3_ position_player;
	{
		component_fetch_ fetch;
		fetch.n_components = 2;
		fetch.component_ids[0] = component_id_::BASE;
		fetch.component_ids[1] = component_id_::CAMERA;
		fetch.n_excludes = 0;

		Populate_Fetch_Table(archetype_data, fetch);

		const __int32 i_archetype_index = 0;
		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		//const __int32 n_entities = archetype.n_entities;
		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < archetype.n_entities; i_entity++) {

			if (Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) {
				position_player = base[i_entity].position_fixed;
			}
		}
	}

	component_fetch_ fetch;
	fetch.n_components = 2;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::BASE;
	fetch.component_ids[1] = component_id_::BEHAVIOUR;
	Populate_Fetch_Table(archetype_data, fetch);

	const static int3_ agro_bb = { 300 * fixed_scale, 100 * fixed_scale, 300 * fixed_scale };

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
		component_::behaviour_* behaviour = (component_::behaviour_*)fetch.table[1][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			unsigned __int32 result = 0x1;
			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				__int32 delta = abs(position_player.i[i_axis] - base[i_entity].position_fixed.i[i_axis]);
				result &= delta < agro_bb.i[i_axis];
			}
			bool is_in_range = result != 0x0;
			behaviour_node_& behaviour_node = behaviour[i_entity].behaviour_nodes[behaviour[i_entity].i_node];
			bool is_passive = (behaviour_node.behaviour_id == component_::behaviour_::id_::STAND) || (behaviour_node.behaviour_id == component_::behaviour_::id_::MOVE_PATROL);

			behaviour[i_entity].trigger_id = is_in_range && is_passive ? component_::behaviour_::id_::ATTACK : behaviour[i_entity].trigger_id;
		}
	}
}

/*
==================
==================
*/
void systems_::monster_::update_patrol_points(

	void* parameters, __int32 i_thread
)
{
	parameters_::monster_::update_patrol_points_* func_parameters = (parameters_::monster_::update_patrol_points_*)parameters;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_::base_* base_patrol_point;
	{
		component_fetch_ fetch;
		fetch.n_components = 2;
		fetch.n_excludes = 0;
		fetch.component_ids[0] = component_id_::BASE;
		fetch.component_ids[1] = component_id_::PATROL_POINT;
		Populate_Fetch_Table(archetype_data, fetch);

		const __int32 i_archetype_index = 0;
		base_patrol_point = (component_::base_*)fetch.table[0][i_archetype_index];
	}

	component_fetch_ fetch;
	fetch.n_components = 3;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::BASE;
	fetch.component_ids[1] = component_id_::PATROL;
	fetch.component_ids[2] = component_id_::COLLIDER;
	Populate_Fetch_Table(archetype_data, fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
		component_::patrol_* patrol = (component_::patrol_*)fetch.table[1][i_archetype_index];
		component_::collider_* collider = (component_::collider_*)fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			const __int32 i_current = patrol[i_entity].i_current;
			const __int32 point_id = patrol[i_entity].point_ids[i_current];

			unsigned __int32 increment = 0x1;
			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				__int32 delta = abs(base_patrol_point[point_id].position_fixed.i[i_axis] - base[i_entity].position_fixed.i[i_axis]);
				increment &= delta < collider[i_entity].extent.i[i_axis];
			}
			patrol[i_entity].i_current = (patrol[i_entity].i_current + increment) % patrol[i_entity].n_points;
		}
	}
}

/*
========================================================================
========================================================================
*/
void systems_::monster_::face_target(

	void* parameters, __int32 i_thread
)
{
	parameters_::monster_::face_target_* func_parameters = (parameters_::monster_::face_target_*)parameters;
	const timer_& timer = *func_parameters->timer;
	const behaviour_manager_& behaviour_manager = *func_parameters->behaviour_manager;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	static const __m128 axis_angle = set(0.0f, 1.0f, 0.0f, 180.0f);
	static const float4_ q_zero = { 0.0f, 0.0f, 0.0f, 1.0f };
	float4_ q_180;
	store_u(Axis_Angle_To_Quaternion(axis_angle), q_180.f);
	static const __m128 angular_velocity = Axis_Angle_To_Quaternion(set(0.0f, 1.0f, 0.0, 1.5f));
	//static const float acceleration_gravity = -200.0f;
	//static const float max_vertical_speed = 400.0f;
	//static const float epsilon_angular = 0.05f;

	int3_ position_player;
	{
		component_fetch_ fetch;
		fetch.n_components = 2;
		fetch.component_ids[0] = component_id_::BASE;
		fetch.component_ids[1] = component_id_::CAMERA;
		fetch.n_excludes = 0;

		Populate_Fetch_Table(archetype_data, fetch);

		const __int32 i_archetype_index = 0;
		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		//const __int32 n_entities = archetype.n_entities;
		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < archetype.n_entities; i_entity++) {

			if (Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) {
				position_player = base[i_entity].position_fixed;
			}
		}
	}

	component_::base_* base_patrol_point;
	{
		component_fetch_ fetch;
		fetch.n_components = 2;
		fetch.component_ids[0] = component_id_::BASE;
		fetch.component_ids[1] = component_id_::PATROL_POINT;
		fetch.n_excludes = 0;

		Populate_Fetch_Table(archetype_data, fetch);

		const __int32 i_archetype_index = 0;
		base_patrol_point = (component_::base_*)fetch.table[0][i_archetype_index];
	}

	component_fetch_ fetch;
	fetch.n_components = 5;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::BASE;
	fetch.component_ids[1] = component_id_::PATROL;
	fetch.component_ids[2] = component_id_::MODEL_SPACE;
	fetch.component_ids[3] = component_id_::MODEL_SPACE_UPDATE;
	fetch.component_ids[4] = component_id_::BEHAVIOUR;

	Populate_Fetch_Table(archetype_data, fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
		component_::patrol_* patrol = (component_::patrol_*)fetch.table[1][i_archetype_index];
		component_::model_space_* model_space = (component_::model_space_*)fetch.table[2][i_archetype_index];
		component_::model_space_update_* model_space_update = (component_::model_space_update_*)fetch.table[3][i_archetype_index];
		component_::behaviour_* behaviour = (component_::behaviour_*)fetch.table[4][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			const __int32 i_current = patrol[i_entity].i_current;
			const __int32 point_id = patrol[i_entity].point_ids[i_current];
			int3_ position_patrol = base_patrol_point[point_id].position_fixed;
			//int3_ position_patrol = patrol[i_entity].points[i_current];

			behaviour_node_& behaviour_node = behaviour[i_entity].behaviour_nodes[behaviour[i_entity].i_node];
			const __int32 target_id = behaviour_manager.behaviour_data[behaviour_node.behaviour_id].target_id;

			int3_ position_target;
			bool is_target_player = target_id == component_::behaviour_::target_id_::PLAYER;
			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				//position_target.i[i_axis] = blend_int(position_player.i[i_axis], position_patrol.i[i_axis], is_target_player);
				position_target.i[i_axis] = is_target_player ? position_player.i[i_axis] : position_patrol.i[i_axis];
			}

			float3_ target_vector;
			target_vector.x = (float)(base[i_entity].position_fixed.x - position_target.x) * r_fixed_scale_real;
			target_vector.y = 0.0f;
			target_vector.z = (float)(base[i_entity].position_fixed.z - position_target.z) * r_fixed_scale_real;

			float4_ q_out;
			{
				const float3_ u = { 1.0f, 0.0f, 0.0f };
				float3_ v;
				Vector_Normalise(target_vector, v);
				float3_ temp;
				Cross_Product(u, v, temp);
				float4_ q_look;
				q_look.x = temp.x;
				q_look.y = temp.y;
				q_look.z = temp.z;
				q_look.w = Dot_Product(u, v) + 1.0f; // since input vectors normalised
				bool is_parallel = v.x > 0.999f;
				bool is_anti_parallel = v.x < -0.999f;

				for (__int32 i_axis = X; i_axis <= W; i_axis++) {
					//q_look.f[i_axis] = blend(q_zero.f[i_axis], q_look.f[i_axis], is_parallel);
					//q_look.f[i_axis] = blend(q_180.f[i_axis], q_look.f[i_axis], is_anti_parallel);
					q_look.f[i_axis] = is_parallel ? q_zero.f[i_axis] : q_look.f[i_axis];
					q_look.f[i_axis] = is_anti_parallel ? q_180.f[i_axis] : q_look.f[i_axis];
				}

				Normalise_Quaternion(q_look);
				float dot = Dot_Product_4D(model_space_update[i_entity].q_rotate, q_look);
				__m128i is_neg = set_all(dot) < set_zero();
				__m128 current = load_u(model_space_update[i_entity].q_rotate.f);
				__m128 next = load_u(q_look.f);
				current = blend(current * Negate_Quaternion, current, is_neg);
				float t = 8.0f * timer.delta_time;
				t = max(t, 0.0f);
				t = min(t, 1.0f);
				__m128 q_lerp = lerp(current, next, t);
				store_u(q_lerp, q_out.f);
				Normalise_Quaternion(q_out);
			}

			bool is_no_target = target_id == component_::behaviour_::target_id_::NULL_;
			for (__int32 i_axis = X; i_axis <= W; i_axis++) {
				//model_space_update[i_entity].q_rotate.f[i_axis] = blend(model_space_update[i_entity].q_rotate.f[i_axis], q_out.f[i_axis], is_no_target);
				model_space_update[i_entity].q_rotate.f[i_axis] = is_no_target ? model_space_update[i_entity].q_rotate.f[i_axis] : q_out.f[i_axis];
			}

			matrix temp;
			temp[X] = load_u(model_space_update[i_entity].q_rotate.f);
			matrix out_m[4];
			Quaternion_To_Matrix(temp, out_m);

			for (__int32 i = 0; i < 4; i++) {
				store_u(out_m[X][i], model_space[i_entity].m_rotate[i].f);
			}
		}
	}
}

/*
========================================================================
========================================================================
*/
void systems_::monster_::update_velocity(

	void* parameters, __int32 i_thread
)
{
	parameters_::monster_::update_velocity_* func_parameters = (parameters_::monster_::update_velocity_*)parameters;
	const timer_& timer = *func_parameters->timer;
	const behaviour_manager_& behaviour_manager = *func_parameters->behaviour_manager;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ fetch;
	fetch.n_components = 3;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::MOVE;
	fetch.component_ids[1] = component_id_::MODEL_SPACE;
	fetch.component_ids[2] = component_id_::BEHAVIOUR;

	Populate_Fetch_Table(archetype_data, fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::move_* move = (component_::move_*)fetch.table[0][i_archetype_index];
		component_::model_space_* model_space = (component_::model_space_*)fetch.table[1][i_archetype_index];
		component_::behaviour_* behaviour = (component_::behaviour_*)fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			static const float3_ unit_vector = { -1.0f, 0.0f, 0.0f };
			float3_ direction;
			Vector_X_Matrix(unit_vector, model_space[i_entity].m_rotate, direction);

			behaviour_node_& behaviour_node = behaviour[i_entity].behaviour_nodes[behaviour[i_entity].i_node];
			const behaviour_data_& behaviour_data = behaviour_manager.behaviour_data[behaviour_node.behaviour_id];
			float3_ horizontal_velocity = move[i_entity].velocity;
			horizontal_velocity.y = 0.0f;
			float3_ horizontal_direction;
			float speed = Vector_Normalise_Magnitude(horizontal_velocity, horizontal_direction);

			float acceleration = (speed < behaviour_data.max_speed) ? behaviour_data.move_acceleration : 0.0f;

			move[i_entity].velocity.x += direction.x * acceleration * timer.delta_time;
			move[i_entity].velocity.z += direction.z * acceleration * timer.delta_time;
			move[i_entity].max_speed = behaviour_data.max_speed;

			//printf_s("%f , %f, %f \n", move[i_entity].velocity.x, move[i_entity].velocity.y, move[i_entity].velocity.z);
		}
	}
}

/*
========================================================================
========================================================================
*/
void systems_::monster_::process_health(

	void* parameters, __int32 i_thread

)
{
	parameters_::monster_::process_effects_* func_parameters = (parameters_::monster_::process_effects_*)parameters;
	const timer_& timer = *func_parameters->timer;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ fetch;
	fetch.n_components = 2;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::BEHAVIOUR;
	fetch.component_ids[1] = component_id_::HEALTH;

	Populate_Fetch_Table(archetype_data, fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::behaviour_* behaviour = (component_::behaviour_*)fetch.table[0][i_archetype_index];
		component_::health_* health = (component_::health_*)fetch.table[1][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			bool hp_is_gone = health[i_entity].hp <= 0;
			behaviour[i_entity].trigger_id = hp_is_gone ? component_::behaviour_::id_::DEATH : behaviour[i_entity].trigger_id;
			health[i_entity].hp = hp_is_gone ? 8 : health[i_entity].hp;
		}
	}
}

/*
========================================================================
========================================================================
*/
void systems_::monster_::process_effects(

	void* parameters, __int32 i_thread

)
{
	parameters_::monster_::process_effects_* func_parameters = (parameters_::monster_::process_effects_*)parameters;
	const timer_& timer = *func_parameters->timer;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ fetch;
	fetch.n_components = 5;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::BASE;
	fetch.component_ids[1] = component_id_::ANIMATION;
	fetch.component_ids[2] = component_id_::EFFECT;
	fetch.component_ids[3] = component_id_::TEXTURE_BLEND;
	fetch.component_ids[4] = component_id_::MASS;

	Populate_Fetch_Table(archetype_data, fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
		component_::animation_* animation = (component_::animation_*)fetch.table[1][i_archetype_index];
		component_::effect_* effect = (component_::effect_*)fetch.table[2][i_archetype_index];
		component_::texture_blend_* texture_blend = (component_::texture_blend_*)fetch.table[3][i_archetype_index];
		component_::mass_* mass = (component_::mass_*)fetch.table[4][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			bool is_shrink_triggered = effect[i_entity].trigger_id == component_::item_::type_::SHRINK;
			bool is_petrify_triggered = effect[i_entity].trigger_id == component_::item_::type_::PETRIFY;

			effect[i_entity].trigger_id = component_::item_::type_::NULL_;

			mass[i_entity].value = mass[i_entity].default_value;
			{
				const float shrink_rate = 0.04f;
				effect[i_entity].shrink.timer = is_shrink_triggered ? effect[i_entity].shrink.duration : effect[i_entity].shrink.timer;
				effect[i_entity].shrink.timer -= timer.delta_time;
				effect[i_entity].shrink.timer = max(effect[i_entity].shrink.timer, 0.0f);
				effect[i_entity].shrink.t_interval += effect[i_entity].shrink.timer > 0.0f ? shrink_rate : -shrink_rate;
				effect[i_entity].shrink.t_interval = max(0.0f, effect[i_entity].shrink.t_interval);
				effect[i_entity].shrink.t_interval = min(1.0f, effect[i_entity].shrink.t_interval);

				float t_smooth = Smooth_Step(0.0f, 1.0f, effect[i_entity].shrink.t_interval);
				float3_ delta = effect[i_entity].shrink.shrunk_scale - effect[i_entity].shrink.base_scale;
				base[i_entity].scale = effect[i_entity].shrink.base_scale + (delta * t_smooth);

				static const float quake_floor_level = -24.0f;
				animation[i_entity].origin.y = quake_floor_level * delta.y * t_smooth;
				animation[i_entity].frame_speed_modifier = 1.0f + t_smooth;

				const float base_mass = 1.0f;
				const float target_mass = 0.5f;
				const float delta_mass = target_mass - base_mass;
				mass[i_entity].value *= base_mass + (delta_mass * t_smooth);
			}
			{
				effect[i_entity].petrify.timer += timer.delta_time;
				effect[i_entity].petrify.timer = min(effect[i_entity].petrify.timer, effect[i_entity].petrify.time_limit);
				//effect[i_entity].petrify.timer = blend(0.0f, effect[i_entity].petrify.timer, is_petrify_triggered);
				effect[i_entity].petrify.timer = is_petrify_triggered ? 0.0f : effect[i_entity].petrify.timer;
				bool is_running = effect[i_entity].petrify.timer < effect[i_entity].petrify.time_limit;

				const float petrify_rate = 0.02f;
				//effect[i_entity].petrify.t_interval += blend(petrify_rate, -petrify_rate, is_running);
				effect[i_entity].petrify.t_interval += is_running ? petrify_rate : -petrify_rate;
				effect[i_entity].petrify.t_interval = max(0.0f, effect[i_entity].petrify.t_interval);
				effect[i_entity].petrify.t_interval = min(1.0f, effect[i_entity].petrify.t_interval);

				float t_smooth = Smooth_Step(0.0f, 1.0f, effect[i_entity].petrify.t_interval);
				float delta = effect[i_entity].petrify.end_time_modifier - effect[i_entity].petrify.begin_time_modifier;
				animation[i_entity].frame_speed_modifier = effect[i_entity].petrify.begin_time_modifier + (delta * t_smooth);

				texture_blend[i_entity].interval = effect[i_entity].petrify.t_interval;

				const float base_mass = 1.0f;
				const float target_mass = 2.0f;
				const float delta_mass = target_mass - base_mass;
				mass[i_entity].value *= base_mass + (delta_mass * t_smooth);
			}
		}
	}
}

/*
========================================================================
========================================================================
*/
void systems_::monster_::fire_projectile(

	void* parameters, __int32 i_thread

)
{
	parameters_::monster_::fire_projectile_* func_parameters = (parameters_::monster_::fire_projectile_*)parameters;
	archetype_data_& archetype_data = *func_parameters->archetype_data;
	model_manager_& model_manager = *func_parameters->model_manager;

	int3_ position_player;
	{
		component_fetch_ fetch;
		fetch.n_components = 2;
		fetch.component_ids[0] = component_id_::BASE;
		fetch.component_ids[1] = component_id_::CAMERA;
		fetch.n_excludes = 0;

		Populate_Fetch_Table(archetype_data, fetch);

		const __int32 i_archetype_index = 0;
		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;
		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < archetype.n_entities; i_entity++) {

			if (Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) {
				position_player = base[i_entity].position_fixed;
			}
		}
	}

	__int32 i_archetype_projectile;
	component_::base_* base_projectile;
	component_::move_* move_projectile;
	component_::colour_* colour_projectile;
	{
		component_fetch_ fetch;
		fetch.n_components = 4;
		fetch.component_ids[0] = component_id_::BASE;
		fetch.component_ids[1] = component_id_::MOVE;
		fetch.component_ids[2] = component_id_::COLOUR;
		fetch.component_ids[3] = component_id_::PROJECTILE_ID;
		fetch.n_excludes = 0;

		Populate_Fetch_Table(archetype_data, fetch);

		const __int32 i_archetype_index = 0;
		base_projectile = (component_::base_*)fetch.table[0][i_archetype_index];
		move_projectile = (component_::move_*)fetch.table[1][i_archetype_index];
		colour_projectile = (component_::colour_*)fetch.table[2][i_archetype_index];
		i_archetype_projectile = fetch.i_archetypes[i_archetype_index];
	}


	//fetch_ fetch;
	//fetch.n_components = 6;
	//fetch.n_components_excluded = 0;
	//fetch.component_ids[0] = component_id_::BASE;
	//fetch.component_ids[1] = component_id_::PROJECTILE;
	//fetch.component_ids[2] = component_id_::BEHAVIOUR;
	//fetch.component_ids[3] = component_id_::DRAW;
	//fetch.component_ids[4] = component_id_::MODEL_SPACE;
	//fetch.component_ids[5] = component_id_::ANIMATE;
	//fetch.i_components[0] = 0;
	//fetch.i_components[1] = 0;
	//fetch.i_components[2] = 0;
	//fetch.i_components[3] = 0;
	//fetch.i_components[4] = 0;
	//fetch.i_components[5] = 0;

	matrix_ m_scale;
	matrix_ m_translate;
	memset(m_scale, 0, sizeof(matrix_));
	memset(m_translate, 0, sizeof(matrix_));

	m_translate[X].x = 1.0f;
	m_translate[Y].y = 1.0f;
	m_translate[Z].z = 1.0f;
	m_translate[W].w = 1.0f;

	m_scale[W].w = 1.0f;

	component_fetch_ fetch;
	fetch.n_components = 7;
	fetch.n_excludes = 0;
	fetch.component_ids[0] = component_id_::BASE;
	fetch.component_ids[1] = component_id_::PROJECTILE;
	fetch.component_ids[2] = component_id_::BEHAVIOUR;
	fetch.component_ids[3] = component_id_::MODEL;
	fetch.component_ids[4] = component_id_::MODEL_SPACE;
	fetch.component_ids[5] = component_id_::ANIMATION;
	fetch.component_ids[6] = component_id_::COLOUR;

	Populate_Fetch_Table(archetype_data, fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)fetch.table[0][i_archetype_index];
		component_::projectile_* projectile = (component_::projectile_*)fetch.table[1][i_archetype_index];
		component_::behaviour_* behaviour = (component_::behaviour_*)fetch.table[2][i_archetype_index];
		component_::model_* model_component = (component_::model_*)fetch.table[3][i_archetype_index];
		component_::model_space_* model_space = (component_::model_space_*)fetch.table[4][i_archetype_index];
		component_::animation_* animation = (component_::animation_*)fetch.table[5][i_archetype_index];
		component_::colour_* colour = (component_::colour_*)fetch.table[6][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, fetch.component_masks, archetype)) { continue; };

			const model_& model = model_manager.model[model_component[i_entity].id];

			m_scale[X].x = base[i_entity].scale.x;
			m_scale[Y].y = base[i_entity].scale.y;
			m_scale[Z].z = base[i_entity].scale.z;

			m_translate[W].x = (float)(base[i_entity].position_fixed.x) * r_fixed_scale_real;
			m_translate[W].y = (float)(base[i_entity].position_fixed.y) * r_fixed_scale_real;
			m_translate[W].z = (float)(base[i_entity].position_fixed.z) * r_fixed_scale_real;

			m_translate[W].x -= model.bounding_origin.x + animation[i_entity].origin.x;
			m_translate[W].y -= model.bounding_origin.y + animation[i_entity].origin.y;
			m_translate[W].z -= model.bounding_origin.z + animation[i_entity].origin.z;

			matrix_ m_intermediate;
			matrix_ m_transform;
			Matrix_X_Matrix(model_space[i_entity].m_rotate, m_scale, m_intermediate);
			Matrix_X_Matrix(m_translate, m_intermediate, m_transform);

			const __int32 i_vertices[] = {

				projectile[i_entity].model_data.i_vertex_left,
				projectile[i_entity].model_data.i_vertex_right,
			};

			float3_ vertices_animated[2];
			for (__int32 i = 0; i < 2; i++) {

				float3_ vertices_frame[2];
				vertices_frame[0] = model.vertices_frame[animation[i_entity].i_frames[component_::animation_::CURRENT_FRAME_LOW]][i_vertices[i]];
				vertices_frame[1] = model.vertices_frame[animation[i_entity].i_frames[component_::animation_::CURRENT_FRAME_HI]][i_vertices[i]];

				for (__int32 i_axis = X; i_axis < W; i_axis++) {
					vertices_animated[i].f[i_axis] = vertices_frame[0].f[i_axis] + ((vertices_frame[1].f[i_axis] - vertices_frame[0].f[i_axis]) * animation[i_entity].frame_interval);
				}
			}

			float3_ mid_vertex;
			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				mid_vertex.f[i_axis] = vertices_animated[0].f[i_axis] + ((vertices_animated[1].f[i_axis] - vertices_animated[0].f[i_axis]) * 0.5f);
			}

			mid_vertex += projectile[i_entity].model_data.displacement;

			float3_ t_vertex;
			Vector_X_Matrix(mid_vertex, m_transform, t_vertex);

			bool is_fired = (animation[i_entity].i_frames[component_::animation_::CURRENT_FRAME_LOW] == projectile[i_entity].model_data.i_firing_frame) && animation[i_entity].is_frame_change;

			int3_ projectile_position;
			float3_ temp;
			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				projectile_position.i[i_axis] = (__int32)(t_vertex.f[i_axis] * fixed_scale_real);
				temp.f[i_axis] = (float)(position_player.i[i_axis] - projectile_position.i[i_axis]) * r_fixed_scale_real;
			}

			float3_ firing_vector;
			Vector_Normalise(temp, firing_vector);

			const __int32 i_projectile = projectile[i_entity].i_begin + projectile[i_entity].i_projectile;
			base_projectile[i_projectile].position_fixed = projectile_position;
			Clear_Component_Bit(i_archetype_projectile, i_projectile, component_id_::COLLIDER, archetype_data);
			colour_projectile[i_projectile].colour = colour[i_entity].colour;

			if (is_fired) {

				move_projectile[i_projectile].velocity = firing_vector * 400.0f;
				projectile[i_entity].i_projectile = (projectile[i_entity].i_projectile + 1) % projectile[i_entity].n_projectiles;
				Set_Component_Bit(i_archetype_projectile, i_projectile, component_id_::COLLIDER, archetype_data);
			}
		}
	}
}


/*
========================================================================
========================================================================
*/
void systems_::monster_::update_fly(

	void* parameters, __int32 i_thread
)
{
	parameters_::monster_::update_fly_* func_parameters = (parameters_::monster_::update_fly_*)parameters;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ component_fetch;
	component_fetch.n_components = 4;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::BASE;
	component_fetch.component_ids[1] = component_id_::MOVE;
	component_fetch.component_ids[2] = component_id_::BEHAVIOUR;
	component_fetch.component_ids[3] = component_id_::FLY;

	Populate_Fetch_Table(archetype_data, component_fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)component_fetch.table[0][i_archetype_index];
		component_::move_* move = (component_::move_*)component_fetch.table[1][i_archetype_index];
		component_::behaviour_* behaviour = (component_::behaviour_*)component_fetch.table[2][i_archetype_index];
		component_::fly_* fly = (component_::fly_*)component_fetch.table[3][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			bool is_rising = base[i_entity].position_fixed.y < fly[i_entity].ceiling_fixed;
			base[i_entity].position_fixed.y = min(base[i_entity].position_fixed.y, fly[i_entity].ceiling_fixed);

			const __int32 behaviour_id = behaviour[i_entity].behaviour_nodes[behaviour[i_entity].i_node].behaviour_id;
			//bool is_falling = (behaviour_id == component_::behaviour_::id_::DEATH) || (behaviour_id == component_::behaviour_::id_::DEAD);
			//move[i_entity].velocity.y = blend(fly[i_entity].acceleration_rise, 0.0f, is_rising);
			move[i_entity].velocity.y = is_rising ? fly[i_entity].acceleration_rise : 0.0f;
		}
	}
}


/*
==================
==================
*/
void systems_::monster_::update_behaviour(

	void* parameters, __int32 i_thread
)
{
	parameters_::monster_::update_behaviour_* func_parameters = (parameters_::monster_::update_behaviour_*)parameters;
	const timer_& timer = *func_parameters->timer;
	behaviour_manager_& behaviour_manager = *func_parameters->behaviour_manager;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ component_fetch;
	component_fetch.n_components = 2;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::BEHAVIOUR;
	component_fetch.component_ids[1] = component_id_::ANIMATION;

	Populate_Fetch_Table(archetype_data, component_fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::behaviour_* behaviour = (component_::behaviour_*)component_fetch.table[0][i_archetype_index];
		component_::animation_* animation = (component_::animation_*)component_fetch.table[1][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			behaviour_node_& behaviour_node = behaviour[i_entity].behaviour_nodes[behaviour[i_entity].i_node];
			const behaviour_data_& behaviour_data = behaviour_manager.behaviour_data[behaviour_node.behaviour_id];
			behaviour_node.timer += timer.delta_time;

			__int32 i_node_current = behaviour[i_entity].i_node;
			bool is_timed_behaviour = behaviour_data.time_duration != 0.0f;
			{
				bool is_expired = (behaviour_node.timer > behaviour_data.time_duration) && is_timed_behaviour;
				//behaviour[i_entity].i_node = blend_int(behaviour_node.i_next_node, behaviour[i_entity].i_node, is_expired);
				behaviour[i_entity].i_node = is_expired ? behaviour_node.i_next_node : behaviour[i_entity].i_node;
				//behaviour_node.timer = blend(0.0f, behaviour_node.timer, is_expired);
				behaviour_node.timer = is_expired ? 0.0f : behaviour_node.timer;
			}
			{
				bool is_end_animation = (!is_timed_behaviour) && animation[i_entity].is_end_animation;
				//behaviour[i_entity].i_node = blend_int(behaviour_node.i_next_node, behaviour[i_entity].i_node, is_end_animation);
				behaviour[i_entity].i_node = is_end_animation ? behaviour_node.i_next_node : behaviour[i_entity].i_node;
			}
			{
				__int32 i_node_match = INVALID_RESULT;
				for (__int32 i_node = 0; i_node < behaviour[i_entity].n_nodes; i_node++) {
					__int32 behaviour_id = behaviour[i_entity].behaviour_nodes[i_node].behaviour_id;
					bool is_match = behaviour_id == behaviour[i_entity].trigger_id;
					//i_node_match = blend_int(i_node, i_node_match, is_match);
					i_node_match = is_match ? i_node : i_node_match;
				}
				bool is_match = i_node_match != INVALID_RESULT;
				//behaviour[i_entity].i_node = blend_int(i_node_match, behaviour[i_entity].i_node, is_match);
				behaviour[i_entity].i_node = is_match ? i_node_match : behaviour[i_entity].i_node;
				behaviour[i_entity].trigger_id = component_::behaviour_::id_::NULL_;
			}

			bool is_new_behaviour = i_node_current != behaviour[i_entity].i_node;
			const __int32 behaviour_id = behaviour[i_entity].behaviour_nodes[behaviour[i_entity].i_node].behaviour_id;
			const __int32 animation_id = behaviour_manager.behaviour_data[behaviour_id].animation_id;
			//animation[i_entity].trigger_id = blend_int(animation_id, animation[i_entity].trigger_id, is_new_behaviour);
			animation[i_entity].trigger_id = is_new_behaviour ? animation_id : animation[i_entity].trigger_id;
			//behaviour[i_entity].state_trigger = blend_int(behaviour_id, component_::behaviour_::id_::NULL_, is_new_behaviour);
			behaviour[i_entity].state_trigger = is_new_behaviour ? behaviour_id : component_::behaviour_::id_::NULL_;

			{
				__int32 i_nodes[2];
				i_nodes[0] = i_node_current;
				i_nodes[1] = behaviour[i_entity].i_node;
				//__int32 n_nodes = blend_int(2, 0, is_new_behaviour);
				__int32 n_nodes = is_new_behaviour ? 2 : 0;
				for (__int32 i_node = 0; i_node < n_nodes; i_node++) {

					const __int32 behaviour_id = behaviour[i_entity].behaviour_nodes[i_nodes[i_node]].behaviour_id;
					__int32 n_components = behaviour_manager.behaviour_data[behaviour_id].component_toggle.n_components;
					for (__int32 i_component = 0; i_component < n_components; i_component++) {

						const __int32 component_id = behaviour_manager.behaviour_data[behaviour_id].component_toggle.component_ids[i_component];
						const __int32 group_id = component_id / component_data_::NUM_BITS_INTEGER;
						const __int32 group_index = component_id - (group_id * component_data_::NUM_BITS_INTEGER);
						unsigned __int32 and_mask = archetype.component_masks[group_id] & (0x1 << group_index);
						archetype.entity_component_masks[i_entity][group_id] ^= and_mask;
					}
				}
			}
		}
	}
}