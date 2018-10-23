#include "command.h"
#include "component.h"
#include "setup.h"
#include "input.h"

/*
==================
==================
*/
void systems_::command_::write_camera_matrix(

	void* parameters, __int32 i_thread

) {
	parameters_::command_::write_camera_matrix_* func_parameters = (parameters_::command_::write_camera_matrix_*)parameters;
	const archetype_data_& archetype_data = *func_parameters->archetype_data;
	command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	timer_& timer = *func_parameters->timer;
	command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_write];

	component_::camera_* camera;
	__int32 i_entity_camera = Return_Camera_Component(archetype_data, &camera);
	command_buffer.position_camera = camera[i_entity_camera].position_fixed;
	command_buffer.m_clip_space[X] = camera[i_entity_camera].m_clip_space[X];
	command_buffer.m_clip_space[Y] = camera[i_entity_camera].m_clip_space[Y];
	command_buffer.m_clip_space[Z] = camera[i_entity_camera].m_clip_space[Z];
	command_buffer.m_clip_space[W] = camera[i_entity_camera].m_clip_space[W];

	fog_effect_& fog_effect = command_buffer_handler.fog_effect;
	fog_effect.timer += timer.delta_time * 0.1f;
	fog_effect.timer = fog_effect.timer > 1.0f ? -1.0f : fog_effect.timer;
	command_buffer.fog_effect_timer = abs(fog_effect.timer);
}

/*
==================
==================
*/
void systems_::command_::write_colour_space(

	void* parameters, __int32 i_thread
) {

	parameters_::command_::write_colour_space_* func_parameters = (parameters_::command_::write_colour_space_*)parameters;
	const archetype_data_& archetype_data = *func_parameters->archetype_data;
	command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_write];
	draw_call_* draw_calls = command_buffer.draw_calls;

	component_fetch_ component_fetch;
	component_fetch.n_components = 3;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::COLOUR_SPACE;
	component_fetch.component_ids[1] = component_id_::COLOUR;
	component_fetch.component_ids[2] = component_id_::DRAW;

	Populate_Fetch_Table(archetype_data, component_fetch);

	__int32 n_models_local[draw_call_::id_::COUNT];
	for (__int32 i_draw_call = 0; i_draw_call < draw_call_::id_::COUNT; i_draw_call++) {
		n_models_local[i_draw_call] = 0;
	}

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::colour_space_* colour_space = (component_::colour_space_*)component_fetch.table[0][i_archetype_index];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[1][i_archetype_index];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			const __int32 draw_id = draw[i_entity].draw_id;
			draw_call_& draw_call = draw_calls[draw_id];
			__int32& n_models = n_models_local[draw_id];

			draw_call.m_vertex_colour[n_models][X] = colour_space[i_entity].m_rotate[X];
			draw_call.m_vertex_colour[n_models][Y] = colour_space[i_entity].m_rotate[Y];
			draw_call.m_vertex_colour[n_models][Z] = colour_space[i_entity].m_rotate[Z];
			draw_call.m_vertex_colour[n_models][W] = colour_space[i_entity].m_rotate[W];

			draw_call.colour[n_models] = colour[i_entity].colour;
			//draw_call.m_vertex_colour[n_models][W].x = colour[i_entity].colour.x;
			//draw_call.m_vertex_colour[n_models][W].y = colour[i_entity].colour.y;
			//draw_call.m_vertex_colour[n_models][W].z = colour[i_entity].colour.z;
			//draw_call.m_vertex_colour[n_models][W].w = 1.0f;

			n_models++;
		}
	}
}

