
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
void Clear_Tile_Buffer(

	const __int32 i_thread,
	display_& display

) {
	//const __m128 zero_depth = set_all(FLT_MAX);
	const __m128 zero_depth = set_all(-1.0f);
	const __m128i zero_colour = set_all_bits();

	for (__int32 i = 0; i < display_::BIN_SIZE * display_::BIN_SIZE; i += 4) {

		store(zero_depth, display.depth_buffer_bin[i_thread] + i);
		store(zero_colour, display.colour_buffer_bin[i_thread] + i);
		store(zero_colour, display.index_buffer_bin[i_thread] + i);
	}
}

/*
==================
==================
*/
void Write_Frame_Buffer_UPSCALE(

	const __int32 i_thread,
	const int2_ tile_origin,
	display_& display

) {
	int2_ tile_origin_new;
	tile_origin_new.x = tile_origin.x * 2;
	tile_origin_new.y = tile_origin.y * 2;
	const __int32 write_pitch = display.locked_rect.Pitch / 4;
	unsigned __int32* write_buffer = (unsigned __int32*)display.locked_rect.pBits;
	__int32 i_write_y = (tile_origin_new.y * write_pitch) + tile_origin_new.x;

	const unsigned __int32 x_mask = 0x33333333;
	const unsigned __int32 y_mask = ~x_mask;
	const unsigned __int32 x_inc = y_mask | 0x1;
	const unsigned __int32 y_inc = x_mask | 0x4;

	__int32 y_morton = 0;

	for (__int32 y_dest = 0; y_dest < display_::BIN_SIZE; y_dest++) {

		__int32 x_morton = 0;

		for (__int32 x_dest = 0; x_dest < display_::BIN_SIZE; x_dest++) {

			__int32 i_read = x_morton | y_morton;
			__int32 x_dest_new = x_dest * 2;
			unsigned __int32 colour = display.colour_buffer_bin[i_thread][i_read];
			write_buffer[i_write_y + x_dest_new] = colour;
			write_buffer[i_write_y + x_dest_new + 1] = colour;
			write_buffer[i_write_y + write_pitch + x_dest_new] = colour;
			write_buffer[i_write_y + write_pitch + x_dest_new + 1] = colour;
			x_morton = (x_morton + x_inc) & x_mask;
		}
		y_morton = (y_morton + y_inc) & y_mask;
		i_write_y += write_pitch * 2;
	}
}

/*
==================
==================
*/
void Write_Frame_Buffer(

	const __int32 i_thread,
	const int2_ tile_origin,
	display_& display

) {
	const __int32 write_pitch = display.locked_rect.Pitch / 4;
	unsigned __int32* write_buffer = (unsigned __int32*)display.locked_rect.pBits;

	const unsigned __int32 x_mask = 0x33333333;
	const unsigned __int32 y_mask = ~x_mask;
	const unsigned __int32 x_inc = y_mask | 0x1;
	const unsigned __int32 y_inc = x_mask | 0x4;

	__int32 i_tile = 0;

	for (__int32 y_tile = 0; y_tile < display_::BIN_SIZE; y_tile += display_::TILE_SIZE) {
		for (__int32 x_tile = 0; x_tile < display_::BIN_SIZE; x_tile += display_::TILE_SIZE) {

			__int32 i_write_y = ((tile_origin.y + y_tile) * write_pitch) + (tile_origin.x + x_tile);
			__int32 y_morton = 0;

			for (__int32 y_dest = 0; y_dest < display_::TILE_SIZE; y_dest++) {

				__int32 x_morton = 0;

				for (__int32 x_dest = 0; x_dest < display_::TILE_SIZE; x_dest++) {

					__int32 i_read = x_morton | y_morton;
					write_buffer[i_write_y + x_dest] = display.colour_buffer_bin[i_thread][i_tile + i_read];
					x_morton = (x_morton + x_inc) & x_mask;
				}
				y_morton = (y_morton + y_inc) & y_mask;
				i_write_y += write_pitch;
			}
			i_tile += (display_::TILE_SIZE * display_::TILE_SIZE);
		}
	}

}

