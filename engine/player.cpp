#include "player.h"
#include "component.h"
#include "input.h"
#include "vector_math.h"
#include "setup.h"
#include "render_front.h"


static const float impulse_jump = 300.0f;
static const float max_ground_speed = 80.0f;
static const float max_air_speed = 1200.0f;

//======================================================================



/*
==================
==================
*/
void systems_::player_::model_select(

	void* parameters, __int32 i_thread
)
{
	parameters_::player_::model_select_* func_parameters = (parameters_::player_::model_select_*)parameters;
	user_input_ const& user_input = *func_parameters->user_input;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ component_fetch;
	component_fetch.n_components = 2;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::BASE;
	component_fetch.component_ids[1] = component_id_::CAMERA;

	Populate_Fetch_Table(archetype_data, component_fetch);

	component_::base_* base = (component_::base_*)component_fetch.table[0][0];
	component_::camera_* camera = (component_::camera_*)component_fetch.table[1][0];

	__int32 i_current = Return_Camera_Component(archetype_data, &camera);

	bool result[4];
	result[0] = (user_input.input_mask & (0x1 << user_input_::PLAYER_ONE)) != 0x0;
	result[1] = (user_input.input_mask & (0x1 << user_input_::PLAYER_TWO)) != 0x0;
	result[2] = (user_input.input_mask & (0x1 << user_input_::PLAYER_THREE)) != 0x0;
	result[3] = (user_input.input_mask & (0x1 << user_input_::PLAYER_FOUR)) != 0x0;

	__int32 i_new = i_current;
	i_new = blend_int(0, i_new, result[0]);
	i_new = blend_int(1, i_new, result[1]);
	i_new = blend_int(2, i_new, result[2]);
	i_new = blend_int(3, i_new, result[3]);

	camera[i_new].is_stepped = i_new != i_current;

	for (__int32 i_axis = X; i_axis < W; i_axis++) {

		__int32 current_position = base[i_current].position_fixed.i[i_axis] + camera[i_current].offset.i[i_axis];
		camera[i_new].previous_position.i[i_axis] = blend_int(current_position, camera[i_new].previous_position.i[i_axis], camera[i_new].is_stepped);
	}

	__int32 i_archetype = component_fetch.i_archetypes[0];
	Toggle_Component_Bit(i_archetype, i_current, component_id_::CAMERA, archetype_data);
	Toggle_Component_Bit(i_archetype, i_current, component_id_::DRAW, archetype_data);
	Toggle_Component_Bit(i_archetype, i_new, component_id_::CAMERA, archetype_data);
	Toggle_Component_Bit(i_archetype, i_new, component_id_::DRAW, archetype_data);
}


