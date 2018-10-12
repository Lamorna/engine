
#include <io.h>

#include "setup.h"
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
#include "archetype.h"
#include "model.h"
#include "obj_reader.h"
#include "texture.h"
#include "string.h"

/*
==================
==================
*/
void Load_UI(

	user_interface_& user_interface,
	memory_chunk_& memory_chunk

) {

	const char* file_names[user_interface_::NUM_ELEMENTS] = {

	{ "UI/NUM_0.bmp" },
	{ "UI/NUM_1.bmp" },
	{ "UI/NUM_2.bmp" },
	{ "UI/NUM_3.bmp" },
	{ "UI/NUM_4.bmp" },
	{ "UI/NUM_5.bmp" },
	{ "UI/NUM_6.bmp" },
	{ "UI/NUM_7.bmp" },
	{ "UI/NUM_8.bmp" },
	};

	for (__int32 i_element = 0; i_element < user_interface_::NUM_ELEMENTS; i_element++) {
		Load_Image_STB(file_names[i_element], user_interface.texture_handler[i_element], memory_chunk);
	}

	user_interface.frame_rate = 6278;
}

/*
==================
==================
*/
void Get_System_Info(

	thread_pool_& thread_pool

) {
	SYSTEM_INFO system_info;
	GetSystemInfo(&system_info);

	//printf("Hardware information: \n");
	//printf("  OEM ID: %u\n", siSysInfo.dwOemId);
	//printf("  Number of processors: %u\n",
	//	siSysInfo.dwNumberOfProcessors);
	//printf("  Page size: %u\n", siSysInfo.dwPageSize);
	//printf("  Processor type: %u\n", siSysInfo.dwProcessorType);
	//printf("  Minimum application address: %lx\n",
	//	siSysInfo.lpMinimumApplicationAddress);
	//printf("  Maximum application address: %lx\n",
	//	siSysInfo.lpMaximumApplicationAddress);
	//printf("  Active processor mask: %u\n",
	//	siSysInfo.dwActiveProcessorMask);

	thread_pool.n_threads = system_info.dwNumberOfProcessors;
}

/*
==================
==================
*/
void Create_BVH(

	model_token_manager_& model_token_manager,
	lightmap_manager_& lightmap_manager,
	model_manager_& model_manager,
	memory_chunk_& memory_chunk,
	grid_& grid

) {

		model_token_& model_token = model_token_manager.model_tokens[model_token_::id_::MAP];

		int3_ bound_max;
		int3_ bound_min;

		bound_max.x = INT_MIN;
		bound_max.y = INT_MIN;
		bound_max.z = INT_MIN;

		bound_min.x = INT_MAX;
		bound_min.y = INT_MAX;
		bound_min.z = INT_MAX;

		for (__int32 i_model = 0; i_model < model_token.n_models; i_model++) {

			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				__int32 centre = model_token.centre[i_model].i[i_axis];
				__int32 extent = model_token.extent[i_model].i[i_axis];
				bound_max.i[i_axis] = max(centre + extent, bound_max.i[i_axis]);
				bound_min.i[i_axis] = min(centre - extent, bound_min.i[i_axis]);
			}
		}

		int3_ extent;
		int3_ half_extent;
		for (__int32 i_axis = X; i_axis < W; i_axis++) {
			__int64 denom_sum = 4 * fixed_scale;
			__int64 denom_diff = 2 * fixed_scale;
			__int64 sum = bound_max.i[i_axis] + bound_min.i[i_axis];
			__int64 diff = bound_max.i[i_axis] - bound_min.i[i_axis];
			extent.i[i_axis] = __int32((diff * fixed_scale) / denom_diff);
			half_extent.i[i_axis] = __int32((diff * fixed_scale) / denom_sum);
		}

		for (__int32 i_node = 0; i_node < grid_::NUM_NODES; i_node++) {
			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				grid.nodes[i_node].map.extent.i[i_axis] = half_extent.i[i_axis];
			}
		}

		grid.nodes[0].map.centre.x = bound_min.x + half_extent.x;
		grid.nodes[0].map.centre.y = bound_min.y + half_extent.y;
		grid.nodes[0].map.centre.z = bound_min.z + half_extent.z;

		grid.nodes[1].map.centre = grid.nodes[0].map.centre;
		grid.nodes[1].map.centre.x += extent.x;

		grid.nodes[2].map.centre = grid.nodes[0].map.centre;
		grid.nodes[3].map.centre = grid.nodes[1].map.centre;
		grid.nodes[2].map.centre.z += extent.z;
		grid.nodes[3].map.centre.z += extent.z;

		grid.nodes[4].map.centre = grid.nodes[0].map.centre;
		grid.nodes[5].map.centre = grid.nodes[1].map.centre;
		grid.nodes[6].map.centre = grid.nodes[2].map.centre;
		grid.nodes[7].map.centre = grid.nodes[3].map.centre;
		grid.nodes[4].map.centre.y += extent.y;
		grid.nodes[5].map.centre.y += extent.y;
		grid.nodes[6].map.centre.y += extent.y;
		grid.nodes[7].map.centre.y += extent.y;


		// bin models
		{
			__int32 n_binned_models = 0;
			for (__int32 i_node = 0; i_node < grid_::NUM_NODES; i_node++) {

				grid.nodes[i_node].map.n_entries = 0;

				for (__int32 i_model = 0; i_model < model_token.n_models; i_model++) {

					unsigned __int32 increment = 0x1;
					for (__int32 i_axis = X; i_axis < W; i_axis++) {
						__int64 delta_centre = abs(grid.nodes[i_node].map.centre.i[i_axis] - model_token.centre[i_model].i[i_axis]);
						increment &= delta_centre < grid.nodes[i_node].map.extent.i[i_axis];;
					}
					grid.nodes[i_node].indices[grid.nodes[i_node].map.n_entries] = i_model;
					grid.nodes[i_node].map.n_entries += increment;
					n_binned_models += increment;
				}
			}

			//printf_s("num models: %i ; num binned: %i \n", n_binned_models, model_token.n_models);
			assert(n_binned_models == model_token.n_models);
		}
		// fit grid about binned models
		{
			for (__int32 i_node = 0; i_node < grid_::NUM_NODES; i_node++) {

				int3_ bound_max;
				int3_ bound_min;

				for (__int32 i_axis = X; i_axis < W; i_axis++) {

					bound_max.i[i_axis] = grid.nodes[i_node].map.centre.i[i_axis] + grid.nodes[i_node].map.extent.i[i_axis];
					bound_min.i[i_axis] = grid.nodes[i_node].map.centre.i[i_axis] - grid.nodes[i_node].map.extent.i[i_axis];
				}

				for (__int32 i_entry = 0; i_entry < grid.nodes[i_node].map.n_entries; i_entry++) {

					__int32 i_model = grid.nodes[i_node].indices[i_entry];
					for (__int32 i_axis = X; i_axis < W; i_axis++) {
						__int32 centre = model_token.centre[i_model].i[i_axis];
						__int32 extent = model_token.extent[i_model].i[i_axis];
						bound_max.i[i_axis] = max(centre + extent, bound_max.i[i_axis]);
						bound_min.i[i_axis] = min(centre - extent, bound_min.i[i_axis]);
					}
				}

				const __int32  nudge = 10 * fixed_scale;
				for (__int32 i_axis = X; i_axis < W; i_axis++) {
					__int64 denom = 2 * fixed_scale;
					__int64 sum = bound_max.i[i_axis] + bound_min.i[i_axis];
					__int64 diff = bound_max.i[i_axis] - bound_min.i[i_axis];
					grid.nodes[i_node].map.centre.i[i_axis] = __int32((sum * fixed_scale) / denom);
					grid.nodes[i_node].map.extent.i[i_axis] = __int32((diff * fixed_scale) / denom);
					grid.nodes[i_node].map.extent.i[i_axis] += nudge;
				}
			}
		}

		for (__int32 i_node = 0; i_node < grid_::NUM_NODES; i_node++) {

			model_token_& model_token_write = model_token_manager.model_tokens[model_token_::id_::MAP + i_node + 1];
			model_token_write.index = model_token_manager.index + model_token_manager.n_blocks;
			model_token_write.centre = model_token_manager.centre + model_token_manager.n_blocks;
			model_token_write.extent = model_token_manager.extent + model_token_manager.n_blocks;
			model_token_write.normals = model_token_manager.normals + model_token_manager.n_blocks;
			model_token_write.n_models = grid.nodes[i_node].map.n_entries;
			model_token_manager.n_blocks += model_token.n_models;

			for (__int32 i_model = 0; i_model < model_token_write.n_models; i_model++) {

				const __int32 index = grid.nodes[i_node].indices[i_model];
				model_token_write.centre[i_model] = model_token.centre[index];
				model_token_write.extent[i_model] = model_token.extent[index];

				lightmap_manager.map_nodes_TEMP[index] = i_node;
				lightmap_manager.map_models_TEMP[index] = i_model;
			}
		}
		// map models to node centre
		{
			for (__int32 i_node = 0; i_node < grid_::NUM_NODES; i_node++) {

				model_token_& model_token = model_token_manager.model_tokens[model_token_::id_::MAP + i_node + 1];

				for (__int32 i_model = 0; i_model < model_token.n_models; i_model++) {
					for (__int32 i_axis = X; i_axis < W; i_axis++) {
						model_token.centre[i_model].i[i_axis] -= grid.nodes[i_node].map.centre.i[i_axis];
					}
				}
			}
		}

		float3_ colour = { 0.0f, 0.0f, 0.0f };

		for (__int32 i_node = 0; i_node < grid_::NUM_NODES + 1; i_node++) {

			Create_Map_Model(

				model_token_manager.model_tokens[model_token_::id_::MAP + i_node],
				model_manager.model[model_::id_::CUBE],
				colour,
				model_manager.model[model_::id_::MAP + i_node],
				memory_chunk
			);
		}

		lightmap_manager.grid_TEMP = &grid;

		for (__int32 i_node = 0; i_node < grid_::NUM_NODES; i_node++) {

			lightmap_manager.n_model_nodes[i_node] = model_token_manager.model_tokens[model_token_::id_::MAP + i_node + 1].n_models;

			Allocate_Memory_Lightmaps(

				i_node,
				model_token_manager.model_tokens[model_token_::id_::MAP + i_node + 1],
				lightmap_manager,
				model_manager.model[model_::id_::MAP + i_node + 1],
				memory_chunk
			);
		}
}

