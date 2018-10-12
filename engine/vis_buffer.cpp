#include "vis_buffer.h"
#include "render_back.h"
#include "render_front.h"
#include "command.h"
#include "raster.h"
#include "shader.h"
#include "model.h"
#include "texture.h"

/*
==================
==================
*/
void Populate_Visibility_Buffer(

	const __int32 bin_x,
	const __int32 bin_y,
	const __int32 n_threads,
	int2_ bin_origin,
	display_& display,
	raster_output_& raster_output,
	shader_input_& shader_input
)
{
	for (__int32 i_thread_entry = 0; i_thread_entry < n_threads; i_thread_entry++) {

		screen_bin_& screen_bin = display.screen_bin[i_thread_entry][bin_y][bin_x];
		const bin_triangle_* bin_triangle = screen_bin.bin_triangle;
		const bin_triangle_data_* bin_triangle_data = screen_bin.bin_triangle_data;

		const __int32 n_triangles = screen_bin.n_triangles;

		for (__int32 i_triangle_4 = 0; i_triangle_4 < n_triangles; i_triangle_4 += 4) {

			__int32 n_loop_triangles = min(n_triangles - i_triangle_4, 4);

			__int32 draw_id[4];
			float3_ position[3][4];
			__m128 position_4[3][4];
			for (__int32 i_vertex = 0; i_vertex < 3; i_vertex++) {
				for (__int32 i_triangle = 0; i_triangle < n_loop_triangles; i_triangle++) {

					draw_id[i_triangle] = bin_triangle_data[i_triangle_4 + i_triangle].draw_id;
					position[i_vertex][i_triangle] = bin_triangle[i_triangle_4 + i_triangle].position[i_vertex];
					position_4[i_vertex][i_triangle] = load_u(bin_triangle[i_triangle_4 + i_triangle].position[i_vertex].f);
				}
				Transpose(position_4[i_vertex]);
			}

			__m128 area =
				((position_4[1][X] - position_4[0][X]) * (position_4[2][Y] - position_4[0][Y])) -
				((position_4[1][Y] - position_4[0][Y]) * (position_4[2][X] - position_4[0][X]));
			__m128 r_area = reciprocal(area);
			store_u(r_area, shader_input.r_area);

			__m128 depth_delta[3];
			depth_delta[0] = position_4[2][Z] - position_4[0][Z];
			depth_delta[1] = position_4[1][Z] - position_4[0][Z];
			depth_delta[2] = position_4[0][Z];

			__m128i draw_id_4 = load_u(draw_id);
			__m128i is_depth_write = draw_id_4 != set_all(draw_call_::id_::SKYBOX);

			depth_delta[0] &= is_depth_write;
			depth_delta[1] &= is_depth_write;
			depth_delta[2] &= is_depth_write;

			store_u(depth_delta[0], shader_input.z_delta[0]);
			store_u(depth_delta[1], shader_input.z_delta[1]);
			store_u(depth_delta[2], shader_input.z_delta[2]);

			for (__int32 i_triangle = 0; i_triangle < n_loop_triangles; i_triangle++) {

				__int32 i_current_triangle = i_triangle_4 + i_triangle;
				const __int32 triangle_id = (i_current_triangle << 16) | screen_bin.bin_triangle_data[i_current_triangle].draw_id;

				raster_data_ raster_data;
				raster_data.tile_offset.x = display_::TILE_SIZE;
				raster_data.tile_offset.y = display_::TILE_SIZE;

				int2_ bin_centre;
				bin_centre.x = bin_origin.x + raster_data.tile_offset.x;
				bin_centre.y = bin_origin.y + raster_data.tile_offset.y;

				Raster_Setup(

					i_triangle,
					position,
					bin_centre,
					raster_data
				);

				Rasteriser(raster_data, raster_output);

				shader_input.triangle_id = triangle_id;
				shader_input.i_triangle = i_triangle;

				Process_Fragments(raster_output, shader_input);
			}
		}

		screen_bin.n_triangles = 0;
	}
}

