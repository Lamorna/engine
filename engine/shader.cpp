#include "shader.h"
#include "vector.h"
#include "render_front.h"
#include "raster.h"
#include "texture.h"

static const __m128 series = set(0.0f, 1.0f, 2.0f, 3.0f);

static const float r_fixed_scale = 1.0f / fixed_point_scale_g;

/*
==================
==================
*/
void depth_NO_MASK(

	const unsigned __int32 i_buffer,
	const unsigned __int32 coverage_mask,
	const __m128i bazza[3][4],
	const shader_input_& shader_input
) {

	const __m128i triangle_index = set_all(shader_input.triangle_id);

	__m128 z_interpolant[3];
	z_interpolant[X] = set_all(shader_input.z_delta[0][shader_input.i_triangle]);
	z_interpolant[Y] = set_all(shader_input.z_delta[1][shader_input.i_triangle]);
	z_interpolant[Z] = set_all(shader_input.z_delta[2][shader_input.i_triangle]);

	__m128 r_area = set_all(shader_input.r_area[shader_input.i_triangle] * r_fixed_scale);

	__m128 w_screen[2][4];
	w_screen[0][0] = convert_float(bazza[0][0]) * r_area;
	w_screen[0][1] = convert_float(bazza[0][1]) * r_area;
	w_screen[0][2] = convert_float(bazza[0][2]) * r_area;
	w_screen[0][3] = convert_float(bazza[0][3]) * r_area;

	w_screen[1][0] = convert_float(bazza[1][0]) * r_area;
	w_screen[1][1] = convert_float(bazza[1][1]) * r_area;
	w_screen[1][2] = convert_float(bazza[1][2]) * r_area;
	w_screen[1][3] = convert_float(bazza[1][3]) * r_area;

	__m128 z_screen[4];
	z_screen[0] = (z_interpolant[X] * w_screen[0][0]) + (z_interpolant[Y] * w_screen[1][0]) + z_interpolant[Z];
	z_screen[1] = (z_interpolant[X] * w_screen[0][1]) + (z_interpolant[Y] * w_screen[1][1]) + z_interpolant[Z];
	z_screen[2] = (z_interpolant[X] * w_screen[0][2]) + (z_interpolant[Y] * w_screen[1][2]) + z_interpolant[Z];
	z_screen[3] = (z_interpolant[X] * w_screen[0][3]) + (z_interpolant[Y] * w_screen[1][3]) + z_interpolant[Z];

	__m128 z_clip[4];
	z_clip[0] = z_screen[0];
	z_clip[1] = z_screen[1];
	z_clip[2] = z_screen[2];
	z_clip[3] = z_screen[3];

	__m128 z_buffer[4];
	z_buffer[0] = load(shader_input.depth_buffer + i_buffer + 0);
	z_buffer[1] = load(shader_input.depth_buffer + i_buffer + 4);
	z_buffer[2] = load(shader_input.depth_buffer + i_buffer + 8);
	z_buffer[3] = load(shader_input.depth_buffer + i_buffer + 12);

	__m128i z_mask[4];
	z_mask[0] = (z_clip[0] > z_buffer[0]);
	z_mask[1] = (z_clip[1] > z_buffer[1]);
	z_mask[2] = (z_clip[2] > z_buffer[2]);
	z_mask[3] = (z_clip[3] > z_buffer[3]);

	__m128 z_write[4];
	z_write[0] = blend(z_clip[0], z_buffer[0], z_mask[0]);
	z_write[1] = blend(z_clip[1], z_buffer[1], z_mask[1]);
	z_write[2] = blend(z_clip[2], z_buffer[2], z_mask[2]);
	z_write[3] = blend(z_clip[3], z_buffer[3], z_mask[3]);

	store(z_write[0], shader_input.depth_buffer + i_buffer + 0);
	store(z_write[1], shader_input.depth_buffer + i_buffer + 4);
	store(z_write[2], shader_input.depth_buffer + i_buffer + 8);
	store(z_write[3], shader_input.depth_buffer + i_buffer + 12);

	__m128i index_buffer[4];
	index_buffer[0] = load(shader_input.index_buffer_begin + i_buffer + 0);
	index_buffer[1] = load(shader_input.index_buffer_begin + i_buffer + 4);
	index_buffer[2] = load(shader_input.index_buffer_begin + i_buffer + 8);
	index_buffer[3] = load(shader_input.index_buffer_begin + i_buffer + 12);

	__m128i index_blend[4];
	index_blend[0] = blend(triangle_index, index_buffer[0], z_mask[0]);
	index_blend[1] = blend(triangle_index, index_buffer[1], z_mask[1]);
	index_blend[2] = blend(triangle_index, index_buffer[2], z_mask[2]);
	index_blend[3] = blend(triangle_index, index_buffer[3], z_mask[3]);

	store(index_blend[0], shader_input.index_buffer_begin + i_buffer + 0);
	store(index_blend[1], shader_input.index_buffer_begin + i_buffer + 4);
	store(index_blend[2], shader_input.index_buffer_begin + i_buffer + 8);
	store(index_blend[3], shader_input.index_buffer_begin + i_buffer + 12);
}


/*
==================
==================
*/
void depth_MASK(

	const unsigned __int32 i_buffer,
	const unsigned __int32 coverage_mask,
	const __m128i bazza[3][4],
	const shader_input_& shader_input
) {


	const __m128i triangle_index = set_all(shader_input.triangle_id);

	__m128 z_interpolant[3];
	z_interpolant[X] = set_all(shader_input.z_delta[0][shader_input.i_triangle]);
	z_interpolant[Y] = set_all(shader_input.z_delta[1][shader_input.i_triangle]);
	z_interpolant[Z] = set_all(shader_input.z_delta[2][shader_input.i_triangle]);

	__m128 r_area = set_all(shader_input.r_area[shader_input.i_triangle] * r_fixed_scale);

	__m128 w_screen[2][4];
	w_screen[0][0] = convert_float(bazza[0][0]) * r_area;
	w_screen[0][1] = convert_float(bazza[0][1]) * r_area;
	w_screen[0][2] = convert_float(bazza[0][2]) * r_area;
	w_screen[0][3] = convert_float(bazza[0][3]) * r_area;

	w_screen[1][0] = convert_float(bazza[1][0]) * r_area;
	w_screen[1][1] = convert_float(bazza[1][1]) * r_area;
	w_screen[1][2] = convert_float(bazza[1][2]) * r_area;
	w_screen[1][3] = convert_float(bazza[1][3]) * r_area;

	__m128 z_screen[4];
	z_screen[0] = (z_interpolant[X] * w_screen[0][0]) + (z_interpolant[Y] * w_screen[1][0]) + z_interpolant[Z];
	z_screen[1] = (z_interpolant[X] * w_screen[0][1]) + (z_interpolant[Y] * w_screen[1][1]) + z_interpolant[Z];
	z_screen[2] = (z_interpolant[X] * w_screen[0][2]) + (z_interpolant[Y] * w_screen[1][2]) + z_interpolant[Z];
	z_screen[3] = (z_interpolant[X] * w_screen[0][3]) + (z_interpolant[Y] * w_screen[1][3]) + z_interpolant[Z];

	__m128 z_clip[4];
	z_clip[0] = z_screen[0];
	z_clip[1] = z_screen[1];
	z_clip[2] = z_screen[2];
	z_clip[3] = z_screen[3];

	__m128i pixel_mask[4];
	pixel_mask[0] = load_mask[(coverage_mask >> 0) & 0xf];
	pixel_mask[1] = load_mask[(coverage_mask >> 4) & 0xf];
	pixel_mask[2] = load_mask[(coverage_mask >> 8) & 0xf];
	pixel_mask[3] = load_mask[(coverage_mask >> 12) & 0xf];

	__m128 z_buffer[4];
	z_buffer[0] = load(shader_input.depth_buffer + i_buffer + 0);
	z_buffer[1] = load(shader_input.depth_buffer + i_buffer + 4);
	z_buffer[2] = load(shader_input.depth_buffer + i_buffer + 8);
	z_buffer[3] = load(shader_input.depth_buffer + i_buffer + 12);

	__m128i z_mask[4];
	z_mask[0] = (z_clip[0] > z_buffer[0]) & pixel_mask[0];
	z_mask[1] = (z_clip[1] > z_buffer[1]) & pixel_mask[1];
	z_mask[2] = (z_clip[2] > z_buffer[2]) & pixel_mask[2];
	z_mask[3] = (z_clip[3] > z_buffer[3]) & pixel_mask[3];

	__m128 z_write[4];
	z_write[0] = blend(z_clip[0], z_buffer[0], z_mask[0]);
	z_write[1] = blend(z_clip[1], z_buffer[1], z_mask[1]);
	z_write[2] = blend(z_clip[2], z_buffer[2], z_mask[2]);
	z_write[3] = blend(z_clip[3], z_buffer[3], z_mask[3]);

	store(z_write[0], shader_input.depth_buffer + i_buffer + 0);
	store(z_write[1], shader_input.depth_buffer + i_buffer + 4);
	store(z_write[2], shader_input.depth_buffer + i_buffer + 8);
	store(z_write[3], shader_input.depth_buffer + i_buffer + 12);

	__m128i index_buffer[4];
	index_buffer[0] = load(shader_input.index_buffer_begin + i_buffer + 0);
	index_buffer[1] = load(shader_input.index_buffer_begin + i_buffer + 4);
	index_buffer[2] = load(shader_input.index_buffer_begin + i_buffer + 8);
	index_buffer[3] = load(shader_input.index_buffer_begin + i_buffer + 12);

	__m128i index_blend[4];
	index_blend[0] = blend(triangle_index, index_buffer[0], z_mask[0]);
	index_blend[1] = blend(triangle_index, index_buffer[1], z_mask[1]);
	index_blend[2] = blend(triangle_index, index_buffer[2], z_mask[2]);
	index_blend[3] = blend(triangle_index, index_buffer[3], z_mask[3]);

	store(index_blend[0], shader_input.index_buffer_begin + i_buffer + 0);
	store(index_blend[1], shader_input.index_buffer_begin + i_buffer + 4);
	store(index_blend[2], shader_input.index_buffer_begin + i_buffer + 8);
	store(index_blend[3], shader_input.index_buffer_begin + i_buffer + 12);
}