/*
==================
==================
*/
void Specialise_Cube_Template(

	model_manager_& model_manager,
	memory_chunk_& memory_chunk

) {

	const __int32 n_models = 10;
	const __int32 write_model_ids[n_models] = {

		model_::id_::PLATFORM,
		model_::id_::BOUNCE_PAD,
		model_::id_::PLATE,
		model_::id_::BUTTON,
		model_::id_::SPOTLIGHT,
		model_::id_::PATROL_POINT,
		model_::id_::PARTICLE,
		model_::id_::SKY_BOX,
		model_::id_::TELEPORTER,
		model_::id_::DOOR,
	};

	const __int32 read_model_ids[n_models] = {

		model_::id_::CUBE,
		model_::id_::CUBE,
		model_::id_::CUBE,
		model_::id_::CUBE,
		model_::id_::CUBE,
		model_::id_::CUBE,
		model_::id_::CUBE,
		model_::id_::CUBE_BACKFACE,
		model_::id_::CUBE,
		model_::id_::CUBE,
	};

	__int32 n_textures[n_models];
	const char* file_names[n_models][4];

	n_textures[0] = 2;
	file_names[0][0] = "textures/platform.bmp";
	file_names[0][1] = "textures/credit.bmp";

	n_textures[1] = 1;
	file_names[1][0] = "textures/bounce_pad.jpg";

	n_textures[2] = 1;
	file_names[2][0] = "textures/switch.bmp";

	n_textures[3] = 1;
	file_names[3][0] = "textures/button.bmp";

	n_textures[4] = 1;
	file_names[4][0] = "textures/spotlight.bmp";

	n_textures[5] = 1;
	file_names[5][0] = "textures/test.png";

	n_textures[6] = 1;
	file_names[6][0] = "textures/test.png";

	n_textures[7] = 1;
	file_names[7][0] = "textures/sky.jpg";

	n_textures[8] = 1;
	file_names[8][0] = "textures/star_field.bmp";

	n_textures[9] = 1;
	file_names[9][0] = "textures/door.png";


	for (__int32 i_model = 0; i_model < n_models; i_model++) {

		const __int32 read_model_id = read_model_ids[i_model];
		const __int32 write_model_id = write_model_ids[i_model];

		Create_Cube_Model(

			n_textures[i_model],
			file_names[i_model],
			model_manager.model[read_model_id],
			model_manager.model[write_model_id],
			memory_chunk
		);
	}

	// dodgy fixups!
	{
		float3_ colour = { 0.0f, 0.0f, 0.0f };
		model_& model = model_manager.model[model_::id_::PARTICLE];
		for (__int32 i_vertex = 0; i_vertex < model.n_colour_vertices; i_vertex++) {
			model.attribute_vertices[model_::ATTRIBUTE_COLOUR][i_vertex] = colour;
			colour.r += 10.0f;
			colour.g += 10.0f;
		}
	}

}

