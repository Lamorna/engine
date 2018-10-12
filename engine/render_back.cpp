
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
void Process_Bin_Triangles(

	const __int32 bin_x,
	const __int32 bin_y,
	const __int32 n_threads,
	int2_ bin_origin,
	display_& display,
	raster_output_& raster_output,
	shader_input_& shader_input,
	const command_buffer_& command_buffer

)
{
	for (__int32 i_thread_entry = 0; i_thread_entry < n_threads; i_thread_entry++) {

		screen_bin_& screen_bin = display.screen_bin[i_thread_entry][bin_y][bin_x];
		const bin_triangle_* bin_triangle = screen_bin.bin_triangle;
		const bin_triangle_data_* bin_triangle_data = screen_bin.bin_triangle_data;

		__int32 i_triangle_read = 0;

		for (__int32 i_draw_call = 0; i_draw_call < screen_bin.n_draw_calls + 1; i_draw_call++) {

			const __int32 n_triangles = screen_bin.n_tris[i_draw_call];
			const __int32 draw_id = screen_bin.draw_id[i_draw_call];
			const draw_call_& draw_call = command_buffer.draw_calls[draw_id];

			for (__int32 i_triangle_4 = 0; i_triangle_4 < n_triangles; i_triangle_4 += 4) {

				__int32 n = min(n_triangles - i_triangle_4, 4);

				float3_ position[3][4];
				__m128 position_4[3][4];
				float3_ barycentric[3][4];

				float4_ vertices[MAX_VERTEX_ATTRIBUTES][4][3];


				for (__int32 i_vertex = 0; i_vertex < 3; i_vertex++) {
					for (__int32 i_triangle = 0; i_triangle < n; i_triangle++) {

						position[i_vertex][i_triangle] = bin_triangle[i_triangle_read + i_triangle].position[i_vertex];
						position_4[i_vertex][i_triangle] = load_u(bin_triangle[i_triangle_read + i_triangle].position[i_vertex].f);
						barycentric[i_vertex][i_triangle] = bin_triangle[i_triangle_read + i_triangle].barycentric[i_vertex];

						vertices[0][i_triangle][i_vertex].x = bin_triangle[i_triangle_read + i_triangle].position[i_vertex].x;
						vertices[0][i_triangle][i_vertex].y = bin_triangle[i_triangle_read + i_triangle].position[i_vertex].y;
						vertices[0][i_triangle][i_vertex].z = bin_triangle[i_triangle_read + i_triangle].position[i_vertex].z;

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

				__m128i draw_id_4 = set_all(draw_id);
				__m128i is_depth_write = draw_id_4 != set_all(draw_call_::id_::SKYBOX);

				depth_delta[0] &= is_depth_write;
				depth_delta[1] &= is_depth_write;
				depth_delta[2] &= is_depth_write;

				store_u(depth_delta[0], shader_input.z_delta[0]);
				store_u(depth_delta[1], shader_input.z_delta[1]);
				store_u(depth_delta[2], shader_input.z_delta[2]);


				//===========================================================================
				{

					static const float4_ clear = { 0.0f, 0.0f, 0.0f, 1.0f };

					for (__int32 i_triangle = 0; i_triangle < n; i_triangle++) {

						const __int32 i_index_model = bin_triangle_data[i_triangle_read + i_triangle].i_triangle * 3;
						const __int32 i_model = bin_triangle_data[i_triangle_read + i_triangle].i_model;
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
				}

				// attribute shading
				for (__int32 i_attribute = 0; i_attribute < FIRST_ATTRIBUTE + 1; i_attribute++) {

					if (draw_call.attribute_streams[i_attribute].vertex_shader) {

						draw_call.attribute_streams[i_attribute].vertex_shader(
							n,
							command_buffer,
							command_buffer.draw_calls[draw_id],
							&bin_triangle_data[i_triangle_read],
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
				//===========================================================================

				for (__int32 i_triangle = 0; i_triangle < n; i_triangle++) {

					__int32 i_current_triangle = i_triangle_read + i_triangle;
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

					const __int32 i_model = bin_triangle_data[i_triangle_read + i_triangle].i_model;
					const __int32 model_id = draw_call.model_id[i_model];
					const model_& model = command_buffer.model[model_id];

					float3_ model_colour = draw_call.colour[i_model];
					shader_input.model_colour = __int32(model_colour.b) << 16 | __int32(model_colour.g) << 8 | __int32(model_colour.r) << 0;

					for (__int32 i_interpolant = 0; i_interpolant < 2; i_interpolant++) {

						//bool is_texture = draw_call.attribute_streams[i_interpolant].id == draw_call_::attribute_stream_::id_::TEXTURE_VERTEX;
						bool is_texture = i_interpolant == 1;

						if (is_texture) {
							__int32 i_triangle_model = bin_triangle_data[i_triangle_read + i_triangle].i_triangle;
							__int32 i_texture = model.i_textures[i_interpolant - 1][i_triangle_model];
							i_texture += draw_call.i_texture_offset[i_model];
							shader_input.texture_handlers[i_interpolant] = &model.texture_handlers[i_texture];
						}

						static __int32 stride[] = {

							draw_call_::attribute_stream_::stride_::COLOUR_VERTEX,
							draw_call_::attribute_stream_::stride_::TEXTURE_VERTEX,
						};

						for (__int32 i_component = 0; i_component < stride[i_interpolant]; i_component++) {

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

					shader_input.triangle_id = triangle_id;
					shader_input.i_triangle = i_triangle;
					shader_input.draw_call = &draw_call;

					Process_Fragments(raster_output, shader_input);
				}
				i_triangle_read += n;
			}
			screen_bin.draw_id[i_draw_call] = -1;
			screen_bin.n_tris[i_draw_call] = 0;
		}
		screen_bin.n_draw_calls = -1;
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
	display_& display

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

	//Populate_Visibility_Buffer(

	//	bin_x,
	//	bin_y,
	//	n_threads,
	//	bin_origin,
	//	display,
	//	raster_output,
	//	shader_input
	//);

	Process_Bin_Triangles(

		bin_x,
		bin_y,
		n_threads,
		bin_origin,
		display,
		raster_output,
		shader_input,
		command_buffer
	);

	//visibility_data_& visibility_data = visibility_datas[i_thread];

	//Process_Visibility_Buffer(

	//	i_thread,
	//	display,
	//	visibility_data
	//);

	//Render_Visible_Triangles(

	//	bin_x,
	//	bin_y,
	//	bin_origin,
	//	command_buffer,
	//	display,
	//	raster_output,
	//	shader_input,
	//	visibility_data
	//);

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
		*parameters->display
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