/*
==================
==================
*/
void systems_::command_::write_colour(

	void* parameters, __int32 i_thread
) {

	parameters_::command_::write_colour_space_* func_parameters = (parameters_::command_::write_colour_space_*)parameters;
	const archetype_data_& archetype_data = *func_parameters->archetype_data;
	command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_write];
	draw_call_* draw_calls = command_buffer.draw_calls;

	component_fetch_ component_fetch;
	component_fetch.n_excludes = 1;
	component_fetch.exclude_ids[0] = component_id_::COLOUR_SPACE;
	component_fetch.n_components = 2;
	component_fetch.component_ids[0] = component_id_::COLOUR;
	component_fetch.component_ids[1] = component_id_::DRAW;

	Populate_Fetch_Table(archetype_data, component_fetch);

	__int32 n_models_local[draw_call_::id_::COUNT];
	for (__int32 i_draw_call = 0; i_draw_call < draw_call_::id_::COUNT; i_draw_call++) {
		n_models_local[i_draw_call] = 0;
	}

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::colour_* colour = (component_::colour_*)component_fetch.table[0][i_archetype_index];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[1][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			const __int32 draw_id = draw[i_entity].draw_id;
			draw_call_& draw_call = draw_calls[draw_id];
			__int32& n_models = n_models_local[draw_id];

			//draw_call.m_vertex_colour[n_models][W].x = colour[i_entity].colour.x;
			//draw_call.m_vertex_colour[n_models][W].y = colour[i_entity].colour.y;
			//draw_call.m_vertex_colour[n_models][W].z = colour[i_entity].colour.z;
			//draw_call.m_vertex_colour[n_models][W].w = 1.0f;

			draw_call.colour[n_models] = colour[i_entity].colour;

			n_models++;
		}
	}
}

/*
==================
==================
*/
void systems_::command_::write_texture_space(

	void* parameters, __int32 i_thread
) {

	parameters_::command_::write_texture_space_* func_parameters = (parameters_::command_::write_texture_space_*)parameters;
	const archetype_data_& archetype_data = *func_parameters->archetype_data;
	command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_write];
	draw_call_* draw_calls = command_buffer.draw_calls;

	component_fetch_ component_fetch;
	component_fetch.n_components = 2;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::TEXTURE_SPACE;
	component_fetch.component_ids[1] = component_id_::DRAW;

	Populate_Fetch_Table(archetype_data, component_fetch);

	__int32 n_models_local[draw_call_::id_::COUNT];
	for (__int32 i_draw_call = 0; i_draw_call < draw_call_::id_::COUNT; i_draw_call++) {
		n_models_local[i_draw_call] = 0;
	}

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::texture_space_* texture_space = (component_::texture_space_*)component_fetch.table[0][i_archetype_index];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[1][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			const __int32 draw_id = draw[i_entity].draw_id;
			draw_call_& draw_call = draw_calls[draw_id];
			__int32& n_models = n_models_local[draw_id];

			draw_call.m_vertex_texture[n_models][X] = texture_space[i_entity].m_rotate[X];
			draw_call.m_vertex_texture[n_models][Y] = texture_space[i_entity].m_rotate[Y];
			draw_call.m_vertex_texture[n_models][Z] = texture_space[i_entity].m_rotate[Z];
			draw_call.m_vertex_texture[n_models][W] = texture_space[i_entity].m_rotate[W];

			n_models++;
		}
	}
}

/*
==================
==================
*/
void systems_::command_::write_blend_interval(

	void* parameters, __int32 i_thread
) {

	parameters_::command_::write_blend_interval_* func_parameters = (parameters_::command_::write_blend_interval_*)parameters;
	const archetype_data_& archetype_data = *func_parameters->archetype_data;
	command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_write];
	draw_call_* draw_calls = command_buffer.draw_calls;

	component_fetch_ component_fetch;
	component_fetch.n_components = 2;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::TEXTURE_BLEND;
	component_fetch.component_ids[1] = component_id_::DRAW;

	Populate_Fetch_Table(archetype_data, component_fetch);

	__int32 n_models_local[draw_call_::id_::COUNT];
	for (__int32 i_draw_call = 0; i_draw_call < draw_call_::id_::COUNT; i_draw_call++) {
		n_models_local[i_draw_call] = 0;
	}

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::texture_blend_* texture_blend = (component_::texture_blend_*)component_fetch.table[0][i_archetype_index];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[1][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			const __int32 draw_id = draw[i_entity].draw_id;
			draw_call_& draw_call = draw_calls[draw_id];
			__int32& n_models = n_models_local[draw_id];
			draw_calls[draw_id].i_texture_offset[n_models] = texture_blend[i_entity].interval > 0.5f ? 1 : 0;

			n_models++;

		}
	}
}

