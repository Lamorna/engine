#include "light_maps.h"
#include "vector.h"
#include "threads.h"
#include "frame.h"
#include "setup.h"
#include "vector_math.h"
#include "quake.h"
#include "command.h"
#include "component.h"
#include "input.h"
#include "texture.h"
#include "memory.h"


//======================================================================

static const float light_extent = 30.0f;

static const __int32 axes_index[][lightmap_manager_::N_SURFACES_PER_MODEL] = {		

	{ X, Z, X, Z, X, X, },
	{ Y, Y, Y, Y, Z, Z, },
	{ Z, X, Z, X, Y, Y, },
};

static const float normal_sign[] = {

	1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
};

//======================================================================

/*
==================
==================
*/
void Allocate_Memory_Lightmaps(

	const __int32 i_node,
	const model_token_& token,
	lightmap_manager_& lightmap_manager,
	model_& model,
	memory_chunk_& memory

) {

	unsigned __int32 pack_masks[] = {		// per face

		0x3,		// z
		0x6,		// x
		0x3,		// z
		0x6,		// x
		0x5,		// y
		0x5,		// y
	};

	const __int32 n_vertices_per_surface = 4;
	const __m128 r_scaling_factor = set_all(0.5f);


	__int32 n_textures = 0;

	{
		// DUPLICATED ACROSS NODE MODELS!!!!!!!!!!!!!!
		const char* file_names[] = {

			"textures/floor.jpg",
			"textures/stone.png",
		};
		const __int32 n_base_textures = sizeof(file_names) / sizeof(file_names[0]);
		const __int32 n_textures_total = n_base_textures + (token.n_models * lightmap_manager_::N_SURFACES_PER_MODEL * 2);
		model.texture_handlers = (texture_handler_*)memory.chunk_ptr;
		memory.chunk_ptr = model.texture_handlers + n_textures_total;

		for (__int32 i_base_texture = 0; i_base_texture < n_base_textures; i_base_texture++) {

			Load_Image_STB(file_names[i_base_texture], model.texture_handlers[n_textures], memory);
			n_textures++;
		}
	}

	const unsigned __int32* lightmap_begin = (unsigned __int32*)memory.chunk_ptr;
	const __int32 i_lightmap_begin = n_textures;
	__int32 triangle_index = 0;

	for (__int32 i_model = 0; i_model < token.n_models; i_model++) {

		float3_ extent;
		for (__int32 i_axis = X; i_axis < W; i_axis++) {
			extent.f[i_axis] = (float)(token.extent[i_model].i[i_axis]) * r_fixed_scale_real;
		}
		//__m128 scale = gather(extent.f, Index_AOS, 3) & XYZ_Mask;
		__m128 scale = load_u(extent.f);
		__m128 packed_scale[6];
		__m128 model_scale = scale * r_scaling_factor;

		{
			for (__int32 i_face = 0; i_face < 6; i_face++) {
				packed_scale[i_face] = Pack_Vector4(model_scale, pack_masks[i_face]);
			}
			packed_scale[1] = _mm_shuffle_ps(packed_scale[1], packed_scale[1], _MM_SHUFFLE(W, Z, X, Y));
			packed_scale[3] = _mm_shuffle_ps(packed_scale[3], packed_scale[3], _MM_SHUFFLE(W, Z, X, Y));
		}

		for (__int32 i_face = 0; i_face < lightmap_manager_::N_SURFACES_PER_MODEL; i_face++) {

			__int32 dimensions[4];
			{
				__m128i packed_scale_i = convert_int_trunc(packed_scale[i_face]);
				store_u(packed_scale_i, dimensions);
			}

			__int32 x = dimensions[X];
			__int32 y = dimensions[Y];
			{
				x--;
				x |= x >> 1;
				x |= x >> 2;
				x |= x >> 4;
				x |= x >> 8;
				x |= x >> 16;
				x++;
			}
			{
				y--;
				y |= y >> 1;
				y |= y >> 2;
				y |= y >> 4;
				y |= y >> 8;
				y |= y >> 16;
				y++;
			}

			lightmap_data_& lightmap_data = lightmap_manager.model_nodes[i_node][i_model].lightmap_data[i_face];
			lightmap_data.i_read = 0;
			lightmap_data.i_lightmap[lightmap_data.i_read] = n_textures;

			texture_handler_& texture_handler = model.texture_handlers[n_textures];
			texture_handler.width = x;
			texture_handler.height = y;

			unsigned long width_shift = 0;
			_BitScanForward(&width_shift, x);
			unsigned long height_shift = 0;
			_BitScanForward(&height_shift, y);
			texture_handler.width_shift = width_shift;
			texture_handler.height_shift = height_shift;

			texture_handler.texture[0] = (unsigned __int32*)memory.chunk_ptr;
			memory.chunk_ptr = texture_handler.texture[0] + (x * y);

			model.i_textures[0][triangle_index + 0] = n_textures;
			model.i_textures[0][triangle_index + 1] = n_textures;

			triangle_index += 2;

			Build_MIP_Map_Chain(texture_handler, memory);

			n_textures++;
		}
	}
	//======================================================================
	{
		unsigned __int32* lightmap_end = (unsigned __int32*)memory.chunk_ptr;
		const __int64 size_lightmaps = lightmap_end - lightmap_begin;
		memory.chunk_ptr = lightmap_end + size_lightmaps;
		const __int32 n_lightmaps = n_textures - i_lightmap_begin;

		for (__int64 i_lexel = 0; i_lexel < size_lightmaps; i_lexel++) {

			lightmap_end[i_lexel] = (100 << 16) | (100 << 8) | (200 << 0);
		}

		for (__int64 i_lightmap = 0; i_lightmap < n_lightmaps; i_lightmap++) {

			const texture_handler_& texture_handler_read = model.texture_handlers[i_lightmap_begin + i_lightmap];
			texture_handler_& texture_handler_write = model.texture_handlers[n_textures + i_lightmap];
			texture_handler_write.width = texture_handler_read.width;
			texture_handler_write.height = texture_handler_read.height;
			texture_handler_write.width_shift = texture_handler_read.width_shift;
			texture_handler_write.height_shift = texture_handler_read.height_shift;
			texture_handler_write.n_mip_levels = texture_handler_read.n_mip_levels;
			
			for (__int64 i_mip = 0; i_mip < texture_handler_read.n_mip_levels; i_mip++) {
				texture_handler_write.texture[i_mip] = texture_handler_read.texture[i_mip] + size_lightmaps;
			}
		}

		for (__int32 i_model = 0; i_model < token.n_models; i_model++) {
			for (__int32 i_face = 0; i_face < lightmap_manager_::N_SURFACES_PER_MODEL; i_face++) {

				lightmap_data_& lightmap_data = lightmap_manager.model_nodes[i_node][i_model].lightmap_data[i_face];
				lightmap_data.i_lightmap[lightmap_data.i_read ^ 1] = lightmap_data.i_lightmap[lightmap_data.i_read] + n_lightmaps;
			}
		}

		model.n_textures = n_textures + n_lightmaps;
	}
}