/*
==================
==================
*/
void Process_Visibility_Buffer(

	const __int32 i_thread,
	display_& display,
	visibility_data_& visibility_data
)
{

	for (__int32 i_draw_call = 0; i_draw_call < draw_call_::id_::COUNT; i_draw_call++) {

		visibility_data.hash_maps[i_draw_call].draw_id = i_draw_call;
		visibility_data.hash_maps[i_draw_call].n_triangles = 0;
		for (__int32 i_bucket = 0; i_bucket < visibility_data_::NUM_BUCKETS; i_bucket++) {
			visibility_data.hash_maps[i_draw_call].buckets[i_bucket].n_triangles = 0;
		}
	}

	const __int32 cache_size = 8;
	const unsigned __int32 cache_mod = cache_size - 1;
	__int32 id_cache[cache_size];
	unsigned __int32 i_cache_write = 0;
	for (__int32 i_cache_id = 0; i_cache_id < cache_size; i_cache_id++) {
		id_cache[i_cache_id] = -1;
	}

	__int32 i_read = 0;
	for (__int32 i_row = 0; i_row < display_::BIN_SIZE; i_row++) {

		// record pixel ids at boundaries
		__int32 current_id = -1;
		__int32 pixel_ids[display_::BIN_SIZE];
		__int32 n_pixel_ids = 0;
		for (__int32 i_pixel = 0; i_pixel < display_::BIN_SIZE; i_pixel++) {
			__int32 pixel_id = display.index_buffer_bin[i_thread][i_read];
			bool is_new_id = current_id != pixel_id;
			pixel_ids[n_pixel_ids] = pixel_id;
			n_pixel_ids += is_new_id;
			current_id = blend_int(pixel_id, current_id, is_new_id);
			i_read++;
		}

		// make pixel id's unique
		__int32 n_unique_ids = 0;
		for (__int32 i_pixel_id = 0; i_pixel_id < n_pixel_ids; i_pixel_id++) {

			__int32 pixel_id = pixel_ids[i_pixel_id];
			__int32 inc = 1;
			for (__int32 i_unique_id = 0; i_unique_id < n_unique_ids; i_unique_id++) {
				inc &= pixel_id != pixel_ids[i_unique_id];
			}
			pixel_ids[n_unique_ids] = pixel_id;
			inc &= pixel_id != -1;		// catch degenerates
			n_unique_ids += inc;
		}

		// check cache
		__int32 n_uncached_ids = 0;
		for (__int32 i_pixel_id = 0; i_pixel_id < n_unique_ids; i_pixel_id++) {

			__int32 pixel_id = pixel_ids[i_pixel_id];
			__int32 inc = 1;
			for (__int32 i_cache_id = 0; i_cache_id < cache_size; i_cache_id++) {
				inc &= pixel_id != id_cache[i_cache_id];
			}
			pixel_ids[n_uncached_ids] = pixel_id;
			n_uncached_ids += inc;
		}

		// bin
		const static __m128i loop_var_static = set(0, 1, 2, 3);
		const __m128i loop_inc = set_all(4);
		const __m128i all_bits_set = set_true();
		__int32 n_binned_ids = 0;

		for (__int32 i_pixel_id = 0; i_pixel_id < n_uncached_ids; i_pixel_id++) {

			__int32 pixel_id = pixel_ids[i_pixel_id];
			__int32 draw_id = pixel_id & 0xffff;
			__int32 triangle_id = pixel_id >> 16;
			//__int32 inc = 1;
			__int32 hash_key = triangle_id / visibility_data_::NUM_ENTRIES;

			visibility_data_::hash_map_& hash_map = visibility_data.hash_maps[draw_id];
			visibility_data_::bucket_& bucket = hash_map.buckets[hash_key];

			__m128i pixel_id_4 = set_all(pixel_id);
			__m128i check_mask = set_true();
			__m128i loop_var = loop_var_static;
			__m128i loop_condition = set_all(bucket.n_triangles);

			for (__int32 i_triangle = 0; i_triangle < bucket.n_triangles; i_triangle += 4) {

				__m128i compare_mask = pixel_id_4 != load_u(bucket.triangle_ids + i_triangle);
				__m128i condition_mask = loop_var < loop_condition;
				compare_mask = blend(compare_mask, all_bits_set, condition_mask);
				check_mask &= compare_mask;
				loop_var += loop_inc;
			}
			bucket.triangle_ids[bucket.n_triangles] = pixel_id;
			pixel_ids[n_binned_ids] = pixel_id;
			bool is_new = store_mask(check_mask) == 0xf;
			n_binned_ids += is_new;
			bucket.n_triangles += is_new;
			hash_map.n_triangles += is_new;
		}

		// update cache
		for (__int32 i_cache_id = 0; i_cache_id < n_binned_ids; i_cache_id++) {

			id_cache[i_cache_write] = pixel_ids[i_cache_id];
			i_cache_write = (i_cache_write + 1) & cache_mod;
		}
	}

	visibility_data.n_active_hash_maps = 0;
	for (__int32 i_draw_id = 0; i_draw_id < draw_call_::id_::COUNT; i_draw_id++) {

		visibility_data.active_hash_maps[visibility_data.n_active_hash_maps] = &visibility_data.hash_maps[i_draw_id];
		visibility_data.n_active_hash_maps += visibility_data.hash_maps[i_draw_id].n_triangles > 0;
	}
}