/*
==================
==================
*/
void Load_External_Models(

	model_manager_& model_manager,
	memory_chunk_& memory_chunk

) {

	const char* file_spec = "models/*.mdl";
	_finddata_t file_info;
	intptr_t  file_handle;

	file_handle = _findfirst(file_spec, &file_info);

	if (file_handle == -1) {
		printf_s("ERROR: no model files found \n");
		exit(0);
	}

	const char* path = "models/";
	__int32 n_models = 0;

	do {
		//printf_s("found %s \n", file_info.name);

		model_& model = model_manager.model[model_::id_::COUNT_NAMED + n_models];
		string_copy(file_info.name, model.name);
		char file_name[128];
		string_concatenate(path, file_info.name, file_name);

		const __int32 i_skin = 0;

		Load_Quake_Model(

			file_name,
			i_skin,
			model,
			memory_chunk
		);

		n_models++;

	} while (_findnext(file_handle, &file_info) == 0);

	_findclose(file_handle);

	model_manager.n_models = model_::id_::COUNT_NAMED + n_models;

	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 model_id_shambler = Return_Model_ID("mon_shambler", model_manager);
		const model_& model_shambler = model_manager.model[model_id_shambler];
		const char* texture_name = "textures/stone.png";
		Load_Image_STB(texture_name, model_shambler.texture_handlers[1], memory_chunk);

		{
			const __int32 model_id = Return_Model_ID("mon_bossnour", model_manager);
			const model_& model = model_manager.model[model_id];
			memcpy(&model.texture_handlers[1], &model_shambler.texture_handlers[1], sizeof(texture_handler_));
		}
		{
			const __int32 model_id = Return_Model_ID("mon_demon", model_manager);
			const model_& model = model_manager.model[model_id];
			memcpy(&model.texture_handlers[1], &model_shambler.texture_handlers[1], sizeof(texture_handler_));
		}
		{
			const __int32 model_id = Return_Model_ID("mon_hknight", model_manager);
			const model_& model = model_manager.model[model_id];
			memcpy(&model.texture_handlers[1], &model_shambler.texture_handlers[1], sizeof(texture_handler_));
		}
		{
			const __int32 model_id = Return_Model_ID("mon_ogre", model_manager);
			const model_& model = model_manager.model[model_id];
			memcpy(&model.texture_handlers[1], &model_shambler.texture_handlers[1], sizeof(texture_handler_));
		}
		{
			const __int32 model_id = Return_Model_ID("mon_seeker", model_manager);
			const model_& model = model_manager.model[model_id];
			memcpy(&model.texture_handlers[1], &model_shambler.texture_handlers[1], sizeof(texture_handler_));

		}
		{
			const __int32 model_id = Return_Model_ID("mon_lostsoul", model_manager);
			const model_& model = model_manager.model[model_id];
			memcpy(&model.texture_handlers[1], &model_shambler.texture_handlers[1], sizeof(texture_handler_));
		}
	}
	// ----------------------------------------------------------------------------------------------------------
	{
		float3_ colour = { 0.0f, 0.0f, 0.0f };

		const __int32 model_id = Return_Model_ID("mon_shambler", model_manager);
		model_& model = model_manager.model[model_id];
		for (__int32 i_vertex = 0; i_vertex < model.n_colour_vertices; i_vertex++) {
			model.attribute_vertices[model_::ATTRIBUTE_COLOUR][i_vertex] = colour;
		}
	}

	{
		float3_ colours[] = {

			{ 0.5f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.5f },
		};
		const __int32 model_id = Return_Model_ID("proj_ballslime", model_manager);
		model_& model = model_manager.model[model_id];
		for (__int32 i_vertex = 0; i_vertex < model.n_colour_vertices; i_vertex++) {
			//model.attribute_vertices[model_::ATTRIBUTE_COLOUR][i_vertex] = colours[i_vertex % 2];
			model.attribute_vertices[model_::ATTRIBUTE_COLOUR][i_vertex] = (model.vertices_frame[0][i_vertex] - model.bounding_origin) / model.bounding_extent;
			model.attribute_vertices[model_::ATTRIBUTE_COLOUR][i_vertex] *= 1.0f;
		}
	}
	// ----------------------------------------------------------------------------------------------------------

}

/*
==================
==================
*/
void Load_Map_Elements(

	model_token_manager_& model_token_manager
)
{

	const char* file_names[] = {

		"models/map.obj",
		"models/player.obj",
		"models/item.obj",
		"models/platform.obj",
		"models/teleporter.obj",
		"models/bounce_pad.obj",
		"models/shambler.obj",
		"models/scrag.obj",
		"models/patrol_one.obj",
		"models/patrol_two.obj",
		"models/patrol_scrag.obj",
		"models/trapdoor.obj",
		"models/switch.obj",
		"models/button.obj",
		"models/door.obj",
		"models/prop.obj",
		"models/mob.obj",
	};

	const __int32 i_tokens[] = {

		model_token_::id_::MAP,
		model_token_::id_::CAMERA,
		model_token_::id_::ITEM,
		model_token_::id_::PLATFORM,
		model_token_::id_::TELEPORTER,
		model_token_::id_::BOUNCE_PAD,
		model_token_::id_::SHAMBLER,
		model_token_::id_::SCRAG,
		model_token_::id_::PATROL_ONE,
		model_token_::id_::PATROL_TWO,
		model_token_::id_::SCRAG_WAYPOINT,
		model_token_::id_::TRAP_DOOR,
		model_token_::id_::PLATE,
		model_token_::id_::BUTTON,
		model_token_::id_::DOOR,
		model_token_::id_::PROP,
		model_token_::id_::MOB,
	};

	const __int32 n_proxies = sizeof(i_tokens) / sizeof(i_tokens[0]);

	for (__int32 i_token_read = 0; i_token_read < n_proxies; i_token_read++) {

		const __int32 i_token = i_tokens[i_token_read];
		model_token_& model_token = model_token_manager.model_tokens[i_token];
		model_token.index = model_token_manager.index + model_token_manager.n_blocks;
		model_token.centre = model_token_manager.centre + model_token_manager.n_blocks;
		model_token.extent = model_token_manager.extent + model_token_manager.n_blocks;
		model_token.normals = model_token_manager.normals + model_token_manager.n_blocks;
		model_token.n_models = 0;

		load_obj_file(i_tokens[i_token_read], file_names[i_token_read], model_token);

		model_token_manager.n_blocks += model_token.n_models;
	}
}