/*
==================
==================
*/
void systems_::command_::write_animated(

	void* parameters, __int32 i_thread
) {

	parameters_::command_::write_animated_* func_parameters = (parameters_::command_::write_animated_*)parameters;
	const archetype_data_& archetype_data = *func_parameters->archetype_data;
	command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_write];
	draw_call_* draw_calls = command_buffer.draw_calls;

	component_fetch_ component_fetch;
	component_fetch.n_components = 4;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::BASE;
	component_fetch.component_ids[1] = component_id_::MODEL_SPACE;
	component_fetch.component_ids[2] = component_id_::ANIMATION;
	component_fetch.component_ids[3] = component_id_::DRAW;

	Populate_Fetch_Table(archetype_data, component_fetch);

	struct data_ {

		enum {
			MAX_ENTRIES = 32,
		};

		__int32 i_archetype_index;
		__int32 i_entity;
		float depth;
	};

	data_ data[data_::MAX_ENTRIES];
	__int32 n_entries = 0;

	{
		for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

			const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
			const archetype_& archetype = archetype_data.archetypes[i_archetype];
			const __int32 n_entities = archetype.n_entities;

			component_::base_* base = (component_::base_*)component_fetch.table[0][i_archetype_index];

			for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

				if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

				float3_ local_position;
				local_position.x = (float)(base[i_entity].position_fixed.x - command_buffer.position_camera.x) * r_fixed_scale_real;
				local_position.y = (float)(base[i_entity].position_fixed.y - command_buffer.position_camera.y) * r_fixed_scale_real;
				local_position.z = (float)(base[i_entity].position_fixed.z - command_buffer.position_camera.z) * r_fixed_scale_real;

				float3_ t_position;
				Vector_X_Matrix(local_position, command_buffer.m_clip_space, t_position);

				data[n_entries].depth = - t_position.z;
				data[n_entries].i_archetype_index = i_archetype_index;
				data[n_entries].i_entity = i_entity;
				n_entries++;
			}
		}
	}
	{
		for (__int32 i_entry = 1; i_entry < n_entries; i_entry++) {

			for (__int32 i_iterate = i_entry; i_iterate > 0; i_iterate--) {

				data_ temp[2];
				unsigned __int32 index = 0;
				temp[index] = data[i_iterate];
				temp[index ^ 1] = data[i_iterate - 1];
				index ^= data[i_iterate].depth < data[i_iterate - 1].depth;
				data[i_iterate] = temp[index];
				data[i_iterate - 1] = temp[index ^ 1];
			}
		}
	}
	{
		//printf_s(" %i \n", n_entries);

		for (__int32 i_entry = 0; i_entry < n_entries; i_entry++) {

			//printf_s(" %f ", data[i_entry].depth);

			const __int32 i_archetype_index = data[i_entry].i_archetype_index;
			const __int32 i_entity = data[i_entry].i_entity;
			const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
			const archetype_& archetype = archetype_data.archetypes[i_archetype];

			component_::base_* base = (component_::base_*)component_fetch.table[0][i_archetype_index];
			component_::model_space_* model_space = (component_::model_space_*)component_fetch.table[1][i_archetype_index];
			component_::animation_* animation = (component_::animation_*)component_fetch.table[2][i_archetype_index];
			component_::draw_* draw = (component_::draw_*)component_fetch.table[3][i_archetype_index];

			const __int32 draw_id = draw[i_entity].draw_id;
			draw_call_& draw_call = draw_calls[draw_id];
			__int32& n_models = draw_call.n_models;

			draw_call.position[n_models] = base[i_entity].position_fixed;
			draw_call.scale[n_models] = base[i_entity].scale;
			draw_call.m_rotate[n_models][X] = model_space[i_entity].m_rotate[X];
			draw_call.m_rotate[n_models][Y] = model_space[i_entity].m_rotate[Y];
			draw_call.m_rotate[n_models][Z] = model_space[i_entity].m_rotate[Z];
			draw_call.m_rotate[n_models][W] = model_space[i_entity].m_rotate[W];
			draw_call.animation_data[n_models].i_frames[0] = animation[i_entity].i_frames[0];
			draw_call.animation_data[n_models].i_frames[1] = animation[i_entity].i_frames[1];
			draw_call.animation_data[n_models].frame_interval = animation[i_entity].frame_interval;
			draw_call.animation_data[n_models].origin = animation[i_entity].origin;
			draw_call.model_id[n_models] = draw[i_entity].model_id;

			n_models++;
		}
		//printf_s(" \n");
	}
}