/*
==================
==================
*/
void Depth_Fog(

	const __int32 x_bin,
	const __int32 y_bin,
	const __int32 i_thread,
	const command_buffer_& command_buffer,
	display_& display

) {


	const __m128i bit_mask = set_all(0xff);
	const __m128 fall_off_coeff = set_all(1.0f / 80000.0f);

	float timer = command_buffer.fog_effect_timer;
	__m128 timer_4 = set_all(timer);

	static const float max_fog = 3000.0f;
	static const float min_fog = 100.0f;
	float distance = min_fog + (timer * (max_fog - min_fog));
	__m128 fog_distance = set_all(distance);

	const __m128 zero = set_all(0.0f);
	const __m128 one = set_all(1.0f);

	static const float3_ colours[2] = {

	//{ 20.0f, 100.0, 100.0f },
	{200.0f, 200.0, 200.0f },
	{80.0f, 40.0, 40.0f},
	};

	float3_ current_colour = colours[0] + ((colours[1] - colours[0]) * timer);
	__m128 fog_colour[3];
	fog_colour[R] = set_all(current_colour.r);
	fog_colour[G] = set_all(current_colour.g);
	fog_colour[B] = set_all(current_colour.b);

	for (__int32 i = 0; i < display_::BIN_SIZE * display_::BIN_SIZE; i += 4) {

		__m128 depth = load(display.depth_buffer_bin[i_thread] + i);
		__m128i colour = load(display.colour_buffer_bin[i_thread] + i);

		__m128 components[3];
		components[R] = convert_float((colour >> 16) & bit_mask);
		components[G] = convert_float((colour >> 8) & bit_mask);
		components[B] = convert_float(colour & bit_mask);

		__m128 linear_depth;
		linear_depth = reciprocal(depth);
		linear_depth -= fog_distance;
		linear_depth = max_vec(linear_depth, zero);
		__m128 depth_scale = linear_depth * linear_depth * fall_off_coeff;
		depth_scale = max_vec(min_vec(depth_scale, one), zero);

		components[R] = components[R] + (depth_scale * (fog_colour[R] - components[R]));
		components[G] = components[G] + (depth_scale * (fog_colour[G] - components[G]));
		components[B] = components[B] + (depth_scale * (fog_colour[B] - components[B]));

		__m128i new_colour = convert_int_trunc(components[B]) | (convert_int_trunc(components[G]) << 8) | (convert_int_trunc(components[R]) << 16);
		store(new_colour, display.colour_buffer_bin[i_thread] + i);
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

					for (__int32 i_attribute = 0; i_attribute < draw_call.n_attributes; i_attribute++) {

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

				for (__int32 i_interpolant = 0; i_interpolant < draw_call.n_attributes; i_interpolant++) {

					const __int32 i_index_model = bin_triangle_data[i_triangle].i_triangle * 3;
					const __int32 i_model = bin_triangle_data[i_triangle].i_model;
					const __int32 model_id = draw_call.model_id[i_model];
					const model_& model = command_buffer.model[model_id];

					bool is_texture = draw_call.attribute_streams[i_interpolant].id == draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;

					if (is_texture) {
						__int32 i_triangle_model = bin_triangle_data[i_triangle].i_triangle;
						__int32 i_texture = model.i_textures[i_interpolant - 1][i_triangle_model];
						shader_input.blend_interval = draw_call.blend_interval[i_model];
						shader_input.texture_handlers[i_interpolant] = &model.texture_handlers[i_texture];
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

				shader_DISPATCH_PRE(raster_output, shader_input);
			}
		}
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
void Render_Screen_Bins(

	const __int32 i_thread,
	const __int32 n_threads,
	const __int32 bin_x,
	const __int32 bin_y,
	const command_buffer_& command_buffer,
	const model_manager_& model_manager,
	display_& display,
	visibility_data_ visibility_datas[]

)
{

	raster_output_ raster_output;

	int2_ bin_origin;
	bin_origin.x = bin_x << display_::BIN_SIZE_SHIFT;
	bin_origin.y = bin_y << display_::BIN_SIZE_SHIFT;

	shader_input_ shader_input;
	shader_input.depth_buffer = display.depth_buffer_bin[i_thread];
	shader_input.index_buffer_begin = display.index_buffer_bin[i_thread];
	shader_input.colour_buffer_begin = display.colour_buffer_bin[i_thread];

	Populate_Visibility_Buffer(

		bin_x,
		bin_y,
		n_threads,
		bin_origin,
		display,
		raster_output,
		shader_input
	);

	visibility_data_& visibility_data = visibility_datas[i_thread];

	Process_Visibility_Buffer(

		i_thread,
		display,
		visibility_data
	);

	Render_Visible_Triangles(

		bin_x,
		bin_y,
		bin_origin,
		command_buffer,
		display,
		raster_output,
		shader_input,
		visibility_data
	);

	Depth_Fog(

		bin_x,
		bin_y,
		i_thread,
		command_buffer,
		display
	);

	Write_Frame_Buffer(

		i_thread,
		bin_origin,
		display
	);

	Clear_Tile_Buffer(

		i_thread,
		display
	);

}

/*
==================
==================
*/

void systems_::render_::process_screen_bins(void* void_parameter, __int32 i_thread) {

	parameters_::render_::process_screen_bins_ *parameters = (parameters_::render_::process_screen_bins_ *)void_parameter;
	const __int32 i_read = parameters->command_buffer_handler->i_read;
	const command_buffer_& command_buffer = parameters->command_buffer_handler->command_buffers[i_read];
	const __int32 x = parameters->i_bin % display_::N_BINS_X;
	const __int32 y = parameters->i_bin / display_::N_BINS_X;

	Render_Screen_Bins(

		i_thread,
		parameters->n_threads,
		x,
		y,
		command_buffer,
		*parameters->model_manager,
		*parameters->display,
		parameters->visibility_data
	);
}

/*
==================
==================
*/

void systems_::render_::render_UI(void* void_parameter, __int32 i_thread) {

	parameters_::render_::render_UI_ *parameters = (parameters_::render_::render_UI_ *)void_parameter;
	const display_& display = *parameters->display;
	const user_interface_& user_interface = *parameters->user_interface;

	const __int32 write_pitch = display.locked_rect.Pitch / 4;
	unsigned __int32* write_buffer = (unsigned __int32*)display.locked_rect.pBits;

	const unsigned __int32 alpha = (255 << 24) | (159 << 16) | (91 << 8) | (83 << 0);
	const __int32 i_mip_map = 0;
	const __int32 n_elements = 4;
	const __int32 element_width = user_interface.texture_handler[0].width;
	const __int32 element_height = user_interface.texture_handler[0].height;
	__int32 offset_x = 0;

	__int32 value = user_interface.frame_rate;
	__int32 radix[6];
	__int32 base = 1000;

	for (__int32 i_element = 0; i_element < 4; i_element++) {

		radix[i_element] = value / base;
		value -= radix[i_element] * base;
		base /= 10;
		//printf_s(" %i , ", radix[i_element]);
	}
	//printf_s("\n");

	for (__int32 i_element = 0; i_element < n_elements; i_element++) {

		__int32 y_dest = 0;
		__int32 index = radix[i_element];

		for (__int32 y_source = 0; y_source < element_width * element_height; y_source += element_width) {

			__int32 x_dest = offset_x;

			for (__int32 x_source = 0; x_source < element_width; x_source++) {

				unsigned __int32 srce_colour = user_interface.texture_handler[index].texture[i_mip_map][y_source + x_source];
				unsigned __int32 dest_colour = write_buffer[y_dest + x_dest];
				bool is_transparent = srce_colour == alpha;
				write_buffer[y_dest + x_dest] = is_transparent ? dest_colour : srce_colour;
				x_dest++;
			}
			y_dest += write_pitch;
		}

		offset_x += element_width;
	}
}