/*
==================
==================
*/
void Static_Initialise_Data(

	sound_event_ sound_event_table[colliding_type_::COUNT][colliding_type_::COUNT],
	animation_manager_& animation_manager,
	behaviour_manager_& behaviour_manager,
	collision_manager_& collision_manager,
	way_point_manager_& way_point_manager,
	model_manager_& model_manager,
	model_token_manager_& map_token,
	particle_manager_& particle_manager,
	command_buffer_handler_& command_buffer_handler,
	systems_::collision_response_& collision_response
)
{

	//=======================================================================================================================
	{
		float3_ zero = { 0.0f, 0.0f, 0.0f };

		{
			float3_ extent = { 20.0f, 20.0f, 20.0f };
			float3_ origin = { 0.0f, 0.0f, 0.0f };
			float3_ x_offset = { 20.0f, 0.0f, 0.0f };
			float3_ z_offset = { 0.0f, 0.0f, 20.0f };
			float3_ y_offset = { 0.0f, 20.0f, 0.0f };

			float3_* position = particle_manager.impact.position;

			for (__int32 i_particle = 0; i_particle < particle_manager_::NUM_PARTICLES_PER_EMITTER; i_particle++) {
				position[i_particle] = zero;
			}

			float3_* velocity = particle_manager.impact.velocity;

			velocity[0] = origin;
			velocity[1] = velocity[0] + x_offset;

			velocity[2] = velocity[0] + z_offset;
			velocity[3] = velocity[1] + z_offset;

			velocity[4] = velocity[0] + y_offset;
			velocity[5] = velocity[1] + y_offset;
			velocity[6] = velocity[2] + y_offset;
			velocity[7] = velocity[3] + y_offset;

			velocity[8] = velocity[4] + y_offset;
			velocity[9] = velocity[5] + y_offset;
			velocity[10] = velocity[6] + y_offset;
			velocity[11] = velocity[7] + y_offset;

			velocity[12] = velocity[8] + y_offset;
			velocity[13] = velocity[9] + y_offset;
			velocity[14] = velocity[10] + y_offset;
			velocity[15] = velocity[11] + y_offset;

			const float velocity_scale = 4.0f;

			for (__int32 i_particle = 0; i_particle < particle_manager_::NUM_PARTICLES_PER_EMITTER; i_particle++) {
				velocity[i_particle] -= extent * 0.5f;
				velocity[i_particle] *= velocity_scale;
			}

			particle_manager.impact.scale = { 4.0f, 4.0f, 4.0f };
			particle_manager.impact.colour = zero;
		}
		{
			float spacing = 20.0f;
			float n_wide = 4;
			float length = (n_wide - 1) * spacing;
			float3_ extent = { length, 0.0f, length };
			float3_ origin = { 0.0f, 0.0f, 0.0f };
			float3_ x_offset = { spacing, 0.0f, 0.0f };
			float3_ z_offset = { 0.0f, 0.0f, spacing };

			float3_* position = particle_manager.trapdoor.position;

			position[0] = origin;
			position[1] = position[0] + x_offset;
			position[2] = position[1] + x_offset;
			position[3] = position[2] + x_offset;

			position[4] = position[0] + z_offset;
			position[5] = position[1] + z_offset;
			position[6] = position[2] + z_offset;
			position[7] = position[3] + z_offset;

			position[8] = position[4] + z_offset;
			position[9] = position[5] + z_offset;
			position[10] = position[6] + z_offset;
			position[11] = position[7] + z_offset;

			position[12] = position[8] + z_offset;
			position[13] = position[9] + z_offset;
			position[14] = position[10] + z_offset;
			position[15] = position[11] + z_offset;

			for (__int32 i_particle = 0; i_particle < particle_manager_::NUM_PARTICLES_PER_EMITTER; i_particle++) {
				position[i_particle] -= extent * 0.5f;
			}

			for (__int32 i_particle = 0; i_particle < particle_manager_::NUM_PARTICLES_PER_EMITTER; i_particle++) {
				particle_manager.trapdoor.velocity[i_particle] = position[i_particle] * 1.0f;
				particle_manager.trapdoor.velocity[i_particle].y -= i_particle * 8.0f;
			}

			particle_manager.trapdoor.scale = { 10.0f, 6.0f, 10.0f };
			particle_manager.trapdoor.colour = { 4.0f, 4.0f, 4.0f };
		}
	}
	//=======================================================================================================================
	{
		component_::weapon_::projectile_data[component_::weapon_::id_::KILL].reload_time = 0.3f;
		component_::weapon_::projectile_data[component_::weapon_::id_::KILL].speed = 600.0f;
		component_::weapon_::projectile_data[component_::weapon_::id_::KILL].colour = { 100.0f, 0.0f, 0.0f };
		component_::weapon_::projectile_data[component_::weapon_::id_::KILL].scale = { 4.0f, 4.0f, 4.0f };

		component_::weapon_::projectile_data[component_::weapon_::id_::SHRINK].reload_time = 0.5f;
		component_::weapon_::projectile_data[component_::weapon_::id_::SHRINK].speed = 400.0f;
		component_::weapon_::projectile_data[component_::weapon_::id_::SHRINK].colour = { 0.0f, 100.0f, 0.0f };
		component_::weapon_::projectile_data[component_::weapon_::id_::SHRINK].scale = { 6.0f, 6.0f, 6.0f };


		component_::weapon_::projectile_data[component_::weapon_::id_::PETRIFY].reload_time = 0.5f;
		component_::weapon_::projectile_data[component_::weapon_::id_::PETRIFY].speed = 400.0f;
		component_::weapon_::projectile_data[component_::weapon_::id_::PETRIFY].colour = { 0.0f, 0.0f, 100.0f };
		component_::weapon_::projectile_data[component_::weapon_::id_::PETRIFY].scale = { 6.0f, 6.0f, 6.0f };

		component_::weapon_::projectile_data[component_::weapon_::id_::PUSH].reload_time = 0.5f;
		component_::weapon_::projectile_data[component_::weapon_::id_::PUSH].speed = 400.0f;
		component_::weapon_::projectile_data[component_::weapon_::id_::PUSH].colour = { 100.0f, 0.0f, 100.0f };
		component_::weapon_::projectile_data[component_::weapon_::id_::PUSH].scale = { 4.0f, 4.0f, 4.0f };

		component_::weapon_::projectile_data[component_::weapon_::id_::PULL].reload_time = 0.5f;
		component_::weapon_::projectile_data[component_::weapon_::id_::PULL].speed = 400.0f;
		component_::weapon_::projectile_data[component_::weapon_::id_::PULL].colour = { 0.0f, 100.0f, 100.0f };
		component_::weapon_::projectile_data[component_::weapon_::id_::PULL].scale = { 4.0f, 4.0f, 4.0f };
	}
	//=======================================================================================================================
	{
		for (__int32 i_collider = 0; i_collider < colliding_type_::COUNT; i_collider++) {
			for (__int32 i_collidee = 0; i_collidee < colliding_type_::COUNT; i_collidee++) {
				collision_response.n_entries[i_collider][i_collidee] = 0;
				collision_response.Entity_Reaction[i_collider][i_collidee][0] = systems_::collision_response_::entity_vs_null;
			}
		}
		for (__int32 i_collider = 0; i_collider < colliding_type_::COUNT; i_collider++) {

			{
				__int32 i_collidee = colliding_type_::TELEPORTER;
				__int32& n_entries = collision_response.n_entries[i_collider][i_collidee];
				collision_response.Entity_Reaction[i_collider][i_collidee][n_entries++] = systems_::collision_response_::entity_vs_teleporter;
			}
			{
				__int32 i_collidee = colliding_type_::BOUNCE_PAD;
				__int32& n_entries = collision_response.n_entries[i_collider][i_collidee];
				collision_response.Entity_Reaction[i_collider][i_collidee][n_entries++] = systems_::collision_response_::entity_vs_bounce_pad;
			}
			{
				__int32 i_collidee = colliding_type_::PLATFORM;
				__int32& n_entries = collision_response.n_entries[i_collider][i_collidee];
				collision_response.Entity_Reaction[i_collider][i_collidee][n_entries++] = systems_::collision_response_::apply_displacement;
			}
		}

		{
			__int32 i_collider = colliding_type_::PLAYER;
			__int32 i_collidee = colliding_type_::PLATE;
			__int32& n_entries = collision_response.n_entries[i_collider][i_collidee];
			collision_response.Entity_Reaction[i_collider][i_collidee][n_entries++] = systems_::collision_response_::entity_vs_switch;
		}
		{
			__int32 i_collider = colliding_type_::MONSTER;
			__int32 i_collidee = colliding_type_::PLATE;
			__int32& n_entries = collision_response.n_entries[i_collider][i_collidee];
			collision_response.Entity_Reaction[i_collider][i_collidee][n_entries++] = systems_::collision_response_::entity_vs_switch;
		}
		{
			__int32 i_collider = colliding_type_::PROJECTILES;
			__int32 i_collidee = colliding_type_::BUTTON;
			__int32& n_entries = collision_response.n_entries[i_collider][i_collidee];
			collision_response.Entity_Reaction[i_collider][i_collidee][n_entries++] = systems_::collision_response_::entity_vs_button;
		}
		{
			__int32 i_collider = colliding_type_::MONSTER;
			__int32 i_collidee = colliding_type_::BUTTON;
			__int32& n_entries = collision_response.n_entries[i_collider][i_collidee];
			collision_response.Entity_Reaction[i_collider][i_collidee][n_entries++] = systems_::collision_response_::entity_vs_button;
		}
		{
			__int32 i_collider = colliding_type_::MONSTER;
			__int32 i_collidee = colliding_type_::PLATFORM;
			__int32& n_entries = collision_response.n_entries[i_collider][i_collidee];
			collision_response.Entity_Reaction[i_collider][i_collidee][n_entries++] = systems_::collision_response_::entity_vs_platform;
		}
		{
			__int32 i_collider = colliding_type_::PROJECTILES;
			__int32 i_collidee = colliding_type_::MONSTER;
			__int32& n_entries = collision_response.n_entries[i_collider][i_collidee];
			collision_response.Entity_Reaction[i_collider][i_collidee][n_entries++] = systems_::collision_response_::projectile_vs_monster;
		}
		{
			__int32 i_collider = colliding_type_::PROJECTILES;
			__int32 i_collidee = colliding_type_::MOB;
			__int32& n_entries = collision_response.n_entries[i_collider][i_collidee];
			collision_response.Entity_Reaction[i_collider][i_collidee][n_entries++] = systems_::collision_response_::projectile_vs_mob;
		}
		{
			__int32 i_collider = colliding_type_::PROJECTILES;
			__int32 i_collidee = colliding_type_::PLAYER;
			__int32& n_entries = collision_response.n_entries[i_collider][i_collidee];
			collision_response.Entity_Reaction[i_collider][i_collidee][n_entries++] = systems_::collision_response_::projectile_vs_player;
		}
		{
			__int32 i_collider = colliding_type_::PLAYER;
			__int32 i_collidee = colliding_type_::TRAP_DOOR;
			__int32& n_entries = collision_response.n_entries[i_collider][i_collidee];
			collision_response.Entity_Reaction[i_collider][i_collidee][n_entries++] = systems_::collision_response_::entity_vs_trapdoor;
		}
		{
			__int32 i_collider = colliding_type_::PROJECTILES;
			__int32 i_collidee = colliding_type_::TRAP_DOOR;
			__int32& n_entries = collision_response.n_entries[i_collider][i_collidee];
			collision_response.Entity_Reaction[i_collider][i_collidee][n_entries++] = systems_::collision_response_::entity_vs_trapdoor;
		}
		for (__int32 i_collidee = 0; i_collidee < colliding_type_::COUNT; i_collidee++) {

			{
				__int32 i_collider = colliding_type_::PARTICLES;
				__int32& n_entries = collision_response.n_entries[i_collider][i_collidee];
				collision_response.Entity_Reaction[i_collider][i_collidee][n_entries++] = systems_::collision_response_::particle_vs_entity;
			}
		}
		//collision_response.Entity_Reaction[colliding_type_::PROJECTILES][colliding_type_::TRAP_DOOR][0] = systems_::collision_response_::entity_vs_trapdoor;
	}

	//=======================================================================================================================
	// BITONIC SORT TEST
	//=======================================================================================================================
	{
		//{
		//	__int32 input_array[16] = {

		//		15, 13, 14, 9, 11, 10, 9, 1, 7, 6, 12, 4, 0, 2, 8, 3,
		//	};

		//	__int32 output_array[16];

		//	bitonic_sort_16(bitonic_::UP, input_array, output_array);

		//	for (__int32 i = 0; i < 16; i++) {
		//		printf_s("%i,", output_array[i]);
		//	}
		//	printf_s("\n");
		//}
	}
	//=======================================================================================================================
	{

		//const animation_data_ animation_data_player[] = {

		//	{ animation_data_::id_::IDLE,	0,	8 },
		//{ animation_data_::id_::WALK,	73,	8 },
		//{ animation_data_::id_::ATTACK,	81,	9 },
		//{ animation_data_::id_::PAIN,	40,	6 },
		//};
		//const animation_data_ animation_data_shambler[] = {

		//	{ animation_data_::id_::IDLE,	0,	17 },
		//{ animation_data_::id_::WALK,	17,	12 },
		//{ animation_data_::id_::RUN,	29, 6 },
		//{ animation_data_::id_::ATTACK,	65, 11 },
		//{ animation_data_::id_::PAIN,	77, 6 },
		//{ animation_data_::id_::DEATH,	83, 10 },
		//{ animation_data_::id_::DEAD,	93, 1 },
		//};
		//const animation_data_ animation_data_scrag[] = {

		//	{ animation_data_::id_::IDLE, 0, 11 },
		//{ animation_data_::id_::WALK, 11, 13 },
		//{ animation_data_::id_::RUN, 99, 18 },
		//{ animation_data_::id_::ATTACK, 59, 17 },
		//{ animation_data_::id_::PAIN, 24, 6 },
		//{ animation_data_::id_::DEATH, 39, 9 },
		//{ animation_data_::id_::DEAD,	47, 1 },
		//};

		for (__int32 i_model = 0; i_model < animation_model_::id_::MAX_MODELS; i_model++) {

			animation_model_& animation_model = animation_manager.animation_model[i_model];

			for (__int32 i_animation = 0; i_animation < animation_data_::id_::COUNT; i_animation++) {

				animation_model.animation_data[i_animation].id = i_animation;
				animation_model.animation_data[i_animation].i_start = 0;
				animation_model.animation_data[i_animation].n_frames = 8;
			}
		}

		const __int32 model_id_named[] = {

			Return_Model_ID("mon_soldier", model_manager),
			Return_Model_ID("mon_shambler", model_manager),
		};

		{
			animation_manager.animation_model[animation_model_::id_::PLAYER].model_id = model_id_named[0];

			animation_manager.animation_model[animation_model_::id_::PLAYER].animation_data[animation_data_::id_::IDLE].i_start = 0;
			animation_manager.animation_model[animation_model_::id_::PLAYER].animation_data[animation_data_::id_::IDLE].n_frames = 8;

			animation_manager.animation_model[animation_model_::id_::PLAYER].animation_data[animation_data_::id_::WALK].i_start = 73;
			animation_manager.animation_model[animation_model_::id_::PLAYER].animation_data[animation_data_::id_::WALK].n_frames = 8;

			animation_manager.animation_model[animation_model_::id_::PLAYER].animation_data[animation_data_::id_::ATTACK].i_start = 81;
			animation_manager.animation_model[animation_model_::id_::PLAYER].animation_data[animation_data_::id_::ATTACK].n_frames = 9;

			animation_manager.animation_model[animation_model_::id_::PLAYER].animation_data[animation_data_::id_::PAIN].i_start = 40;
			animation_manager.animation_model[animation_model_::id_::PLAYER].animation_data[animation_data_::id_::PAIN].n_frames = 6;
		}
		{
			animation_manager.animation_model[animation_model_::id_::SHAMBLER].model_id = model_id_named[1];

			animation_manager.animation_model[animation_model_::id_::SHAMBLER].animation_data[animation_data_::id_::IDLE].i_start = 0;
			animation_manager.animation_model[animation_model_::id_::SHAMBLER].animation_data[animation_data_::id_::IDLE].n_frames = 17;

			animation_manager.animation_model[animation_model_::id_::SHAMBLER].animation_data[animation_data_::id_::WALK].i_start = 17;
			animation_manager.animation_model[animation_model_::id_::SHAMBLER].animation_data[animation_data_::id_::WALK].n_frames = 12;

			animation_manager.animation_model[animation_model_::id_::SHAMBLER].animation_data[animation_data_::id_::RUN].i_start = 29;
			animation_manager.animation_model[animation_model_::id_::SHAMBLER].animation_data[animation_data_::id_::RUN].n_frames = 6;

			animation_manager.animation_model[animation_model_::id_::SHAMBLER].animation_data[animation_data_::id_::ATTACK].i_start = 65;
			animation_manager.animation_model[animation_model_::id_::SHAMBLER].animation_data[animation_data_::id_::ATTACK].n_frames = 11;

			animation_manager.animation_model[animation_model_::id_::SHAMBLER].animation_data[animation_data_::id_::PAIN].i_start = 77;
			animation_manager.animation_model[animation_model_::id_::SHAMBLER].animation_data[animation_data_::id_::PAIN].n_frames = 6;

			animation_manager.animation_model[animation_model_::id_::SHAMBLER].animation_data[animation_data_::id_::DEATH].i_start = 83;
			animation_manager.animation_model[animation_model_::id_::SHAMBLER].animation_data[animation_data_::id_::DEATH].n_frames = 10;

			animation_manager.animation_model[animation_model_::id_::SHAMBLER].animation_data[animation_data_::id_::DEAD].i_start = 93;
			animation_manager.animation_model[animation_model_::id_::SHAMBLER].animation_data[animation_data_::id_::DEAD].n_frames = 1;
		}
		{

			for (__int32 model_id = 0; model_id < model_manager.n_models; model_id++) {

				model_& model = model_manager.model[model_id];
				bool is_animated_model = model.n_frames > 1;

				if (is_animated_model) {

					for (__int32 i_frame = 0; i_frame < model.n_frames; i_frame++) {

						__int32 n_chars = 0;
						for (__int32 i_char = 0; i_char < model_::MAX_FRAME_NAME_CHARS; i_char++) {

							char read = model.frame_name[i_frame][i_char];
							model.frame_name[i_frame][n_chars] = read;
							n_chars += (read < '0') | (read > '9');
						}
						//model.frame_name[i_frame][n_chars] = '\0';

						//printf_s("model name: %s \n", model.frame_name[i_frame]);

					}
				}
			}
		}
		{
			struct feed_ {

				__int32 animation_id;
				__int32 n_search_strings;
				char search_strings[8][16];
			};

			feed_ feeds[animation_data_::id_::COUNT];

			__int32 n_feeds = 0;
			{
				feeds[n_feeds].animation_id = animation_data_::id_::IDLE;
				feeds[n_feeds].n_search_strings = 0;
				string_copy("idle", feeds[n_feeds].search_strings[feeds[n_feeds].n_search_strings++]);
				string_copy("stand", feeds[n_feeds].search_strings[feeds[n_feeds].n_search_strings++]);
				string_copy("aidle", feeds[n_feeds].search_strings[feeds[n_feeds].n_search_strings++]);
				n_feeds++;
			}
			{
				feeds[n_feeds].animation_id = animation_data_::id_::ATTACK;
				feeds[n_feeds].n_search_strings = 0;
				string_copy("fire", feeds[n_feeds].search_strings[feeds[n_feeds].n_search_strings++]);
				string_copy("shoot", feeds[n_feeds].search_strings[feeds[n_feeds].n_search_strings++]);
				string_copy("melee", feeds[n_feeds].search_strings[feeds[n_feeds].n_search_strings++]);
				string_copy("attacka", feeds[n_feeds].search_strings[feeds[n_feeds].n_search_strings++]);
				string_copy("smash", feeds[n_feeds].search_strings[feeds[n_feeds].n_search_strings++]);
				n_feeds++;
			}
			{
				feeds[n_feeds].animation_id = animation_data_::id_::PAIN;
				feeds[n_feeds].n_search_strings = 0;
				string_copy("pain", feeds[n_feeds].search_strings[feeds[n_feeds].n_search_strings++]);
				string_copy("paina", feeds[n_feeds].search_strings[feeds[n_feeds].n_search_strings++]);
				string_copy("painb", feeds[n_feeds].search_strings[feeds[n_feeds].n_search_strings++]);
				string_copy("fire", feeds[n_feeds].search_strings[feeds[n_feeds].n_search_strings++]);
				n_feeds++;
			}
			{
				feeds[n_feeds].animation_id = animation_data_::id_::DEATH;
				feeds[n_feeds].n_search_strings = 0;
				string_copy("die", feeds[n_feeds].search_strings[feeds[n_feeds].n_search_strings++]);
				string_copy("death", feeds[n_feeds].search_strings[feeds[n_feeds].n_search_strings++]);
				string_copy("painc", feeds[n_feeds].search_strings[feeds[n_feeds].n_search_strings++]);
				n_feeds++;
			}

			__int32 n_animated_models = 0;

			for (__int32 model_id = 0; model_id < model_manager.n_models; model_id++) {

				const model_& model = model_manager.model[model_id];
				const __int32 n_frames = model.n_frames;
				bool is_valid = (n_frames > 1) && (model_id != model_id_named[0]) && (model_id != model_id_named[1]);

				if (is_valid) {

					animation_model_& animation_model = animation_manager.animation_model[animation_model_::id_::COUNT_NAMED + n_animated_models];
					animation_model.model_id = model_id;

					//printf_s("model name: %s , model id: %i \n", model.name, model_id);

					//const char* search_string[] = { "idle","stand", "aidle" };
					//const char* search_string[] = { "attack","fire", "shoot", "melee", "attacka", "smash" };
					//const char* search_string[] = { "pain", "paina", "painb", "die_headless"};
					//const __int32 n_strings = sizeof(search_string) / sizeof(search_string[0]);

					for (__int32 i_animation = 0; i_animation < n_feeds; i_animation++) {

						feed_& feed = feeds[i_animation];

						for (__int32 i_search = 0; i_search < feed.n_search_strings; i_search++) {

							__int32 i_start = -1;
							__int32 n_frames = 0;

							for (__int32 i_frame = 0; i_frame < model.n_frames; i_frame++) {

								__int32 match = string_compare(feed.search_strings[i_search], model.frame_name[i_frame]);
								i_start = (match != 0) && (i_start < 0) ? i_frame : i_start;
								n_frames += match;
							}

							if (i_start >= 0) {
								animation_model.animation_data[feed.animation_id].i_start = i_start;
								animation_model.animation_data[feed.animation_id].n_frames = n_frames;
							}

							//printf_s(" start: %i, num frames: %i \n", i_start, n_frames);
						}
					}

					n_animated_models++;
				}
			}

			animation_manager.n_animated_models = n_animated_models + animation_model_::id_::COUNT_NAMED;

			//printf_s("n animated models: %i \n", n_animated_models);

		}
	}
	//=======================================================================================================================
	{
		{
			behaviour_data_& behaviour = behaviour_manager.behaviour_data[component_::behaviour_::id_::STAND];

			behaviour.animation_id = animation_data_::id_::IDLE;
			behaviour.target_id = component_::behaviour_::target_id_::NULL_;
			//behaviour.time_duration = 10.0f;
			behaviour.time_duration = 0.0f;
			behaviour.move_acceleration = 0.0f;
			behaviour.max_speed = 0.0f;
			behaviour.component_toggle.n_components = 0;
		}
		{
			behaviour_data_& behaviour = behaviour_manager.behaviour_data[component_::behaviour_::id_::MOVE_PATROL];

			behaviour.animation_id = animation_data_::id_::WALK;
			behaviour.target_id = component_::behaviour_::target_id_::PATROL_POINT;
			//behaviour.time_duration = 30.0f;
			behaviour.time_duration = 0.0f;
			behaviour.move_acceleration = 600.0f;
			behaviour.max_speed = 45.0f;
			behaviour.component_toggle.n_components = 0;

		}
		{
			behaviour_data_& behaviour = behaviour_manager.behaviour_data[component_::behaviour_::id_::CHARGE_PLAYER];

			behaviour.animation_id = animation_data_::id_::RUN;
			behaviour.target_id = component_::behaviour_::target_id_::PLAYER;
			behaviour.time_duration = 3.0f;
			behaviour.move_acceleration = 300.0f;
			behaviour.max_speed = 80.0f;
			behaviour.component_toggle.n_components = 0;
		}
		{
			behaviour_data_& behaviour = behaviour_manager.behaviour_data[component_::behaviour_::id_::ATTACK];

			behaviour.animation_id = animation_data_::id_::ATTACK;
			behaviour.target_id = component_::behaviour_::target_id_::PLAYER;
			behaviour.time_duration = 0.0f;
			behaviour.move_acceleration = 300.0f;
			behaviour.max_speed = 0.0f;
			behaviour.component_toggle.n_components = 0;
		}
		{
			behaviour_data_& behaviour = behaviour_manager.behaviour_data[component_::behaviour_::id_::DEATH];

			behaviour.animation_id = animation_data_::id_::DEATH;
			behaviour.target_id = component_::behaviour_::target_id_::NULL_;
			behaviour.time_duration = 0.0f;
			behaviour.move_acceleration = 0.0f;
			behaviour.max_speed = 0.0f;
			behaviour.component_toggle.n_components = 2;
			behaviour.component_toggle.component_ids[0] = component_id_::COLLIDEE_STATIC;
			behaviour.component_toggle.component_ids[1] = component_id_::FLY;
		}
		{
			behaviour_data_& behaviour = behaviour_manager.behaviour_data[component_::behaviour_::id_::DEAD];

			behaviour.animation_id = animation_data_::id_::DEAD;
			behaviour.target_id = component_::behaviour_::target_id_::NULL_;
			behaviour.time_duration = 30.0f;
			behaviour.move_acceleration = 0.0f;
			behaviour.max_speed = 0.0f;
			behaviour.component_toggle.n_components = 2;
			behaviour.component_toggle.component_ids[0] = component_id_::COLLIDEE_STATIC;
			behaviour.component_toggle.component_ids[1] = component_id_::FLY;
		}
		{
			behaviour_data_& behaviour = behaviour_manager.behaviour_data[component_::behaviour_::id_::SHRINK];

			behaviour.animation_id = animation_data_::id_::IDLE;
			behaviour.target_id = component_::behaviour_::target_id_::NULL_;
			behaviour.time_duration = 10.0f;
			behaviour.move_acceleration = 0.0f;
			behaviour.max_speed = 0.0f;
			behaviour.component_toggle.n_components = 0;
			//behaviour.component_toggle.component_ids[0] = component_id_::COLLIDEE_STATIC;
		}
		{
			behaviour_data_& behaviour = behaviour_manager.behaviour_data[component_::behaviour_::id_::PAIN];

			behaviour.animation_id = animation_data_::id_::PAIN;
			behaviour.target_id = component_::behaviour_::target_id_::NULL_;
			behaviour.time_duration = 0.0f;
			behaviour.move_acceleration = 0.0f;
			behaviour.max_speed = 0.0f;
			behaviour.component_toggle.n_components = 0;
		}
	}
	//=======================================================================================================================

	{
		for (__int32 i_collider = 0; i_collider < colliding_type_::COUNT; i_collider++) {
			for (__int32 i_collidee = 0; i_collidee < colliding_type_::COUNT; i_collidee++) {

				sound_event_& sound_event = sound_event_table[i_collider][i_collidee];
				sound_event.sound_id = sound_id_::NULL_SOUND;
				sound_event.source_id = sound_triggers_::source_id_::DEFAULT;
				sound_event.is_valid = 0;
			}
		}
		{
			sound_event_& sound_event = sound_event_table[colliding_type_::PROJECTILES][colliding_type_::MAP];
			sound_event.sound_id = sound_id_::TINK;
			sound_event.source_id = sound_triggers_::source_id_::PROJECTILE;
			sound_event.is_valid = 1;
		}
		{
			sound_event_& sound_event = sound_event_table[colliding_type_::PROJECTILES][colliding_type_::MONSTER];
			sound_event.sound_id = sound_id_::EXPLODE;
			sound_event.source_id = sound_triggers_::source_id_::PROJECTILE;
			sound_event.is_valid = 1;
		}
		{
			sound_event_& sound_event = sound_event_table[colliding_type_::PROJECTILES][colliding_type_::PLAYER];
			sound_event.sound_id = sound_id_::EXPLODE;
			sound_event.source_id = sound_triggers_::source_id_::PROJECTILE;
			sound_event.is_valid = 1;
		}
		{
			for (__int32 i_collider = 0; i_collider < colliding_type_::COUNT; i_collider++) {

				sound_event_& sound_event = sound_event_table[i_collider][colliding_type_::BOUNCE_PAD];
				sound_event.sound_id = sound_id_::BOUNCE;
				sound_event.source_id = sound_triggers_::source_id_::BOUNCE_PAD;
				sound_event.is_valid = 1;
			}
		}
		{
			for (__int32 i_collider = 0; i_collider < colliding_type_::COUNT; i_collider++) {

				sound_event_& sound_event = sound_event_table[i_collider][colliding_type_::TELEPORTER];
				sound_event.sound_id = sound_id_::TELEPORT;
				sound_event.source_id = sound_triggers_::source_id_::TELEPORTER;
				sound_event.is_valid = 1;
			}
		}
	}
	//=======================================================================================================================
	{

		for (__int32 i_collider = 0; i_collider < colliding_type_::COUNT; i_collider++) {
			for (__int32 i_collidee = 0; i_collidee < colliding_type_::COUNT; i_collidee++) {
				collision_manager.i_response_type[i_collider][i_collidee] = collision_response_type_::DEFLECT;
			}
		}
		for (__int32 i_collider = 0; i_collider < colliding_type_::COUNT; i_collider++) {
			collision_manager.i_response_type[i_collider][colliding_type_::BOUNCE_PAD] = collision_response_type_::REPULSE;
			collision_manager.i_response_type[i_collider][colliding_type_::TELEPORTER] = collision_response_type_::STOP;
		}
		for (__int32 i_collidee = 0; i_collidee < colliding_type_::COUNT; i_collidee++) {
			collision_manager.i_response_type[colliding_type_::PROJECTILES][i_collidee] = collision_response_type_::REFLECT;
			collision_manager.i_response_type[colliding_type_::PARTICLES][i_collidee] = collision_response_type_::REFLECT;
		}
		collision_manager.i_response_type[colliding_type_::PROJECTILES][colliding_type_::BOUNCE_PAD] = collision_response_type_::REPULSE;
		collision_manager.i_response_type[colliding_type_::PROJECTILES][colliding_type_::BUTTON] = collision_response_type_::STOP;

		collision_manager.i_response_type[colliding_type_::MONSTER][colliding_type_::PLAYER] = collision_response_type_::STOP;



		for (__int32 i_collider = 0; i_collider < colliding_type_::COUNT; i_collider++) {

			for (__int32 i_collidee = 0; i_collidee < colliding_type_::COUNT; i_collidee++) {
				collision_manager.allow_table[i_collider][i_collidee] = true;
			}
		}

		collision_manager.allow_table[colliding_type_::MONSTER][colliding_type_::MONSTER] = false;
		collision_manager.allow_table[colliding_type_::PLAYER][colliding_type_::PLAYER] = false;
		collision_manager.allow_table[colliding_type_::PLAYER][colliding_type_::MONSTER] = true;
		collision_manager.allow_table[colliding_type_::MONSTER][colliding_type_::PLAYER] = true;

		collision_manager.allow_table[colliding_type_::PARTICLES][colliding_type_::MONSTER] = false;
		collision_manager.allow_table[colliding_type_::PARTICLES][colliding_type_::PLAYER] = false;
		collision_manager.allow_table[colliding_type_::PARTICLES][colliding_type_::MOB] = false;
		collision_manager.allow_table[colliding_type_::PARTICLES][colliding_type_::PROJECTILES] = false;


		for (__int32 i_collider = 0; i_collider < colliding_type_::COUNT; i_collider++) {

			for (__int32 i_collidee = 0; i_collidee < colliding_type_::COUNT; i_collidee++) {
				collision_manager.can_step[i_collider][i_collidee] = false;
			}
		}

		collision_manager.can_step[colliding_type_::PLAYER][colliding_type_::MAP] = true;
		collision_manager.can_step[colliding_type_::PLAYER][colliding_type_::PLATE] = true;
		collision_manager.can_step[colliding_type_::MONSTER][colliding_type_::MAP] = true;
		collision_manager.can_step[colliding_type_::MONSTER][colliding_type_::PLATE] = true;

		for (__int32 i_collidee = 0; i_collidee < colliding_type_::COUNT; i_collidee++) {
			collision_manager.is_ground[i_collidee] = false;
		}

		collision_manager.is_ground[colliding_type_::MAP] = true;
		collision_manager.is_ground[colliding_type_::PLATFORM] = true;
		collision_manager.is_ground[colliding_type_::PLATE] = true;

	}
	//=======================================================================================================================
	{
		way_point_manager.n_patrols = 2;		// HARDCODED FFS

		__int32 offset = 0;
		for (__int32 i_patrol = 0; i_patrol < way_point_manager.n_patrols; i_patrol++) {

			const model_token_& model_token = map_token.model_tokens[model_token_::id_::PATROL_ONE + i_patrol];

			for (__int32 i_point = 0; i_point < model_token.n_models; i_point++) {

				__int32 i_model_token = INVALID_RESULT;
				for (__int32 i_token = 0; i_token < model_token.n_models; i_token++) {
					i_model_token = i_point == model_token.index[i_token] ? i_token : i_model_token;
				}
				way_point_manager.point_ids[i_patrol][i_point] = i_model_token + offset;
			}

			offset += model_token.n_models;
			way_point_manager.n_points[i_patrol] = model_token.n_models;
		}

	}
	//=======================================================================================================================
	{
		particle_manager.i_emitter = 0;
		__int32 particle_count = 0;

		for (__int32 i_emitter = 0; i_emitter < particle_manager_::NUM_EMITTERS; i_emitter++) {

			particle_manager.emitter[i_emitter].n_particles = particle_manager_::NUM_PARTICLES_PER_EMITTER;
			particle_manager.emitter[i_emitter].start_id = particle_count;
			particle_count += particle_manager.emitter[i_emitter].n_particles;
		}

	}
	//=======================================================================================================================
	{
		command_buffer_& command_buffer = command_buffer_handler.command_buffers[0];

		for (__int32 i_draw_call = 0; i_draw_call < draw_call_::id_::COUNT; i_draw_call++) {

			for (__int32 i_attribute = 0; i_attribute < MAX_VERTEX_ATTRIBUTES; i_attribute++) {

				command_buffer.draw_calls[i_draw_call].attribute_streams[i_attribute].vertex_shader = NULL;
			}

			command_buffer.draw_calls[i_draw_call].lighting_function = Vertex_Lighting_NULL;
			command_buffer.draw_calls[i_draw_call].n_additional_pixel_shaders = 0;
		}

		command_buffer_handler.command_buffers[0].model = model_manager.model;
		command_buffer_handler.command_buffers[1].model = model_manager.model;
	}
	//=======================================================================================================================
	{
		for (__int32 i_draw_call = 0; i_draw_call < draw_call_::id_::COUNT; i_draw_call++) {

			
		}
	}
}