/*
==================
==================
*/
void systems_::player_::impart_velocity(

	void* parameters, __int32 i_thread

)
{
	parameters_::player_::impart_velocity_* func_parameters = (parameters_::player_::impart_velocity_*)parameters;
	mouse_ const& mouse = *func_parameters->mouse;
	user_input_ const& user_input = *func_parameters->user_input;
	timer_ const& timer = *func_parameters->timer;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	const float scale = 500.0f;
	const float gain = 50.0f;

	component_fetch_ component_fetch;
	component_fetch.n_components = 3;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::CAMERA;
	component_fetch.component_ids[1] = component_id_::MOVE;
	component_fetch.component_ids[2] = component_id_::COLLIDER;

	Populate_Fetch_Table(archetype_data, component_fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::camera_* camera = (component_::camera_*)component_fetch.table[0][i_archetype_index];
		component_::move_* move = (component_::move_*)component_fetch.table[1][i_archetype_index];
		component_::collider_* collider = (component_::collider_*)component_fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			{
				float delta_per_pixel = (float)mouse.delta_x / (float)display_::DISPLAY_WIDTH;
				camera[i_entity].angle_yaw -= (delta_per_pixel * scale) + (delta_per_pixel * gain);
				float4_ axis_angle = { 0.0f, 1.0f, 0.0f ,camera[i_entity].angle_yaw };
				Axis_Angle_To_Quaternion(axis_angle, camera[i_entity].q_yaw);
			}

			float4_ q_pitch;
			{
				float delta_per_pixel = (float)mouse.delta_y / (float)display_::DISPLAY_HEIGHT;;
				camera[i_entity].angle_pitch -= (delta_per_pixel * scale) + (delta_per_pixel * gain);;
				camera[i_entity].angle_pitch = __min(camera[i_entity].angle_pitch, 60.0f);
				camera[i_entity].angle_pitch = __max(camera[i_entity].angle_pitch, -60.0f);
				float4_ axis_angle = { 1.0f, 0.0f, 0.0f ,camera[i_entity].angle_pitch };
				Axis_Angle_To_Quaternion(axis_angle, q_pitch);
			}

			float4_ q_camera_space;
			matrix_ m_yaw;
			Quaternion_X_Quaternion(camera[i_entity].q_yaw, q_pitch, q_camera_space);
			Normalise_Quaternion(q_camera_space);
			Quaternion_To_Matrix(camera[i_entity].q_yaw, m_yaw);
			Quaternion_To_Matrix(q_camera_space, camera[i_entity].m_camera_space);

			float speed;
			{
				float3_ horizontal_velocity = move[i_entity].velocity;
				horizontal_velocity.y = 0.0f;
				float3_ horizontal_direction;
				speed = Vector_Normalise_Magnitude(horizontal_velocity, horizontal_direction);
			}

			float3_ horizontal_velocity;
			{
				float3_ move_direction = { 0.0f, 0.0f, 0.f };
				float3_ move_forward = { 0.0f, 0.0f, 1.0f };
				float3_ move_left = { 1.0f, 0.0f, 0.0f };
				bool is_forward = (user_input.input_mask & (1 << user_input_::FORWARD)) != 0x0;
				bool is_back = (user_input.input_mask & (1 << user_input_::BACK)) != 0x0;
				bool is_left = (user_input.input_mask & (1 << user_input_::LEFT)) != 0x0;
				bool is_right = (user_input.input_mask & (1 << user_input_::RIGHT)) != 0x0;

				for (__int32 i = X; i < W; i++) {

					move_direction.f[i] += blend(-move_forward.f[i], move_direction.f[i], is_forward);
					move_direction.f[i] += blend(move_forward.f[i], move_direction.f[i], is_back);
					move_direction.f[i] += blend(-move_left.f[i], move_direction.f[i], is_left);
					move_direction.f[i] += blend(move_left.f[i], move_direction.f[i], is_right);
				}

				float3_ move_mapped;
				Vector_X_Matrix(move_direction, m_yaw, move_mapped);
				float3_ move_normalised;
				Vector_Normalise(move_mapped, move_normalised);

				float acceleration = (speed < move[i_entity].max_speed) ? camera[i_entity].acceleration : 0.0f;
				horizontal_velocity.x = move_normalised.x * acceleration * timer.delta_time;
				horizontal_velocity.z = move_normalised.z * acceleration * timer.delta_time;
			}
			float vertical_velocity;
			{
				camera[i_entity].is_jumped = (user_input.input_mask & (1 << user_input_::JUMP)) != 0;
				vertical_velocity = blend(impulse_jump, 0.0f, camera[i_entity].is_jumped);
			}

			move[i_entity].velocity.x += blend(horizontal_velocity.x, 0.0f, collider[i_entity].is_ground_contact);
			move[i_entity].velocity.y += blend(vertical_velocity, 0.0f, collider[i_entity].is_ground_contact);
			move[i_entity].velocity.z += blend(horizontal_velocity.z, 0.0f, collider[i_entity].is_ground_contact);
		}
	}
}


	/*
	==================
	==================
	*/
	void systems_::camera_::compute_camera_matrix(

		void* parameters, __int32 i_thread

	) {

		parameters_::camera_::camera_to_model_* func_parameters = (parameters_::camera_::camera_to_model_*)parameters;
		archetype_data_& archetype_data = *func_parameters->archetype_data;

		component_fetch_ component_fetch;
		component_fetch.n_components = 2;
		component_fetch.n_excludes = 0;
		component_fetch.component_ids[0] = component_id_::BASE;
		component_fetch.component_ids[1] = component_id_::CAMERA;

		Populate_Fetch_Table(archetype_data, component_fetch);

		for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

			const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
			const archetype_& archetype = archetype_data.archetypes[i_archetype];
			const __int32 n_entities = archetype.n_entities;

			component_::base_* base = (component_::base_*)component_fetch.table[0][i_archetype_index];
			component_::camera_* camera = (component_::camera_*)component_fetch.table[1][i_archetype_index];

			for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

				if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

				matrix m_camera;
				matrix m_projection;
				for (__int32 i = 0; i < 4; i++) {
					m_camera[i] = load_u(camera[i_entity].m_camera_space[i].f);
					m_projection[i] = load_u(camera[i_entity].m_projection[i].f);
				}

				matrix m_view_space;
				Transpose(m_camera, m_view_space);

				matrix m_clip_space;
				Matrix_X_Matrix(m_projection, m_view_space, m_clip_space);

				int3_ end_position;

				end_position.x = base[i_entity].position_fixed.x + camera[i_entity].offset.x;
				end_position.y = base[i_entity].position_fixed.y + camera[i_entity].offset.y;
				end_position.z = base[i_entity].position_fixed.z + camera[i_entity].offset.z;

				int3_ delta;
				delta.x = end_position.x - camera[i_entity].previous_position.x;
				delta.y = end_position.y - camera[i_entity].previous_position.y;
				delta.z = end_position.z - camera[i_entity].previous_position.z;

				static const float t_increment = 0.05f;

				camera[i_entity].t_smooth_step = blend(0.0f, camera[i_entity].t_smooth_step, camera[i_entity].is_stepped);
				float t_smoothed = Smooth_Step(0.0f, 1.0f, camera[i_entity].t_smooth_step);
				__int64 t_smooth_int = __int64(t_smoothed * fixed_scale_real);

				for (__int32 i_axis = X; i_axis < W; i_axis++) {
					__int64 delta = end_position.i[i_axis] - camera[i_entity].previous_position.i[i_axis];
					__int64 increment = (delta * t_smooth_int) / fixed_scale;
					camera[i_entity].position_fixed.i[i_axis] = camera[i_entity].previous_position.i[i_axis] + (__int32)increment;
				}
				camera[i_entity].t_smooth_step += t_increment;
				camera[i_entity].t_smooth_step = min(camera[i_entity].t_smooth_step, 1.0f);

				for (__int32 i = 0; i < 4; i++) {
					store_u(m_clip_space[i], camera[i_entity].m_clip_space[i].f);
				}
			}
		}
	}