/*
==================
==================
*/
void systems_::lightmap_::buffer_swap(

	void* parameters, __int32 i_thread
) {

	parameters_::lightmap_::buffer_swap_* func_parameters = (parameters_::lightmap_::buffer_swap_*)parameters;
	lightmap_manager_& lightmap_manager = *func_parameters->lightmap_manager;
	model_manager_& model_manager = *func_parameters->model_manager;

	for (__int32 i_thread = 0; i_thread < func_parameters->n_threads; i_thread++) {

		for (__int32 i_active_model = 0; i_active_model < lightmap_manager.active_node_handlers[i_thread].n_active_nodes; i_active_model++) {

			const __int32 i_node = lightmap_manager.active_node_handlers[i_thread].i_node[i_active_model];
			const __int32 i_model_node = lightmap_manager.active_node_handlers[i_thread].i_model_node[i_active_model];
			unsigned __int32 surface_mask = lightmap_manager.model_nodes[i_node][i_model_node].active_surface_mask;
			const __int32 n_bits_set = _mm_popcnt_u32(surface_mask);

			model_& model = model_manager.model[model_::id_::MAP + i_node + 1];

			for (__int32 i_bit = 0; i_bit < n_bits_set; i_bit++) {

				unsigned long i_surface = 0;
				_BitScanForward(&i_surface, surface_mask);
				surface_mask ^= 0x1 << i_surface;
				lightmap_data_& lightmap_data = lightmap_manager.model_nodes[i_node][i_model_node].lightmap_data[i_surface];

				const __int32 n_triangles = 6 * 2;
				const __int32 i_triangle = (i_model_node * n_triangles) + (i_surface * 2);
				model.i_textures[0][i_triangle + 0] = lightmap_data.i_lightmap[lightmap_data.i_read ^ 1];
				model.i_textures[0][i_triangle + 1] = lightmap_data.i_lightmap[lightmap_data.i_read ^ 1];

				lightmap_data.i_read ^= 1;
			}
			lightmap_manager.model_nodes[i_node][i_model_node].active_surface_mask = 0x0;
		}
		lightmap_manager.active_node_handlers[i_thread].n_active_nodes = 0;
	}
}