/*
==================
==================
*/
void Process_Fragments(

	const raster_output_& raster_output,
	const shader_input_& shader_input
) {

	{
		const __int32 n_fragments = raster_output.n_fragments[raster_output_::TRIVIAL_ACCEPT];
		for (__int32 i_fragment = 0; i_fragment < n_fragments; i_fragment++) {

			const raster_fragment_& raster_fragment = raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT][i_fragment];
			__m128i bazza[3][4];

			for (__int32 i_edge = 0; i_edge < 2; i_edge++) {
				__m128i w_row = set_all(raster_fragment.w[i_edge]);
				bazza[i_edge][0] = w_row + load_u(raster_output.reject_table[0][i_edge][0]);
				bazza[i_edge][1] = w_row + load_u(raster_output.reject_table[0][i_edge][1]);
				bazza[i_edge][2] = w_row + load_u(raster_output.reject_table[0][i_edge][2]);
				bazza[i_edge][3] = w_row + load_u(raster_output.reject_table[0][i_edge][3]);
			}

			depth_NO_MASK(raster_fragment.i_buffer, raster_fragment.coverage_mask, bazza, shader_input);
		}
	}
	{
		const __int32 n_fragments = raster_output.n_fragments[raster_output_::PARTIAL_ACCEPT];
		for (__int32 i_fragment = 0; i_fragment < n_fragments; i_fragment++) {

			const raster_fragment_& raster_fragment = raster_output.raster_fragment[raster_output_::PARTIAL_ACCEPT][i_fragment];
			__m128i bazza[3][4];

			for (__int32 i_edge = 0; i_edge < 2; i_edge++) {
				__m128i w_row = set_all(raster_fragment.w[i_edge]);
				bazza[i_edge][0] = w_row + load_u(raster_output.reject_table[0][i_edge][0]);
				bazza[i_edge][1] = w_row + load_u(raster_output.reject_table[0][i_edge][1]);
				bazza[i_edge][2] = w_row + load_u(raster_output.reject_table[0][i_edge][2]);
				bazza[i_edge][3] = w_row + load_u(raster_output.reject_table[0][i_edge][3]);
			}

			depth_MASK(raster_fragment.i_buffer, raster_fragment.coverage_mask, bazza, shader_input);
		}
	}
	{
		const __int32 n_fragments = raster_output.n_fragments_COMPLETE;
		for (__int32 i_fragment = 0; i_fragment < n_fragments; i_fragment++) {

			const raster_fragment_complete_& raster_fragment = raster_output.raster_fragment_complete[i_fragment];

			depth_MASK(raster_fragment.i_buffer, raster_fragment.coverage_mask, raster_fragment.bazza, shader_input);
		}
	}
}

/*
==================
==================
*/
void shade_colour(
	
	shader_input_& data

) {

	static const __m128 zero = set_zero();
	static const __m128 half = set_all(0.5f);
	static const __m128 one = set_one();
	static const __m128 two = one + one;
	static const __m128 three = two + one;
	static const __m128 colour_clamp = broadcast(load_s(255.0f));

	const vertex4_* gradients = data.gradients[data.i_attribute];

	__m128 red_float[4];
	red_float[0] = (gradients[R].x * data.w_screen[0][0]) + (gradients[R].y * data.w_screen[1][0]) + gradients[R].z;
	red_float[1] = (gradients[R].x * data.w_screen[0][1]) + (gradients[R].y * data.w_screen[1][1]) + gradients[R].z;
	red_float[2] = (gradients[R].x * data.w_screen[0][2]) + (gradients[R].y * data.w_screen[1][2]) + gradients[R].z;
	red_float[3] = (gradients[R].x * data.w_screen[0][3]) + (gradients[R].y * data.w_screen[1][3]) + gradients[R].z;

	__m128 green_float[4];
	green_float[0] = (gradients[G].x * data.w_screen[0][0]) + (gradients[G].y * data.w_screen[1][0]) + gradients[G].z;
	green_float[1] = (gradients[G].x * data.w_screen[0][1]) + (gradients[G].y * data.w_screen[1][1]) + gradients[G].z;
	green_float[2] = (gradients[G].x * data.w_screen[0][2]) + (gradients[G].y * data.w_screen[1][2]) + gradients[G].z;
	green_float[3] = (gradients[G].x * data.w_screen[0][3]) + (gradients[G].y * data.w_screen[1][3]) + gradients[G].z;

	__m128 blue_float[4];
	blue_float[0] = (gradients[B].x * data.w_screen[0][0]) + (gradients[B].y * data.w_screen[1][0]) + gradients[B].z;
	blue_float[1] = (gradients[B].x * data.w_screen[0][1]) + (gradients[B].y * data.w_screen[1][1]) + gradients[B].z;
	blue_float[2] = (gradients[B].x * data.w_screen[0][2]) + (gradients[B].y * data.w_screen[1][2]) + gradients[B].z;
	blue_float[3] = (gradients[B].x * data.w_screen[0][3]) + (gradients[B].y * data.w_screen[1][3]) + gradients[B].z;

	red_float[0] = min_vec(max_vec(red_float[0], zero), colour_clamp);
	red_float[1] = min_vec(max_vec(red_float[1], zero), colour_clamp);
	red_float[2] = min_vec(max_vec(red_float[2], zero), colour_clamp);
	red_float[3] = min_vec(max_vec(red_float[3], zero), colour_clamp);

	green_float[0] = min_vec(max_vec(green_float[0], zero), colour_clamp);
	green_float[1] = min_vec(max_vec(green_float[1], zero), colour_clamp);
	green_float[2] = min_vec(max_vec(green_float[2], zero), colour_clamp);
	green_float[3] = min_vec(max_vec(green_float[3], zero), colour_clamp);

	blue_float[0] = min_vec(max_vec(blue_float[0], zero), colour_clamp);
	blue_float[1] = min_vec(max_vec(blue_float[1], zero), colour_clamp);
	blue_float[2] = min_vec(max_vec(blue_float[2], zero), colour_clamp);
	blue_float[3] = min_vec(max_vec(blue_float[3], zero), colour_clamp);

	__m128i red_int[4];
	red_int[0] = convert_int_trunc(red_float[0]);
	red_int[1] = convert_int_trunc(red_float[1]);
	red_int[2] = convert_int_trunc(red_float[2]);
	red_int[3] = convert_int_trunc(red_float[3]);

	__m128i green_int[4];
	green_int[0] = convert_int_trunc(green_float[0]);
	green_int[1] = convert_int_trunc(green_float[1]);
	green_int[2] = convert_int_trunc(green_float[2]);
	green_int[3] = convert_int_trunc(green_float[3]);

	__m128i blue_int[4];
	blue_int[0] = convert_int_trunc(blue_float[0]);
	blue_int[1] = convert_int_trunc(blue_float[1]);
	blue_int[2] = convert_int_trunc(blue_float[2]);
	blue_int[3] = convert_int_trunc(blue_float[3]);

	data.colour_out[data.i_attribute][0] = red_int[0] | (green_int[0] << 8) | (blue_int[0] << 16);
	data.colour_out[data.i_attribute][1] = red_int[1] | (green_int[1] << 8) | (blue_int[1] << 16);
	data.colour_out[data.i_attribute][2] = red_int[2] | (green_int[2] << 8) | (blue_int[2] << 16);
	data.colour_out[data.i_attribute][3] = red_int[3] | (green_int[3] << 8) | (blue_int[3] << 16);
}