/*
==================
==================
*/
void Copy_Command_Buffer_Data(command_buffer_handler_& command_buffer_handler) {

	command_buffer_& command_buffer = command_buffer_handler.command_buffers[0];

	for (__int32 i_draw_call = 0; i_draw_call < draw_call_::id_::COUNT; i_draw_call++) {
		command_buffer.draw_calls[i_draw_call].n_models = 0;
		command_buffer.draw_calls[i_draw_call].renderer_function = Render_STATIC_SMALL;

		for (__int32 i_model = 0; i_model < draw_call_::MAX_MODELS; i_model++) {

			command_buffer.draw_calls[i_draw_call].model_id[i_model] = 0;
		}
	}

	command_buffer.draw_calls[draw_call_::id_::PLAYER].renderer_function = Render_Animated_Model;
	command_buffer.draw_calls[draw_call_::id_::SHAMBLER].renderer_function = Render_Animated_Model;
	command_buffer.draw_calls[draw_call_::id_::SCRAG].renderer_function = Render_Animated_Model;
	command_buffer.draw_calls[draw_call_::id_::PROJECTILES].renderer_function = Render_Static_Model;

	command_buffer.draw_calls[draw_call_::id_::BOUNCE_PAD].renderer_function = Render_Rotated_Model;
	command_buffer.draw_calls[draw_call_::id_::ITEMS].renderer_function = Render_Rotated_Model;

	// lol so hack
	for (__int32 i_draw_call = draw_call_::id_::MAP; i_draw_call < draw_call_::id_::MAP + grid_::NUM_NODES + 1; i_draw_call++) {
		command_buffer.draw_calls[i_draw_call].renderer_function = Render_Static_Model;
	}

	command_buffer_handler.i_read = 0;
	command_buffer_handler.i_write = 1;
	memcpy(&command_buffer_handler.command_buffers[1], &command_buffer_handler.command_buffers[0], sizeof(command_buffer_));

}