/*
==================
==================
*/
void Propagate_To_MIP_MAPS(

	const __int32 i_buffer,
	const __int32 i_node,
	const __int32 i_model,
	const __int32 i_surface,
	const lightmap_manager_& lightmap_manager,
	model_& model

) {

	const lightmap_data_& lightmap_data = lightmap_manager.model_nodes[i_node][i_model].lightmap_data[i_surface];
	const __int32 i_lightmap = lightmap_data.i_lightmap[i_buffer];
	const texture_handler_& texture_handler = model.texture_handlers[i_lightmap];

	const __int32 n_mip_writes = texture_handler.n_mip_levels - 1;
	const unsigned __int32 mask = 0xff;
	__int32 height_read = texture_handler.height;
	__int32 width_read = texture_handler.width;

	for (__int32 i_mip_level = 0; i_mip_level < n_mip_writes; i_mip_level++) {

		const unsigned __int32* lexel_read = texture_handler.texture[i_mip_level];
		unsigned __int32* lexel_write = texture_handler.texture[i_mip_level + 1];
		__int32 i_lexel_read = 0;
		__int32 i_lexel_write = 0;
		unsigned __int32 mod_width = width_read - 1;
		unsigned __int32 mod_height = height_read - 1;

		for (__int32 y = 0; y < height_read; y += 2) {
			for (__int32 x = 0; x < width_read; x += 2) {

				__int32 x_step = (x + 1) & mod_width;
				__int32 y_step = (y + 1) & mod_height;

				__int32 i_read[4];
				i_read[0] = x + (y * width_read);
				i_read[1] = x_step + (y * width_read);
				i_read[2] = x + (y_step * width_read);
				i_read[3] = x_step + (y_step * width_read);

				__int32 temp[3];
				temp[R] = 0;
				temp[G] = 0;
				temp[B] = 0;

				for (__int32 i_lexel = 0; i_lexel < 4; i_lexel++) {

					unsigned __int32 colour = lexel_read[i_read[i_lexel]];
					temp[R] += (colour >> 0) & mask;
					temp[G] += (colour >> 8) & mask;
					temp[B] += (colour >> 16) & mask;
				}

				temp[R] >>= 2;
				temp[G] >>= 2;
				temp[B] >>= 2;

				lexel_write[i_lexel_write] = temp[R] | (temp[G] << 8) | (temp[B] << 16);

				i_lexel_read += 2;
				i_lexel_write++;
			}
			i_lexel_read += (width_read * 2);
		}

		height_read >>= 1;
		width_read >>= 1;
	}
}

	