/*
==================
==================
*/
void systems_::command_::write_model_space(

	void* parameters, __int32 i_thread
) {

	parameters_::command_::write_model_space_* func_parameters = (parameters_::command_::write_model_space_*)parameters;
	const archetype_data_& archetype_data = *func_parameters->archetype_data;
	command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_write];
	draw_call_* draw_calls = command_buffer.draw_calls;

	component_fetch_ component_fetch;
	component_fetch.n_components = 3;
	component_fetch.component_ids[0] = component_id_::BASE;
	component_fetch.component_ids[1] = component_id_::MODEL_SPACE;
	component_fetch.component_ids[2] = component_id_::DRAW;
	component_fetch.n_excludes = 1;
	component_fetch.exclude_ids[0] = component_id_::ANIMATION;

	Populate_Fetch_Table(archetype_data, component_fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)component_fetch.table[0][i_archetype_index];
		component_::model_space_* model_space = (component_::model_space_*)component_fetch.table[1][i_archetype_index];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			const __int32 draw_id = draw[i_entity].draw_id;
			draw_call_& draw_call = draw_calls[draw_id];
			__int32& n_models = draw_call.n_models;

			draw_call.position[n_models] = base[i_entity].position_fixed;
			draw_call.scale[n_models] = base[i_entity].scale;
			draw_call.m_rotate[n_models][X] = model_space[i_entity].m_rotate[X];
			draw_call.m_rotate[n_models][Y] = model_space[i_entity].m_rotate[Y];
			draw_call.m_rotate[n_models][Z] = model_space[i_entity].m_rotate[Z];
			draw_call.m_rotate[n_models][W] = model_space[i_entity].m_rotate[W];
			draw_call.model_id[n_models] = draw[i_entity].model_id;

			n_models++;
		}
	}
}


/*
==================
==================
*/
void systems_::command_::write_static_model(

	void* parameters, __int32 i_thread
) {

	parameters_::command_::write_static_model_* func_parameters = (parameters_::command_::write_static_model_*)parameters;
	const archetype_data_& archetype_data = *func_parameters->archetype_data;
	command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_write];
	draw_call_* draw_calls = command_buffer.draw_calls;

	component_fetch_ component_fetch;
	component_fetch.n_components = 2;
	component_fetch.component_ids[0] = component_id_::BASE;
	component_fetch.component_ids[1] = component_id_::DRAW;
	component_fetch.n_excludes = 3;
	component_fetch.exclude_ids[0] = component_id_::SMALL_MODEL_ID;
	component_fetch.exclude_ids[1] = component_id_::ANIMATION;
	component_fetch.exclude_ids[2] = component_id_::MODEL_SPACE;

	Populate_Fetch_Table(archetype_data, component_fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)component_fetch.table[0][i_archetype_index];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[1][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			const __int32 draw_id = draw[i_entity].draw_id;
			draw_call_& draw_call = draw_calls[draw_id];
			__int32& n_models = draw_call.n_models;

			draw_call.position[n_models] = base[i_entity].position_fixed;
			draw_call.scale[n_models] = base[i_entity].scale;
			draw_call.model_id[n_models] = draw[i_entity].model_id;

			n_models++;
		}
	}
}