/*
==================
==================
*/
__int32 log_2_integer(__int32 in){

	const unsigned __int32 b_table[] = { 0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000 };
	const unsigned __int32 s_table[] = { 1, 2, 4, 8, 16 };

	unsigned __int32 r = 0; 

	{
		bool is_and = in & b_table[4];
		in >>= is_and ? s_table[4] : 0;
		r |= is_and ? s_table[4] : 0;
	}
	{
		bool is_and = in & b_table[3];
		in >>= is_and ? s_table[3] : 0;
		r |= is_and ? s_table[3] : 0;
	}
	{
		bool is_and = in & b_table[2];
		in >>= is_and ? s_table[2] : 0;
		r |= is_and ? s_table[2] : 0;
	}
	{
		bool is_and = in & b_table[1];
		in >>= is_and ? s_table[1] : 0;
		r |= is_and ? s_table[1] : 0;
	}
	{
		bool is_and = in & b_table[0];
		in >>= is_and ? s_table[0] : 0;
		r |= is_and ? s_table[0] : 0;
	}

	return r;
}

/*
==================
==================
*/
void shade_texture_CLAMP(
	
	shader_input_& data

) {

	static const __m128 zero = set_zero();
	static const __m128 half = set_all(0.5f);
	static const __m128 one = set_one();
	static const __m128 two = one + one;
	static const __m128 three = two + one;
	static const __m128i zero_int = set_zero_si128();

	const vertex4_* gradients = data.gradients[data.i_attribute];

	float4_ u_table[4];
	float4_ v_table[4];

	{
		__m128 u_axis[4];
		u_axis[0] = (gradients[U].x * data.w_screen[0][0]) + (gradients[U].y * data.w_screen[1][0]) + gradients[U].z;
		u_axis[1] = (gradients[U].x * data.w_screen[0][1]) + (gradients[U].y * data.w_screen[1][1]) + gradients[U].z;
		u_axis[2] = (gradients[U].x * data.w_screen[0][2]) + (gradients[U].y * data.w_screen[1][2]) + gradients[U].z;
		u_axis[3] = (gradients[U].x * data.w_screen[0][3]) + (gradients[U].y * data.w_screen[1][3]) + gradients[U].z;

		__m128 v_axis[4];
		v_axis[0] = (gradients[V].x * data.w_screen[0][0]) + (gradients[V].y * data.w_screen[1][0]) + gradients[V].z;
		v_axis[1] = (gradients[V].x * data.w_screen[0][1]) + (gradients[V].y * data.w_screen[1][1]) + gradients[V].z;
		v_axis[2] = (gradients[V].x * data.w_screen[0][2]) + (gradients[V].y * data.w_screen[1][2]) + gradients[V].z;
		v_axis[3] = (gradients[V].x * data.w_screen[0][3]) + (gradients[V].y * data.w_screen[1][3]) + gradients[V].z;

		store_u(u_axis[0], u_table[0].f);
		store_u(u_axis[1], u_table[1].f);
		store_u(u_axis[2], u_table[2].f);
		store_u(u_axis[3], u_table[3].f);

		store_u(v_axis[0], v_table[0].f);
		store_u(v_axis[1], v_table[1].f);
		store_u(v_axis[2], v_table[2].f);
		store_u(v_axis[3], v_table[3].f);
	}

	const texture_handler_& texture_handler = *data.texture_handlers[data.i_attribute];

	float2_ du;
	du.x = (u_table[0].f[3] - u_table[0].f[0]) * (float)texture_handler.width;
	du.y = (u_table[3].f[0] - u_table[0].f[0]) * (float)texture_handler.width;

	float2_ dv;
	dv.x = (v_table[0].f[3] - v_table[0].f[0]) * (float)texture_handler.height;
	dv.y = (v_table[3].f[0] - v_table[0].f[0]) * (float)texture_handler.height;

	float area = abs((du.x * dv.y) - (du.y * dv.x))  * data.mip_level_bias;
	unsigned long area_int = 1 + (unsigned long)(area + 0.5f);
	__int32 i_mip_floor;
	_BitScanReverse((unsigned long*)&i_mip_floor, area_int);

	//__int32 test = log_2_integer(area_int);
	//printf_s("good: %i, test: %i ", i_mip_floor, test);

	//if (test != i_mip_floor) {
	//	printf_s("soft vaginal folds");
	//}

	i_mip_floor = max(i_mip_floor, 0);
	i_mip_floor = min(i_mip_floor, texture_handler.n_mip_levels - 1);

	const __int32 width = texture_handler.width >> i_mip_floor;
	const __int32 height = texture_handler.height >> i_mip_floor;
	const __int32 shift = texture_handler.width_shift - i_mip_floor;

	const __m128i texture_width_int = set_all(width);
	const __m128 texture_width = convert_float(set_all(width));
	const __m128 texture_height = convert_float(set_all(height));
	const __m128i width_clamp = set_all(width - 1);
	const __m128i height_clamp = set_all(height - 1);
	const __m128i width_shift = load_s(shift);

	{
		__m128 u_axis[4];
		u_axis[0] = (load_u(u_table[0].f) * texture_width); // - half;
		u_axis[1] = (load_u(u_table[1].f) * texture_width); // - half;
		u_axis[2] = (load_u(u_table[2].f) * texture_width); // - half;
		u_axis[3] = (load_u(u_table[3].f) * texture_width); // - half;

		__m128 v_axis[4];
		v_axis[0] = (load_u(v_table[0].f) * texture_height); // - half;
		v_axis[1] = (load_u(v_table[1].f) * texture_height); // - half;
		v_axis[2] = (load_u(v_table[2].f) * texture_height); // - half;
		v_axis[3] = (load_u(v_table[3].f) * texture_height); // - half;

		__m128i u_int[4];
		u_int[0] = convert_int_trunc(u_axis[0]);
		u_int[1] = convert_int_trunc(u_axis[1]);
		u_int[2] = convert_int_trunc(u_axis[2]);
		u_int[3] = convert_int_trunc(u_axis[3]);

		__m128i v_int[4];
		v_int[0] = convert_int_trunc(v_axis[0]);
		v_int[1] = convert_int_trunc(v_axis[1]);
		v_int[2] = convert_int_trunc(v_axis[2]);
		v_int[3] = convert_int_trunc(v_axis[3]);

		u_int[0] = max_vec(min_vec(u_int[0], width_clamp), zero_int);
		u_int[1] = max_vec(min_vec(u_int[1], width_clamp), zero_int);
		u_int[2] = max_vec(min_vec(u_int[2], width_clamp), zero_int);
		u_int[3] = max_vec(min_vec(u_int[3], width_clamp), zero_int);

		v_int[0] = max_vec(min_vec(v_int[0], height_clamp), zero_int);
		v_int[1] = max_vec(min_vec(v_int[1], height_clamp), zero_int);
		v_int[2] = max_vec(min_vec(v_int[2], height_clamp), zero_int);
		v_int[3] = max_vec(min_vec(v_int[3], height_clamp), zero_int);

		//__m128i i_texels[4];
		//i_texels[0] = u_int[0] + (v_int[0] << width_shift);
		//i_texels[1] = u_int[1] + (v_int[1] << width_shift);
		//i_texels[2] = u_int[2] + (v_int[2] << width_shift);
		//i_texels[3] = u_int[3] + (v_int[3] << width_shift);

		__m128i i_texels[4];
		i_texels[0] = u_int[0] + (v_int[0] * texture_width_int);
		i_texels[1] = u_int[1] + (v_int[1] * texture_width_int);
		i_texels[2] = u_int[2] + (v_int[2] * texture_width_int);
		i_texels[3] = u_int[3] + (v_int[3] * texture_width_int);

		__int32 i_texels_in[4][4];
		store_u(i_texels[0], i_texels_in[0]);
		store_u(i_texels[1], i_texels_in[1]);
		store_u(i_texels[2], i_texels_in[2]);
		store_u(i_texels[3], i_texels_in[3]);

		unsigned __int32 texels_out[4][4];
		texels_out[0][0] = texture_handler.texture[i_mip_floor][i_texels_in[0][0]];
		texels_out[0][1] = texture_handler.texture[i_mip_floor][i_texels_in[0][1]];
		texels_out[0][2] = texture_handler.texture[i_mip_floor][i_texels_in[0][2]];
		texels_out[0][3] = texture_handler.texture[i_mip_floor][i_texels_in[0][3]];
							  			  
		texels_out[1][0] = texture_handler.texture[i_mip_floor][i_texels_in[1][0]];
		texels_out[1][1] = texture_handler.texture[i_mip_floor][i_texels_in[1][1]];
		texels_out[1][2] = texture_handler.texture[i_mip_floor][i_texels_in[1][2]];
		texels_out[1][3] = texture_handler.texture[i_mip_floor][i_texels_in[1][3]];
							  			  
		texels_out[2][0] = texture_handler.texture[i_mip_floor][i_texels_in[2][0]];
		texels_out[2][1] = texture_handler.texture[i_mip_floor][i_texels_in[2][1]];
		texels_out[2][2] = texture_handler.texture[i_mip_floor][i_texels_in[2][2]];
		texels_out[2][3] = texture_handler.texture[i_mip_floor][i_texels_in[2][3]];
							  			  
		texels_out[3][0] = texture_handler.texture[i_mip_floor][i_texels_in[3][0]];
		texels_out[3][1] = texture_handler.texture[i_mip_floor][i_texels_in[3][1]];
		texels_out[3][2] = texture_handler.texture[i_mip_floor][i_texels_in[3][2]];
		texels_out[3][3] = texture_handler.texture[i_mip_floor][i_texels_in[3][3]];

		data.colour_out[data.i_attribute][0] = load_u(texels_out[0]);
		data.colour_out[data.i_attribute][1] = load_u(texels_out[1]);
		data.colour_out[data.i_attribute][2] = load_u(texels_out[2]);
		data.colour_out[data.i_attribute][3] = load_u(texels_out[3]);
	}
}