/*
==================
==================
*/
void Process_Lightmap_Bin(
	
	const __int32 i_thread,
	const command_buffer_& command_buffer,
	const lightmap_bin_& lightmap_bin,
	const model_& model_spotlight,
	lightmap_manager_& lightmap_manager,
	model_& model_map

	) {

	const __m128 fall_off_coefficient = set_all(0.4f);
	//const __m128 fall_off_coefficient = set_all(0.005f);
	const __m128i colour_mask = set_all(0xff);
	const __m128 increment = set( 0.0f, 1.0f, 2.0f, 3.0f );
	const __m128i increment_int = set( 0, 1, 2, 3 );
	const __m128i colour_clamp = broadcast(load_s(255));

	const texture_handler_& texture_handler_spotlight = model_spotlight.texture_handlers[0];

	unsigned __int32 surface_mask = 0x0;
	unsigned __int32 light_masks[256];

	const __int32 i_node = lightmap_bin.i_node;

	{
		const __int32 i_model = lightmap_bin.i_model_node;

		for (__int32 i_light_index = 0; i_light_index < lightmap_bin.n_light_sources; i_light_index++) {

			const __int32 i_light = lightmap_bin.i_light_sources[i_light_index];
			light_masks[i_light_index] = 0x0;

			for (__int32 i_surface = 0; i_surface < lightmap_manager_::N_SURFACES_PER_MODEL; i_surface++) {

				const __int32 z_index = axes_index[Z][i_surface];
				const lightmap_data_& lightmap_data = lightmap_manager.model_nodes[i_node][i_model].lightmap_data[i_surface];
				float position = (float)(command_buffer.lightmap_lights[i_light].position.i[z_index]) * r_fixed_scale_real;
				const float d = (position - lightmap_data.origin.f[z_index]) * normal_sign[i_surface];
				const bool is_forward_facing = d >= 0.0f;
				light_masks[i_light_index] |= is_forward_facing << i_surface;
			}
			surface_mask |= light_masks[i_light_index];
		}
		__int32& n_active_nodes = lightmap_manager.active_node_handlers[i_thread].n_active_nodes;
		lightmap_manager.active_node_handlers[i_thread].i_node[n_active_nodes] = i_node;
		lightmap_manager.active_node_handlers[i_thread].i_model_node[n_active_nodes] = i_model;
		lightmap_manager.model_nodes[i_node][i_model].active_surface_mask = surface_mask;
		n_active_nodes++;
	}

	unsigned __int32 surface_mask_copy = surface_mask;
	{
		const __int32 i_model = lightmap_bin.i_model_node;
		const __int32 n_bits_set = _mm_popcnt_u32(surface_mask);

		for (__int32 i_bit = 0; i_bit < n_bits_set; i_bit++) {

			unsigned long i_surface = 0;
			_BitScanForward(&i_surface, surface_mask);
			surface_mask ^= 0x1 << i_surface;
			lightmap_data_& lightmap_data = lightmap_manager.model_nodes[i_node][i_model].lightmap_data[i_surface];
			const __int32 i_lightmap_read = lightmap_data.i_lightmap[lightmap_data.i_read];
			const __int32 i_lightmap_write = lightmap_data.i_lightmap[lightmap_data.i_read ^ 1];

			const texture_handler_& texture_handler_read = model_map.texture_handlers[i_lightmap_read];
			const texture_handler_& texture_handler_write = model_map.texture_handlers[i_lightmap_write];
			const __int32 height = texture_handler_read.height;
			const __int32 width = texture_handler_read.width;
			const unsigned __int32* lexel_read = texture_handler_read.texture[0];
			unsigned __int32* lexel_write = texture_handler_write.texture[0];

			for (__int32 i_lexel = 0; i_lexel < (width * height); i_lexel++) {

				lexel_write[i_lexel] = lexel_read[i_lexel];
				//lexel_write[i_lexel] = 0;
			}
		}
	}


	for (__int32 i_light_index = 0; i_light_index < lightmap_bin.n_light_sources; i_light_index++) {

		const __int32 i_light = lightmap_bin.i_light_sources[i_light_index];
		float3_ position_light;
		position_light.x = (float)(command_buffer.lightmap_lights[i_light].position.x) * r_fixed_scale_real;
		position_light.y = (float)(command_buffer.lightmap_lights[i_light].position.y) * r_fixed_scale_real;
		position_light.z = (float)(command_buffer.lightmap_lights[i_light].position.z) * r_fixed_scale_real;
		float intensity = command_buffer.lightmap_lights[i_light].intensity;
		__m128 colour_light[3];
		colour_light[R] = set_all(command_buffer.lightmap_lights[i_light].colour.r * intensity);
		colour_light[G] = set_all(command_buffer.lightmap_lights[i_light].colour.g * intensity);
		colour_light[B] = set_all(command_buffer.lightmap_lights[i_light].colour.b * intensity);

		const __int32 n_bits_set = _mm_popcnt_u32(light_masks[i_light_index]);

		for (__int32 i_bit = 0; i_bit < n_bits_set; i_bit++) {

			unsigned long i_surface = 0;
			_BitScanForward(&i_surface, light_masks[i_light_index]);
			light_masks[i_light_index] ^= 0x1 << i_surface;
			const __int32 i_model = lightmap_bin.i_model_node;
			const lightmap_data_& lightmap_data = lightmap_manager.model_nodes[i_node][i_model].lightmap_data[i_surface];
			const __int32 i_lightmap = lightmap_data.i_lightmap[lightmap_data.i_read ^ 1];
			const texture_handler_& texture_handler = model_map.texture_handlers[i_lightmap];

			unsigned __int32* texel = texture_handler.texture[0];
			const __int32 height = texture_handler.height;
			const __int32 width = texture_handler.width;

			const __int32 x_index = axes_index[X][i_surface];
			const __int32 y_index = axes_index[Y][i_surface];
			const __int32 z_index = axes_index[Z][i_surface];

			float2_ mapped_position;
			mapped_position.x = (position_light.f[x_index] - lightmap_data.origin.f[x_index]) * lightmap_data.dx_position.f[x_index];
			mapped_position.y = (position_light.f[y_index] - lightmap_data.origin.f[y_index]) * lightmap_data.dy_position.f[y_index];
			float2_ mapped_extent;
			mapped_extent.x = light_extent * lightmap_data.dx_position.f[x_index];
			mapped_extent.y = light_extent * lightmap_data.dy_position.f[y_index];

			float3_ spotlight_mapping;

			spotlight_mapping.x = 1.0f / (1.0f * light_extent);
			spotlight_mapping.y = 1.0f / (1.0f * light_extent);
			spotlight_mapping.z = 1.0f / (1.0f * light_extent);

			__int32 x_min = (__int32)(mapped_position.x - mapped_extent.x);
			__int32 x_max = (__int32)(mapped_position.x + mapped_extent.x);
			__int32 y_min = (__int32)(mapped_position.y - mapped_extent.y);
			__int32 y_max = (__int32)(mapped_position.y + mapped_extent.y);
			__int32 x_left = max(min(x_min, x_max), 0);
			__int32 x_right = min(max(x_min, x_max), width);
			__int32 y_top = max(min(y_min, y_max), 0);
			__int32 y_bottom = min(max(y_min, y_max), height);

			float3_ local_origin = lightmap_data.origin - position_light;
			__int32 i_texel_row = y_top * width;
			const __int32 mip_width = texture_handler_spotlight.width >> 0;

			union rgba_ {

				unsigned __int32 colour;
				unsigned __int8 channels[4];
			};

			float map_scale = (float)mip_width / 2.0f;

			for (__int32 y = y_top; y < y_bottom; y++) {

				for (__int32 x = x_left; x < x_right; x++) {

					float3_ mapped;
					mapped.x = local_origin.f[x_index] + ((float)x * lightmap_data.dx_lightmap.f[x_index]);
					mapped.y = local_origin.f[y_index] + ((float)y * lightmap_data.dy_lightmap.f[y_index]);
					mapped.x *= spotlight_mapping.y;
					mapped.y *= spotlight_mapping.y;

					mapped.z = (abs(local_origin.f[z_index]) * spotlight_mapping.z) + 1.0f;

					mapped.x *= mapped.z;
					mapped.y *= mapped.z;

					mapped.x += 1.0f;
					mapped.y += 1.0f;

					__int32 x_read = __int32(mapped.x * map_scale);
					__int32 y_read = __int32(mapped.y * map_scale);
					x_read = min(x_read, mip_width - 1);
					x_read = max(x_read, 0);
					y_read = min(y_read, mip_width - 1);
					y_read = max(y_read, 0);

					__int32 i_texel = x_read + (y_read * mip_width);

					rgba_ read_source;
					read_source.colour = texture_handler_spotlight.texture[0][i_texel];
					rgba_ read_dest;
					read_dest.colour = texel[i_texel_row + x];
					rgba_ output;

					for (__int32 i_axis = R; i_axis < A; i_axis++) {
						__int32 temp = read_source.channels[i_axis];
						float colour = command_buffer.lightmap_lights[i_light].colour.f[i_axis] * ((float)temp / 255.0f);
						colour *= command_buffer.lightmap_lights[i_light].intensity;
						__int32 colour_int = (__int32)colour;
						colour_int += read_dest.channels[i_axis];
						output.channels[i_axis] = max(colour_int, 0);
						output.channels[i_axis] = min(colour_int, 255);
					}
					texel[i_texel_row + x] = output.colour;
				}
				i_texel_row += width;
			}
		}
	}
	{    
		const __int32 i_model_node = lightmap_bin.i_model_node;

		const __int32 n_bits_set = _mm_popcnt_u32(surface_mask_copy);

		for (__int32 i_bit = 0; i_bit < n_bits_set; i_bit++) {

			unsigned long i_surface = 0;
			_BitScanForward(&i_surface, surface_mask_copy);
			surface_mask_copy ^= 0x1 << i_surface;

			lightmap_data_& lightmap_data = lightmap_manager.model_nodes[i_node][i_model_node].lightmap_data[i_surface];

			Propagate_To_MIP_MAPS(
				
				lightmap_data.i_read ^ 1,
				i_node,
				i_model_node,
				i_surface, 
				lightmap_manager, 
				model_map
			);
		}
	}
}