/*
==================
==================
*/
void systems_::command_::write_small_model(

	void* parameters, __int32 i_thread
) {

	parameters_::command_::write_small_model_* func_parameters = (parameters_::command_::write_small_model_*)parameters;
	const archetype_data_& archetype_data = *func_parameters->archetype_data;
	command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_write];
	draw_call_* draw_calls = command_buffer.draw_calls;

	component_fetch_ component_fetch;
	component_fetch.n_components = 3;
	component_fetch.n_excludes = 0;
	component_fetch.component_ids[0] = component_id_::BASE;
	component_fetch.component_ids[1] = component_id_::SMALL_MODEL_ID;
	component_fetch.component_ids[2] = component_id_::DRAW;

	Populate_Fetch_Table(archetype_data, component_fetch);

	for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

		const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
		const archetype_& archetype = archetype_data.archetypes[i_archetype];
		const __int32 n_entities = archetype.n_entities;

		component_::base_* base = (component_::base_*)component_fetch.table[0][i_archetype_index];
		component_::small_model_id_* small_model_id = (component_::small_model_id_*)component_fetch.table[1][i_archetype_index];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[2][i_archetype_index];

		for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

			if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

			const __int32 draw_id = draw[i_entity].draw_id;
			draw_call_& draw_call = draw_calls[draw_id];
			__int32& n_models = draw_call.n_models;

			draw_call.position[n_models] = base[i_entity].position_fixed;
			draw_call.scale[n_models] = base[i_entity].scale;
			draw_call.model_id[n_models] = draw[i_entity].model_id;

			n_models++;
		}
	}
}

/*
==================
==================
*/
void systems_::command_::write_light_sources(

	void* parameters, __int32 i_thread
) {

	parameters_::command_::write_light_sources_* func_parameters = (parameters_::command_::write_light_sources_*)parameters;
	const archetype_data_& archetype_data = *func_parameters->archetype_data;
	command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_write];
	draw_call_* draw_calls = command_buffer.draw_calls;

	{
		component_fetch_ component_fetch;
		component_fetch.n_components = 3;
		component_fetch.n_excludes = 0;
		component_fetch.component_ids[0] = component_id_::BASE;
		component_fetch.component_ids[1] = component_id_::COLOUR;
		component_fetch.component_ids[2] = component_id_::VERTEX_LIGHT_SOURCE;

		Populate_Fetch_Table(archetype_data, component_fetch);

		command_buffer.n_vertex_lights = 0;

		for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

			const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
			const archetype_& archetype = archetype_data.archetypes[i_archetype];
			const __int32 n_entities = archetype.n_entities;

			component_::base_* base = (component_::base_*)component_fetch.table[0][i_archetype_index];
			component_::colour_* colour = (component_::colour_*)component_fetch.table[1][i_archetype_index];
			component_::vertex_light_source_* vertex_light_source = (component_::vertex_light_source_*)component_fetch.table[2][i_archetype_index];

			for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

				if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

				command_buffer.vertex_lights[command_buffer.n_vertex_lights].position = base[i_entity].position_fixed;
				command_buffer.vertex_lights[command_buffer.n_vertex_lights].colour = colour[i_entity].colour;
				command_buffer.vertex_lights[command_buffer.n_vertex_lights].intensity = vertex_light_source[i_entity].intensity;

				command_buffer.n_vertex_lights++;
			}
		}
	}
	{
		component_fetch_ component_fetch;
		component_fetch.n_components = 3;
		component_fetch.n_excludes = 0;
		component_fetch.component_ids[0] = component_id_::BASE;
		component_fetch.component_ids[1] = component_id_::COLOUR;
		component_fetch.component_ids[2] = component_id_::LIGHTMAP_LIGHT_SOURCE;

		Populate_Fetch_Table(archetype_data, component_fetch);

		command_buffer.n_lightmap_lights = 0;

		for (__int32 i_archetype_index = 0; i_archetype_index < component_fetch.n_archetypes; i_archetype_index++) {

			const __int32 i_archetype = component_fetch.i_archetypes[i_archetype_index];
			const archetype_& archetype = archetype_data.archetypes[i_archetype];
			const __int32 n_entities = archetype.n_entities;

			component_::base_* base = (component_::base_*)component_fetch.table[0][i_archetype_index];
			component_::colour_* colour = (component_::colour_*)component_fetch.table[1][i_archetype_index];
			component_::lightmap_light_source_* lightmap_light_source = (component_::lightmap_light_source_*)component_fetch.table[2][i_archetype_index];

			for (__int32 i_entity = 0; i_entity < n_entities; i_entity++) {

				if (!Match_Entity_Mask(i_entity, component_fetch.component_masks, archetype)) { continue; };

				command_buffer.lightmap_lights[command_buffer.n_lightmap_lights].position = base[i_entity].position_fixed;
				command_buffer.lightmap_lights[command_buffer.n_lightmap_lights].colour = colour[i_entity].colour;
				command_buffer.lightmap_lights[command_buffer.n_lightmap_lights].intensity = lightmap_light_source[i_entity].intensity;

				command_buffer.n_lightmap_lights++;
			}
		}
	}

}