/*
==================
==================
*/
void shade_barycentric(

	const __m128i bazza[3][4],
	shader_input_& data

) {
	static const float r_fixed_scale = 1.0f / fixed_point_scale_g;

	__m128 screen_barry[2][4];
	__m128 r_depth[4];

	{
		__m128 z_interpolant[3];
		z_interpolant[X] = set_all(data.z_delta[0][data.i_triangle]);
		z_interpolant[Y] = set_all(data.z_delta[1][data.i_triangle]);
		z_interpolant[Z] = set_all(data.z_delta[2][data.i_triangle]);

		__m128 r_area = set_all(data.r_area[data.i_triangle] * r_fixed_scale);

		__m128 w_screen[2][4];
		w_screen[0][0] = convert_float(bazza[0][0]) * r_area;
		w_screen[0][1] = convert_float(bazza[0][1]) * r_area;
		w_screen[0][2] = convert_float(bazza[0][2]) * r_area;
		w_screen[0][3] = convert_float(bazza[0][3]) * r_area;
									   
		w_screen[1][0] = convert_float(bazza[1][0]) * r_area;
		w_screen[1][1] = convert_float(bazza[1][1]) * r_area;
		w_screen[1][2] = convert_float(bazza[1][2]) * r_area;
		w_screen[1][3] = convert_float(bazza[1][3]) * r_area;

		__m128 z_screen[4];
		z_screen[0] = (z_interpolant[X] * w_screen[0][0]) + (z_interpolant[Y] * w_screen[1][0]) + z_interpolant[Z];
		z_screen[1] = (z_interpolant[X] * w_screen[0][1]) + (z_interpolant[Y] * w_screen[1][1]) + z_interpolant[Z];
		z_screen[2] = (z_interpolant[X] * w_screen[0][2]) + (z_interpolant[Y] * w_screen[1][2]) + z_interpolant[Z];
		z_screen[3] = (z_interpolant[X] * w_screen[0][3]) + (z_interpolant[Y] * w_screen[1][3]) + z_interpolant[Z];

		r_depth[0] = reciprocal(z_screen[0]);
		r_depth[1] = reciprocal(z_screen[1]);
		r_depth[2] = reciprocal(z_screen[2]);
		r_depth[3] = reciprocal(z_screen[3]);

		screen_barry[0][0] = (w_screen[0][0] * data.barycentric[0][X]) + (w_screen[1][0] * data.barycentric[0][Y]) + data.barycentric[0][Z];
		screen_barry[0][1] = (w_screen[0][1] * data.barycentric[0][X]) + (w_screen[1][1] * data.barycentric[0][Y]) + data.barycentric[0][Z];
		screen_barry[0][2] = (w_screen[0][2] * data.barycentric[0][X]) + (w_screen[1][2] * data.barycentric[0][Y]) + data.barycentric[0][Z];
		screen_barry[0][3] = (w_screen[0][3] * data.barycentric[0][X]) + (w_screen[1][3] * data.barycentric[0][Y]) + data.barycentric[0][Z];

		screen_barry[1][0] = (w_screen[0][0] * data.barycentric[1][X]) + (w_screen[1][0] * data.barycentric[1][Y]) + data.barycentric[1][Z];
		screen_barry[1][1] = (w_screen[0][1] * data.barycentric[1][X]) + (w_screen[1][1] * data.barycentric[1][Y]) + data.barycentric[1][Z];
		screen_barry[1][2] = (w_screen[0][2] * data.barycentric[1][X]) + (w_screen[1][2] * data.barycentric[1][Y]) + data.barycentric[1][Z];
		screen_barry[1][3] = (w_screen[0][3] * data.barycentric[1][X]) + (w_screen[1][3] * data.barycentric[1][Y]) + data.barycentric[1][Z];
	}

	data.w_screen[0][0] = screen_barry[0][0] * r_depth[0];
	data.w_screen[0][1] = screen_barry[0][1] * r_depth[1];
	data.w_screen[0][2] = screen_barry[0][2] * r_depth[2];
	data.w_screen[0][3] = screen_barry[0][3] * r_depth[3];

	data.w_screen[1][0] = screen_barry[1][0] * r_depth[0];
	data.w_screen[1][1] = screen_barry[1][1] * r_depth[1];
	data.w_screen[1][2] = screen_barry[1][2] * r_depth[2];
	data.w_screen[1][3] = screen_barry[1][3] * r_depth[3];

}

/*
==================
==================
*/
void shade_texture_WRAP(

	shader_input_& data
	
) {

	static const __m128 zero = set_zero();
	static const __m128 half = set_all(0.5f);
	static const __m128 one = set_one();
	static const __m128 two = one + one;
	static const __m128 three = two + one;
	static const __m128i zero_int = set_zero_si128();

	const vertex4_* gradients = data.gradients[data.i_attribute];

		//================================================================================================

	float4_ u_table[4];
	float4_ v_table[4];

	{
		__m128 u_axis[4];
		u_axis[0] = (gradients[U].x * data.w_screen[0][0]) + (gradients[U].y * data.w_screen[1][0]) + gradients[U].z;
		u_axis[1] = (gradients[U].x * data.w_screen[0][1]) + (gradients[U].y * data.w_screen[1][1]) + gradients[U].z;
		u_axis[2] = (gradients[U].x * data.w_screen[0][2]) + (gradients[U].y * data.w_screen[1][2]) + gradients[U].z;
		u_axis[3] = (gradients[U].x * data.w_screen[0][3]) + (gradients[U].y * data.w_screen[1][3]) + gradients[U].z;

		__m128 v_axis[4];
		v_axis[0] = (gradients[V].x * data.w_screen[0][0]) + (gradients[V].y * data.w_screen[1][0]) + gradients[V].z;
		v_axis[1] = (gradients[V].x * data.w_screen[0][1]) + (gradients[V].y * data.w_screen[1][1]) + gradients[V].z;
		v_axis[2] = (gradients[V].x * data.w_screen[0][2]) + (gradients[V].y * data.w_screen[1][2]) + gradients[V].z;
		v_axis[3] = (gradients[V].x * data.w_screen[0][3]) + (gradients[V].y * data.w_screen[1][3]) + gradients[V].z;

		store_u(u_axis[0], u_table[0].f);
		store_u(u_axis[1], u_table[1].f);
		store_u(u_axis[2], u_table[2].f);
		store_u(u_axis[3], u_table[3].f);

		store_u(v_axis[0], v_table[0].f);
		store_u(v_axis[1], v_table[1].f);
		store_u(v_axis[2], v_table[2].f);
		store_u(v_axis[3], v_table[3].f);
	}

	const texture_handler_& texture_handler = *data.texture_handlers[data.i_attribute];

	float2_ du;
	du.x = (u_table[0].f[3] - u_table[0].f[0]) * (float)texture_handler.width;
	du.y = (u_table[3].f[0] - u_table[0].f[0]) * (float)texture_handler.width;

	float2_ dv;
	dv.x = (v_table[0].f[3] - v_table[0].f[0]) * (float)texture_handler.height;
	dv.y = (v_table[3].f[0] - v_table[0].f[0]) * (float)texture_handler.height;

	float area = abs((du.x * dv.y) - (du.y * dv.x)) * data.mip_level_bias;
	unsigned long area_int = 1 + (unsigned long)(area + 0.5f);
	__int32 i_mip_floor;
	_BitScanReverse((unsigned long*)&i_mip_floor, area_int);
	i_mip_floor = max(i_mip_floor, 0);
	i_mip_floor = min(i_mip_floor, texture_handler.n_mip_levels - 1);

	const __int32 width = texture_handler.width >> i_mip_floor;
	const __int32 height = texture_handler.height >> i_mip_floor;
	const __int32 shift = texture_handler.width_shift - i_mip_floor;

	const __m128 texture_width = convert_float(set_all(width));
	const __m128 texture_height = convert_float(set_all(height));
	const __m128i width_wrap = set_all(width - 1);
	const __m128i height_wrap = set_all(height - 1);
	const __m128i width_shift = load_s(shift);

	{
		__m128 u_axis[4];
		u_axis[0] = (load_u(u_table[0].f) * texture_width); // - half;
		u_axis[1] = (load_u(u_table[1].f) * texture_width); // - half;
		u_axis[2] = (load_u(u_table[2].f) * texture_width); // - half;
		u_axis[3] = (load_u(u_table[3].f) * texture_width); // - half;

		__m128 v_axis[4];
		v_axis[0] = (load_u(v_table[0].f) * texture_height); // - half;
		v_axis[1] = (load_u(v_table[1].f) * texture_height); // - half;
		v_axis[2] = (load_u(v_table[2].f) * texture_height); // - half;
		v_axis[3] = (load_u(v_table[3].f) * texture_height); // - half;

		__m128i u_int[4];
		u_int[0] = convert_int_trunc(u_axis[0]);
		u_int[1] = convert_int_trunc(u_axis[1]);
		u_int[2] = convert_int_trunc(u_axis[2]);
		u_int[3] = convert_int_trunc(u_axis[3]);

		__m128i v_int[4];
		v_int[0] = convert_int_trunc(v_axis[0]);
		v_int[1] = convert_int_trunc(v_axis[1]);
		v_int[2] = convert_int_trunc(v_axis[2]);
		v_int[3] = convert_int_trunc(v_axis[3]);

		u_int[0] &= width_wrap;
		u_int[1] &= width_wrap;
		u_int[2] &= width_wrap;
		u_int[3] &= width_wrap;

		v_int[0] &= height_wrap;
		v_int[1] &= height_wrap;
		v_int[2] &= height_wrap;
		v_int[3] &= height_wrap;

		__m128i i_texels[4];
		i_texels[0] = u_int[0] + (v_int[0] << width_shift);
		i_texels[1] = u_int[1] + (v_int[1] << width_shift);
		i_texels[2] = u_int[2] + (v_int[2] << width_shift);
		i_texels[3] = u_int[3] + (v_int[3] << width_shift);

		__int32 i_texels_in[4][4];
		store_u(i_texels[0], i_texels_in[0]);
		store_u(i_texels[1], i_texels_in[1]);
		store_u(i_texels[2], i_texels_in[2]);
		store_u(i_texels[3], i_texels_in[3]);

		unsigned __int32 texels_out[4][4];
		texels_out[0][0] = texture_handler.texture[i_mip_floor][i_texels_in[0][0]];
		texels_out[0][1] = texture_handler.texture[i_mip_floor][i_texels_in[0][1]];
		texels_out[0][2] = texture_handler.texture[i_mip_floor][i_texels_in[0][2]];
		texels_out[0][3] = texture_handler.texture[i_mip_floor][i_texels_in[0][3]];
										  
		texels_out[1][0] = texture_handler.texture[i_mip_floor][i_texels_in[1][0]];
		texels_out[1][1] = texture_handler.texture[i_mip_floor][i_texels_in[1][1]];
		texels_out[1][2] = texture_handler.texture[i_mip_floor][i_texels_in[1][2]];
		texels_out[1][3] = texture_handler.texture[i_mip_floor][i_texels_in[1][3]];
										  
		texels_out[2][0] = texture_handler.texture[i_mip_floor][i_texels_in[2][0]];
		texels_out[2][1] = texture_handler.texture[i_mip_floor][i_texels_in[2][1]];
		texels_out[2][2] = texture_handler.texture[i_mip_floor][i_texels_in[2][2]];
		texels_out[2][3] = texture_handler.texture[i_mip_floor][i_texels_in[2][3]];
										  
		texels_out[3][0] = texture_handler.texture[i_mip_floor][i_texels_in[3][0]];
		texels_out[3][1] = texture_handler.texture[i_mip_floor][i_texels_in[3][1]];
		texels_out[3][2] = texture_handler.texture[i_mip_floor][i_texels_in[3][2]];
		texels_out[3][3] = texture_handler.texture[i_mip_floor][i_texels_in[3][3]];

		data.colour_out[data.i_attribute][0] = load_u(texels_out[0]);
		data.colour_out[data.i_attribute][1] = load_u(texels_out[1]);
		data.colour_out[data.i_attribute][2] = load_u(texels_out[2]);
		data.colour_out[data.i_attribute][3] = load_u(texels_out[3]);
	}
}