/*
==================
==================
*/
void systems_::camera_::camera_to_model(

	void* parameters, __int32 i_thread

) {

	parameters_::camera_::camera_to_model_* func_parameters = (parameters_::camera_::camera_to_model_*)parameters;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ component_fetch;
	component_fetch.n_components = 2;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::CAMERA;
	component_fetch.component_ids[1] = component_id_::MODEL_SPACE_UPDATE;

	Populate_Fetch_Table(archetype_data, component_fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::camera_* camera = (component_::camera_*)component_fetch.table[0][i_archetype_index];
		component_::model_space_update_* model_space_update = (component_::model_space_update_*)component_fetch.table[1][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			const float4_ axis_angle = { 0.0f, 1.0f, 0.0f, -90.0f };
			float4_ q_orient;
			Axis_Angle_To_Quaternion(axis_angle, q_orient);
			float4_ q_total;
			Quaternion_X_Quaternion(camera[i_entity].q_yaw, q_orient, q_total);
			Normalise_Quaternion(q_total);
			model_space_update[i_entity].q_rotate = q_total;
		}
	
	}
}

/*
==================
==================
*/
void systems_::player_::fire_projectile(
	
	void* parameters, __int32 i_thread

) {
	parameters_::player_::fire_projectile_* func_parameters = (parameters_::player_::fire_projectile_*)parameters;
	const user_input_& user_input = *func_parameters->user_input;
	const timer_& timer = *func_parameters->timer;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	static const float3_ z_axis = { 0.0f, 0.0f, -1.0f };
	static const float3_ zero = { 0.0f, 0.0f, 0.0f };

	component_fetch_ player_fetch;
	{
		player_fetch.n_components = 4;
		player_fetch.n_excludes = 0;
		player_fetch.component_ids[0] = component_id_::BASE;
		player_fetch.component_ids[1] = component_id_::COLOUR;
		player_fetch.component_ids[2] = component_id_::CAMERA;
		player_fetch.component_ids[3] = component_id_::WEAPON;

		Populate_Fetch_Table(archetype_data, player_fetch);
	}

	component_fetch_ projectile_fetch;
	{
		projectile_fetch.n_components = 4;
		projectile_fetch.n_excludes = 0;
		projectile_fetch.component_ids[0] = component_id_::BASE;
		projectile_fetch.component_ids[1] = component_id_::MOVE;
		projectile_fetch.component_ids[2] = component_id_::COLOUR;
		projectile_fetch.component_ids[3] = component_id_::PROJECTILE_ID;

		Populate_Fetch_Table(archetype_data, projectile_fetch);
	}

	__int32 i_archetype = 0;
	component_::base_* base_projectile = (component_::base_*)projectile_fetch.table[0][i_archetype];
	component_::move_* move_projectile = (component_::move_*)projectile_fetch.table[1][i_archetype];
	component_::colour_* colour_projectile = (component_::colour_*)projectile_fetch.table[2][i_archetype];
	component_::projectile_id_* projectile_id = (component_::projectile_id_*)projectile_fetch.table[3][i_archetype];

	for (__int32 i_archetype_index = 0; i_archetype_index < player_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = player_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)player_fetch.table[0][i_archetype_index];
		//component_::colour_* colour = (component_::colour_*)player_fetch.table[1][i_archetype_index];
		component_::camera_* camera = (component_::camera_*)player_fetch.table[2][i_archetype_index];
		component_::weapon_* weapon = (component_::weapon_*)player_fetch.table[3][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, player_fetch.component_masks, archetype)) { continue; };

			float3_ firing_axis;
			Vector_X_Matrix(z_axis, camera[i_entity].m_camera_space, firing_axis);

			int3_ position_projectile;
			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				position_projectile.i[i_axis] = base[i_entity].position_fixed.i[i_axis] + (__int32)(firing_axis.f[i_axis] * 30.0f * fixed_scale_real);
			}

			weapon[i_entity].timer -= timer.delta_time;
			weapon[i_entity].timer = max(weapon[i_entity].timer, 0.0f);
			bool is_time_elapsed = weapon[i_entity].timer == 0.0f;
			bool is_key_pressed = (user_input.input_mask & (1 << user_input_::FIRE)) != 0x0;
			weapon[i_entity].is_fired = is_time_elapsed && is_key_pressed;

			weapon[i_entity].projectile_id += (user_input.input_mask & (0x1 << user_input_::WEAPON_NEXT)) != 0x0;
			weapon[i_entity].projectile_id -= (user_input.input_mask & (0x1 << user_input_::WEAPON_PREV)) != 0x0;
			weapon[i_entity].projectile_id = weapon[i_entity].projectile_id < 0 ? component_::weapon_::id_::COUNT - 1 : weapon[i_entity].projectile_id;
			weapon[i_entity].projectile_id = weapon[i_entity].projectile_id == component_::weapon_::id_::COUNT ? 0 : weapon[i_entity].projectile_id;

			const __int32 i_projectile = weapon[i_entity].i_begin + weapon[i_entity].i_projectile;
			base_projectile[i_projectile].position_fixed = position_projectile;
			move_projectile[i_projectile].velocity = zero;
			move_projectile[i_projectile].displacement = zero;
			const __int32 id = weapon[i_entity].projectile_id;
			base_projectile[i_projectile].scale = component_::weapon_::projectile_data[id].scale;
			colour_projectile[i_projectile].colour = component_::weapon_::projectile_data[id].colour;



			if (weapon[i_entity].is_fired) {

				weapon[i_entity].timer = component_::weapon_::projectile_data[id].reload_time;
				move_projectile[i_projectile].velocity = firing_axis * component_::weapon_::projectile_data[id].speed;
				projectile_id[i_projectile].type_id = weapon[i_entity].projectile_id;
				weapon[i_entity].i_projectile = (weapon[i_entity].i_projectile + 1) % weapon[i_entity].n_projectiles;
			}
		}
	}
}