/*
==================
==================
*/
void Render_Visible_Triangles(

	const __int32 bin_x,
	const __int32 bin_y,
	const int2_ bin_origin,
	const command_buffer_& command_buffer,
	const display_& display,
	raster_output_& raster_output,
	shader_input_& shader_input,
	visibility_data_& visibility_data

)
{
	for (__int32 i_draw_id = 0; i_draw_id < visibility_data.n_active_hash_maps; i_draw_id++) {

		visibility_data_::hash_map_& current_hash_map = *visibility_data.active_hash_maps[i_draw_id];
		visibility_data.n_active_buckets = 0;
		for (__int32 i_bucket = 0; i_bucket < visibility_data_::NUM_BUCKETS; i_bucket++) {

			visibility_data.active_buckets[visibility_data.n_active_buckets] = &current_hash_map.buckets[i_bucket];
			visibility_data.n_active_buckets += current_hash_map.buckets[i_bucket].n_triangles > 0;
		}

		__int32 draw_id = current_hash_map.draw_id;
		const draw_call_& draw_call = command_buffer.draw_calls[draw_id];
		const screen_bin_& screen_bin = display.screen_bin[draw_call.i_thread][bin_y][bin_x];
		const bin_triangle_* bin_triangle = screen_bin.bin_triangle;
		shader_input.mip_level_bias = draw_call.mip_level_bias;

		__int32 i_bucket = 0;
		__int32 i_triangle_bucket = 0;
		for (__int32 i_triangle_4 = 0; i_triangle_4 < current_hash_map.n_triangles; i_triangle_4 += 4) {

			const __int32 n = min(current_hash_map.n_triangles - i_triangle_4, 4);

			__int32 packed_ids[4];
			bin_triangle_data_ bin_triangle_data[4];
			for (__int32 i_triangle = 0; i_triangle < n; i_triangle++) {

				packed_ids[i_triangle] = visibility_data.active_buckets[i_bucket]->triangle_ids[i_triangle_bucket];
				const __int32 i_triangle_bin = packed_ids[i_triangle] >> 16;
				bin_triangle_data[i_triangle] = screen_bin.bin_triangle_data[i_triangle_bin];

				i_triangle_bucket++;
				bool is_next_bucket = i_triangle_bucket == visibility_data.active_buckets[i_bucket]->n_triangles;
				i_bucket += is_next_bucket;
				i_triangle_bucket = blend_int(0, i_triangle_bucket, is_next_bucket);
			}

			float4_ vertices[MAX_VERTEX_ATTRIBUTES][4][3];
			static const float4_ clear = { 0.0f, 0.0f, 0.0f, 1.0f };

			for (__int32 i_triangle = 0; i_triangle < n; i_triangle++) {

				const __int32 i_index_model = bin_triangle_data[i_triangle].i_triangle * 3;
				const __int32 i_model = bin_triangle_data[i_triangle].i_model;
				const __int32 model_id = draw_call.model_id[i_model];
				const model_& model = command_buffer.model[model_id];

				for (__int32 i_vertex = 0; i_vertex < 3; i_vertex++) {

					//for (__int32 i_attribute = 0; i_attribute < draw_call.n_attributes; i_attribute++) {
					for (__int32 i_attribute = 0; i_attribute < FIRST_ATTRIBUTE + 1; i_attribute++) {

						__int32 stride = draw_call.attribute_streams[i_attribute].stride;
						__int32 i_attribute_read = model.i_attribute_vertices[i_attribute][i_index_model + i_vertex];

						vertices[FIRST_ATTRIBUTE + i_attribute][i_triangle][i_vertex] = clear;
						for (__int32 i_axis = X; i_axis < stride; i_axis++) {
							vertices[FIRST_ATTRIBUTE + i_attribute][i_triangle][i_vertex].f[i_axis] = model.attribute_vertices[i_attribute][i_attribute_read].f[i_axis];
						}
					}
				}
			}



			float3_ positions_rasteriser[3][4];
			float3_ barycentric[3][4];
			{
				__m128 position_4[3][4];

				for (__int32 i_vertex = 0; i_vertex < 3; i_vertex++) {
					for (__int32 i_triangle = 0; i_triangle < n; i_triangle++) {

						const __int32 i_triangle_bin = packed_ids[i_triangle] >> 16;

						vertices[VERTEX_POSITION][i_triangle][i_vertex].x = bin_triangle[i_triangle_bin].position[i_vertex].x;
						vertices[VERTEX_POSITION][i_triangle][i_vertex].y = bin_triangle[i_triangle_bin].position[i_vertex].y;
						vertices[VERTEX_POSITION][i_triangle][i_vertex].z = bin_triangle[i_triangle_bin].position[i_vertex].z;

						positions_rasteriser[i_vertex][i_triangle] = bin_triangle[i_triangle_bin].position[i_vertex];
						barycentric[i_vertex][i_triangle] = bin_triangle[i_triangle_bin].barycentric[i_vertex];
						position_4[i_vertex][i_triangle] = load_u(bin_triangle[i_triangle_bin].position[i_vertex].f);
					}
					Transpose(position_4[i_vertex]);
				}

				__m128 area =
					((position_4[1][X] - position_4[0][X]) * (position_4[2][Y] - position_4[0][Y])) -
					((position_4[1][Y] - position_4[0][Y]) * (position_4[2][X] - position_4[0][X]));
				__m128 r_area = reciprocal(area);
				store_u(r_area, shader_input.r_area);

				store_u(position_4[2][Z] - position_4[0][Z], shader_input.z_delta[0]);
				store_u(position_4[1][Z] - position_4[0][Z], shader_input.z_delta[1]);
				store_u(position_4[0][Z], shader_input.z_delta[2]);
			}

			// attribute shading
			for (__int32 i_attribute = 0; i_attribute < draw_call.n_attributes; i_attribute++) {

				if (draw_call.attribute_streams[i_attribute].vertex_shader) {

					draw_call.attribute_streams[i_attribute].vertex_shader(
						n,
						command_buffer,
						command_buffer.draw_calls[draw_id],
						bin_triangle_data,
						vertices[FIRST_ATTRIBUTE + i_attribute]
					);
				}
			}

			draw_call.lighting_function(
				n,
				command_buffer.vertex_light_manager,
				vertices[VERTEX_POSITION],
				vertices[FIRST_ATTRIBUTE + 0]
			);

			for (__int32 i_triangle = 0; i_triangle < n; i_triangle++) {

				raster_data_ raster_data;
				raster_data.tile_offset.x = display_::TILE_SIZE;
				raster_data.tile_offset.y = display_::TILE_SIZE;

				int2_ bin_centre;
				bin_centre.x = bin_origin.x + raster_data.tile_offset.x;
				bin_centre.y = bin_origin.y + raster_data.tile_offset.y;

				Raster_Setup(

					i_triangle,
					positions_rasteriser,
					bin_centre,
					raster_data
				);

				Rasteriser(raster_data, raster_output);

				//const __int32 i_triangle_bin = packed_ids[i_triangle] >> 16;

				//for (__int32 i_interpolant = 0; i_interpolant < draw_call.n_attributes; i_interpolant++) {
				for (__int32 i_interpolant = 0; i_interpolant < 2; i_interpolant++) {

					const __int32 i_index_model = bin_triangle_data[i_triangle].i_triangle * 3;
					const __int32 i_model = bin_triangle_data[i_triangle].i_model;
					const __int32 model_id = draw_call.model_id[i_model];
					const model_& model = command_buffer.model[model_id];

					//bool is_texture = draw_call.attribute_streams[i_interpolant].id == draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
					bool is_texture = i_interpolant == 1;

					if (is_texture) {
						__int32 i_triangle_model = bin_triangle_data[i_triangle].i_triangle;
						__int32 i_texture = model.i_textures[i_interpolant - 1][i_triangle_model];
						//shader_input.blend_interval = draw_call.blend_interval[i_model];
						//shader_input.texture_handlers[i_interpolant] = &model.texture_handlers[i_texture];
					}


					for (__int32 i_component = 0; i_component < draw_call.attribute_streams[i_interpolant].stride; i_component++) {

						float v0 = vertices[FIRST_ATTRIBUTE + i_interpolant][i_triangle][0].f[i_component];
						float v1 = vertices[FIRST_ATTRIBUTE + i_interpolant][i_triangle][1].f[i_component];
						float v2 = vertices[FIRST_ATTRIBUTE + i_interpolant][i_triangle][2].f[i_component];

						shader_input.gradients[i_interpolant][i_component].x = set_all(v2 - v0);
						shader_input.gradients[i_interpolant][i_component].y = set_all(v1 - v0);
						shader_input.gradients[i_interpolant][i_component].z = set_all(v0);
					}
				}

				float3_ b0 = barycentric[0][i_triangle];
				float3_ b1 = barycentric[1][i_triangle];
				float3_ b2 = barycentric[2][i_triangle];

				shader_input.barycentric[1][X] = set_all(b2.y - b0.y);
				shader_input.barycentric[1][Y] = set_all(b1.y - b0.y);
				shader_input.barycentric[1][Z] = set_all(b0.y);

				shader_input.barycentric[0][X] = set_all(b2.z - b0.z);
				shader_input.barycentric[0][Y] = set_all(b1.z - b0.z);
				shader_input.barycentric[0][Z] = set_all(b0.z);

				shader_input.triangle_id = packed_ids[i_triangle];
				shader_input.i_triangle = i_triangle;
				shader_input.draw_call = &draw_call;

				//shader_DISPATCH_PRE(raster_output, shader_input);
			}
		}
	}
}