/*
==================
==================
*/
void shader_texture_BLEND(

	shader_input_& data

) {
	static const __m128i channel_mask = set_all(0xff);

	const __int32 i_attribute_0 = 1;
	const __int32 i_attribute_1 = 2;
	__m128 colour_channels[2][3][4];

	colour_channels[0][R][0] = convert_float((data.colour_out[i_attribute_0][0] >> 0) & channel_mask);
	colour_channels[0][R][1] = convert_float((data.colour_out[i_attribute_0][1] >> 0) & channel_mask);
	colour_channels[0][R][2] = convert_float((data.colour_out[i_attribute_0][2] >> 0) & channel_mask);
	colour_channels[0][R][3] = convert_float((data.colour_out[i_attribute_0][3] >> 0) & channel_mask);

	colour_channels[0][G][0] = convert_float((data.colour_out[i_attribute_0][0] >> 8) & channel_mask);
	colour_channels[0][G][1] = convert_float((data.colour_out[i_attribute_0][1] >> 8) & channel_mask);
	colour_channels[0][G][2] = convert_float((data.colour_out[i_attribute_0][2] >> 8) & channel_mask);
	colour_channels[0][G][3] = convert_float((data.colour_out[i_attribute_0][3] >> 8) & channel_mask);

	colour_channels[0][B][0] = convert_float((data.colour_out[i_attribute_0][0] >> 16) & channel_mask);
	colour_channels[0][B][1] = convert_float((data.colour_out[i_attribute_0][1] >> 16) & channel_mask);
	colour_channels[0][B][2] = convert_float((data.colour_out[i_attribute_0][2] >> 16) & channel_mask);
	colour_channels[0][B][3] = convert_float((data.colour_out[i_attribute_0][3] >> 16) & channel_mask);

	colour_channels[1][R][0] = convert_float((data.colour_out[i_attribute_1][0] >> 0) & channel_mask);
	colour_channels[1][R][1] = convert_float((data.colour_out[i_attribute_1][1] >> 0) & channel_mask);
	colour_channels[1][R][2] = convert_float((data.colour_out[i_attribute_1][2] >> 0) & channel_mask);
	colour_channels[1][R][3] = convert_float((data.colour_out[i_attribute_1][3] >> 0) & channel_mask);
																		  
	colour_channels[1][G][0] = convert_float((data.colour_out[i_attribute_1][0] >> 8) & channel_mask);
	colour_channels[1][G][1] = convert_float((data.colour_out[i_attribute_1][1] >> 8) & channel_mask);
	colour_channels[1][G][2] = convert_float((data.colour_out[i_attribute_1][2] >> 8) & channel_mask);
	colour_channels[1][G][3] = convert_float((data.colour_out[i_attribute_1][3] >> 8) & channel_mask);
																		  
	colour_channels[1][B][0] = convert_float((data.colour_out[i_attribute_1][0] >> 16) & channel_mask);
	colour_channels[1][B][1] = convert_float((data.colour_out[i_attribute_1][1] >> 16) & channel_mask);
	colour_channels[1][B][2] = convert_float((data.colour_out[i_attribute_1][2] >> 16) & channel_mask);
	colour_channels[1][B][3] = convert_float((data.colour_out[i_attribute_1][3] >> 16) & channel_mask);

	const __m128 blend_interval = set_all(data.blend_interval);

	__m128 blended_channels[3][4];
	blended_channels[R][0] = colour_channels[0][R][0] + ((colour_channels[1][R][0] - colour_channels[0][R][0]) * blend_interval);
	blended_channels[R][1] = colour_channels[0][R][1] + ((colour_channels[1][R][1] - colour_channels[0][R][1]) * blend_interval);
	blended_channels[R][2] = colour_channels[0][R][2] + ((colour_channels[1][R][2] - colour_channels[0][R][2]) * blend_interval);
	blended_channels[R][3] = colour_channels[0][R][3] + ((colour_channels[1][R][3] - colour_channels[0][R][3]) * blend_interval);

	blended_channels[G][0] = colour_channels[0][G][0] + ((colour_channels[1][G][0] - colour_channels[0][G][0]) * blend_interval);
	blended_channels[G][1] = colour_channels[0][G][1] + ((colour_channels[1][G][1] - colour_channels[0][G][1]) * blend_interval);
	blended_channels[G][2] = colour_channels[0][G][2] + ((colour_channels[1][G][2] - colour_channels[0][G][2]) * blend_interval);
	blended_channels[G][3] = colour_channels[0][G][3] + ((colour_channels[1][G][3] - colour_channels[0][G][3]) * blend_interval);

	blended_channels[B][0] = colour_channels[0][B][0] + ((colour_channels[1][B][0] - colour_channels[0][B][0]) * blend_interval);
	blended_channels[B][1] = colour_channels[0][B][1] + ((colour_channels[1][B][1] - colour_channels[0][B][1]) * blend_interval);
	blended_channels[B][2] = colour_channels[0][B][2] + ((colour_channels[1][B][2] - colour_channels[0][B][2]) * blend_interval);
	blended_channels[B][3] = colour_channels[0][B][3] + ((colour_channels[1][B][3] - colour_channels[0][B][3]) * blend_interval);

	__m128i integer_colour[3][4];
	integer_colour[R][0] = convert_int_trunc(blended_channels[R][0]);
	integer_colour[R][1] = convert_int_trunc(blended_channels[R][1]);
	integer_colour[R][2] = convert_int_trunc(blended_channels[R][2]);
	integer_colour[R][3] = convert_int_trunc(blended_channels[R][3]);

	integer_colour[G][0] = convert_int_trunc(blended_channels[G][0]);
	integer_colour[G][1] = convert_int_trunc(blended_channels[G][1]);
	integer_colour[G][2] = convert_int_trunc(blended_channels[G][2]);
	integer_colour[G][3] = convert_int_trunc(blended_channels[G][3]);

	integer_colour[B][0] = convert_int_trunc(blended_channels[B][0]);
	integer_colour[B][1] = convert_int_trunc(blended_channels[B][1]);
	integer_colour[B][2] = convert_int_trunc(blended_channels[B][2]);
	integer_colour[B][3] = convert_int_trunc(blended_channels[B][3]);

	data.colour_out[i_attribute_0][0] = integer_colour[R][0] | (integer_colour[G][0] << 8) | (integer_colour[B][0] << 16);
	data.colour_out[i_attribute_0][1] = integer_colour[R][1] | (integer_colour[G][1] << 8) | (integer_colour[B][1] << 16);
	data.colour_out[i_attribute_0][2] = integer_colour[R][2] | (integer_colour[G][2] << 8) | (integer_colour[B][2] << 16);
	data.colour_out[i_attribute_0][3] = integer_colour[R][3] | (integer_colour[G][3] << 8) | (integer_colour[B][3] << 16);

	const __m128i zero = set_zero_si128();
	data.colour_out[i_attribute_1][0] = zero;
	data.colour_out[i_attribute_1][1] = zero;
	data.colour_out[i_attribute_1][2] = zero;
	data.colour_out[i_attribute_1][3] = zero;

}