/*
==================
==================
*/
void systems_::command_::bin_vertex_lights(

	void* parameters, __int32 i_thread
) {

	parameters_::command_::bin_vertex_lights_* func_parameters = (parameters_::command_::bin_vertex_lights_*)parameters;
	command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_write];
	vertex_light_manager_& vertex_light_manager = command_buffer.vertex_light_manager;

	//for (__int32 i_bin = 0; i_bin < vertex_light_manager_::NUM_BINS; i_bin++) {
	//	vertex_light_manager.bin[i_bin].n_lights = 0;
	//}

	//__int32 n_lights = 0;

	//for (__int32 i_light = 0; i_light < command_buffer.n_vertex_lights; i_light++) {

	//	float3_ delta;
	//	for (__int32 i_axis = X; i_axis < W; i_axis++) {
	//		delta.f[i_axis] = (float)(command_buffer.vertex_lights[i_light].position.i[i_axis] - command_buffer.position_camera.i[i_axis]) * r_fixed_scale_real;
	//	}

	//	float4_ transformed_position;
	//	Vector_X_Matrix(delta, command_buffer.m_clip_space, transformed_position);
	//	vertex_light_manager.light_sources[n_lights].position.x = transformed_position.x;
	//	vertex_light_manager.light_sources[n_lights].position.y = transformed_position.y;
	//	vertex_light_manager.light_sources[n_lights].position.z = transformed_position.w;
	//	vertex_light_manager.light_sources[n_lights].colour = command_buffer.vertex_lights[i_light].colour;
	//	vertex_light_manager.light_sources[n_lights].intensity = command_buffer.vertex_lights[i_light].intensity;

	//	__int32 i_bin = __int32(transformed_position.w / vertex_light_manager.bin_interval);
	//	bool is_valid = (i_bin >= 0) & (i_bin < vertex_light_manager_::NUM_BINS);
	//	i_bin = is_valid ? i_bin : 0;
	//	vertex_light_manager_::bin_& bin = vertex_light_manager.bin[i_bin];
	//	vertex_light_manager.i_bin[n_lights] = i_bin;
	//	bin.n_lights += is_valid;
	//	n_lights += is_valid;
	//}

	//__int32 i_current = 0;
	//for (__int32 i_bin = 0; i_bin < vertex_light_manager_::NUM_BINS; i_bin++) {
	//	vertex_light_manager_::bin_& bin = vertex_light_manager.bin[i_bin];
	//	bin.i_start = i_current;
	//	i_current += bin.n_lights;
	//	bin.n_lights = 0;
	//}

	//for (__int32 i_light = 0; i_light < n_lights; i_light++) {

	//	__int32 i_bin = vertex_light_manager.i_bin[i_light];
	//	vertex_light_manager_::bin_& bin = vertex_light_manager.bin[i_bin];
	//	vertex_light_manager.i_light[bin.i_start + bin.n_lights] = i_light;
	//	bin.n_lights++;
	//}
}

/*
==================
==================
*/
void systems_::command_::command_buffer_swap(

	void* parameters, __int32 i_thread
) {

	parameters_::command_::command_buffer_swap_* func_parameters = (parameters_::command_::command_buffer_swap_*)parameters;
	command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;

	command_buffer_handler.i_read ^= 1;
	command_buffer_handler.i_write ^= 1;

}