/*
==================
==================
*/
void systems_::lightmap_::process_BVH(

	void* parameters, __int32 i_thread
)
{
	parameters_::lightmap_::process_BVH_* func_parameters = (parameters_::lightmap_::process_BVH_*)parameters;
	const command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	const grid_& grid = *func_parameters->grid;
	lightmap_manager_& lightmap_manager = *func_parameters->lightmap_manager;

	const command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_read];
	const __int32 light_extent_fixed = (__int32)(light_extent * fixed_scale_real);

	struct valid_data_ {

		__int32 n_entries;
		__int32 i_node[lightmap_manager_::bvh_::MAX_LIGHT_MODELS];
		__int32 i_lit_model[lightmap_manager_::bvh_::MAX_LIGHT_MODELS];
		__int32 i_light_model[lightmap_manager_::bvh_::MAX_LIGHT_MODELS];
	};

	valid_data_ data;
	data.n_entries = 0;

	{
		const __int32 n_light_models = command_buffer.n_lightmap_lights;
		const __m128i indices_inc = set_all(4);

		for (__int32 i_light_model = 0; i_light_model < n_light_models; i_light_model++) {

			__m128i light_centre[4];
			__m128i light_extent[4];
			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				light_centre[i_axis] = set_all(command_buffer.lightmap_lights[i_light_model].position.i[i_axis]);
				light_extent[i_axis] = set_all(light_extent_fixed);
			}

			__int32 n_valid_nodes = 0;
			__int32 i_valid_nodes[grid_::NUM_NODES];

			{
				__m128i indices = set(0, 1, 2, 3);

				for (__int32 i_node_4 = 0; i_node_4 < grid_::NUM_NODES; i_node_4 += 4) {

					__int32 n = min(grid_::NUM_NODES - i_node_4, 4);

					__m128i node_position[4];
					__m128i node_extent[4];
					unsigned __int32 valid_mask = 0x0;
					for (__int32 i_node = 0; i_node < n; i_node++) {

						valid_mask |= 0x1 << i_node;
						node_position[i_node] = load_u(grid.nodes[i_node_4 + i_node].map.centre.i);
						node_extent[i_node] = load_u(grid.nodes[i_node_4 + i_node].map.extent.i);
					}
					Transpose(node_position);
					Transpose(node_extent);

					__m128i result = set_all(-1);
					for (__int32 i_axis = X; i_axis < W; i_axis++) {

						__m128i delta = abs(light_centre[i_axis] - node_position[i_axis]);
						__m128i extent = light_extent[i_axis] + node_extent[i_axis];
						result &= delta < extent;
					}

					unsigned __int32 pack_mask = store_mask(result) & valid_mask;
					__m128i packed_indices = Pack_Vector4(indices, pack_mask);
					store_u(packed_indices, i_valid_nodes + n_valid_nodes);
					n_valid_nodes += __popcnt(pack_mask);
					indices += indices_inc;
				}
			}
			{
				const __m128i i_light_model_4 = set_all(i_light_model);

				for (__int32 i_valid_node = 0; i_valid_node < n_valid_nodes; i_valid_node++) {

					const __int32 i_node = i_valid_nodes[i_valid_node];
					const __int32 n_lit_models = grid.nodes[i_node].map.n_entries;
					const __m128i i_node_4 = set_all(i_node);
					__m128i indices = set(0, 1, 2, 3);

					for (__int32 i_lit_model_4 = 0; i_lit_model_4 < n_lit_models; i_lit_model_4 += 4) {

						__int32 n = min(n_lit_models - i_lit_model_4, 4);

						__m128i lit_centre[4];
						__m128i lit_extent[4];
						unsigned __int32 loop_mask = 0x0;
						for (__int32 i_lit_model = 0; i_lit_model < n; i_lit_model++) {
							__int32 index = grid.nodes[i_node].indices[i_lit_model_4 + i_lit_model];
							loop_mask |= 0x1 << i_lit_model;
							lit_centre[i_lit_model] = load_u(grid.centre[index].i);
							lit_extent[i_lit_model] = load_u(grid.extent[index].i);
						}
						Transpose(lit_centre);
						Transpose(lit_extent);

						__m128i result = set_all(-1);
						for (__int32 i_axis = X; i_axis < W; i_axis++) {
							__m128i delta = abs(lit_centre[i_axis] - light_centre[i_axis]);
							__m128i extent = lit_extent[i_axis] + light_extent[i_axis];
							result &= delta < extent;
						}

						unsigned __int32 pack_mask = store_mask(result) & loop_mask;
						__m128i packed_indices = Pack_Vector4(indices, pack_mask);
						store_u(packed_indices, data.i_lit_model + data.n_entries);
						store_u(i_light_model_4, data.i_light_model + data.n_entries);
						store_u(i_node_4, data.i_node + data.n_entries);
						data.n_entries += __popcnt(pack_mask);

						indices += indices_inc;
					}
				}
			}
		}
	}
	{
		for (__int32 i_sort = 0; i_sort < data.n_entries; i_sort++) {

			__int32 min = INT_MAX;
			__int32 i_min = -1;
			for (__int32 i_iterate = i_sort; i_iterate < data.n_entries; i_iterate++) {
				bool is_less = data.i_lit_model[i_iterate] < min;
				i_min = is_less ? i_iterate : i_min;
				min = is_less ? data.i_lit_model[i_iterate] : min;
			}

			__int32 swap[3];
			swap[0] = data.i_node[i_sort];
			swap[1] = data.i_light_model[i_sort];
			swap[2] = data.i_lit_model[i_sort];

			data.i_node[i_sort] = data.i_node[i_min];
			data.i_light_model[i_sort] = data.i_light_model[i_min];
			data.i_lit_model[i_sort] = data.i_lit_model[i_min];

			data.i_node[i_min] = swap[0];
			data.i_light_model[i_min] = swap[1];
			data.i_lit_model[i_min] = swap[2];
		}
	}
	{

		lightmap_manager_::bvh_& bvh = lightmap_manager.bvh;

		__int32 i_current_lit = -1;
		bvh.n_lit_models = -1;
		__int32 n_light_models = 0;

		for (__int32 i_entry = 0; i_entry < data.n_entries; i_entry++) {

			bool is_next_lit = data.i_lit_model[i_entry] != i_current_lit;
			bvh.n_lit_models += is_next_lit;
			bvh.n_lit_models = min(bvh.n_lit_models, lightmap_manager_::bvh_::MAX_LIT_MODELS);
			i_current_lit = is_next_lit ? data.i_lit_model[i_entry] : i_current_lit;
			bvh.lit_model_data[bvh.n_lit_models].i_start = is_next_lit ? n_light_models : bvh.lit_model_data[bvh.n_lit_models].i_start;
			bvh.lit_model_data[bvh.n_lit_models].n_models = is_next_lit ? 0 : bvh.lit_model_data[bvh.n_lit_models].n_models;

			bvh.lit_model_data[bvh.n_lit_models].i_node = data.i_node[i_entry];
			bvh.lit_model_data[bvh.n_lit_models].i_model = data.i_lit_model[i_entry];
			bvh.i_light_models[n_light_models] = data.i_light_model[i_entry];
			n_light_models++;
			bvh.lit_model_data[bvh.n_lit_models].n_models++;
			n_light_models = min(n_light_models, lightmap_manager_::bvh_::MAX_LIGHT_MODELS);
		}

		//for (__int32 i_lit_model = 0; i_lit_model < bvh.n_lit_models; i_lit_model++) {

		//	printf_s(" %i | ", bvh.i_lit_models[i_lit_model]);
		//	printf_s(" %i : ", bvh.i_lit_model_nodes[i_lit_model]);

		//	for (__int32 i_light_model = 0; i_light_model < bvh.n_light_models[i_lit_model]; i_light_model++) {
		//		printf_s(" %i ,", bvh.i_light_models[i_lit_model][i_light_model]);
		//	}

		//	printf_s(" \n");
		//}
		//printf_s(" \n");
	}
}