/*
==================
==================
*/
void write_colour_buffer(

	const __int32 n_blocks,
	const shader_input_& data

) {

	__m128i triangle_id = set_all(data.triangle_id);

	__m128i buffer_id[4];
	buffer_id[0] = load(data.index_buffer + 0);
	buffer_id[1] = load(data.index_buffer + 4);
	buffer_id[2] = load(data.index_buffer + 8);
	buffer_id[3] = load(data.index_buffer + 12);

	__m128i mask[4];
	mask[0] = buffer_id[0] == triangle_id;
	mask[1] = buffer_id[1] == triangle_id;
	mask[2] = buffer_id[2] == triangle_id;
	mask[3] = buffer_id[3] == triangle_id;

	__m128i colour_buffer[4];
	colour_buffer[0] = load(data.colour_buffer + 0);
	colour_buffer[1] = load(data.colour_buffer + 4);
	colour_buffer[2] = load(data.colour_buffer + 8);
	colour_buffer[3] = load(data.colour_buffer + 12);

	colour_buffer[0] = _mm_andnot_si128(mask[0], colour_buffer[0]);
	colour_buffer[1] = _mm_andnot_si128(mask[1], colour_buffer[1]);
	colour_buffer[2] = _mm_andnot_si128(mask[2], colour_buffer[2]);
	colour_buffer[3] = _mm_andnot_si128(mask[3], colour_buffer[3]);

	for (__int32 i_block = 0; i_block < n_blocks; i_block++) {

		colour_buffer[0] = add_uint8_saturate(colour_buffer[0], data.colour_out[i_block][0] & mask[0]);
		colour_buffer[1] = add_uint8_saturate(colour_buffer[1], data.colour_out[i_block][1] & mask[1]);
		colour_buffer[2] = add_uint8_saturate(colour_buffer[2], data.colour_out[i_block][2] & mask[2]);
		colour_buffer[3] = add_uint8_saturate(colour_buffer[3], data.colour_out[i_block][3] & mask[3]);
	}

	store(colour_buffer[0], data.colour_buffer + 0);
	store(colour_buffer[1], data.colour_buffer + 4);
	store(colour_buffer[2], data.colour_buffer + 8);
	store(colour_buffer[3], data.colour_buffer + 12);
}

/*
==================
==================
*/
void shader_DISPATCH(

	const unsigned __int32 i_buffer,
	const unsigned __int32 coverage_mask,
	const __m128i bazza[3][4],
	shader_input_& input

) {

	const draw_call_& draw_call = *input.draw_call;
	input.colour_buffer = input.colour_buffer_begin + i_buffer;
	input.index_buffer = input.index_buffer_begin + i_buffer;

	shade_barycentric(bazza, input);

	for (input.i_attribute = 0; input.i_attribute < draw_call.n_attributes; input.i_attribute++) {

		draw_call.attribute_streams[input.i_attribute].pixel_shader(input);
	}
	for (__int32 i_temp = 0; i_temp < draw_call.n_additional_pixel_shaders; i_temp++) {

		draw_call.additional_pixel_shaders[i_temp](input);
	}
	write_colour_buffer(draw_call.n_attributes, input);

}

/*
==================
==================
*/
void shader_DISPATCH_PRE(

	const raster_output_& raster_output,
	shader_input_& shader_input

) {

	{
		const __int32 n_fragments = raster_output.n_fragments[raster_output_::TRIVIAL_ACCEPT];
		for (__int32 i_fragment = 0; i_fragment < n_fragments; i_fragment++) {

			const raster_fragment_& raster_fragment = raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT][i_fragment];
			__m128i bazza[3][4];

			for (__int32 i_edge = 0; i_edge < 2; i_edge++) {
				__m128i w_row = set_all(raster_fragment.w[i_edge]);
				bazza[i_edge][0] = w_row + load_u(raster_output.reject_table[0][i_edge][0]);
				bazza[i_edge][1] = w_row + load_u(raster_output.reject_table[0][i_edge][1]);
				bazza[i_edge][2] = w_row + load_u(raster_output.reject_table[0][i_edge][2]);
				bazza[i_edge][3] = w_row + load_u(raster_output.reject_table[0][i_edge][3]);
			}

			shader_DISPATCH(raster_fragment.i_buffer, raster_fragment.coverage_mask, bazza, shader_input);
		}
	}
	{
		const __int32 n_fragments = raster_output.n_fragments[raster_output_::PARTIAL_ACCEPT];
		for (__int32 i_fragment = 0; i_fragment < n_fragments; i_fragment++) {

			const raster_fragment_& raster_fragment = raster_output.raster_fragment[raster_output_::PARTIAL_ACCEPT][i_fragment];
			__m128i bazza[3][4];

			for (__int32 i_edge = 0; i_edge < 2; i_edge++) {
				__m128i w_row = set_all(raster_fragment.w[i_edge]);
				bazza[i_edge][0] = w_row + load_u(raster_output.reject_table[0][i_edge][0]);
				bazza[i_edge][1] = w_row + load_u(raster_output.reject_table[0][i_edge][1]);
				bazza[i_edge][2] = w_row + load_u(raster_output.reject_table[0][i_edge][2]);
				bazza[i_edge][3] = w_row + load_u(raster_output.reject_table[0][i_edge][3]);
			}

			shader_DISPATCH(raster_fragment.i_buffer, raster_fragment.coverage_mask, bazza, shader_input);
		}
	}
	{
		const __int32 n_fragments = raster_output.n_fragments_COMPLETE;
		for (__int32 i_fragment = 0; i_fragment < n_fragments; i_fragment++) {

			const raster_fragment_complete_& raster_fragment = raster_output.raster_fragment_complete[i_fragment];

			shader_DISPATCH(raster_fragment.i_buffer, raster_fragment.coverage_mask, raster_fragment.bazza, shader_input);
		}
	}

}

/*
==================
==================
*/
void Vertex_Lighting_NULL(

	const __int32 n_triangles,
	const vertex_light_manager_& vertex_light_manager,
	const float4_ positions[4][3],
	float4_ colour[4][3]

) {

}

