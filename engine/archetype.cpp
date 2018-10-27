
#include "setup.h"
#include "stb_image.h"
#include "quake.h"
#include "render_front.h"
#include "sound.h"
#include "shader.h"
#include "vector_math.h"
#include "entity.h"
#include "command.h"
#include "light_maps.h"
#include "collide.h"
#include "update.h"
#include "memory.h"

/*
==================
==================
*/
void Allocate_Memory_Archetype(

	const __int32 n_entities,
	const __int32 n_components,
	const __int32 component_list[],
	const component_data_& component_data,
	memory_chunk_& memory_chunk,
	archetype_& archetype

) {

	archetype.n_entities = n_entities;
	archetype.n_components = n_components;

	Convert_Component_list_To_Mask(n_components, component_list, archetype.component_masks);

	archetype.component_ids = (__int32*)memory_chunk.chunk_ptr;
	memory_chunk.chunk_ptr = archetype.component_ids + n_components;
	archetype.mem_ptrs = (__int8**)memory_chunk.chunk_ptr;
	memory_chunk.chunk_ptr = archetype.mem_ptrs + n_components;
	archetype.entity_component_masks = (unsigned __int32(*)[NUM_MASKS_RAW])memory_chunk.chunk_ptr;
	memory_chunk.chunk_ptr = archetype.entity_component_masks + (n_entities * component_data_::NUM_MASKS);

	for (__int32 i_component = 0; i_component < n_components; i_component++) {

		const __int32 component_id = component_list[i_component];
		const __int32 size_byte = component_data.size_byte[component_id];
		archetype.component_ids[i_component] = component_id;
		archetype.mem_ptrs[i_component] = (__int8*)memory_chunk.chunk_ptr;
		memory_chunk.chunk_ptr = archetype.mem_ptrs[i_component] + (size_byte * n_entities);
	}

	__int64 n_bytes_used = (__int8*)memory_chunk.chunk_ptr - memory_chunk.chunk;
	bool is_out_of_mem = n_bytes_used >= memory_chunk_::NUM_BYTES;

	if (is_out_of_mem) {
		printf_s("ERROR: archetype memory budget exceeded \n");
		exit(0);
	}
}