/*
==================
==================
*/
void systems_::lightmap_::process_lightmaps(

	void* parameters, __int32 i_thread
)
{
	parameters_::lightmap_::process_lightmaps_* func_parameters = (parameters_::lightmap_::process_lightmaps_*)parameters;
	const __int32 i_lit_model_index = func_parameters->i_lit_model_index;
	const command_buffer_handler_& command_buffer_handler = *func_parameters->command_buffer_handler;
	lightmap_manager_& lightmap_manager = *func_parameters->lightmap_manager;
	model_manager_& model_manager = *func_parameters->model_manager;
	model_& model_spotlight = *func_parameters->model_spotlight;

	const command_buffer_& command_buffer = command_buffer_handler.command_buffers[command_buffer_handler.i_read];
	lightmap_manager_::bvh_& bvh = lightmap_manager.bvh;

	const __int32 n_light_models = bvh.lit_model_data[i_lit_model_index].n_models;

	if (n_light_models == 0) {
		return;
	}

	const __int32 i_node = bvh.lit_model_data[i_lit_model_index].i_node;
	const __int32 i_lit_model = bvh.lit_model_data[i_lit_model_index].i_model;

	model_& model = model_manager.model[model_::id_::MAP + i_node + 1];

	lightmap_bin_ lightmap_bin;
	lightmap_bin.i_node = i_node;
	lightmap_bin.i_model_node = i_lit_model;
	lightmap_bin.n_light_sources = 0;

	const __int32 i_start = bvh.lit_model_data[i_lit_model_index].i_start;
	const __int32 i_end = i_start + n_light_models;

	for (__int32 i_light_model_index = i_start; i_light_model_index < i_end; i_light_model_index++) {
		lightmap_bin.i_light_sources[lightmap_bin.n_light_sources] = bvh.i_light_models[i_light_model_index];
		lightmap_bin.n_light_sources++;
	}

	Process_Lightmap_Bin(

		i_thread,
		command_buffer,
		lightmap_bin,
		model_spotlight,
		lightmap_manager,
		model
	);

	bvh.lit_model_data[i_lit_model_index].n_models = 0;
}