/*
==================
==================
*/
void Vertex_Lighting(

	const __int32 n_triangles,
	const vertex_light_manager_& vertex_light_manager,
	const float4_ positions[4][3],
	float4_ colour[4][3]

) {

	static const float r_screen_scale_x = 1.0f / screen_scale_x;
	static const float r_screen_scale_y = 1.0f / screen_scale_y;
	const __m128 attenuation_factor = set_all(800.0f);
	const __m128 specular_scale = set_all(100.0f);
	const __m128 diffuse_scale = set_all(20.0f);

	const __m128 zero = set_all(0.0f);
	const __m128 one = set_all(1.0f);

	__m128 r_screen_scale[2];
	r_screen_scale[X] = set_all(r_screen_scale_x);
	r_screen_scale[Y] = set_all(r_screen_scale_y);
	__m128 screen_shift[2];
	screen_shift[X] = set_all(screen_shift_x);
	screen_shift[Y] = set_all(screen_shift_y);

	__m128 clip_space_position[3][4];
	__m128 vertex_colour[3][4];

	for (__int32 i_vertex = 0; i_vertex < 3; i_vertex++) {

		__m128 vertex_position[4];
		for (__int32 i_triangle = 0; i_triangle < n_triangles; i_triangle++) {
			vertex_position[i_triangle] = load_u(positions[i_triangle][i_vertex].f);
			vertex_colour[i_vertex][i_triangle] = load_u(colour[i_triangle][i_vertex].f);
		}
		Transpose(vertex_position);
		Transpose(vertex_colour[i_vertex]);

		__m128 depth = reciprocal(vertex_position[Z]);
		clip_space_position[i_vertex][X] = ((vertex_position[X] - screen_shift[X]) * r_screen_scale[X]) * depth;
		clip_space_position[i_vertex][Y] = ((vertex_position[Y] - screen_shift[Y]) * r_screen_scale[Y]) * depth;
		clip_space_position[i_vertex][Z] = depth;
	}

	__m128 a[3];
	a[X] = clip_space_position[1][X] - clip_space_position[0][X];
	a[Y] = clip_space_position[1][Y] - clip_space_position[0][Y];
	a[Z] = clip_space_position[1][Z] - clip_space_position[0][Z];

	__m128 b[3];
	b[X] = clip_space_position[2][X] - clip_space_position[0][X];
	b[Y] = clip_space_position[2][Y] - clip_space_position[0][Y];
	b[Z] = clip_space_position[2][Z] - clip_space_position[0][Z];


	__m128 normal[4];
	normal[X] = (a[Y] * b[Z]) - (a[Z] * b[Y]);
	normal[Y] = (a[Z] * b[X]) - (a[X] * b[Z]);
	normal[Z] = (a[X] * b[Y]) - (a[Y] * b[X]);

	__m128 mag = (normal[X] * normal[X]) + (normal[Y] * normal[Y]) + (normal[Z] * normal[Z]);
	mag = _mm_rsqrt_ps(mag);
	normal[X] *= mag;
	normal[Y] *= mag;
	normal[Z] *= mag;

	for (__int32 i_light = 0; i_light < 1; i_light++) {


		for (__int32 i_vertex = 0; i_vertex < 3; i_vertex++) {


			__m128 light_position[3];
			__m128 light_colour[3];
			const float intensity = vertex_light_manager.light_sources[i_light].intensity;
			for (__int32 i_axis = X; i_axis < W; i_axis++) {

				light_position[i_axis] = set_all(vertex_light_manager.light_sources[i_light].position.f[i_axis]);
				light_colour[i_axis] = set_all(vertex_light_manager.light_sources[i_light].colour.f[i_axis] * intensity);
			}

			const __m128 extent = set_all(40.0f);
			__m128i is_valid = set_true();
			is_valid &= (clip_space_position[i_vertex][X] - light_position[X]) < extent;
			is_valid &= (clip_space_position[i_vertex][Y] - light_position[Y]) < extent;
			is_valid &= (clip_space_position[i_vertex][Z] - light_position[Z]) < extent;

			light_position[X] = set_all(0.0f);
			light_position[Y] = set_all(0.0f);
			light_position[Z] = set_all(0.0f);

			light_colour[X] = set_all(100.0f);
			light_colour[Y] = set_all(100.0f);
			light_colour[Z] = set_all(100.0f);

			__m128 light_ray[3];
			light_ray[X] = clip_space_position[i_vertex][X] - light_position[X];
			light_ray[Y] = clip_space_position[i_vertex][Y] - light_position[Y];
			light_ray[Z] = clip_space_position[i_vertex][Z] - light_position[Z];

			__m128 mag = (light_ray[X] * light_ray[X]) + (light_ray[Y] * light_ray[Y]) + (light_ray[Z] * light_ray[Z]);
			mag = _mm_rsqrt_ps(mag);
			light_ray[X] *= mag;
			light_ray[Y] *= mag;
			light_ray[Z] *= mag;

			__m128 dot = (normal[X] * light_ray[X]) + (normal[Y] * light_ray[Y]) + (normal[Z] * light_ray[Z]);
			dot &= dot > zero;
			dot = (dot * dot) * mag;

			__m128 distance = set_zero();
			for (__int32 i_axis = X; i_axis < W; i_axis++) {
				__m128 d = light_position[i_axis] - clip_space_position[i_vertex][i_axis];
				distance += (d * d);
			}
			__m128 scalar = reciprocal(distance) * attenuation_factor;
			scalar = max_vec(scalar, zero);
			scalar = min_vec(scalar, one);

			for (__int32 i_channel = R; i_channel < A; i_channel++) {
				vertex_colour[i_vertex][i_channel] += dot * specular_scale * light_colour[i_channel];
				vertex_colour[i_vertex][i_channel] += mag * diffuse_scale * light_colour[i_channel];
			}
		}
	}
	for (__int32 i_vertex = 0; i_vertex < 3; i_vertex++) {
		Transpose(vertex_colour[i_vertex]);
		for (__int32 i_triangle = 0; i_triangle < n_triangles; i_triangle++) {
			store_u(vertex_colour[i_vertex][i_triangle], colour[i_triangle][i_vertex].f);
		}
	}


}