/*
==================
==================
*/
void Initialise_Systems(

	sound_event_ sound_event_table[colliding_type_::COUNT][colliding_type_::COUNT],
	animation_manager_& animation_manager,
	behaviour_manager_& behaviour_manager,
	collision_manager_& collision_manager,
	way_point_manager_& way_point_manager,
	lightmap_manager_& lightmap_manager,
	model_manager_& model_manager,
	component_data_& component_data,
	model_token_manager_& map_token,
	timer_& timer,
	particle_manager_& particle_manager,
	command_buffer_handler_& command_buffer_handler,
	systems_::collision_response_& collision_response,
	grid_& grid,
	memory_chunk_& memory_chunk
)
{
	//=======================================================================================================================
	{
		Static_Initialise_Data(

			sound_event_table,
			animation_manager,
			behaviour_manager,
			collision_manager,
			way_point_manager,
			model_manager,
			map_token,
			particle_manager,
			command_buffer_handler,
			collision_response
		);
	}
	//=======================================================================================================================
	{
		command_buffer_& command_buffer = command_buffer_handler.command_buffers[0];

		COMPONENT_Populate_Table(

			component_data,
			way_point_manager,
			behaviour_manager,
			animation_manager,
			model_manager,
			map_token.model_tokens,
			command_buffer.draw_calls,
			grid,
			memory_chunk
		);
	}
	//=======================================================================================================================
	// light maps
	//=======================================================================================================================
	{

		Compute_Light_Map_Gradients(

			map_token,
			model_manager.model[model_::id_::CUBE],
			lightmap_manager,
			model_manager

		);

		Initialise_Light_Maps(lightmap_manager, model_manager);
	}
	//=======================================================================================================================
	// copy static command buffer data across buffers
	//=======================================================================================================================
	{
		Copy_Command_Buffer_Data(command_buffer_handler);
	}
}