/*
==================
==================
*/
void systems_::lightmap_::fade_lightmaps(

	void* parameters, __int32 i_thread

){

	return;

	parameters_::lightmap_::fade_lightmaps_* func_parameters = (parameters_::lightmap_::fade_lightmaps_*)parameters;
	const timer_& timer = *func_parameters->timer;
	lightmap_manager_& lightmap_manager = *func_parameters->lightmap_manager;
	model_& model_map = *func_parameters->model_map;

	const __int32 i_model_node = timer.frame_count % lightmap_manager.n_model_nodes[0];
	lightmap_manager_::model_node_& model_node = lightmap_manager.model_nodes[0][i_model_node];
	bool is_node_active = model_node.active_surface_mask != 0x0;
	__int32& n_active_nodes = lightmap_manager.active_node_handlers[i_thread].n_active_nodes;
	lightmap_manager.active_node_handlers[i_thread].i_model_node[n_active_nodes] = i_model_node;
	n_active_nodes += is_node_active;

	const __int32 reduce = 10;
	unsigned __int8 vector[16];
	for (__int32 i = 0; i < 16; i++) {
		vector[i] = reduce;
	}
	__m128i test = load_u((unsigned __int32*)vector);

	{
		const __m128i colour_reduce = broadcast(load_s(2));
		const __m128i bit_mask = broadcast(load_s(0xff));
		const __m128i zero = set_zero_si128();

		for (__int32 i_surface = 0; i_surface < lightmap_manager_::N_SURFACES_PER_MODEL; i_surface++) {

			lightmap_data_& lightmap_data = model_node.lightmap_data[i_surface];
			bool is_active = (model_node.active_surface_mask & (0x1 << i_surface)) != 0x0;
			const __int32 i_read = is_active ? lightmap_data.i_read ^ 1 : lightmap_data.i_read;
			const __int32 i_lightmap_read = lightmap_data.i_lightmap[i_read];
			const __int32 i_lightmap_write = lightmap_data.i_lightmap[lightmap_data.i_read ^ 1];
			const texture_handler_& texture_handler_read = model_map.texture_handlers[i_lightmap_read];
			const texture_handler_& texture_handler_write = model_map.texture_handlers[i_lightmap_write];

			const __int32 width_lightmap = texture_handler_read.width;
			const __int32 height_lightmap = texture_handler_read.height;
			unsigned __int32* lexel_read = texture_handler_read.texture[0];
			unsigned __int32* lexel_write = texture_handler_write.texture[0];

			__int32 index = 0;
			for (__int32 y = 0; y < height_lightmap; y++) {

				for (__int32 x = 0; x < width_lightmap; x += 4) {

					__m128i lexel = load_u(&lexel_read[index]);
					lexel = _mm_subs_epu8(lexel, test);
					store_u(lexel, &lexel_write[index]);

					index += 4;
				}
			}
			model_node.active_surface_mask |= 0x1 << i_surface;
		}
	}
	{
		for (__int32 i_surface = 0; i_surface < lightmap_manager_::N_SURFACES_PER_MODEL; i_surface++) {

			const lightmap_data_& lightmap_data = model_node.lightmap_data[i_surface];

			Propagate_To_MIP_MAPS(

				lightmap_data.i_read ^ 1,
				0,
				i_model_node,
				i_surface,
				lightmap_manager,
				model_map
			);
		}
	}
}