/*
==================
==================
*/
void Vertex_Lighting_REM(

	const __int32 n_triangles,
	const vertex_light_manager_& vertex_light_manager,
	const float4_ positions[4][3],
	float4_ colour[4][3]

) {

	//const __int32 VERTEX_COLOUR = FIRST_ATTRIBUTE + 0;

	static const float r_screen_scale_x = 1.0f / screen_scale_x;
	static const float r_screen_scale_y = 1.0f / screen_scale_y;
	//const __m128 attenuation_factor = set_all(200.0f);
	//const __m128 attenuation_factor = set_all(800.0f);
	//const __m128 specular_scale = set_all(100.0f);
	//const __m128 diffuse_scale = set_all(20.0f);

	__m128 r_screen_scale[2];
	r_screen_scale[X] = set_all(r_screen_scale_x);
	r_screen_scale[Y] = set_all(r_screen_scale_y);
	__m128 screen_shift[2];
	screen_shift[X] = set_all(screen_shift_x);
	screen_shift[Y] = set_all(screen_shift_y);

	__m128 clip_space_position[3][4];
	//__m128 vertex_colour[3][4];

	float4_ new_position[4][3];
	for (__int32 i_vertex = 0; i_vertex < 3; i_vertex++) {

		__m128 vertex_position[4];
		for (__int32 i_triangle = 0; i_triangle < n_triangles; i_triangle++) {
			vertex_position[i_triangle] = load_u(positions[i_triangle][i_vertex].f);
			//vertex_colour[i_vertex][i_triangle] = load_u(colour[i_triangle][i_vertex].f);
		}
		Transpose(vertex_position);
		//Transpose(vertex_colour[i_vertex]);

		__m128 depth = reciprocal(vertex_position[Z]);
		clip_space_position[i_vertex][X] = ((vertex_position[X] - screen_shift[X]) * r_screen_scale[X]) * depth;
		clip_space_position[i_vertex][Y] = ((vertex_position[Y] - screen_shift[Y]) * r_screen_scale[Y]) * depth;
		clip_space_position[i_vertex][Z] = depth;


	}

	__m128 a[3];
	a[X] = clip_space_position[1][X] - clip_space_position[0][X];
	a[Y] = clip_space_position[1][Y] - clip_space_position[0][Y];
	a[Z] = clip_space_position[1][Z] - clip_space_position[0][Z];

	__m128 b[3];
	b[X] = clip_space_position[2][X] - clip_space_position[0][X];
	b[Y] = clip_space_position[2][Y] - clip_space_position[0][Y];
	b[Z] = clip_space_position[2][Z] - clip_space_position[0][Z];


	__m128 normal[4];
	normal[X] = (a[Y] * b[Z]) - (a[Z] * b[Y]);
	normal[Y] = (a[Z] * b[X]) - (a[X] * b[Z]);
	normal[Z] = (a[X] * b[Y]) - (a[Y] * b[X]);

	__m128 mag = (normal[X] * normal[X]) + (normal[Y] * normal[Y]) + (normal[Z] * normal[Z]);
	mag = _mm_rsqrt_ps(mag);
	normal[X] *= mag;
	normal[Y] *= mag;
	normal[Z] *= mag;

	float normal_4[3][4];
	store_u(normal[X], normal_4[X]);
	store_u(normal[Y], normal_4[Y]);
	store_u(normal[Z], normal_4[Z]);

	float centre_4[3][4];
	float extent_4[3][4];
	const __m128 half = set_all(0.5f);
	for (__int32 i_axis = X; i_axis < W; i_axis++) {

		__m128 max;
		__m128 min;
		max = min = clip_space_position[0][i_axis];
		max = max_vec(max_vec(max, clip_space_position[1][i_axis]), clip_space_position[2][i_axis]);
		min = min_vec(min_vec(min, clip_space_position[1][i_axis]), clip_space_position[2][i_axis]);
		store_u((max + min) * half, centre_4[i_axis]);
		store_u((max - min) * half, extent_4[i_axis]);
	}

	for (__int32 i_vertex = 0; i_vertex < 3; i_vertex++) {

		Transpose(clip_space_position[i_vertex]);
		for (__int32 i_triangle = 0; i_triangle < n_triangles; i_triangle++) {
			store_u(clip_space_position[i_vertex][i_triangle], new_position[i_triangle][i_vertex].f);
		}
	}

	const __m128 zero = set_all(0.0f);
	const __m128 one = set_all(1.0f);

	enum {
		MAX_LIGHTS_PER_VERTEX = 128,
	};

	for (__int32 i_triangle = 0; i_triangle < n_triangles; i_triangle++) {

		__m128 centre[3];
		__m128 extent[3];
		for (__int32 i_axis = X; i_axis < W; i_axis++) {
			centre[i_axis] = set_all(centre_4[i_axis][i_triangle]);
			extent[i_axis] = set_all(extent_4[i_axis][i_triangle]);
		}

		float z_min = centre_4[Z][i_triangle] - extent_4[Z][i_triangle];
		float z_max = centre_4[Z][i_triangle] + extent_4[Z][i_triangle];
		__int32 bin_min = __int32(z_min / vertex_light_manager.bin_interval);
		__int32 bin_max = __int32(z_max / vertex_light_manager.bin_interval);
		bin_min = min(bin_min, vertex_light_manager_::NUM_BINS - 1);
		bin_max = min(bin_max, vertex_light_manager_::NUM_BINS - 1);
		bin_min = max(bin_min, 0);
		bin_max = max(bin_max, 0);

		//bin_max = bin_max >= 10 ? 0 : bin_max;
		//printf_s(" %i , %i \n", bin_min, bin_max);

		__int32 i_lights[MAX_LIGHTS_PER_VERTEX];
		__int32 n_lights = 0;
		{
			for (__int32 i_bin = bin_min; i_bin <= bin_max; i_bin++) {

				const vertex_light_manager_::bin_& bin = vertex_light_manager.bin[i_bin];

				for (__int32 i_light_4 = 0; i_light_4 < bin.n_lights; i_light_4 += 4) {

					const __int32 n = min(bin.n_lights - i_light_4, 4);

					__m128 light_position[4];
					for (__int32 i_light = 0; i_light < n; i_light++) {
						__int32 index = vertex_light_manager.i_light[bin.i_start + i_light_4 + i_light];
						light_position[i_light] = load_u(vertex_light_manager.light_sources[index].position.f);
					}
					Transpose(light_position);

					const __m128 light_extent = set_all(100.0f);
					__m128i is_valid = set_true();
					is_valid &= abs(centre[X] - light_position[X]) < (extent[X] + light_extent);
					is_valid &= abs(centre[Y] - light_position[Y]) < (extent[Y] + light_extent);
					is_valid &= abs(centre[Z] - light_position[Z]) < (extent[Z] + light_extent);

					unsigned __int32 result_mask = store_mask(is_valid);

					for (__int32 i_light = 0; i_light < n; i_light++) {

						__int32 index = vertex_light_manager.i_light[bin.i_start + i_light_4 + i_light];
						i_lights[n_lights] = index;
						n_lights += (result_mask >> i_light) & 0x1;
					}

					if (n_lights > MAX_LIGHTS_PER_VERTEX) {

						n_lights = MAX_LIGHTS_PER_VERTEX;
						break;
					}
				}
			}
		}

		for (__int32 i_vertex = 0; i_vertex < 3; i_vertex++) {

			__m128 vertex_position[3];
			vertex_position[X] = set_all(new_position[i_triangle][i_vertex].x);
			vertex_position[Y] = set_all(new_position[i_triangle][i_vertex].y);
			vertex_position[Z] = set_all(new_position[i_triangle][i_vertex].z);

			__m128 vertex_colour[4];
			vertex_colour[R] = set_all(0.0f);
			vertex_colour[G] = set_all(0.0f);
			vertex_colour[B] = set_all(0.0f);

			__m128 normal[3];
			normal[X] = set_all(normal_4[X][i_triangle]);
			normal[Y] = set_all(normal_4[Y][i_triangle]);
			normal[Z] = set_all(normal_4[Z][i_triangle]);

			for (__int32 i_light_4 = 0; i_light_4 < n_lights; i_light_4 += 4) {

				const __int32 n = min(n_lights - i_light_4, 4);

				__m128 light_position[4];
				__m128 light_colour[4];
				unsigned __int32 mask = 0x0;
				float intensity_4[4];
				for (__int32 i_light = 0; i_light < n; i_light++) {

					mask |= 0x1 << i_light;
					const __int32 index = i_lights[i_light_4 + i_light];
					intensity_4[i_light] = vertex_light_manager.light_sources[index].intensity;
					light_position[i_light] = load_u(vertex_light_manager.light_sources[index].position.f);
					light_colour[i_light] = load_u(vertex_light_manager.light_sources[index].colour.f);
				}
				Transpose(light_position);
				Transpose(light_colour);
				__m128 light_intensity = load_u(intensity_4);

				__m128 light_ray[3];
				light_ray[X] = vertex_position[X] - light_position[X];
				light_ray[Y] = vertex_position[Y] - light_position[Y];
				light_ray[Z] = vertex_position[Z] - light_position[Z];

				__m128 mag = (light_ray[X] * light_ray[X]) + (light_ray[Y] * light_ray[Y]) + (light_ray[Z] * light_ray[Z]);
				__m128 r_mag = _mm_rsqrt_ps(mag);
				light_ray[X] *= r_mag;
				light_ray[Y] *= r_mag;
				light_ray[Z] *= r_mag;

				__m128 dot = (normal[X] * light_ray[X]) + (normal[Y] * light_ray[Y]) + (normal[Z] * light_ray[Z]);
				dot &= dot > zero;

				__m128 r_distance = reciprocal(one + mag);
				__m128 spec = (dot * dot) * r_distance;

				static const __m128 specular_coefficient = set_all(2000.0f);
				static const __m128 diffuse_coefficient = set_all(200.0f);

				//printf_s(" %f ", dot);
				__m128i loop_mask = load_mask[mask];

				for (__int32 i_channel = R; i_channel < A; i_channel++) {
					__m128 final = spec * specular_coefficient * light_colour[i_channel] * light_intensity;
					final += r_distance * diffuse_coefficient * light_colour[i_channel] * light_intensity;
					vertex_colour[i_channel] += final & loop_mask;
				}
			}

			Transpose(vertex_colour);
			vertex_colour[0] += vertex_colour[1] + vertex_colour[2] + vertex_colour[3];
			float4_ temp;
			store_u(vertex_colour[0], temp.f);
			colour[i_triangle][i_vertex].x += temp.x;
			colour[i_triangle][i_vertex].y += temp.y;
			colour[i_triangle][i_vertex].z += temp.z;
		}
	}

}

/*
==================
==================
*/
void Shade_Vertex_Colour_Simple(

	const __int32 n_triangles,
	const command_buffer_& render_buffer,
	const draw_call_& draw_buffer,
	const bin_triangle_data_ bin_triangle_data[4],
	float4_ vertex_buffer[4][3]

) {

	for (__int32 i_triangle = 0; i_triangle < n_triangles; i_triangle++) {
		const __int32 i_model = bin_triangle_data[i_triangle].i_model;
		float4_* vertex_colour = vertex_buffer[i_triangle];
		for (__int32 i_vertex = 0; i_vertex < NUM_VERTICES; i_vertex++) {

			vertex_colour[i_vertex].x += draw_buffer.colour[i_model].r;
			vertex_colour[i_vertex].y += draw_buffer.colour[i_model].g;
			vertex_colour[i_vertex].z += draw_buffer.colour[i_model].b;
		}
	}
}

/*
==================
==================
*/
void Shade_Vertex_Colour(

	const __int32 n_triangles,
	const command_buffer_& render_buffer,
	const draw_call_& draw_call,
	const bin_triangle_data_ bin_triangle_data[4],
	float4_ vertex_buffer[4][3]

) {
	//const __int32 VERTEX_COLOUR = FIRST_ATTRIBUTE + 0;

	for (__int32 i_triangle = 0; i_triangle < n_triangles; i_triangle++) {
		const __int32 i_model = bin_triangle_data[i_triangle].i_model;
		float4_* vertex = vertex_buffer[i_triangle];
		for (__int32 i_vertex = 0; i_vertex < NUM_VERTICES; i_vertex++) {
			Vector_X_Matrix(vertex[i_vertex], draw_call.m_vertex_colour[i_model], vertex[i_vertex]);

			float3_ colour = draw_call.colour[i_model];

			vertex[i_vertex].x = colour.r * vertex[i_vertex].z;
			vertex[i_vertex].y = colour.g * vertex[i_vertex].z;
			vertex[i_vertex].z = colour.b * vertex[i_vertex].z;
		}
	}
}
/*
==================
==================
*/
void Shade_Vertex_Texture(

	const __int32 n_triangles,
	const command_buffer_& render_buffer,
	const draw_call_& draw_call,
	const bin_triangle_data_ bin_triangle_data[4],
	float4_ vertex_buffer[4][3]

) {

	for (__int32 i_triangle = 0; i_triangle < n_triangles; i_triangle++) {
		const __int32 i_model = bin_triangle_data[i_triangle].i_model;
		float4_* vertex = vertex_buffer[i_triangle];
		for (__int32 i_vertex = 0; i_vertex < NUM_VERTICES; i_vertex++) {
			Vector_X_Matrix(vertex[i_vertex], draw_call.m_vertex_texture[i_model], vertex[i_vertex]);
		}
	}
}