/*
==================
==================
*/
void systems_::player_::update_animation_driver(

	void* parameters, __int32 i_thread

) {
	parameters_::player_::update_animation_driver_* func_parameters = (parameters_::player_::update_animation_driver_*)parameters;
	archetype_data_& archetype_data = *func_parameters->archetype_data;

	component_fetch_ component_fetch;
	component_fetch.n_components = 3;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::MOVE;
	component_fetch.component_ids[1] = component_id_::ANIMATION;
	component_fetch.component_ids[2] = component_id_::ANIMATION_DRIVER;

	Populate_Fetch_Table(archetype_data, component_fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::move_* move = (component_::move_*)component_fetch.table[0][i_archetype_index];
		component_::animation_* animation = (component_::animation_*)component_fetch.table[1][i_archetype_index];
		component_::animation_driver_* animation_driver = (component_::animation_driver_*)component_fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			__int32 new_animation_id = animation_driver[i_entity].i_current_animation;
			bool is_pain = (animation_driver[i_entity].i_current_animation == animation_data_::id_::PAIN) && (!animation[i_entity].is_end_animation);

			float speed = Vector_Magnitude(move[i_entity].velocity);
			bool is_moving = speed > 0.0f;
			__int32 moving_id = blend_int(animation_data_::id_::WALK, animation_data_::id_::IDLE, is_moving);
			new_animation_id = blend_int(new_animation_id, moving_id, is_pain);

			bool is_triggered = animation_driver[i_entity].trigger_id != animation_data_::id_::NULL_;
			new_animation_id = blend_int(animation_driver[i_entity].trigger_id, new_animation_id, is_triggered);
			animation_driver[i_entity].trigger_id = animation_data_::id_::NULL_;

			bool is_changed = new_animation_id != animation_driver[i_entity].i_current_animation;
			animation[i_entity].trigger_id = blend_int(new_animation_id, animation[i_entity].trigger_id, is_changed || is_triggered);
			animation_driver[i_entity].i_current_animation = new_animation_id;
		}
	}
}