/*
==================
==================
*/
void Initialise_Light_Maps(

	const lightmap_manager_& lightmap_manager,
	model_manager_& model_manager

	) {

	const unsigned __int32 clear_colour = 100 << 8;

	const __int32 colours[] = {

		20 << 16 | 10 << 8 | 10 << 0,
		10 << 16 | 20 << 8 | 10 << 0,
		10 << 16 | 10 << 8 | 20 << 0,
	};
	const __int32 n_colours = sizeof(colours) / sizeof(colours[0]);

	const model_& model_texture = model_manager.model[model_::id_::CUBE];
	texture_handler_& read_texture = model_texture.texture_handlers[0];

	for (__int32 i_node = 0; i_node < grid_::NUM_NODES; i_node++) {

		model_& model = model_manager.model[model_::id_::MAP + i_node + 1];

		for (__int32 i_model = 0; i_model < lightmap_manager.n_model_nodes[i_node]; i_model++) {

			for (__int32 i_surface = 0; i_surface < lightmap_manager_::N_SURFACES_PER_MODEL; i_surface++) {

			const lightmap_data_& lightmap_data = lightmap_manager.model_nodes[i_node][i_model].lightmap_data[i_surface];
			texture_handler_& texture_handler = model.texture_handlers[lightmap_data.i_lightmap[lightmap_data.i_read]];

				__int32 width_lightmap = texture_handler.width;
				__int32 height_lightmap = texture_handler.height;

				for (__int32 i_mip_level = 0; i_mip_level < texture_handler.n_mip_levels; i_mip_level++) {

					__int32 index = 0;
					__int32 index_read = 0;
					for (__int32 y = 0; y < height_lightmap; y++) {

						for (__int32 x = 0; x < width_lightmap; x++) {

							//texture_handler.texture[i_mip_level][index] = clear_colour;
							__int32 width_read = x % read_texture.width;
							__int32 height_read = y % read_texture.height;
							texture_handler.texture[i_mip_level][index] = read_texture.texture[0][(height_read * read_texture.width) + width_read] + colours[i_model % n_colours];
							//texture_handler.texture[i_mip_level][index] = read_texture.texture[0][index_read] + colours[i_model % n_colours];
							index++;
							index_read++;
							index_read %= (read_texture.width * read_texture.height);
						}
					}

					width_lightmap >>= 1;
					height_lightmap >>= 1;
				}
			}
		}
	}
}

/*
==================
==================
*/
void Compute_Light_Map_Gradients(

	const model_token_manager_& model_token_manager,
	const model_& model_cube,
	lightmap_manager_& lightmap_manager,
	model_manager_& model_manager

	)
{
	for (__int32 i_node = 0; i_node < grid_::NUM_NODES; i_node++) {

		const model_token_& model_token = model_token_manager.model_tokens[model_token_::id_::MAP + i_node + 1];
		const model_& model = model_manager.model[model_::id_::MAP + i_node + 1];
		const int3_ node_position = lightmap_manager.grid_TEMP->nodes[i_node].map.centre;

		for (__int32 i_model = 0; i_model < model_token.n_models; i_model++) {

			float3_ vertices[8];
			for (__int32 i_vertex = 0; i_vertex < model_cube.n_vertices; i_vertex++) {
				for (__int32 i_axis = X; i_axis < W; i_axis++) {
					__int32 world_position = model_token.centre[i_model].i[i_axis] + node_position.i[i_axis];
					float centre = (float)world_position * r_fixed_scale_real;
					float extent = (float)model_token.extent[i_model].i[i_axis] * r_fixed_scale_real;
					vertices[i_vertex].f[i_axis] = (model_cube.vertices_frame[0][i_vertex].f[i_axis] * extent) + centre;
				}
			}

			__int32 i_index = 0;

			for (__int32 i_surface = 0; i_surface < lightmap_manager_::N_SURFACES_PER_MODEL; i_surface++) {

				__int32 vertex_index[3] = { 5, 4, 3 };
				float3_ surface_vertices[3];
				for (__int32 i_vertex = 0; i_vertex < 3; i_vertex++) {

					__int32 index = model_cube.i_vertices[i_index + vertex_index[i_vertex]];
					surface_vertices[i_vertex] = vertices[index];
				}

				lightmap_data_& lightmap_data = lightmap_manager.model_nodes[i_node][i_model].lightmap_data[i_surface];

				const __int32 i_lightmap = lightmap_data.i_lightmap[lightmap_data.i_read];
				texture_handler_& texture_handler = model.texture_handlers[i_lightmap];
				const float width = (float)texture_handler.width;
				const float height = (float)texture_handler.height;

				for (__int32 i = X; i < W; i++) {
					lightmap_data.dx_lightmap.f[i] = (surface_vertices[1].f[i] - surface_vertices[0].f[i]) / width;
					lightmap_data.dy_lightmap.f[i] = (surface_vertices[2].f[i] - surface_vertices[0].f[i]) / height;
					lightmap_data.origin.f[i] = surface_vertices[0].f[i] + (lightmap_data.dx_lightmap.f[i] * 0.5f) + (lightmap_data.dy_lightmap.f[i] * 0.5f);

					lightmap_data.dx_position.f[i] = width / (surface_vertices[1].f[i] - surface_vertices[0].f[i]);
					lightmap_data.dy_position.f[i] = height / (surface_vertices[2].f[i] - surface_vertices[0].f[i]);
				}

				i_index += 6;
			}
		}
	}
}