/*
==================
==================
*/
void COMPONENT_Populate_Table(

	component_data_& component_data,
	way_point_manager_& way_point_manager,
	behaviour_manager_& behaviour_manager,
	animation_manager_& animation_manager,
	model_manager_& model_manager,
	model_token_ model_tokens[],
	draw_call_ draw_calls[],
	grid_& grid,
	memory_chunk_& memory_chunk
) {

	const float4_ q_unit = { 0.0f, 0.0f, 0.0f, 1.0f };
	const float3_ zero = { 0.0f, 0.0f, 0.0f };

	__int32 i_monster = 0;

	__int32 i_archetype_bounce_pad = 0;
	__int32 i_archetype_platform = 0;

	model_* model_animate = model_manager.model;

	{
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_data.n_archetypes = 0;
	}
	// ----------------------------------------------------------------------------------------------------------

	matrix_ unit_matrix = {

		{ 1.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 1.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 1.0f },
	};
	// ----------------------------------------------------------------------------------------------------------
	{

		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::COLOUR,
			component_id_::ITEM,
			component_id_::SOUND_TRIGGER,
			component_id_::TRIGGER,
			component_id_::MODEL_SPACE,
			component_id_::MODEL_SPACE_UPDATE,
			component_id_::BOUNDING_BOX,
			component_id_::DRAW,

		};


		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		const float3_ default_scale = float3_{ 1.0f, 1.0f, 1.0f };
		model_token_& model_token = model_tokens[model_token_::id_::ITEM];
		const __int32 n_models = model_token.n_models;
		const __int32 draw_id = draw_call_::id_::ITEMS;

		//const __int32 model_id = model_::id_::ARTIFACT_QUAD;
		//const model_& model = model_animate[model_id];

		const __int32 n_ammo_types = 3;

		const float3_ colours[n_ammo_types] = {

			{ 100.0f, 0.0f, 0.0f },
		{ 0.0f, 100.0f, 0.0f },
		{ 0.0f, 0.0f, 100.0f },
		};

		const __int32 model_ids[n_ammo_types] = {

			Return_Model_ID("ammo", model_manager),
			Return_Model_ID("artifact_sharp", model_manager),
			Return_Model_ID("artifact_quad", model_manager),
		};

		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;
		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::ITEM;
		component_fetch.component_ids[n_entries++] = component_id_::TRIGGER;
		component_fetch.component_ids[n_entries++] = component_id_::SOUND_TRIGGER;
		component_fetch.component_ids[n_entries++] = component_id_::MODEL_SPACE;
		component_fetch.component_ids[n_entries++] = component_id_::MODEL_SPACE_UPDATE;
		component_fetch.component_ids[n_entries++] = component_id_::BOUNDING_BOX;
		component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::item_* item = (component_::item_*)component_fetch.table[n_entries++][0];
		component_::trigger_* trigger = (component_::trigger_*)component_fetch.table[n_entries++][0];
		component_::sound_trigger_* sound_trigger = (component_::sound_trigger_*)component_fetch.table[n_entries++][0];
		component_::model_space_* model_space = (component_::model_space_*)component_fetch.table[n_entries++][0];
		component_::model_space_update_* model_space_update = (component_::model_space_update_*)component_fetch.table[n_entries++][0];
		component_::bounding_box_* bounding_box = (component_::bounding_box_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];

		for (__int32 i_model = 0; i_model < n_models; i_model++) {



			for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
				archetype.entity_component_masks[i_model][i_group] = component_fetch.component_masks[i_group];
			}

			base[i_model].position_fixed = model_token.centre[i_model];
			base[i_model].scale = default_scale;

			const __int32 index = model_token.index[i_model];
			const __int32 model_id = model_ids[index % n_ammo_types];
			const model_& model = model_animate[model_id];

			colour[i_model].colour = colours[index % n_ammo_types];

			//printf_s("INDEX: %i \n", index);

			item[i_model].id = (component_::item_::type_::KILL + index) % n_ammo_types;
			item[i_model].state = component_::item_::state_::SPAWNED;
			item[i_model].state_trigger = component_::item_::state_::NULL_;
			item[i_model].timer = 10.0f;
			item[i_model].spawn_time = 5.0f;
			item[i_model].default_scale = default_scale;
			item[i_model].spawn_position = model_token.centre[i_model];
			item[i_model].despawn_position = { 0, 0, 0 };

			trigger[i_model].position_fixed = model_token.centre[i_model];
			trigger[i_model].extent_fixed = model_token.extent[i_model];
			trigger[i_model].is_triggered = false;

			sound_trigger[i_model].source_id = sound_triggers_::source_id_::ITEM + i_model;

			memcpy(model_space[i_model].m_rotate, unit_matrix, sizeof(matrix_));

			float4_ axis_angle = { 0.0f, 1.0f, 0.0f, 4.0f };
			Axis_Angle_To_Quaternion(axis_angle, model_space_update[i_model].q_add);
			model_space_update[i_model].q_rotate = q_unit;

			bounding_box[i_model].centre = model_token.centre[i_model];
			bounding_box[i_model].extent.x = __int32(model.bounding_extent.x * fixed_scale_real);
			bounding_box[i_model].extent.y = __int32(model.bounding_extent.y * fixed_scale_real);
			bounding_box[i_model].extent.z = __int32(model.bounding_extent.z * fixed_scale_real);

			draw[i_model].draw_id = draw_id;
			draw[i_model].model_id = model_id;
		}

		draw_call_& draw_call = draw_calls[draw_id];

		draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour_Simple;

		draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].vertex_shader = NULL;
		draw_call.n_attributes = 2;

		draw_call.mip_level_bias = 0.2f;
		draw_call.lighting_function = Vertex_Lighting_PLAYER;

	}
	// ----------------------------------------------------------------------------------------------------------
	{

		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::COLOUR,
			component_id_::MODEL_SPACE,
			component_id_::MODEL_SPACE_UPDATE,
			component_id_::BOUNDING_BOX,
			component_id_::PROP,
			component_id_::DRAW,

		};


		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		const float3_ default_scale = float3_{ 1.0f, 1.0f, 1.0f };
		model_token_& model_token = model_tokens[model_token_::id_::PROP];
		const __int32 n_models = model_token.n_models;
		const __int32 draw_id = draw_call_::id_::ITEMS;

		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;
		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::MODEL_SPACE;
		component_fetch.component_ids[n_entries++] = component_id_::MODEL_SPACE_UPDATE;
		component_fetch.component_ids[n_entries++] = component_id_::BOUNDING_BOX;
		component_fetch.component_ids[n_entries++] = component_id_::PROP;
		component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::model_space_* model_space = (component_::model_space_*)component_fetch.table[n_entries++][0];
		component_::model_space_update_* model_space_update = (component_::model_space_update_*)component_fetch.table[n_entries++][0];
		component_::bounding_box_* bounding_box = (component_::bounding_box_*)component_fetch.table[n_entries++][0];
		component_::prop_* prop = (component_::prop_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];

		const __int32 model_ids[] = {

			Return_Model_ID("artifact_quad", model_manager),
			Return_Model_ID("armour", model_manager),
			Return_Model_ID("ammo_nails0", model_manager),
			Return_Model_ID("artifact_sharp", model_manager),
			Return_Model_ID("g_plasma", model_manager),
			Return_Model_ID("g_rock2", model_manager),
		};

		const __int32 n_model_ids = sizeof(model_ids) / sizeof(model_ids[0]);

		for (__int32 i_model = 0; i_model < n_models; i_model++) {

			const __int32 model_id = model_ids[i_model % n_model_ids];
			const model_& model = model_animate[model_id];

			for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
				archetype.entity_component_masks[i_model][i_group] = component_fetch.component_masks[i_group];
			}

			base[i_model].position_fixed = model_token.centre[i_model];
			base[i_model].scale = default_scale;


			colour[i_model].colour = { 0.0f, 0.0f, 0.0f };

			//printf_s("INDEX: %i \n", index);

			memcpy(model_space[i_model].m_rotate, unit_matrix, sizeof(matrix_));

			float4_ axis_angle = { 0.0f, 1.0f, 0.0f, 4.0f };
			Axis_Angle_To_Quaternion(axis_angle, model_space_update[i_model].q_add);
			model_space_update[i_model].q_rotate = q_unit;

			bounding_box[i_model].centre = model_token.centre[i_model];
			bounding_box[i_model].extent.x = __int32(model.bounding_extent.x * fixed_scale_real);
			bounding_box[i_model].extent.y = __int32(model.bounding_extent.y * fixed_scale_real);
			bounding_box[i_model].extent.z = __int32(model.bounding_extent.z * fixed_scale_real);

			prop[i_model].id = 0;

			draw[i_model].draw_id = draw_id;
			draw[i_model].model_id = model_id;
		}

		draw_call_& draw_call = draw_calls[draw_id];

		draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour_Simple;

		draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].vertex_shader = NULL;
		draw_call.n_attributes = 2;

		draw_call.mip_level_bias = 0.2f;
		draw_call.lighting_function = Vertex_Lighting_PLAYER;

	}
	// ----------------------------------------------------------------------------------------------------------
	{

		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::MOVE,
			component_id_::COLOUR,
			component_id_::MODEL_SPACE,
			component_id_::MODEL_SPACE_UPDATE,
			component_id_::BOUNDING_BOX,
			component_id_::COLLIDER,
			component_id_::COLLIDEE_STATIC,
			component_id_::ANIMATION,
			component_id_::ANIMATION_DRIVER,
			component_id_::EFFECT,
			component_id_::TEXTURE_BLEND,
			component_id_::MASS,
			component_id_::MOB,
			component_id_::DRAW,

		};


		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		const float3_ default_scale = float3_{ 1.0f, 1.0f, 1.0f };
		model_token_& model_token = model_tokens[model_token_::id_::MOB];
		const __int32 n_models = model_token.n_models;
		const __int32 draw_id = draw_call_::id_::SHAMBLER;

		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;
		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::MOVE;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::MODEL_SPACE;
		component_fetch.component_ids[n_entries++] = component_id_::MODEL_SPACE_UPDATE;
		component_fetch.component_ids[n_entries++] = component_id_::BOUNDING_BOX;
		component_fetch.component_ids[n_entries++] = component_id_::COLLIDER;
		component_fetch.component_ids[n_entries++] = component_id_::COLLIDEE_STATIC;
		component_fetch.component_ids[n_entries++] = component_id_::ANIMATION;
		component_fetch.component_ids[n_entries++] = component_id_::ANIMATION_DRIVER;
		component_fetch.component_ids[n_entries++] = component_id_::EFFECT;
		component_fetch.component_ids[n_entries++] = component_id_::TEXTURE_BLEND;
		component_fetch.component_ids[n_entries++] = component_id_::MASS;
		component_fetch.component_ids[n_entries++] = component_id_::MOB;
		component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::move_* move = (component_::move_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::model_space_* model_space = (component_::model_space_*)component_fetch.table[n_entries++][0];
		component_::model_space_update_* model_space_update = (component_::model_space_update_*)component_fetch.table[n_entries++][0];
		component_::bounding_box_* bounding_box = (component_::bounding_box_*)component_fetch.table[n_entries++][0];
		component_::collider_* collider = (component_::collider_*)component_fetch.table[n_entries++][0];
		component_::collidee_static_* collidee_static = (component_::collidee_static_*)component_fetch.table[n_entries++][0];
		component_::animation_* animation = (component_::animation_*)component_fetch.table[n_entries++][0];
		component_::animation_driver_* animation_driver = (component_::animation_driver_*)component_fetch.table[n_entries++][0];
		component_::effect_* effect = (component_::effect_*)component_fetch.table[n_entries++][0];
		component_::texture_blend_* texture_blend = (component_::texture_blend_*)component_fetch.table[n_entries++][0];
		component_::mass_* mass = (component_::mass_*)component_fetch.table[n_entries++][0];
		component_::mob_* mob = (component_::mob_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];

		const __int32 model_ids[] = {

			Return_Model_ID("mon_bossnour", model_manager),
			Return_Model_ID("mon_demon", model_manager),
			Return_Model_ID("mon_hknight", model_manager),
			Return_Model_ID("mon_ogre", model_manager),
			Return_Model_ID("mon_seeker", model_manager),
			Return_Model_ID("mon_lostsoul", model_manager),
		};

		const __int32 n_model_ids = sizeof(model_ids) / sizeof(model_ids[0]);

		for (__int32 i_model = 0; i_model < n_models; i_model++) {

			const __int32 model_id = model_ids[i_model % n_model_ids];
			const model_& model = model_animate[model_id];

			for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
				archetype.entity_component_masks[i_model][i_group] = component_fetch.component_masks[i_group];
			}

			base[i_model].position_fixed = model_token.centre[i_model];
			base[i_model].scale = default_scale;

			move[i_model].max_speed = 0.0f;
			move[i_model].velocity = zero;
			move[i_model].displacement = zero;


			colour[i_model].colour = { 0.0f, 0.0f, 0.0f };

			//printf_s("INDEX: %i \n", index);

			memcpy(model_space[i_model].m_rotate, unit_matrix, sizeof(matrix_));

			float4_ axis_angle = { 0.0f, 1.0f, 0.0f, 0.0f };
			Axis_Angle_To_Quaternion(axis_angle, model_space_update[i_model].q_add);
			model_space_update[i_model].q_rotate = q_unit;

			bounding_box[i_model].centre = model_token.centre[i_model];
			bounding_box[i_model].extent.x = __int32(model.bounding_extent.x * fixed_scale_real);
			bounding_box[i_model].extent.y = __int32(model.bounding_extent.y * fixed_scale_real);
			bounding_box[i_model].extent.z = __int32(model.bounding_extent.z * fixed_scale_real);

			collider[i_model].extent.x = __int32(model.bounding_extent.x * 0.5f * fixed_scale_real);
			collider[i_model].extent.y = __int32(model.bounding_extent.y * 1.0f * fixed_scale_real);
			collider[i_model].extent.z = __int32(model.bounding_extent.z * 0.5f * fixed_scale_real);

			collidee_static[i_model].extent.x = (__int32)(model.bounding_extent.x * 0.5f * fixed_scale_real);
			collidee_static[i_model].extent.y = (__int32)(model.bounding_extent.y * 0.5f * fixed_scale_real);
			collidee_static[i_model].extent.z = (__int32)(model.bounding_extent.z * 0.5f * fixed_scale_real);
			collidee_static[i_model].entity_type = colliding_type_::MOB;
			collidee_static[i_model].entity_id = i_model;

			__int32 i_animation_model = -1;
			for (__int32 i_animated_model = 0; i_animated_model < animation_manager.n_animated_models; i_animated_model++) {

				const __int32 read_model_id = animation_manager.animation_model[i_animated_model].model_id;
				i_animation_model = read_model_id == model_id ? i_animated_model : i_animation_model;

				//printf_s("model id: %i , read model id: %i \n", model_id, read_model_id);
			}

			assert(i_animation_model >= 0);

			const __int32 animation_id = animation_data_::id_::IDLE;

			memset(&animation[i_model], 0, sizeof(component_::animation_));
			animation[i_model].model_id = i_animation_model;
			animation[i_model].i_current = animation_data_::id_::IDLE;
			animation[i_model].is_end_animation = false;
			animation[i_model].is_frame_change = false;
			animation[i_model].trigger_id = animation_data_::id_::NULL_;

			const __int32 i_start = animation_manager.animation_model[i_animation_model].animation_data[animation_id].i_start;

			animation[i_model].i_frames[component_::animation_::CURRENT_FRAME_LOW] = i_start + 0;
			animation[i_model].i_frames[component_::animation_::CURRENT_FRAME_HI] = i_start + 1;
			animation[i_model].i_frames[component_::animation_::NEXT_FRAME] = i_start + 2;
			animation[i_model].frame_speed = 10.0f;
			animation[i_model].frame_speed_modifier = 1.0f;

			animation_driver[i_model].i_current_animation = animation_id;
			animation_driver[i_model].trigger_id = animation_data_::id_::NULL_;

			const float3_ scale = { 1.0f, 1.0f, 1.0f };
			effect[i_model].trigger_id = component_::item_::type_::NULL_;
			effect[i_model].shrink.base_scale = scale;
			effect[i_model].shrink.shrunk_scale.x = 0.5f;
			effect[i_model].shrink.shrunk_scale.y = 0.5f;
			effect[i_model].shrink.shrunk_scale.z = 0.5f;
			effect[i_model].shrink.t_interval = 0.0f;
			effect[i_model].shrink.timer = 0.0f;
			effect[i_model].shrink.duration = 10.0f;

			float effect_duration = 10.0f;
			float time_modifier = 1.0f;
			effect[i_model].petrify.t_interval = 0.0f;
			effect[i_model].petrify.begin_time_modifier = time_modifier;
			effect[i_model].petrify.end_time_modifier = 0.0f;
			effect[i_model].petrify.time_limit = effect_duration;
			effect[i_model].petrify.timer = effect_duration;
			effect[i_model].petrify.is_running = false;

			texture_blend[i_model].interval = 0.0f;

			mass[i_model].default_value = 8.0f;
			mass[i_model].value = 8.0f;

			mob[i_model].id = 0;

			draw[i_model].draw_id = draw_id;
			draw[i_model].model_id = model_id;
		}

		//draw_call_& draw_call = draw_calls[draw_id];

		//draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		//draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		//draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour_Simple;
		//draw_call.attribute_streams[0].pixel_shader = shade_colour;

		//draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		//draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		//draw_call.attribute_streams[1].vertex_shader = NULL;
		//draw_call.attribute_streams[1].pixel_shader = shade_texture_CLAMP;
		//draw_call.n_attributes = 2;

		//draw_call.mip_level_bias = 0.2f;
		//draw_call.lighting_function = Vertex_Lighting;

	}

	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::COLOUR,
			component_id_::CLOUD,
			component_id_::SMALL_MODEL_ID,
			component_id_::DRAW,

		};

		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		float3_ model_colour = { 100.0f, 50.0f, 20.0f };
		float3_ system_position = { 0.0f, 2500.0f, 0.0f };
		float3_ scale = { 240.0f, 80.0f, 240.0f };
		const __int32 draw_id = draw_call_::id_::CLOUDS;
		float cloud_layers[] = { 10.0f, -10.0f };
		const float cloud_buffer = 240.0f;
		float3_ map_back = { 4.0f * (scale.x + cloud_buffer), 0.0f, 4.0f * (scale.z + cloud_buffer) };
		const __int32 n_clouds_length = 8;
		const float angle_radians = (360.0f / (n_clouds_length * n_clouds_length)) * RADIANS_PER_DEGREE;
		//float current_angle = 0.0f;
		float current_angle_NEW = 0.0f;

		__int32 n_models = 0;

		__int32 n_models_total = n_clouds_length * n_clouds_length;

		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models_total,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;
		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::CLOUD;
		component_fetch.component_ids[n_entries++] = component_id_::SMALL_MODEL_ID;
		component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::cloud_* cloud = (component_::cloud_*)component_fetch.table[n_entries++][0];
		component_::small_model_id_* small_model_id = (component_::small_model_id_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];
		// -----------------------------------------------------------------------------------

		for (__int32 x = 0; x < n_clouds_length; x++) {

			for (__int32 z = 0; z < n_clouds_length; z++) {

				float3_ position = {
					x * (scale.x + cloud_buffer),
					cloud_layers[x % 2],
					z * (scale.z + cloud_buffer) };

				for (__int32 i = X; i < W; i++) {
					position.f[i] -= map_back.f[i];
					position.f[i] += system_position.f[i];
				}

				for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
					archetype.entity_component_masks[n_models][i_group] = component_fetch.component_masks[i_group];
				}

				for (__int32 i_axis = X; i_axis < W; i_axis++) {
					base[n_models].position_fixed.i[i_axis] = __int32(position.f[i_axis] * fixed_scale_real);
				}
				base[n_models].scale = scale;

				colour[n_models].colour = model_colour;

				float angle_x = cos(current_angle_NEW);
				float angle_z = sin(current_angle_NEW);
				current_angle_NEW += angle_radians;

				cloud[n_models].base_scale = scale;
				cloud[n_models].base_colour = model_colour;
				cloud[n_models].seed_scale = { angle_x, 0.0f, angle_z };

				small_model_id[n_models].id = n_models;

				draw[n_models].draw_id = draw_id;
				draw[n_models].model_id = model_::id_::CLOUD;

				n_models++;
			}
		}

		draw_call_& draw_call = draw_calls[draw_id];

		//draw_call.i_vertices = model.i_vertices;
		//draw_call.vertices = model.vertices_frame[0];

		draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		//draw_call.attribute_streams[0].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR];
		//draw_call.attribute_streams[0].vertices = model.attribute_vertices[model_::ATTRIBUTE_COLOUR]->f;
		draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour_Simple;
		//draw_call.n_attributes = 1;

		draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].vertex_shader = NULL;
		draw_call.n_attributes = 2;

		draw_call.mip_level_bias = 0.2f;

		draw_call.lighting_function = Vertex_Lighting_PLAYER;

	}
	// ----------------------------------------------------------------------------------------------------------
	{

		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::COLOUR,
			component_id_::LAVA,
			component_id_::SMALL_MODEL_ID,
			component_id_::DRAW,

		};

		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);

		const float3_ colours[] = {

			{ 100.0f, 100.0f, 10.0f },
		{ 100.0f, 100.0f, 10.0f },
		{ 100.0f, 100.0f, 10.0f },
		};

		const float	lava_height = 400.0f;
		float3_ system_position = { 0.0f, lava_height, 0.0f };
		const __int32 draw_id = draw_call_::id_::LAVA;

		//const float3_ distances[] = {

		//	{ 1.0f, 0.0f, 0.0f },
		//{ 0.0f, 0.0f, 1.0f }
		//};

		const float angle_increment = 50.0f;
		const float spacing = 900.0f;
		//float angle = 0.0f;
		float angle_NEW = 0.0f;

		__int32 n_models = 0;

		const __int32 n_elements_wide = 8;
		const __int32 n_models_total = n_elements_wide * n_elements_wide;

		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models_total,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;
		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::LAVA;
		component_fetch.component_ids[n_entries++] = component_id_::SMALL_MODEL_ID;
		component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::lava_* lava = (component_::lava_*)component_fetch.table[n_entries++][0];
		component_::small_model_id_* small_model_id = (component_::small_model_id_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];
		// -----------------------------------------------------------------------------------

		for (__int32 i = 0; i < n_elements_wide; i++) {

			for (__int32 j = 0; j < n_elements_wide; j++) {

				float y = 0.0f;

				float3_ position = { (i - 5) * spacing, y, (j - 5) * spacing };
				for (__int32 i_axis = X; i_axis < W; i_axis++) {
					position.f[i_axis] += system_position.f[i_axis];
				}

				for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
					archetype.entity_component_masks[n_models][i_group] = component_fetch.component_masks[i_group];
				}

				for (__int32 i_axis = X; i_axis < W; i_axis++) {
					base[n_models].position_fixed.i[i_axis] = __int32(position.f[i_axis] * fixed_scale_real);
				}
				base[n_models].scale = { 400.0f, 400.0f, 400.0f };

				colour[n_models].colour = colours[n_models % 3];;

				float a = cos(angle_NEW * RADIANS_PER_DEGREE);
				float b = sin(angle_NEW * RADIANS_PER_DEGREE);
				lava[n_models].origin.x = (__int32)(system_position.x * fixed_scale_real);
				lava[n_models].origin.y = (__int32)(system_position.y * fixed_scale_real);
				lava[n_models].origin.z = (__int32)(system_position.z * fixed_scale_real);
				lava[n_models].base_distance = { a, 0.0f, b };
				lava[n_models].base_colour = colours[n_models % 3];

				angle_NEW += angle_increment;

				small_model_id[n_models].id = n_models;

				draw[n_models].draw_id = draw_id;
				draw[n_models].model_id = model_::id_::LAVA;

				n_models++;
			}
		}
		draw_call_& draw_call = draw_calls[draw_id];

		//draw_call.i_vertices = model.i_vertices;
		//draw_call.vertices = model.vertices_frame[0];

		draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		//draw_call.attribute_streams[0].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR];
		//draw_call.attribute_streams[0].vertices = model.attribute_vertices[model_::ATTRIBUTE_COLOUR]->f;
		draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour_Simple;
		//draw_call.n_attributes = 1;

		draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].vertex_shader = NULL;
		draw_call.n_attributes = 2;


		draw_call.lighting_function = Vertex_Lighting_PLAYER;
	}
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::COLOUR,
			component_id_::SMALL_MODEL_ID,
			//component_id_::DRAW,

		};

		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		const struct model_& model = model_animate[model_::id_::SKY_BOX];
		const float3_ model_colour = { 0.0f, 0.0f, 0.0f };
		const __int32 draw_id = draw_call_::id_::SKYBOX;

		float3_ position = { 0.0f, 0.0f, 0.0f };
		const float scale_singular = 1.0f;
		float3_ scale = { scale_singular, scale_singular, scale_singular };

		const __int32 n_models = 1;

		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 2;
		__int32 n_entries = 0;
		component_fetch.exclude_ids[n_entries++] = component_id_::CLOUD;
		component_fetch.exclude_ids[n_entries++] = component_id_::LAVA;

		n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::SMALL_MODEL_ID;
		//component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::small_model_id_* small_model_id = (component_::small_model_id_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];

		const __int32 i_model = 0;

		for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
			archetype.entity_component_masks[i_model][i_group] = component_fetch.component_masks[i_group];
		}

		for (__int32 i_axis = X; i_axis < W; i_axis++) {
			base[i_model].position_fixed.i[i_axis] = __int32(position.f[i_axis] * fixed_scale_real);
		}
		base[i_model].scale = scale;

		colour[i_model].colour = model_colour;

		small_model_id[i_model].id = i_model;

		//draw[i_model].draw_id = draw_id;
		//draw[i_model].model_id = model_::id_::SKY_BOX;

		draw_call_& draw_call = draw_calls[draw_id];

		draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour_Simple;

		draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].vertex_shader = NULL;
		draw_call.n_attributes = 2;

		draw_call.mip_level_bias = 0.2f;

	}
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::COLOUR,
			component_id_::DOOR,
			component_id_::TRIGGER,
			component_id_::SOUND_TRIGGER,
			component_id_::COLLIDEE_STATIC,
			component_id_::SMALL_MODEL_ID,
			component_id_::DRAW,

		};

		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		const struct model_& model = model_animate[model_::id_::DOOR];
		struct model_token_& model_token = model_tokens[model_token_::id_::DOOR];
		const __int32 n_models = model_token.n_models;

		const float3_ displacement[] = {

			{ 0.0f,		-80.0f,		0.0f, },
		{ -80.0f,	0.0f,		0.0f, },
		{ 0.0f,		80.0f,		0.0f, },
		};
		const float3_ model_colour = { 0.0f, 0.0f, 0.0f };
		const __int32 draw_id = draw_call_::id_::DOOR;

		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;

		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::DOOR;
		component_fetch.component_ids[n_entries++] = component_id_::TRIGGER;
		component_fetch.component_ids[n_entries++] = component_id_::SOUND_TRIGGER;
		component_fetch.component_ids[n_entries++] = component_id_::COLLIDEE_STATIC;
		component_fetch.component_ids[n_entries++] = component_id_::SMALL_MODEL_ID;
		component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::door_* door = (component_::door_*)component_fetch.table[n_entries++][0];
		component_::trigger_* trigger = (component_::trigger_*)component_fetch.table[n_entries++][0];
		component_::sound_trigger_* sound_trigger = (component_::sound_trigger_*)component_fetch.table[n_entries++][0];
		component_::collidee_static_* collidee_static = (component_::collidee_static_*)component_fetch.table[n_entries++][0];
		component_::small_model_id_* small_model_id = (component_::small_model_id_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];
		// -----------------------------------------------------------------------------------

		for (__int32 i_model = 0; i_model < n_models; i_model++) {

			for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
				archetype.entity_component_masks[i_model][i_group] = component_fetch.component_masks[i_group];
			}

			base[i_model].position_fixed = model_token.centre[i_model];
			base[i_model].scale.x = (float)(model_token.extent[i_model].x) * r_fixed_scale_real;
			base[i_model].scale.y = (float)(model_token.extent[i_model].y) * r_fixed_scale_real;
			base[i_model].scale.z = (float)(model_token.extent[i_model].z) * r_fixed_scale_real;

			colour[i_model].colour = model_colour;

			door[i_model].position_default.x = model_token.centre[i_model].x;
			door[i_model].position_default.y = model_token.centre[i_model].y;
			door[i_model].position_default.z = model_token.centre[i_model].z;
			door[i_model].move = displacement[0];
			door[i_model].interval = 0.0f;
			door[i_model].state = component_::door_::state_::STATIC;
			door[i_model].state_trigger = component_::door_::state_::NULL_;

			__int32 fixed_extent = 50 * fixed_scale;
			trigger[i_model].position_fixed.x = model_token.centre[i_model].x + (__int32)(displacement[0].x * fixed_scale_real);
			trigger[i_model].position_fixed.y = model_token.centre[i_model].y + (__int32)(displacement[0].y * fixed_scale_real);
			trigger[i_model].position_fixed.z = model_token.centre[i_model].z + (__int32)(displacement[0].z * fixed_scale_real);
			trigger[i_model].extent_fixed = { fixed_extent, fixed_extent, fixed_extent };
			trigger[i_model].is_triggered = false;

			sound_trigger[i_model].source_id = sound_triggers_::source_id_::DOOR + i_model;

			collidee_static[i_model].entity_type = colliding_type_::DOOR;
			collidee_static[i_model].entity_id = i_model;
			collidee_static[i_model].extent = model_token.extent[i_model];

			small_model_id[i_model].id = i_model;

			draw[i_model].draw_id = draw_id;
			draw[i_model].model_id = model_::id_::DOOR;
		}

		draw_call_& draw_call = draw_calls[draw_id];

		//draw_call.i_vertices = model.i_vertices;
		//draw_call.vertices = model.vertices_frame[0];

		draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		//draw_call.attribute_streams[0].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR];
		//draw_call.attribute_streams[0].vertices = model.attribute_vertices[model_::ATTRIBUTE_COLOUR]->f;
		draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour_Simple;

		draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		//draw_call.attribute_streams[1].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY];
		//draw_call.attribute_streams[1].vertices = model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY]->f;
		//draw_call.attribute_streams[1].i_textures = model.i_textures[0];
		//draw_call.attribute_streams[1].texture_handlers = model.texture_handlers;
		draw_call.attribute_streams[1].vertex_shader = NULL;
		draw_call.n_attributes = 2;

		draw_call.mip_level_bias = 0.2f;
	}
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::COLOUR,
			component_id_::TRAP_DOOR_ID,
			component_id_::COLLIDEE_STATIC,
			component_id_::SMALL_MODEL_ID,
			component_id_::DRAW,

		};

		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		const struct model_& model = model_animate[model_::id_::CUBE];
		struct model_token_& model_token = model_tokens[model_token_::id_::TRAP_DOOR];
		const __int32 n_models = model_token.n_models;

		const float3_ model_colour = { 0.0f, 0.0f, 0.0f };
		const __int32 draw_id = draw_call_::id_::TRAP_DOOR;

		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;

		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::TRAP_DOOR_ID;
		component_fetch.component_ids[n_entries++] = component_id_::COLLIDEE_STATIC;
		component_fetch.component_ids[n_entries++] = component_id_::SMALL_MODEL_ID;
		component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::trap_door_id_* trap_door_id = (component_::trap_door_id_*)component_fetch.table[n_entries++][0];
		component_::collidee_static_* collidee_static = (component_::collidee_static_*)component_fetch.table[n_entries++][0];
		component_::small_model_id_* small_model_id = (component_::small_model_id_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];
		// -----------------------------------------------------------------------------------

		for (__int32 i_model = 0; i_model < n_models; i_model++) {

			for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
				archetype.entity_component_masks[i_model][i_group] = component_fetch.component_masks[i_group];
			}

			base[i_model].position_fixed = model_token.centre[i_model];
			base[i_model].scale.x = (float)(model_token.extent[i_model].x) * r_fixed_scale_real;
			base[i_model].scale.y = (float)(model_token.extent[i_model].y) * r_fixed_scale_real;
			base[i_model].scale.z = (float)(model_token.extent[i_model].z) * r_fixed_scale_real;

			colour[i_model].colour = model_colour;

			trap_door_id[i_model].id = i_model;

			collidee_static[i_model].entity_type = colliding_type_::TRAP_DOOR;
			collidee_static[i_model].entity_id = i_model;
			collidee_static[i_model].extent = model_token.extent[i_model];

			small_model_id[i_model].id = i_model;

			draw[i_model].draw_id = draw_id;
			draw[i_model].model_id = model_::id_::CUBE;
		}

		draw_call_& draw_call = draw_calls[draw_id];

		//draw_call.i_vertices = model.i_vertices;
		//draw_call.vertices = model.vertices_frame[0];

		draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		//draw_call.attribute_streams[0].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR];
		//draw_call.attribute_streams[0].vertices = model.attribute_vertices[model_::ATTRIBUTE_COLOUR]->f;
		draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour_Simple;

		draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		//draw_call.attribute_streams[1].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY];
		//draw_call.attribute_streams[1].vertices = model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY]->f;
		//draw_call.attribute_streams[1].i_textures = model.i_textures[0];
		//draw_call.attribute_streams[1].texture_handlers = model.texture_handlers;
		draw_call.attribute_streams[1].vertex_shader = NULL;
		draw_call.n_attributes = 2;

		draw_call.mip_level_bias = 0.2f;
	}
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::COLOUR,
			component_id_::TELEPORTER,
			component_id_::TEXTURE_SPACE,
			component_id_::SMALL_MODEL_ID,
			component_id_::COLLIDEE_STATIC,
			component_id_::DRAW,
		};

		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		const __int32 model_id = model_::id_::TELEPORTER;
		const struct model_& model = model_animate[model_id];
		struct model_token_& model_token = model_tokens[model_token_::id_::TELEPORTER];
		const __int32 n_models = model_token.n_models;
		const __int32 draw_id = draw_call_::id_::TELEPORTER;

		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;
		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::TELEPORTER;
		component_fetch.component_ids[n_entries++] = component_id_::TEXTURE_SPACE;
		component_fetch.component_ids[n_entries++] = component_id_::COLLIDEE_STATIC;
		component_fetch.component_ids[n_entries++] = component_id_::SMALL_MODEL_ID;
		component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::teleporter_* teleporter = (component_::teleporter_*)component_fetch.table[n_entries++][0];
		component_::texture_space_* texture_space = (component_::texture_space_*)component_fetch.table[n_entries++][0];
		component_::collidee_static_* collidee_static = (component_::collidee_static_*)component_fetch.table[n_entries++][0];
		component_::small_model_id_* small_model_id = (component_::small_model_id_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];
		// -----------------------------------------------------------------------------------

		for (__int32 i_model = 0; i_model < n_models; i_model++) {

			for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
				archetype.entity_component_masks[i_model][i_group] = component_fetch.component_masks[i_group];
			}

			base[i_model].position_fixed = model_token.centre[i_model];
			base[i_model].scale.x = (float)(model_token.extent[i_model].x) * r_fixed_scale_real;
			base[i_model].scale.y = (float)(model_token.extent[i_model].y) * r_fixed_scale_real;
			base[i_model].scale.z = (float)(model_token.extent[i_model].z) * r_fixed_scale_real;

			colour[i_model].colour = { 0.0f, 0.0f, 0.0f };

			teleporter[i_model].destination.x = 200 * fixed_scale;
			teleporter[i_model].destination.y = 2800 * fixed_scale;
			teleporter[i_model].destination.z = -500 * fixed_scale;

			//texture_space[i_model].q_rotate = q_unit;
			//memcpy(texture_space[i_model].m_rotate, unit_matrix, sizeof(matrix_));

			{
				float3_ axis = { 0.0f, 0.0f, 1.0f };
				Vector_Normalise(axis);
				float4_ axis_angle = { axis.x, axis.y, axis.z, 4.0f };
				Axis_Angle_To_Quaternion(axis_angle, texture_space[i_model].q_add);
				texture_space[i_model].q_rotate = q_unit;
				memcpy(texture_space[i_model].m_rotate, unit_matrix, sizeof(matrix_));
			}

			collidee_static[i_model].extent = model_token.extent[i_model];
			collidee_static[i_model].entity_type = colliding_type_::TELEPORTER;
			collidee_static[i_model].entity_id = i_model;

			small_model_id[i_model].id = i_model;

			draw[i_model].draw_id = draw_id;
			draw[i_model].model_id = model_id;
		}

		draw_call_& draw_call = draw_calls[draw_id];

		//draw_call.i_vertices = model.i_vertices;
		//draw_call.vertices = model.vertices_frame[0];

		draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		//draw_call.attribute_streams[0].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR];
		//draw_call.attribute_streams[0].vertices = model.attribute_vertices[model_::ATTRIBUTE_COLOUR]->f;
		draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour_Simple;

		draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		//draw_call.attribute_streams[1].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY];
		//draw_call.attribute_streams[1].vertices = model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY]->f;
		//draw_call.attribute_streams[1].i_textures = model.i_textures[0];
		//draw_call.attribute_streams[1].texture_handlers = model.texture_handlers;
		draw_call.attribute_streams[1].vertex_shader = Shade_Vertex_Texture;
		draw_call.n_attributes = 2;

		draw_call.mip_level_bias = 0.2f;
		draw_call.lighting_function = Vertex_Lighting_PLAYER;

	}
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::MODEL_SPACE,
			component_id_::COLOUR,
			component_id_::COLOUR_SPACE,
			component_id_::TEXTURE_SPACE,
			component_id_::COLLIDEE_ROTATED,
			component_id_::POWER,
			component_id_::DRAW,

		};

		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		const struct model_& model = model_animate[model_::id_::BOUNCE_PAD];
		struct model_token_& model_token = model_tokens[model_token_::id_::BOUNCE_PAD];
		const __int32 n_models = model_token.n_models;
		const __int32 draw_id = draw_call_::id_::BOUNCE_PAD;
		const float3_ model_colour = { 0.0f, 0.0f, 0.0f };
		static const float3_ model_scale = { 40.0f, 15.0f, 40.0f };

		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		i_archetype_bounce_pad = archetype_data.n_archetypes;
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;

		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::MODEL_SPACE;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR_SPACE;
		component_fetch.component_ids[n_entries++] = component_id_::TEXTURE_SPACE;
		component_fetch.component_ids[n_entries++] = component_id_::COLLIDEE_ROTATED;
		component_fetch.component_ids[n_entries++] = component_id_::POWER;
		component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::model_space_* model_space = (component_::model_space_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::colour_space_* colour_space = (component_::colour_space_*)component_fetch.table[n_entries++][0];
		component_::texture_space_* texture_space = (component_::texture_space_*)component_fetch.table[n_entries++][0];
		component_::collidee_rotated_* collidee_rotated = (component_::collidee_rotated_*)component_fetch.table[n_entries++][0];
		component_::power_* power = (component_::power_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];
		// -----------------------------------------------------------------------------------

		for (__int32 i_model = 0; i_model < n_models; i_model++) {

			__int32 i_model_token = INVALID_RESULT;
			for (__int32 i_token = 0; i_token < model_token.n_models; i_token++) {
				i_model_token = i_model == model_token.index[i_token] ? i_token : i_model_token;
			}

			assert(i_model_token != INVALID_RESULT);

			for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
				archetype.entity_component_masks[i_model][i_group] = component_fetch.component_masks[i_group];
			}

			base[i_model].position_fixed = model_token.centre[i_model_token];
			base[i_model].scale = model_scale;

			colour[i_model].colour = model_colour;

			{
				float3_ axis = { 0.0f, 1.0f, 0.0f };
				Vector_Normalise(axis);
				float4_ axis_angle = { axis.x, axis.y, axis.z, 4.0f };
				Axis_Angle_To_Quaternion(axis_angle, colour_space[i_model].q_add);
				colour_space[i_model].q_rotate = q_unit;
				memcpy(colour_space[i_model].m_rotate, unit_matrix, sizeof(matrix_));
			}

			memcpy(model_space[i_model].m_rotate, unit_matrix, sizeof(matrix_));

			model_space[i_model].m_rotate[X].x = model_token.normals[i_model_token][5].x;
			model_space[i_model].m_rotate[X].y = model_token.normals[i_model_token][5].y;
			model_space[i_model].m_rotate[X].z = model_token.normals[i_model_token][5].z;

			model_space[i_model].m_rotate[Y].x = model_token.normals[i_model_token][3].x;
			model_space[i_model].m_rotate[Y].y = model_token.normals[i_model_token][3].y;
			model_space[i_model].m_rotate[Y].z = model_token.normals[i_model_token][3].z;

			model_space[i_model].m_rotate[Z].x = model_token.normals[i_model_token][2].x;
			model_space[i_model].m_rotate[Z].y = model_token.normals[i_model_token][2].y;
			model_space[i_model].m_rotate[Z].z = model_token.normals[i_model_token][2].z;

			{
				float3_ axis = { 0.0f, 0.0f, 1.0f };
				Vector_Normalise(axis);
				float4_ axis_angle = { axis.x, axis.y, axis.z, 4.0f };
				Axis_Angle_To_Quaternion(axis_angle, texture_space[i_model].q_add);
				texture_space[i_model].q_rotate = q_unit;
				memcpy(texture_space[i_model].m_rotate, unit_matrix, sizeof(matrix_));
			}

			collidee_rotated[i_model].entity_type = colliding_type_::BOUNCE_PAD;
			collidee_rotated[i_model].entity_id = i_model;

			power[i_model].is_on = true;
			power[i_model].q_null = q_unit;
			power[i_model].q_add_colour = colour_space[i_model].q_add;
			power[i_model].q_add_texture = texture_space[i_model].q_add;

			const int3_ i_normal = { 5, 3, 2 };

			for (__int32 i_axis = X; i_axis < W; i_axis++) {

				__int32 mag = __int32(50.0f * fixed_scale_real);
				collidee_rotated[i_model].extent.i[i_axis] = __int32(model_scale.f[i_axis] * fixed_scale_real);
				//collidee_rotated[i_model].extent.i[i_axis] = mag;

				collidee_rotated[i_model].normals[X].i[i_axis] = __int32(model_token.normals[i_model_token][i_normal.x].f[i_axis] * fixed_scale_real);
				collidee_rotated[i_model].normals[Y].i[i_axis] = __int32(model_token.normals[i_model_token][i_normal.y].f[i_axis] * fixed_scale_real);
				collidee_rotated[i_model].normals[Z].i[i_axis] = __int32(model_token.normals[i_model_token][i_normal.z].f[i_axis] * fixed_scale_real);
			}

			draw[i_model].draw_id = draw_id;
			draw[i_model].model_id = model_::id_::BOUNCE_PAD;
		}

		draw_call_& draw_call = draw_calls[draw_id];

		//draw_call.i_vertices = model.i_vertices;
		//draw_call.vertices = model.vertices_frame[0];

		draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		//draw_call.attribute_streams[0].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR];
		//draw_call.attribute_streams[0].vertices = model.attribute_vertices[model_::ATTRIBUTE_COLOUR]->f;
		draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour;

		draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		//draw_call.attribute_streams[1].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY];
		//draw_call.attribute_streams[1].vertices = model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY]->f;
		//draw_call.attribute_streams[1].i_textures = model.i_textures[0];
		//draw_call.attribute_streams[1].texture_handlers = model.texture_handlers;
		draw_call.attribute_streams[1].vertex_shader = Shade_Vertex_Texture;
		draw_call.n_attributes = 2;

		draw_call.mip_level_bias = 0.2f;
		draw_call.lighting_function = Vertex_Lighting_PLAYER;

	}
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::COLOUR,
			component_id_::MOVE,
			component_id_::PLATFORM,
			component_id_::POWER,
			component_id_::ATTACHED,
			component_id_::COLLIDEE_MOVING,
			component_id_::SMALL_MODEL_ID,
			component_id_::TEXTURE_BLEND,
			component_id_::DRAW,

		};

		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		const struct model_& model = model_animate[model_::id_::PLATFORM];
		struct model_token_& model_token = model_tokens[model_token_::id_::PLATFORM];
		const __int32 n_models = model_token.n_models;
		const __int32 draw_id = draw_call_::id_::PLATFORMS;
		const float3_ model_colour = { 0.0f, 0.0f, 0.0f };
		const float3_ model_scale = { 40.0f, 40.0f, 40.0f };
		//const float model_velocity = 30.0f;

		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		i_archetype_platform = archetype_data.n_archetypes;
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;
		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::MOVE;
		component_fetch.component_ids[n_entries++] = component_id_::PLATFORM;
		component_fetch.component_ids[n_entries++] = component_id_::POWER;
		component_fetch.component_ids[n_entries++] = component_id_::ATTACHED;
		component_fetch.component_ids[n_entries++] = component_id_::COLLIDEE_MOVING;
		component_fetch.component_ids[n_entries++] = component_id_::SMALL_MODEL_ID;
		component_fetch.component_ids[n_entries++] = component_id_::TEXTURE_BLEND;
		component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::move_* move = (component_::move_*)component_fetch.table[n_entries++][0];
		component_::platform_* platform = (component_::platform_*)component_fetch.table[n_entries++][0];
		component_::power_* power = (component_::power_*)component_fetch.table[n_entries++][0];
		component_::attached_* attached = (component_::attached_*)component_fetch.table[n_entries++][0];
		component_::collidee_moving_* collidee_moving = (component_::collidee_moving_*)component_fetch.table[n_entries++][0];
		component_::small_model_id_* small_model_id = (component_::small_model_id_*)component_fetch.table[n_entries++][0];
		component_::texture_blend_* texture_blend = (component_::texture_blend_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];
		// -----------------------------------------------------------------------------------



		for (__int32 i_model = 0; i_model < n_models; i_model++) {

			__int32 i_model_token = INVALID_RESULT;
			for (__int32 i_token = 0; i_token < model_token.n_models; i_token++) {
				i_model_token = i_model == model_token.index[i_token] ? i_token : i_model_token;
			}

			assert(i_model_token != INVALID_RESULT);


			const int3_ centre = model_token.centre[i_model_token];
			const int3_ extent = model_token.extent[i_model_token];
			__int32 max_side = 0;
			__int32 i_axis_out = X;
			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				bool is_greater = extent.i[i_axis] > max_side;
				max_side = blend_int(extent.i[i_axis], max_side, is_greater);
				i_axis_out = blend_int(i_axis, i_axis_out, is_greater);
			}

			for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
				archetype.entity_component_masks[i_model][i_group] = component_fetch.component_masks[i_group];
			}

			base[i_model].scale = model_scale;
			base[i_model].position_fixed = centre;
			base[i_model].position_fixed.i[i_axis_out] -= extent.i[i_axis_out];
			base[i_model].position_fixed.y += extent.i[Y];

			colour[i_model].colour = model_colour;

			move[i_model].velocity = zero;
			move[i_model].displacement = zero;

			platform[i_model].endpoints[0] = centre;
			platform[i_model].endpoints[1] = centre;
			platform[i_model].endpoints[0].i[i_axis_out] = centre.i[i_axis_out] - extent.i[i_axis_out];
			platform[i_model].endpoints[1].i[i_axis_out] = centre.i[i_axis_out] + extent.i[i_axis_out];
			platform[i_model].endpoints[0].i[Y] = centre.i[Y] - extent.i[Y];
			platform[i_model].endpoints[1].i[Y] = centre.i[Y] + extent.i[Y];

			platform[i_model].axis_travel = i_axis_out;
			platform[i_model].prev_position = base[i_model].position_fixed;
			platform[i_model].intervals = zero;
			platform[i_model].intervals.y = 1.0f;
			platform[i_model].speeds = zero;
			platform[i_model].speeds.f[i_axis_out] = 0.1f;
			platform[i_model].speeds.y = 0.2f;

			platform[i_model].blend.t_interval = 0.0f;
			platform[i_model].blend.blend_speed = 0.25f;

			power[i_model].is_on = true;

			//attached[i_model].is_attachment = (i_model == 3) ? true : false;
			attached[i_model].is_attachment = false;
			attached[i_model].i_archetype = 9;
			attached[i_model].i_entity = 1;

			collidee_moving[i_model].extent.x = __int32(model_scale.x * fixed_scale_real);;
			collidee_moving[i_model].extent.y = __int32(model_scale.y * fixed_scale_real);;
			collidee_moving[i_model].extent.z = __int32(model_scale.z * fixed_scale_real);;
			collidee_moving[i_model].entity_type = colliding_type_::PLATFORM;
			collidee_moving[i_model].entity_id = i_model;

			small_model_id[i_model].id = i_model;

			texture_blend[i_model].interval = 0.0f;

			draw[i_model].draw_id = draw_id;
			draw[i_model].model_id = model_::id_::PLATFORM;
		}

		draw_call_& draw_call = draw_calls[draw_id];

		draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour_Simple;

		draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].vertex_shader = NULL;

		draw_call.attribute_streams[2].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[2].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		draw_call.attribute_streams[2].vertex_shader = NULL;
		draw_call.n_attributes = 3;

		draw_call.n_additional_pixel_shaders = 1;

		draw_call.mip_level_bias = 0.2f;
	}
	// ----------------------------------------------------------------------------------------------------------

	struct interaction_ {

		__int32 token_id;
		__int32 token_index;
		__int32 i_archetype;
		__int32 i_entity;
		bool is_hold;
	};

	interaction_ interaction_table[] = {

		{ model_token_::id_::PLATE,		0, i_archetype_bounce_pad,	2, false },
	{ model_token_::id_::PLATE,		1, i_archetype_platform,	5, false },
	{ model_token_::id_::PLATE,		2, i_archetype_platform,	4, true },
	{ model_token_::id_::PLATE,		3, i_archetype_bounce_pad,	3, false },
	{ model_token_::id_::BUTTON,	0, i_archetype_platform,	1, false },
	{ model_token_::id_::BUTTON,	1, i_archetype_platform,	2, false },
	{ model_token_::id_::BUTTON,	2, i_archetype_platform,	3, false },

	};

	__int32 n_interactions = sizeof(interaction_table) / sizeof(interaction_table[0]);

	// ----------------------------------------------------------------------------------------------------------

	{
		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::MOVE,
			component_id_::COLOUR,
			component_id_::PLATE,
			component_id_::SWITCH,
			component_id_::SOUND_TRIGGER,
			component_id_::COLLIDEE_STATIC,
			component_id_::SMALL_MODEL_ID,
			component_id_::DRAW,

		};

		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		const struct model_& model = model_animate[model_::id_::PLATE];
		struct model_token_& model_token = model_tokens[model_token_::id_::PLATE];
		const __int32 n_models = model_token.n_models;
		const __int32 draw_id = draw_call_::id_::PLATE;
		//const float3_ model_colour = { 0.0f, 100.0f, 0.0f };
		//const float3_ model_scale = { 40.0f, 10.0f, 40.0f };

		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];

		//printf_s("ARCHETYPE PLATE: %i \n", archetype_data.n_archetypes);

		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;

		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::MOVE;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::PLATE;
		component_fetch.component_ids[n_entries++] = component_id_::SWITCH;
		component_fetch.component_ids[n_entries++] = component_id_::SOUND_TRIGGER;
		component_fetch.component_ids[n_entries++] = component_id_::COLLIDEE_STATIC;
		component_fetch.component_ids[n_entries++] = component_id_::SMALL_MODEL_ID;
		component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::move_* move = (component_::move_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::plate_* plate = (component_::plate_*)component_fetch.table[n_entries++][0];
		component_::switch_* switch_ = (component_::switch_*)component_fetch.table[n_entries++][0];
		component_::sound_trigger_* sound_trigger = (component_::sound_trigger_*)component_fetch.table[n_entries++][0];
		component_::collidee_static_* collidee_static = (component_::collidee_static_*)component_fetch.table[n_entries++][0];
		component_::small_model_id_* small_model_id = (component_::small_model_id_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];
		// -----------------------------------------------------------------------------------



		for (__int32 i_model = 0; i_model < n_models; i_model++) {

			__int32 i_model_token = INVALID_RESULT;
			for (__int32 i_token = 0; i_token < model_token.n_models; i_token++) {
				i_model_token = i_model == model_token.index[i_token] ? i_token : i_model_token;
			}

			assert(i_model_token != INVALID_RESULT);


			const int3_ centre = model_token.centre[i_model_token];
			const int3_ extent = model_token.extent[i_model_token];

			for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
				archetype.entity_component_masks[i_model][i_group] = component_fetch.component_masks[i_group];
			}

			base[i_model].position_fixed = centre;
			base[i_model].scale.x = (float)extent.x * r_fixed_scale_real;
			base[i_model].scale.y = (float)extent.y * r_fixed_scale_real;
			base[i_model].scale.z = (float)extent.z * r_fixed_scale_real;

			move[i_model].velocity = zero;
			// etc

			int3_ origin = { 0, 0, 0 };
			plate[i_model].t_interval = 0.0f;
			plate[i_model].colours[0] = { 0.0f, 100.0f, 0.0f };
			plate[i_model].colours[1] = { 0.0f, 0.0f, 100.0f };
			plate[i_model].positions[0] = origin;
			plate[i_model].positions[1] = origin;
			plate[i_model].positions[1].y -= 4 * fixed_scale;

			__int32 i_entry = INVALID_RESULT;
			for (__int32 i_interaction = 0; i_interaction < n_interactions; i_interaction++) {
				bool is_token_id = (interaction_table[i_interaction].token_id == model_token_::id_::PLATE);
				bool is_token_index = (interaction_table[i_interaction].token_index == i_model);
				i_entry = is_token_id && is_token_index ? i_interaction : i_entry;
			}

			switch_[i_model].id = i_model;
			switch_[i_model].is_on = false;
			switch_[i_model].was_on = false;
			switch_[i_model].is_hold = true;
			switch_[i_model].i_archetype_target = i_archetype_platform;
			switch_[i_model].i_entity_target = i_model;

			sound_trigger[i_model].source_id = sound_triggers_::source_id_::BUTTON + i_model;

			colour[i_model].colour = plate[i_model].colours[0];

			collidee_static[i_model].extent = extent;
			collidee_static[i_model].entity_type = colliding_type_::PLATE;
			collidee_static[i_model].entity_id = i_model;

			small_model_id[i_model].id = i_model;

			draw[i_model].draw_id = draw_id;
			draw[i_model].model_id = model_::id_::PLATE;
		}

		draw_call_& draw_call = draw_calls[draw_id];

		//draw_call.i_vertices = model.i_vertices;
		//draw_call.vertices = model.vertices_frame[0];

		draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		//draw_call.attribute_streams[0].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR];
		//draw_call.attribute_streams[0].vertices = model.attribute_vertices[model_::ATTRIBUTE_COLOUR]->f;
		draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour_Simple;

		draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		//draw_call.attribute_streams[1].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY];
		//draw_call.attribute_streams[1].vertices = model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY]->f;
		//draw_call.attribute_streams[1].i_textures = model.i_textures[0];
		//draw_call.attribute_streams[1].texture_handlers = model.texture_handlers;
		draw_call.attribute_streams[1].vertex_shader = NULL;
		draw_call.n_attributes = 2;

		draw_call.mip_level_bias = 0.2f;
		draw_call.lighting_function = Vertex_Lighting_PLAYER;

	}
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::COLOUR,
			component_id_::BUTTON,
			component_id_::SWITCH,
			component_id_::SOUND_TRIGGER,
			component_id_::COLLIDEE_STATIC,
			component_id_::SMALL_MODEL_ID,
			component_id_::DRAW,

		};

		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		const struct model_& model = model_animate[model_::id_::BUTTON];
		struct model_token_& model_token = model_tokens[model_token_::id_::BUTTON];
		const __int32 n_models = model_token.n_models;
		const __int32 draw_id = draw_call_::id_::BUTTON;
		//const float3_ model_colour = { 0.0f, 100.0f, 0.0f };

		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;

		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::BUTTON;
		component_fetch.component_ids[n_entries++] = component_id_::SWITCH;
		component_fetch.component_ids[n_entries++] = component_id_::SOUND_TRIGGER;
		component_fetch.component_ids[n_entries++] = component_id_::COLLIDEE_STATIC;
		component_fetch.component_ids[n_entries++] = component_id_::SMALL_MODEL_ID;
		component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::button_* button = (component_::button_*)component_fetch.table[n_entries++][0];
		component_::switch_* switch_ = (component_::switch_*)component_fetch.table[n_entries++][0];
		component_::sound_trigger_* sound_trigger = (component_::sound_trigger_*)component_fetch.table[n_entries++][0];
		component_::collidee_static_* collidee_static = (component_::collidee_static_*)component_fetch.table[n_entries++][0];
		component_::small_model_id_* small_model_id = (component_::small_model_id_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];
		// -----------------------------------------------------------------------------------

		const __int32 n_buttons = 3;
		//const __int32 i_entity_targets[n_buttons] = {

		//	1, 2, 3,
		//};

		for (__int32 i_model = 0; i_model < n_models; i_model++) {

			__int32 i_model_token = INVALID_RESULT;
			for (__int32 i_token = 0; i_token < model_token.n_models; i_token++) {
				i_model_token = i_model == model_token.index[i_token] ? i_token : i_model_token;
			}

			assert(i_model_token != INVALID_RESULT);

			const int3_ centre = model_token.centre[i_model_token];
			const int3_ extent = model_token.extent[i_model_token];

			for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
				archetype.entity_component_masks[i_model][i_group] = component_fetch.component_masks[i_group];
			}

			base[i_model].position_fixed = centre;
			base[i_model].scale.x = (float)extent.x * r_fixed_scale_real;
			base[i_model].scale.y = (float)extent.y * r_fixed_scale_real;
			base[i_model].scale.z = (float)extent.z * r_fixed_scale_real;

			button[i_model].id = i_model;
			button[i_model].colours[0] = { 0.0f, 100.0f, 0.0f };
			button[i_model].colours[1] = { 0.0f, 0.0f, 100.0f };

			__int32 i_entry = INVALID_RESULT;
			for (__int32 i_interaction = 0; i_interaction < n_interactions; i_interaction++) {
				bool is_token_id = (interaction_table[i_interaction].token_id == model_token_::id_::BUTTON);
				bool is_token_index = (interaction_table[i_interaction].token_index == i_model);
				i_entry = is_token_id && is_token_index ? i_interaction : i_entry;
			}

			switch_[i_model].id = i_model;
			switch_[i_model].is_on = false;
			switch_[i_model].was_on = false;
			switch_[i_model].is_hold = interaction_table[i_entry].is_hold;
			switch_[i_model].i_archetype_target = interaction_table[i_entry].i_archetype;
			switch_[i_model].i_entity_target = interaction_table[i_entry].i_entity;

			sound_trigger[i_model].source_id = sound_triggers_::source_id_::BUTTON + i_model;

			colour[i_model].colour = button[i_model].colours[0];

			collidee_static[i_model].extent = extent;
			collidee_static[i_model].entity_type = colliding_type_::BUTTON;
			collidee_static[i_model].entity_id = i_model;

			small_model_id[i_model].id = i_model;

			draw[i_model].draw_id = draw_id;
			draw[i_model].model_id = model_::id_::BUTTON;
		}

		draw_call_& draw_call = draw_calls[draw_id];

		//draw_call.i_vertices = model.i_vertices;
		//draw_call.vertices = model.vertices_frame[0];

		draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		//draw_call.attribute_streams[0].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR];
		//draw_call.attribute_streams[0].vertices = model.attribute_vertices[model_::ATTRIBUTE_COLOUR]->f;
		draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour_Simple;

		draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		//draw_call.attribute_streams[1].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY];
		//draw_call.attribute_streams[1].vertices = model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY]->f;
		//draw_call.attribute_streams[1].i_textures = model.i_textures[0];
		//draw_call.attribute_streams[1].texture_handlers = model.texture_handlers;
		draw_call.attribute_streams[1].vertex_shader = NULL;
		draw_call.n_attributes = 2;

		draw_call.mip_level_bias = 0.2f;
		draw_call.lighting_function = Vertex_Lighting_PLAYER;

	}
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::MAP_COLLIDE,
			//component_id_::COLLIDEE_STATIC,
		};

		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		model_token_& model_token = model_tokens[model_token_::id_::MAP];
		const __int32 n_models_total = model_token.n_models;

		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models_total,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;

		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::MAP_COLLIDE;
		//component_fetch.component_ids[n_entries++] = component_id_::COLLIDEE_STATIC;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::map_collide_* map_collide = (component_::map_collide_*)component_fetch.table[n_entries++][0];
		//component_::collidee_static_* collidee_static = (component_::collidee_static_*)component_fetch.table[n_entries++][0];
		// -----------------------------------------------------------------------------------

		__int32 n_models = 0;

		for (__int32 i_node = 0; i_node < grid_::NUM_NODES; i_node++) {

			model_token_& model_token = model_tokens[model_token_::id_::MAP + i_node + 1];

			grid.nodes[i_node].map.n_entries = 0;

			for (__int32 i_model = 0; i_model < model_token.n_models; i_model++) {

				for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
					archetype.entity_component_masks[n_models][i_group] = component_fetch.component_masks[i_group];
				}

				base[n_models].position_fixed.x = model_token.centre[i_model].x + grid.nodes[i_node].map.centre.x;
				base[n_models].position_fixed.y = model_token.centre[i_model].y + grid.nodes[i_node].map.centre.y;
				base[n_models].position_fixed.z = model_token.centre[i_model].z + grid.nodes[i_node].map.centre.z;

				base[n_models].scale.x = (float)(model_token.extent[i_model].x) * r_fixed_scale_real;
				base[n_models].scale.y = (float)(model_token.extent[i_model].y) * r_fixed_scale_real;
				base[n_models].scale.z = (float)(model_token.extent[i_model].z) * r_fixed_scale_real;

				map_collide[n_models].extent = model_token.extent[i_model];
				map_collide[n_models].entity_type = colliding_type_::MAP;
				map_collide[n_models].entity_id = n_models;

				grid.centre[n_models] = base[n_models].position_fixed;
				grid.extent[n_models] = model_token.extent[i_model];
				grid.bounding_box[n_models] = model_token.extent[i_model];
				grid.data[n_models].entity_id = n_models;
				grid.data[n_models].entity_type = colliding_type_::MAP;
				grid.data[n_models].i_entity = 0;
				grid.data[n_models].i_archetype = 0;
				grid.data[n_models].type_id = grid_::type_::id_::STATIC;

				grid.nodes[i_node].indices[grid.nodes[i_node].map.n_entries] = n_models;
				grid.nodes[i_node].map.n_entries++;

				n_models++;
			}
		}

		grid.n_entries_map = n_models;
	}
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::MAP_BVH,
			//component_id_::COLLIDEE_STATIC,
		};

		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		const __int32 n_models = grid_::NUM_NODES;

		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;

		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::MAP_BVH;
		//component_fetch.component_ids[n_entries++] = component_id_::COLLIDEE_STATIC;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::map_bvh_* map_bvh = (component_::map_bvh_*)component_fetch.table[n_entries++][0];
		//component_::collidee_static_* collidee_static = (component_::collidee_static_*)component_fetch.table[n_entries++][0];
		// -----------------------------------------------------------------------------------

		for (__int32 i_model = 0; i_model < grid_::NUM_NODES; i_model++) {

			for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
				archetype.entity_component_masks[i_model][i_group] = component_fetch.component_masks[i_group];
			}

			//base[i_model].position_fixed = grid.nodes[i_model].centre;

			//base[i_model].scale.x = (float)(grid.nodes[i_model].extent.x) * r_fixed_scale_real;
			//base[i_model].scale.y = (float)(grid.nodes[i_model].extent.y) * r_fixed_scale_real;
			//base[i_model].scale.z = (float)(grid.nodes[i_model].extent.z) * r_fixed_scale_real;

			//map_bvh[i_model].extent = grid.nodes[i_model].extent;
			//map_bvh[i_model].i_start = 0;
			//map_bvh[i_model].n_models = grid.nodes[i_model].n_entries;

		}
	}
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::COLOUR,
			component_id_::MAP_RENDER,
			component_id_::BOUNDING_BOX,
			component_id_::DRAW,

		};

		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		//const struct model_& model = model_animate[model_::id_::MAP + 1];
		const __int32 n_models = grid_::NUM_NODES;
		//const __int32 n_models = 1;

		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;

		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::MAP_RENDER;
		component_fetch.component_ids[n_entries++] = component_id_::BOUNDING_BOX;
		component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::map_render_* map_render = (component_::map_render_*)component_fetch.table[n_entries++][0];
		component_::bounding_box_* bounding_box = (component_::bounding_box_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];
		// -----------------------------------------------------------------------------------

		for (__int32 i_model = 0; i_model < n_models; i_model++) {

			for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
				archetype.entity_component_masks[i_model][i_group] = component_fetch.component_masks[i_group];
			}

			const __int32 i_node = i_model + 1;
			//const __int32 i_node = i_model + 0;

			base[i_model].position_fixed = grid.nodes[i_model].map.centre;
			//base[i_model].position_fixed = { 0, 0, 0 };
			base[i_model].scale = { 1.0f, 1.0f, 1.0f };

			colour[i_model].colour = { 0.0f, 0.0f, 0.0f };

			map_render[i_model].id = i_model;

			bounding_box[i_model].centre = grid.nodes[i_model].map.centre;
			bounding_box[i_model].extent = grid.nodes[i_model].map.extent;

			const __int32 draw_id = draw_call_::id_::MAP + i_node;
			const __int32 model_id = model_::id_::MAP + i_node;
			draw[i_model].draw_id = draw_id;
			draw[i_model].model_id = model_id;
			draw_call_& draw_call = draw_calls[draw_id];

			model_& model = model_animate[model_id];
			//draw_call.i_vertices = model.i_vertices;
			//draw_call.vertices = model.vertices_frame[0];

			draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
			draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
			//draw_call.attribute_streams[0].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR];
			//draw_call.attribute_streams[0].vertices = model.attribute_vertices[model_::ATTRIBUTE_COLOUR]->f;
			draw_call.attribute_streams[0].vertex_shader = NULL;

			draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
			draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
			//draw_call.attribute_streams[1].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY];
			//draw_call.attribute_streams[1].vertices = model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY]->f;
			//draw_call.attribute_streams[1].i_textures = model.i_textures[0];
			//draw_call.attribute_streams[1].texture_handlers = model.texture_handlers;
			draw_call.attribute_streams[1].vertex_shader = NULL;

			draw_call.attribute_streams[2].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
			draw_call.attribute_streams[2].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
			//draw_call.attribute_streams[2].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_SECONDARY];
			//draw_call.attribute_streams[2].vertices = model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_SECONDARY]->f;
			//draw_call.attribute_streams[2].i_textures = model.i_textures[1];
			//draw_call.attribute_streams[2].texture_handlers = model.texture_handlers;
			draw_call.attribute_streams[2].vertex_shader = NULL;

			//draw_call.n_attributes = 1;
			draw_call.n_attributes = 3;
			draw_call.mip_level_bias = 0.1f;


		}



		//draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		//draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		//draw_call.attribute_streams[1].i_vertices = model.i_texture_vertices[0];
		//draw_call.attribute_streams[1].vertices = model.texture_vertices[0][0].f;
		//draw_call.attribute_streams[1].i_textures = model.i_textures[0];
		//draw_call.attribute_streams[1].texture_handlers = model.texture_handlers;
		//draw_call.attribute_streams[1].pixel_shader = shade_texture_CLAMP;
		//draw_call.attribute_streams[1].vertex_shader = NULL;

		//draw_call.attribute_streams[2].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		//draw_call.attribute_streams[2].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		//draw_call.attribute_streams[2].i_vertices = model.i_texture_vertices[1];
		//draw_call.attribute_streams[2].vertices = model.texture_vertices[1][0].f;
		//draw_call.attribute_streams[2].i_textures = model.i_textures[1];
		//draw_call.attribute_streams[2].texture_handlers = model.texture_handlers;
		//draw_call.attribute_streams[2].pixel_shader = shade_texture_WRAP;
		//draw_call.attribute_streams[2].vertex_shader = NULL;

		//draw_call.n_attributes = 3;

		//draw_call.mip_level_bias = 0.1f;
	}
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::MOVE,
			component_id_::MASS,
			component_id_::COLLIDER,
			component_id_::PARTICLE,
			component_id_::LIGHTMAP_LIGHT_SOURCE,
			component_id_::COLOUR,
			component_id_::SMALL_MODEL_ID,
			component_id_::DRAW,

		};

		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		const struct model_& model = model_animate[model_::id_::CUBE];
		const __int32 draw_id = draw_call_::id_::PARTICLES;

		const float3_ model_scale = { 4.0f, 4.0f, 4.0f };
		const float3_ zero = { 0.0f,0.0f,0.0f };
		const float3_ model_colour = { 50.0f, 100.0f, 50.0f };



		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			particle_manager_::NUM_PARTICLES_TOTAL,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;
		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::MOVE;
		component_fetch.component_ids[n_entries++] = component_id_::MASS;
		component_fetch.component_ids[n_entries++] = component_id_::COLLIDER;
		component_fetch.component_ids[n_entries++] = component_id_::PARTICLE;
		component_fetch.component_ids[n_entries++] = component_id_::LIGHTMAP_LIGHT_SOURCE;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::SMALL_MODEL_ID;
		component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::move_* move = (component_::move_*)component_fetch.table[n_entries++][0];
		component_::mass_* mass = (component_::mass_*)component_fetch.table[n_entries++][0];
		component_::collider_* collider = (component_::collider_*)component_fetch.table[n_entries++][0];
		component_::particle_* particle = (component_::particle_*)component_fetch.table[n_entries++][0];
		component_::lightmap_light_source_* lightmap_light_source = (component_::lightmap_light_source_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::small_model_id_* small_model_id = (component_::small_model_id_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];
		// -----------------------------------------------------------------------------------

		const float velocity_scale = 4.0f;
		__int32 n_models = 0;

		for (__int32 i_emitter = 0; i_emitter < particle_manager_::NUM_EMITTERS; i_emitter++) {

			for (__int32 i_particle = 0; i_particle < particle_manager_::NUM_PARTICLES_PER_EMITTER; i_particle++) {

				//const float3_ position = { x_current - half_extent, y_current - half_extent, z_current - half_extent };
				//const float3_ position = position_grid[n_models % particle_manager_::NUM_PARTICLES_PER_EMITTER];
				const float3_ position = zero;
				float3_ base_velocity;
				for (__int32 i = X; i < W; i++) {
					base_velocity.f[i] = position.f[i] * velocity_scale;
				}

				for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
					archetype.entity_component_masks[n_models][i_group] = component_fetch.component_masks[i_group];
				}

				base[n_models].scale = model_scale;
				for (__int32 i_axis = X; i_axis < W; i_axis++) {
					base[n_models].position_fixed.i[i_axis] = __int32(position.f[i_axis] * fixed_scale_real);
				}

				move[n_models].displacement = zero;
				move[n_models].velocity = zero;

				mass[n_models].default_value = 4.0f;
				mass[n_models].value = 4.0f;

				for (__int32 i_axis = X; i_axis < W; i_axis++) {
					collider[n_models].extent.i[i_axis] = __int32(model_scale.f[i_axis] * fixed_scale_real);
				}
				collider[n_models].entity_type = colliding_type_::PARTICLES;
				collider[n_models].entity_id = n_models;

				particle[n_models].id = n_models;
				particle[n_models].base_velocity = base_velocity;

				lightmap_light_source[n_models].id = n_models;
				lightmap_light_source[n_models].intensity = 0.5f;

				colour[n_models].colour = model_colour;

				small_model_id[n_models].id = n_models;

				draw[n_models].draw_id = draw_id;
				draw[n_models].model_id = model_::id_::CUBE;

				n_models++;
			}
		}

		draw_call_& draw_call = draw_calls[draw_id];

		//draw_call.i_vertices = model.i_vertices;
		//draw_call.vertices = model.vertices_frame[0];

		draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		//draw_call.attribute_streams[0].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR];
		//draw_call.attribute_streams[0].vertices = model.attribute_vertices[model_::ATTRIBUTE_COLOUR]->f;
		draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour_Simple;
		//draw_call.n_attributes = 1;

		draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		//draw_call.attribute_streams[1].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY];
		//draw_call.attribute_streams[1].vertices = model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY]->f;
		//draw_call.attribute_streams[1].i_textures = model.i_textures[0];
		//draw_call.attribute_streams[1].texture_handlers = model.texture_handlers;
		draw_call.attribute_streams[1].vertex_shader = NULL;
		draw_call.n_attributes = 2;

		draw_call.mip_level_bias = 0.2f;

	}
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::MOVE,
			component_id_::PROJECTILE_ID,
			component_id_::COLLIDER,
			component_id_::COLOUR,
			component_id_::COLOUR_SPACE,
			component_id_::VERTEX_LIGHT_SOURCE,
			component_id_::LIGHTMAP_LIGHT_SOURCE,
			//component_id_::SMALL_MODEL_ID,
			component_id_::DRAW,

		};

		const __int32 model_id = Return_Model_ID("proj_ballslime", model_manager);
		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		//const model_& model = model_animate[model_id];
		const __int32 n_models = 64;
		const __int32 draw_id = draw_call_::id_::PROJECTILES;
		const float3_ position = { 0.0f, 200.0f, 0.0f };
		const float3_ scale = { 4.0f, 4.0f, 4.0f };
		const float3_ zero = { 0.0f,0.0f,0.0f };
		const float3_ model_colour = { 0.0f, 0.0f, 200.0f };

		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;
		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::MOVE;
		component_fetch.component_ids[n_entries++] = component_id_::PROJECTILE_ID;
		component_fetch.component_ids[n_entries++] = component_id_::COLLIDER;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR_SPACE;
		component_fetch.component_ids[n_entries++] = component_id_::VERTEX_LIGHT_SOURCE;
		component_fetch.component_ids[n_entries++] = component_id_::LIGHTMAP_LIGHT_SOURCE;
		//component_fetch.component_ids[n_entries++] = component_id_::SMALL_MODEL_ID;
		component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);
		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::move_* move = (component_::move_*)component_fetch.table[n_entries++][0];
		component_::projectile_id_* projectile_id = (component_::projectile_id_*)component_fetch.table[n_entries++][0];
		component_::collider_* collider = (component_::collider_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::colour_space_* colour_space = (component_::colour_space_*)component_fetch.table[n_entries++][0];
		component_::vertex_light_source_* vertex_light_source = (component_::vertex_light_source_*)component_fetch.table[n_entries++][0];
		component_::lightmap_light_source_* lightmap_light_source = (component_::lightmap_light_source_*)component_fetch.table[n_entries++][0];
		//component_::small_model_id_* small_model_id = (component_::small_model_id_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];
		// -----------------------------------------------------------------------------------

		for (__int32 i_model = 0; i_model < n_models; i_model++) {

			for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
				archetype.entity_component_masks[i_model][i_group] = component_fetch.component_masks[i_group];
			}

			base[i_model].scale = scale;
			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				base[i_model].position_fixed.i[i_axis] = __int32(position.f[i_axis] * fixed_scale_real);
			}

			move[i_model].displacement = zero;
			move[i_model].velocity = zero;

			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				collider[i_model].extent.i[i_axis] = __int32(scale.f[i_axis] * fixed_scale_real);
			}
			collider[i_model].entity_type = colliding_type_::PROJECTILES;
			collider[i_model].entity_id = i_model;

			vertex_light_source[i_model].id = i_model;
			vertex_light_source[i_model].intensity = 1.0f;

			lightmap_light_source[i_model].id = i_model;
			lightmap_light_source[i_model].intensity = 1.0f;

			projectile_id[i_model].id = i_model;
			projectile_id[i_model].type_id = 0;

			//colour[i_model].colour = model_colour;
			colour[i_model].colour = { 0.5f, 0.0f, 0.5f };

			{
				float3_ axis = { 0.0f, 1.0f, 0.0f };
				Vector_Normalise(axis);
				float4_ axis_angle = { axis.x, axis.y, axis.z, 4.0f };
				Axis_Angle_To_Quaternion(axis_angle, colour_space[i_model].q_add);
				colour_space[i_model].q_rotate = q_unit;
				memcpy(colour_space[i_model].m_rotate, unit_matrix, sizeof(matrix_));
			}

			//small_model_id[i_model].id = i_model;

			draw[i_model].draw_id = draw_id;
			draw[i_model].model_id = model_id;

		}

		struct draw_call_& draw_call = draw_calls[draw_id];

		//draw_call.i_vertices = model.i_vertices;
		//draw_call.vertices = model.vertices_frame[0];

		draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		//draw_call.attribute_streams[0].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR];
		//draw_call.attribute_streams[0].vertices = model.attribute_vertices[model_::ATTRIBUTE_COLOUR]->f;
		draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour;

		draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		//draw_call.attribute_streams[1].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY];
		//draw_call.attribute_streams[1].vertices = model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY]->f;
		//draw_call.attribute_streams[1].i_textures = model.i_textures[0];
		//draw_call.attribute_streams[1].texture_handlers = model.texture_handlers;
		draw_call.attribute_streams[1].vertex_shader = NULL;
		draw_call.n_attributes = 2;

		draw_call.mip_level_bias = 0.2f;

		draw_call.lighting_function = Vertex_Lighting_PLAYER;
	}
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::MOVE,
			component_id_::CAMERA,
			component_id_::SPAWN,
			component_id_::COLOUR,
			component_id_::MODEL_SPACE,
			component_id_::MODEL_SPACE_UPDATE,
			component_id_::WEAPON,
			component_id_::ANIMATION,
			component_id_::ANIMATION_DRIVER,
			component_id_::COLLIDER,
			component_id_::COLLIDEE_STATIC,
			component_id_::BOUNDING_BOX,
			component_id_::SOUND_TRIGGER,
			component_id_::MASS,
			component_id_::DRAW,

		};

		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		const __int32 model_id = Return_Model_ID("mon_soldier", model_manager);
		const struct model_& model = model_animate[model_id];
		model_token_& model_token = model_tokens[model_token_::id_::CAMERA];
		const __int32 n_models = model_token.n_models;
		const __int32 draw_id = draw_call_::id_::PLAYER;
		const float3_ scale = { 1.0f, 1.0f, 1.0f };

		float3_ colours[] = {

			{ 0.0f, 40.f, 0.0f },
		{ 40.0f, 0.f, 0.0f },
		{ 0.0f, 0.f, 40.0f },
		{ 40.0f, 40.f, 0.0f },
		};

		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		const __int32 i_archetype = archetype_data.n_archetypes;
		archetype_& archetype = archetype_data.archetypes[i_archetype];
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;
		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::MOVE;
		component_fetch.component_ids[n_entries++] = component_id_::CAMERA;
		component_fetch.component_ids[n_entries++] = component_id_::SPAWN;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::MODEL_SPACE;
		component_fetch.component_ids[n_entries++] = component_id_::MODEL_SPACE_UPDATE;
		component_fetch.component_ids[n_entries++] = component_id_::WEAPON;
		component_fetch.component_ids[n_entries++] = component_id_::ANIMATION;
		component_fetch.component_ids[n_entries++] = component_id_::ANIMATION_DRIVER;
		component_fetch.component_ids[n_entries++] = component_id_::COLLIDER;
		component_fetch.component_ids[n_entries++] = component_id_::COLLIDEE_STATIC;
		component_fetch.component_ids[n_entries++] = component_id_::BOUNDING_BOX;
		component_fetch.component_ids[n_entries++] = component_id_::SOUND_TRIGGER;
		component_fetch.component_ids[n_entries++] = component_id_::MASS;
		component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::move_* move = (component_::move_*)component_fetch.table[n_entries++][0];
		component_::camera_* camera = (component_::camera_*)component_fetch.table[n_entries++][0];
		component_::spawn_* spawn = (component_::spawn_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::model_space_* model_space = (component_::model_space_*)component_fetch.table[n_entries++][0];
		component_::model_space_update_* model_space_update = (component_::model_space_update_*)component_fetch.table[n_entries++][0];
		component_::weapon_* weapon = (component_::weapon_*)component_fetch.table[n_entries++][0];
		component_::animation_* animation = (component_::animation_*)component_fetch.table[n_entries++][0];
		component_::animation_driver_* animation_driver = (component_::animation_driver_*)component_fetch.table[n_entries++][0];
		component_::collider_* collider = (component_::collider_*)component_fetch.table[n_entries++][0];
		component_::collidee_static_* collidee_static = (component_::collidee_static_*)component_fetch.table[n_entries++][0];
		component_::bounding_box_* bounding_box = (component_::bounding_box_*)component_fetch.table[n_entries++][0];
		component_::sound_trigger_* sound_trigger = (component_::sound_trigger_*)component_fetch.table[n_entries++][0];
		component_::mass_* mass = (component_::mass_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];
		// -----------------------------------------------------------------------------------

		for (__int32 i_model = 0; i_model < n_models; i_model++) {


			for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
				archetype.entity_component_masks[i_model][i_group] = component_fetch.component_masks[i_group];
			}

			__int32 i_model_token = INVALID_RESULT;
			for (__int32 i_token = 0; i_token < model_token.n_models; i_token++) {
				i_model_token = i_model == model_token.index[i_token] ? i_token : i_model_token;
			}

			if (i_model == 0) {
				Toggle_Component_Bit(i_archetype, i_model, component_id_::DRAW, archetype_data);
			}
			if (i_model != 0) {
				Toggle_Component_Bit(i_archetype, i_model, component_id_::CAMERA, archetype_data);
			}

			base[i_model].position_fixed = model_token.centre[i_model_token];
			base[i_model].scale = scale;

			spawn[i_model].position = model_token.centre[i_model_token];

			move[i_model].displacement = zero;
			move[i_model].velocity = zero;
			move[i_model].max_speed = 400.0f;

			camera[i_model].id = i_model;
			camera[i_model].previous_position = { 0, 0, 0 };
			camera[i_model].is_stepped = false;
			camera[i_model].t_smooth_step = 1.0f;
			camera[i_model].angle_yaw = 90.0f;
			camera[i_model].angle_pitch = 0.0f;
			camera[i_model].offset.x = 0;
			camera[i_model].offset.y = __int32(20.0f * fixed_scale_real);
			camera[i_model].offset.z = 0;
			camera[i_model].acceleration = 300.0f;
			memcpy(camera[i_model].m_camera_space, unit_matrix, sizeof(matrix_));
			memcpy(camera[i_model].m_clip_space, unit_matrix, sizeof(matrix_));

			const float	aspect_ratio = (float)display_::DISPLAY_HEIGHT / (float)display_::DISPLAY_WIDTH;
			const float a = 1.0f / aspect_ratio;
			//const float near_l = 1.0f;
			//const float far_l = 500000.0f;
			//const float q = far_l / (far_l - near_l);
			//const float p = q * near_l;
			const float q = 1.0f;
			const float p = 0.0f;

			matrix_ projection;
			memset(projection, 0, sizeof(matrix_));

			projection[0].x = 1.0f;
			projection[1].y = a;
			projection[2].z = q;
			projection[2].w = -1.0f;
			projection[3].z = p;

			memcpy(camera[i_model].m_projection, projection, sizeof(matrix_));

			colour[i_model].colour = colours[i_model % n_models];

			memcpy(model_space[i_model].m_rotate, unit_matrix, sizeof(matrix_));

			model_space_update[i_model].q_rotate = q_unit;
			model_space_update[i_model].q_add = q_unit;

			weapon[i_model].n_projectiles = 16;
			weapon[i_model].i_begin = 0;
			weapon[i_model].i_projectile = 0;
			weapon[i_model].projectile_id = component_::weapon_::id_::KILL;
			weapon[i_model].timer = 0.0f;
			weapon[i_model].ammo_count = component_::weapon_::ammo_::LOAD;
			weapon[i_model].is_fired = false;


			//memset(&animation[i_model], 0, sizeof(component_::animation_));
			animation[i_model].model_id = animation_model_::id_::PLAYER;
			animation[i_model].i_current = animation_data_::id_::IDLE;
			animation[i_model].is_end_animation = false;
			animation[i_model].is_frame_change = false;
			animation[i_model].trigger_id = animation_data_::id_::NULL_;
			animation[i_model].i_frames[component_::animation_::CURRENT_FRAME_LOW] = 0;
			animation[i_model].i_frames[component_::animation_::CURRENT_FRAME_HI] = 1;
			animation[i_model].i_frames[component_::animation_::NEXT_FRAME] = 2;
			animation[i_model].frame_speed = 10.0f;
			animation[i_model].frame_speed_modifier = 1.0f;

			animation_driver[i_model].i_current_animation = animation_data_::id_::IDLE;
			animation_driver[i_model].trigger_id = animation_data_::id_::NULL_;

			for (__int32 i = X; i < W; i++) {
				collider[i_model].extent.i[i] = (__int32)(model.bounding_extent.f[i] * fixed_scale_real);
			}
			collider[i_model].entity_type = colliding_type_::PLAYER;
			collider[i_model].entity_id = i_model;
			collider[i_model].is_ground_contact = false;

			collidee_static[i_model].extent.x = (__int32)(model.bounding_extent.x * 0.5f * fixed_scale_real);
			collidee_static[i_model].extent.y = (__int32)(model.bounding_extent.y * 0.5f * fixed_scale_real);
			collidee_static[i_model].extent.z = (__int32)(model.bounding_extent.z * 0.5f * fixed_scale_real);
			collidee_static[i_model].entity_type = colliding_type_::PLAYER;
			collidee_static[i_model].entity_id = i_model;

			bounding_box[i_model].centre = { 0, 0, 0 };
			bounding_box[i_model].extent.x = (__int32)(model.bounding_extent.x * fixed_scale_real);
			bounding_box[i_model].extent.y = (__int32)(model.bounding_extent.y * fixed_scale_real);
			bounding_box[i_model].extent.z = (__int32)(model.bounding_extent.z * fixed_scale_real);

			sound_trigger[i_model].source_id = sound_triggers_::source_id_::PLAYER + i_model;

			mass[i_model].default_value = 8.0f;
			mass[i_model].value = 8.0f;

			draw[i_model].draw_id = draw_id;
			draw[i_model].model_id = model_id;
		}

		draw_call_& draw_call = draw_calls[draw_id];

		//draw_call.i_vertices = model.i_vertices;
		//draw_call.vertices = model.vertices_frame[0];

		draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		//draw_call.attribute_streams[0].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR];
		//draw_call.attribute_streams[0].vertices = model.attribute_vertices[model_::ATTRIBUTE_COLOUR]->f;
		draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour_Simple;

		draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		//draw_call.attribute_streams[1].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY];
		//draw_call.attribute_streams[1].vertices = model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY]->f;
		//draw_call.attribute_streams[1].i_textures = model.i_textures[0];
		//draw_call.attribute_streams[1].texture_handlers = model.texture_handlers;
		draw_call.attribute_streams[1].vertex_shader = NULL;
		draw_call.n_attributes = 2;

		draw_call.mip_level_bias = 0.2f;
		draw_call.lighting_function = Vertex_Lighting_PLAYER;
	}
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::MOVE,
			component_id_::COLLIDER,
			component_id_::COLLIDEE_STATIC,
			component_id_::BOUNDING_BOX,
			component_id_::COLOUR,
			component_id_::MODEL_SPACE,
			component_id_::MODEL_SPACE_UPDATE,
			component_id_::ANIMATION,
			component_id_::BEHAVIOUR,
			component_id_::PROJECTILE,
			component_id_::PATROL,
			component_id_::EFFECT,
			component_id_::TEXTURE_BLEND,
			component_id_::SOUND_TRIGGER,
			component_id_::MASS,
			component_id_::HEALTH,
			component_id_::DRAW,

		};

		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		const __int32 model_id = Return_Model_ID("mon_shambler", model_manager);
		const struct model_& model = model_animate[model_id];
		model_token_& model_token = model_tokens[model_token_::id_::SHAMBLER];
		const __int32 n_models = model_token.n_models;
		const __int32 draw_id = draw_call_::id_::SHAMBLER;
		const float3_ scale = { 1.0f, 1.0f, 1.0f };
		float time_modifier = 1.0f;

		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;
		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::MOVE;
		component_fetch.component_ids[n_entries++] = component_id_::COLLIDER;
		component_fetch.component_ids[n_entries++] = component_id_::COLLIDEE_STATIC;
		component_fetch.component_ids[n_entries++] = component_id_::BOUNDING_BOX;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::MODEL_SPACE;
		component_fetch.component_ids[n_entries++] = component_id_::MODEL_SPACE_UPDATE;
		component_fetch.component_ids[n_entries++] = component_id_::ANIMATION;
		component_fetch.component_ids[n_entries++] = component_id_::BEHAVIOUR;
		component_fetch.component_ids[n_entries++] = component_id_::PROJECTILE;
		component_fetch.component_ids[n_entries++] = component_id_::PATROL;
		component_fetch.component_ids[n_entries++] = component_id_::EFFECT;
		component_fetch.component_ids[n_entries++] = component_id_::TEXTURE_BLEND;
		component_fetch.component_ids[n_entries++] = component_id_::SOUND_TRIGGER;
		component_fetch.component_ids[n_entries++] = component_id_::MASS;
		component_fetch.component_ids[n_entries++] = component_id_::HEALTH;
		component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::move_* move = (component_::move_*)component_fetch.table[n_entries++][0];
		component_::collider_* collider = (component_::collider_*)component_fetch.table[n_entries++][0];
		component_::collidee_static_* collidee_static = (component_::collidee_static_*)component_fetch.table[n_entries++][0];
		component_::bounding_box_* bounding_box = (component_::bounding_box_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::model_space_* model_space = (component_::model_space_*)component_fetch.table[n_entries++][0];
		component_::model_space_update_* model_space_update = (component_::model_space_update_*)component_fetch.table[n_entries++][0];
		component_::animation_* animation = (component_::animation_*)component_fetch.table[n_entries++][0];
		component_::behaviour_* behaviour = (component_::behaviour_*)component_fetch.table[n_entries++][0];
		component_::projectile_* projectile = (component_::projectile_*)component_fetch.table[n_entries++][0];
		component_::patrol_* patrol = (component_::patrol_*)component_fetch.table[n_entries++][0];
		component_::effect_* effect = (component_::effect_*)component_fetch.table[n_entries++][0];
		component_::texture_blend_* texture_blend = (component_::texture_blend_*)component_fetch.table[n_entries++][0];
		component_::sound_trigger_* sound_trigger = (component_::sound_trigger_*)component_fetch.table[n_entries++][0];
		component_::mass_* mass = (component_::mass_*)component_fetch.table[n_entries++][0];
		component_::health_* health = (component_::health_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];
		// -----------------------------------------------------------------------------------

		const float3_ colours[] = {

		{ 0.0f, 0.0f, 100.0f },
		{ 0.0f, 100.f, 0.0f },
		{ 100.0f, 0.0f, 0.0f },
		{ 100.0f, 100.f, 0.0f },
		{ 0.0f, 100.f, 100.0f },
		};
		const __int32 n_colours = sizeof(colours) / sizeof(colours[0]);

		const float facing_angle[] = { 0.0f, 90.0f, 180.0f, 270.0f };
		const __int32 n_facing_angles = sizeof(facing_angle) / sizeof(facing_angle[0]);

		// stand - pain - death - dead - shrink
		// patrol - pain - death -dead - shrink

		const __int32 n_permutations = 2;
		const __int32 n_nodes = 6;
		const __int32 behaviour_ids[n_permutations][n_nodes] = {

			{
				component_::behaviour_::id_::STAND,
				component_::behaviour_::id_::ATTACK,
				component_::behaviour_::id_::DEATH,
				component_::behaviour_::id_::DEAD,
				component_::behaviour_::id_::SHRINK,
				component_::behaviour_::id_::PAIN,
			},
			{
				component_::behaviour_::id_::MOVE_PATROL,
				component_::behaviour_::id_::ATTACK,
				component_::behaviour_::id_::DEATH,
				component_::behaviour_::id_::DEAD,
				component_::behaviour_::id_::SHRINK,
				component_::behaviour_::id_::PAIN,
			},
		};

		const __int32 i_next_nodes[n_permutations][n_nodes] = {

			{ 0, 0, 3, 0, 0, 0 },
		{ 0, 0, 3, 0, 0, 0 },
		};


		for (__int32 i_model = 0; i_model < n_models; i_model++) {

			for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
				archetype.entity_component_masks[i_model][i_group] = component_fetch.component_masks[i_group];
			}

			__int32 i_model_token = INVALID_RESULT;
			for (__int32 i_token = 0; i_token < model_token.n_models; i_token++) {
				i_model_token = i_model == model_token.index[i_token] ? i_token : i_model_token;
			}

			base[i_model].scale = scale;
			base[i_model].position_fixed = model_token.centre[i_model_token];

			move[i_model].displacement = zero;
			move[i_model].velocity = zero;
			move[i_model].max_speed = 0.0f;

			//for (__int32 i_axis = X; i_axis < W; i_axis++) {
			//	collider[i_model].extent.i[i_axis] = __int32(model.bounding_extent.f[i_axis] * fixed_scale_real);
			//}
			collider[i_model].extent.x = __int32(model.bounding_extent.x * 0.5f * fixed_scale_real);
			collider[i_model].extent.y = __int32(model.bounding_extent.y * 1.0f * fixed_scale_real);
			collider[i_model].extent.z = __int32(model.bounding_extent.z * 0.5f * fixed_scale_real);

			collider[i_model].entity_type = colliding_type_::MONSTER;
			collider[i_model].entity_id = i_monster;

			collidee_static[i_model].extent.x = __int32(model.bounding_extent.x * 1.0f * fixed_scale_real);
			collidee_static[i_model].extent.y = __int32(model.bounding_extent.y * 0.5f * fixed_scale_real);
			collidee_static[i_model].extent.z = __int32(model.bounding_extent.z * 1.0f * fixed_scale_real);
			collidee_static[i_model].entity_type = colliding_type_::MONSTER;
			collidee_static[i_model].entity_id = i_monster;

			bounding_box[i_model].centre = model_token.centre[i_model_token];
			bounding_box[i_model].extent.x = __int32(model.bounding_extent.x * fixed_scale_real);
			bounding_box[i_model].extent.y = __int32(model.bounding_extent.y * fixed_scale_real);
			bounding_box[i_model].extent.z = __int32(model.bounding_extent.z * fixed_scale_real);


			colour[i_model].colour = colours[i_model % n_colours];

			memcpy(model_space[i_model].m_rotate, unit_matrix, sizeof(matrix_));

			__m128 axis_angle = set(0.0f, 1.0f, 0.0f, facing_angle[i_model % n_facing_angles]);
			store_u(Axis_Angle_To_Quaternion(axis_angle), model_space_update[i_model].q_rotate.f);
			model_space_update[i_model].q_add = q_unit;

			behaviour[i_model].i_node = 0;
			behaviour[i_model].trigger_id = component_::behaviour_::id_::NULL_;
			behaviour[i_model].state_trigger = component_::behaviour_::id_::NULL_;
			behaviour[i_model].n_nodes = n_nodes;

			__int32 i_permutation = i_model < way_point_manager.n_patrols ? 1 : 0;

			for (__int32 i_node = 0; i_node < n_nodes; i_node++) {

				//const __int32 i_permutation = i_permutations[i_model];

				behaviour[i_model].behaviour_nodes[i_node].behaviour_id = behaviour_ids[i_permutation][i_node];
				behaviour[i_model].behaviour_nodes[i_node].i_next_node = i_next_nodes[i_permutation][i_node];
				behaviour[i_model].behaviour_nodes[i_node].timer = 0.0f;
			}

			const __int32 behaviour_id = behaviour[i_model].behaviour_nodes[behaviour[i_model].i_node].behaviour_id;
			const __int32 animation_id = behaviour_manager.behaviour_data[behaviour_id].animation_id;

			memset(&animation[i_model], 0, sizeof(component_::animation_));
			animation[i_model].model_id = animation_model_::id_::SHAMBLER;
			animation[i_model].i_current = 0;
			animation[i_model].is_end_animation = false;
			animation[i_model].is_frame_change = false;
			//animation[i_model].trigger_id = animation_data_::id_::NULL_;
			animation[i_model].trigger_id = animation_id;
			animation[i_model].i_frames[component_::animation_::CURRENT_FRAME_LOW] = 0;
			animation[i_model].i_frames[component_::animation_::CURRENT_FRAME_HI] = 1;
			animation[i_model].i_frames[component_::animation_::NEXT_FRAME] = 2;
			animation[i_model].frame_speed = 10.0f;
			animation[i_model].frame_speed_modifier = 1.0f;

			const __int32 i_begin = 16 + (4 * i_model);
			projectile[i_model].i_begin = i_begin;
			projectile[i_model].n_projectiles = 4;
			projectile[i_model].i_projectile = 0;
			projectile[i_model].type_id = component_::item_::type_::KILL;
			projectile[i_model].model_data.i_firing_frame = 68;
			projectile[i_model].model_data.i_vertex_left = 285;
			projectile[i_model].model_data.i_vertex_right = 554;
			projectile[i_model].model_data.displacement = { -40.0f, 0.0f, 0.0f };

			const __int32 i_patrol = i_model % way_point_manager.n_patrols;
			patrol[i_model].i_current = 0;
			patrol[i_model].n_points = way_point_manager.n_points[i_patrol];
			for (__int32 i_point = 0; i_point < way_point_manager.n_points[i_patrol]; i_point++) {
				patrol[i_model].point_ids[i_point] = way_point_manager.point_ids[i_patrol][i_point];
			}

			//patrol[i_model].n_points = 4;
			//__int32 extent = __int32(100.0f * fixed_scale_real);
			//for (__int32 i_point = 0; i_point < 4; i_point++) {
			//	patrol[i_model].points[i_point] = model_token.centre[i_model_token];
			//}
			//patrol[i_model].points[0].x += extent;
			//patrol[i_model].points[0].z += extent;

			//patrol[i_model].points[1].x += extent;
			//patrol[i_model].points[1].z -= extent;

			//patrol[i_model].points[2].x -= extent;
			//patrol[i_model].points[2].z -= extent;

			//patrol[i_model].points[3].x -= extent;
			//patrol[i_model].points[3].z += extent;

			effect[i_model].trigger_id = component_::item_::type_::NULL_;
			effect[i_model].shrink.base_scale = scale;
			effect[i_model].shrink.shrunk_scale.x = 0.5f;
			effect[i_model].shrink.shrunk_scale.y = 0.5f;
			effect[i_model].shrink.shrunk_scale.z = 0.5f;
			effect[i_model].shrink.t_interval = 0.0f;
			effect[i_model].shrink.timer = 0.0f;
			effect[i_model].shrink.duration = 10.0f;

			float effect_duration = 10.0f;
			effect[i_model].petrify.t_interval = 0.0f;
			effect[i_model].petrify.begin_time_modifier = time_modifier;
			effect[i_model].petrify.end_time_modifier = 0.0f;
			effect[i_model].petrify.time_limit = effect_duration;
			effect[i_model].petrify.timer = effect_duration;
			effect[i_model].petrify.is_running = false;

			texture_blend[i_model].interval = 0.0f;

			sound_trigger[i_model].source_id = sound_triggers_::source_id_::MONSTER + i_monster;

			mass[i_model].default_value = 20.0f;
			mass[i_model].value = 20.0f;

			health[i_model].hp = 6;

			draw[i_model].draw_id = draw_id;
			draw[i_model].model_id = model_id;

			i_monster++;
		}

		draw_call_& draw_call = draw_calls[draw_id];

		draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour_Simple;

		draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].vertex_shader = NULL;

		draw_call.attribute_streams[2].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[2].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		draw_call.attribute_streams[2].vertex_shader = NULL;
		draw_call.n_attributes = 3;

		draw_call.n_additional_pixel_shaders = 1;

		draw_call.mip_level_bias = 0.1f;
		draw_call.lighting_function = Vertex_Lighting_PLAYER;

	}
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::COLOUR,
			component_id_::PATROL_POINT,
			component_id_::SMALL_MODEL_ID,
			//component_id_::DRAW,

		};

		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		const __int32 model_id = model_::id_::PATROL_POINT;
		const struct model_& model = model_animate[model_id];
		const __int32 draw_id = draw_call_::id_::PATROL_POINTS;
		//const float3_ scale = { 1.0f, 1.0f, 1.0f };

		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		archetype_data.n_archetypes++;

		__int32 n_models = 0;
		for (__int32 i_patrol = 0; i_patrol < way_point_manager.n_patrols; i_patrol++) {

			const model_token_& map_token = model_tokens[model_token_::id_::PATROL_ONE + i_patrol];
			n_models += map_token.n_models;
		}

		Allocate_Memory_Archetype(

			n_models,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;

		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::PATROL_POINT;
		component_fetch.component_ids[n_entries++] = component_id_::SMALL_MODEL_ID;
		//component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::patrol_point_* patrol_point = (component_::patrol_point_*)component_fetch.table[n_entries++][0];
		component_::small_model_id_* small_model_id = (component_::small_model_id_*)component_fetch.table[n_entries++][0];
		//component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];
		// -----------------------------------------------------------------------------------


		__int32 i_model = 0;
		for (__int32 i_patrol = 0; i_patrol < way_point_manager.n_patrols; i_patrol++) {

			const model_token_& map_token = model_tokens[model_token_::id_::PATROL_ONE + i_patrol];

			for (__int32 i_entry = 0; i_entry < map_token.n_models; i_entry++) {

				for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
					archetype.entity_component_masks[i_model][i_group] = component_fetch.component_masks[i_group];
				}

				base[i_model].position_fixed = map_token.centre[i_entry];
				base[i_model].scale.x = float(map_token.extent[i_entry].x * r_fixed_scale_real);
				base[i_model].scale.y = float(map_token.extent[i_entry].y * r_fixed_scale_real);
				base[i_model].scale.z = float(map_token.extent[i_entry].z * r_fixed_scale_real);

				colour[i_model].colour = { 0.0f, 0.0f, 0.0f };;

				patrol_point[i_model].id = i_model;

				small_model_id[i_model].id = i_model;

				//draw[i_model].draw_id = draw_id;
				//draw[i_model].model_id = model_id;

				i_model++;
			}
		}

		draw_call_& draw_call = draw_calls[draw_id];

		//draw_call.i_vertices = model.i_vertices;
		//draw_call.vertices = model.vertices_frame[0];

		draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		//draw_call.attribute_streams[0].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR];
		//draw_call.attribute_streams[0].vertices = model.attribute_vertices[model_::ATTRIBUTE_COLOUR]->f;
		draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour_Simple;

		draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		//draw_call.attribute_streams[1].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY];
		//draw_call.attribute_streams[1].vertices = model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY]->f;
		//draw_call.attribute_streams[1].i_textures = model.i_textures[0];
		//draw_call.attribute_streams[1].texture_handlers = model.texture_handlers;
		draw_call.attribute_streams[1].vertex_shader = NULL;
		draw_call.n_attributes = 2;

		draw_call.mip_level_bias = 0.2f;
	}
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 component_list[] = {

			component_id_::BASE,
			component_id_::MOVE,
			component_id_::COLLIDER,
			component_id_::COLLIDEE_STATIC,
			component_id_::BOUNDING_BOX,
			component_id_::COLOUR,
			component_id_::MODEL_SPACE,
			component_id_::MODEL_SPACE_UPDATE,
			component_id_::ANIMATION,
			component_id_::BEHAVIOUR,
			//component_id_::PROJECTILE,
			component_id_::EFFECT,
			component_id_::FLY,
			component_id_::TEXTURE_BLEND,
			component_id_::SOUND_TRIGGER,
			component_id_::DRAW,

		};

		const __int32 n_components = sizeof(component_list) / sizeof(component_list[0]);
		const __int32 model_id = Return_Model_ID("mon_bossnour", model_manager);
		const struct model_& model = model_animate[model_id];
		model_token_& model_token = model_tokens[model_token_::id_::SCRAG];
		const __int32 n_models = model_token.n_models;
		const __int32 draw_id = draw_call_::id_::SCRAG;
		const float3_ scale = { 1.0f, 1.0f, 1.0f };
		float time_modifier = 1.0f;

		// -----------------------------------------------------------------------------------
		archetype_data_& archetype_data = component_data.archetype_data;
		archetype_& archetype = archetype_data.archetypes[archetype_data.n_archetypes];
		archetype_data.n_archetypes++;

		Allocate_Memory_Archetype(

			n_models,
			n_components,
			component_list,
			component_data,
			memory_chunk,
			archetype
		);

		component_fetch_ component_fetch;
		component_fetch.n_components = n_components;
		component_fetch.n_excludes = 0;

		__int32 n_entries = 0;
		component_fetch.component_ids[n_entries++] = component_id_::BASE;
		component_fetch.component_ids[n_entries++] = component_id_::MOVE;
		component_fetch.component_ids[n_entries++] = component_id_::COLLIDER;
		component_fetch.component_ids[n_entries++] = component_id_::COLLIDEE_STATIC;
		component_fetch.component_ids[n_entries++] = component_id_::BOUNDING_BOX;
		component_fetch.component_ids[n_entries++] = component_id_::COLOUR;
		component_fetch.component_ids[n_entries++] = component_id_::MODEL_SPACE;
		component_fetch.component_ids[n_entries++] = component_id_::MODEL_SPACE_UPDATE;
		component_fetch.component_ids[n_entries++] = component_id_::ANIMATION;
		component_fetch.component_ids[n_entries++] = component_id_::BEHAVIOUR;
		component_fetch.component_ids[n_entries++] = component_id_::EFFECT;
		component_fetch.component_ids[n_entries++] = component_id_::FLY;
		component_fetch.component_ids[n_entries++] = component_id_::TEXTURE_BLEND;
		component_fetch.component_ids[n_entries++] = component_id_::SOUND_TRIGGER;
		component_fetch.component_ids[n_entries++] = component_id_::DRAW;

		Populate_Fetch_Table(archetype_data, component_fetch);

		n_entries = 0;
		component_::base_* base = (component_::base_*)component_fetch.table[n_entries++][0];
		component_::move_* move = (component_::move_*)component_fetch.table[n_entries++][0];
		component_::collider_* collider = (component_::collider_*)component_fetch.table[n_entries++][0];
		component_::collidee_static_* collidee_static = (component_::collidee_static_*)component_fetch.table[n_entries++][0];
		component_::bounding_box_* bounding_box = (component_::bounding_box_*)component_fetch.table[n_entries++][0];
		component_::colour_* colour = (component_::colour_*)component_fetch.table[n_entries++][0];
		component_::model_space_* model_space = (component_::model_space_*)component_fetch.table[n_entries++][0];
		component_::model_space_update_* model_space_update = (component_::model_space_update_*)component_fetch.table[n_entries++][0];
		component_::animation_* animation = (component_::animation_*)component_fetch.table[n_entries++][0];
		component_::behaviour_* behaviour = (component_::behaviour_*)component_fetch.table[n_entries++][0];
		component_::effect_* effect = (component_::effect_*)component_fetch.table[n_entries++][0];
		component_::fly_* fly = (component_::fly_*)component_fetch.table[n_entries++][0];
		component_::texture_blend_* texture_blend = (component_::texture_blend_*)component_fetch.table[n_entries++][0];
		component_::sound_trigger_* sound_trigger = (component_::sound_trigger_*)component_fetch.table[n_entries++][0];
		component_::draw_* draw = (component_::draw_*)component_fetch.table[n_entries++][0];
		// -----------------------------------------------------------------------------------

		for (__int32 i_model = 0; i_model < n_models; i_model++) {

			for (__int32 i_group = 0; i_group < component_data_::NUM_MASKS; i_group++) {
				archetype.entity_component_masks[i_model][i_group] = component_fetch.component_masks[i_group];
			}

			base[i_model].scale = scale;
			base[i_model].position_fixed = model_token.centre[i_model];

			move[i_model].displacement = zero;
			move[i_model].velocity = zero;
			//move[i_model].max_speed = 20.0f;
			//move[i_model].max_speed_air = 1200.0f;
			//move[i_model].friction_modifier = 1.0f;

			int3_ extent;
			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				extent.i[i_axis] = __int32(model.bounding_extent.f[i_axis] * fixed_scale_real);
			}

			collider[i_model].extent = extent;
			collider[i_model].entity_type = colliding_type_::MONSTER;
			collider[i_model].entity_id = i_monster;

			collidee_static[i_model].extent = extent;
			collidee_static[i_model].entity_type = colliding_type_::MONSTER;
			collidee_static[i_model].entity_id = i_monster;

			bounding_box[i_model].centre = model_token.centre[i_model];
			bounding_box[i_model].extent = extent;

			colour[i_model].colour = { 100.0f, 0.f, 0.0f };

			memcpy(model_space[i_model].m_rotate, unit_matrix, sizeof(matrix_));

			__m128 axis_angle = set(0.0f, 1.0f, 0.0f, 180.0f);
			store_u(Axis_Angle_To_Quaternion(axis_angle), model_space_update[i_model].q_rotate.f);
			model_space_update[i_model].q_add = q_unit;

			memset(&animation[i_model], 0, sizeof(component_::animation_));
			animation[i_model].model_id = animation_model_::id_::SHAMBLER;
			animation[i_model].i_current = 0;
			animation[i_model].is_end_animation = false;
			animation[i_model].is_frame_change = false;
			animation[i_model].trigger_id = animation_data_::id_::NULL_;
			animation[i_model].i_frames[component_::animation_::CURRENT_FRAME_LOW] = 0;
			animation[i_model].i_frames[component_::animation_::CURRENT_FRAME_HI] = 1;
			animation[i_model].i_frames[component_::animation_::NEXT_FRAME] = 2;
			animation[i_model].frame_speed = 10.0f;
			animation[i_model].frame_speed_modifier = 1.0f;

			behaviour[i_model].i_node = 0;
			behaviour[i_model].trigger_id = component_::behaviour_::id_::NULL_;
			behaviour[i_model].state_trigger = component_::behaviour_::id_::NULL_;

			{
				__int32 n_nodes = 0;

				behaviour[i_model].behaviour_nodes[n_nodes].behaviour_id = component_::behaviour_::id_::STAND;
				behaviour[i_model].behaviour_nodes[n_nodes].i_next_node = 0;
				behaviour[i_model].behaviour_nodes[n_nodes].timer = 0.0f;
				n_nodes++;

				behaviour[i_model].behaviour_nodes[n_nodes].behaviour_id = component_::behaviour_::id_::MOVE_PATROL;
				behaviour[i_model].behaviour_nodes[n_nodes].i_next_node = 1;
				behaviour[i_model].behaviour_nodes[n_nodes].timer = 0.0f;
				n_nodes++;

				behaviour[i_model].behaviour_nodes[n_nodes].behaviour_id = component_::behaviour_::id_::CHARGE_PLAYER;
				behaviour[i_model].behaviour_nodes[n_nodes].i_next_node = 3;
				behaviour[i_model].behaviour_nodes[n_nodes].timer = 0.0f;
				n_nodes++;

				behaviour[i_model].behaviour_nodes[n_nodes].behaviour_id = component_::behaviour_::id_::ATTACK;
				behaviour[i_model].behaviour_nodes[n_nodes].i_next_node = 4;
				behaviour[i_model].behaviour_nodes[n_nodes].timer = 0.0f;
				n_nodes++;

				behaviour[i_model].behaviour_nodes[n_nodes].behaviour_id = component_::behaviour_::id_::ATTACK;
				behaviour[i_model].behaviour_nodes[n_nodes].i_next_node = 5;
				behaviour[i_model].behaviour_nodes[n_nodes].timer = 0.0f;
				n_nodes++;

				behaviour[i_model].behaviour_nodes[n_nodes].behaviour_id = component_::behaviour_::id_::ATTACK;
				behaviour[i_model].behaviour_nodes[n_nodes].i_next_node = 0;
				behaviour[i_model].behaviour_nodes[n_nodes].timer = 0.0f;
				n_nodes++;

				behaviour[i_model].behaviour_nodes[n_nodes].behaviour_id = component_::behaviour_::id_::DEATH;
				behaviour[i_model].behaviour_nodes[n_nodes].i_next_node = 7;
				behaviour[i_model].behaviour_nodes[n_nodes].timer = 0.0f;
				n_nodes++;

				behaviour[i_model].behaviour_nodes[n_nodes].behaviour_id = component_::behaviour_::id_::DEAD;
				behaviour[i_model].behaviour_nodes[n_nodes].i_next_node = 0;
				behaviour[i_model].behaviour_nodes[n_nodes].timer = 0.0f;
				n_nodes++;

				behaviour[i_model].behaviour_nodes[n_nodes].behaviour_id = component_::behaviour_::id_::SHRINK;
				behaviour[i_model].behaviour_nodes[n_nodes].i_next_node = 0;
				behaviour[i_model].behaviour_nodes[n_nodes].timer = 0.0f;
				n_nodes++;

				behaviour[i_model].behaviour_nodes[n_nodes].behaviour_id = component_::behaviour_::id_::PAIN;
				behaviour[i_model].behaviour_nodes[n_nodes].i_next_node = 0;
				behaviour[i_model].behaviour_nodes[n_nodes].timer = 0.0f;
				n_nodes++;

				behaviour[i_model].n_nodes = n_nodes;
			}

			//const __int32 i_begin = 32 + (4 * i_model);
			//projectile[i_model].i_begin = i_begin;
			//projectile[i_model].n_projectiles = 4;
			//projectile[i_model].i_projectile = 0;
			//projectile[i_model].model_data.i_firing_frame = 64;
			//projectile[i_model].model_data.i_vertex_left = 116;
			//projectile[i_model].model_data.i_vertex_right = 149;
			//projectile[i_model].model_data.displacement = { -30.0f, 0.0f, 0.0f };

			effect[i_model].trigger_id = component_::item_::type_::NULL_;
			effect[i_model].shrink.base_scale = scale;
			effect[i_model].shrink.shrunk_scale.x = 0.25f;
			effect[i_model].shrink.shrunk_scale.y = 0.25f;
			effect[i_model].shrink.shrunk_scale.z = 0.25f;
			effect[i_model].shrink.t_interval = 0.0f;

			float effect_duration = 5.0f;
			effect[i_model].petrify.t_interval = 0.0f;
			effect[i_model].petrify.begin_time_modifier = time_modifier;
			effect[i_model].petrify.end_time_modifier = 0.0f;
			effect[i_model].petrify.time_limit = effect_duration;
			effect[i_model].petrify.timer = effect_duration;
			effect[i_model].petrify.is_running = false;

			fly[i_model].acceleration_rise = 100.0f;
			fly[i_model].max_air_speed = 1200.0f;
			fly[i_model].min_air_speed = 38.0f;
			fly[i_model].gravity_modifier_max = 1.0f;
			fly[i_model].gravity_modifier_min = 0.0f;
			fly[i_model].ceiling_fixed = model_token.centre[i_model].y;
			fly[i_model].floor_fixed = -150 * fixed_scale;

			texture_blend[i_model].interval = 0.0f;

			sound_trigger[i_model].source_id = sound_triggers_::source_id_::MONSTER + i_monster;

			draw[i_model].draw_id = draw_id;
			draw[i_model].model_id = model_id;

			i_monster++;
		}

		draw_call_& draw_call = draw_calls[draw_id];

		//draw_call.i_vertices = model.i_vertices;
		//draw_call.vertices = model.vertices_frame[0];

		draw_call.attribute_streams[0].id = draw_call_::attribute_stream_::id_::COLOUR_VERTEX;
		draw_call.attribute_streams[0].stride = draw_call_::attribute_stream_::stride_::COLOUR_VERTEX;
		//draw_call.attribute_streams[0].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR];
		//draw_call.attribute_streams[0].vertices = model.attribute_vertices[model_::ATTRIBUTE_COLOUR]->f;
		draw_call.attribute_streams[0].vertex_shader = Shade_Vertex_Colour_Simple;

		draw_call.attribute_streams[1].id = draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
		draw_call.attribute_streams[1].stride = draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX;
		//draw_call.attribute_streams[1].i_vertices = model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY];
		//draw_call.attribute_streams[1].vertices = model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY]->f;
		//draw_call.attribute_streams[1].i_textures = model.i_textures[0];
		//draw_call.attribute_streams[1].texture_handlers = model.texture_handlers;
		draw_call.attribute_streams[1].vertex_shader = NULL;
		draw_call.n_attributes = 2;

		draw_call.mip_level_bias = 0.2f;
		draw_call.lighting_function = Vertex_Lighting_PLAYER;
	}

	assert(component_data.archetype_data.n_archetypes < component_fetch_::MAX_ARCHETYPES);
}