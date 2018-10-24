#include "shader.h"
#include "vector.h"
#include "render_front.h"
#include "raster.h"
#include "texture.h"

static const __m128 series = set(0.0f, 1.0f, 2.0f, 3.0f);

static const unsigned __int64 one_bit_64 = 0x1;



/*
==================
==================
*/
void pixel_shader(

	const unsigned __int32 i_buffer,
	const unsigned __int32 coverage_mask,
	const __m128i bazza[3][4],
	shader_input_& shader_input
) {

	static const __m128 zero = set_zero();
	static const __m128 half = set_all(0.5f);
	static const __m128 one = set_one();
	static const __m128 two = one + one;
	static const __m128 three = two + one;
	static const __m128i zero_int = set_zero_si128();
	static const __m128 colour_clamp = broadcast(load_s(255.0f));


	unsigned __int32 depth_mask = 0x0;

	__m128 w_screen[2][4];
	w_screen[0][0] = convert_float(bazza[0][0]) * shader_input.r_area;
	w_screen[0][1] = convert_float(bazza[0][1]) * shader_input.r_area;
	w_screen[0][2] = convert_float(bazza[0][2]) * shader_input.r_area;
	w_screen[0][3] = convert_float(bazza[0][3]) * shader_input.r_area;

	w_screen[1][0] = convert_float(bazza[1][0]) * shader_input.r_area;
	w_screen[1][1] = convert_float(bazza[1][1]) * shader_input.r_area;
	w_screen[1][2] = convert_float(bazza[1][2]) * shader_input.r_area;
	w_screen[1][3] = convert_float(bazza[1][3]) * shader_input.r_area;

	__m128 z_screen[4];
	z_screen[0] = (shader_input.z_delta[X] * w_screen[0][0]) + (shader_input.z_delta[Y] * w_screen[1][0]) + shader_input.z_delta[Z];
	z_screen[1] = (shader_input.z_delta[X] * w_screen[0][1]) + (shader_input.z_delta[Y] * w_screen[1][1]) + shader_input.z_delta[Z];
	z_screen[2] = (shader_input.z_delta[X] * w_screen[0][2]) + (shader_input.z_delta[Y] * w_screen[1][2]) + shader_input.z_delta[Z];
	z_screen[3] = (shader_input.z_delta[X] * w_screen[0][3]) + (shader_input.z_delta[Y] * w_screen[1][3]) + shader_input.z_delta[Z];

	{
		//if (shader_input.is_test) {

		//	__m128 x = convert_float(set_all(shader_input.x));
		//	__m128 y = convert_float(set_all(shader_input.y));
		//	y += set_all(0.5f);
		//	x += set_all(0.5f);
		//	x += set(0.0f, 1.0f, 2.0f, 3.0f);

		//	__m128 y_block[4];
		//	y_block[0] = y;
		//	y_block[1] = y + one;
		//	y_block[2] = y + two;
		//	y_block[3] = y + three;

		//	__m128 z_interpolant[3];
		//	z_interpolant[X] = set_all(shader_input.depth_interpolants[X]);
		//	z_interpolant[Y] = set_all(shader_input.depth_interpolants[Y]);
		//	z_interpolant[Z] = set_all(shader_input.depth_interpolants[Z]);

		//	z_screen[0] = (z_interpolant[X] * x) + (z_interpolant[Y] * y_block[0]) + z_interpolant[Z];
		//	z_screen[1] = (z_interpolant[X] * x) + (z_interpolant[Y] * y_block[1]) + z_interpolant[Z];
		//	z_screen[2] = (z_interpolant[X] * x) + (z_interpolant[Y] * y_block[2]) + z_interpolant[Z];
		//	z_screen[3] = (z_interpolant[X] * x) + (z_interpolant[Y] * y_block[3]) + z_interpolant[Z];
		//}
	}

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
	z_mask[0] = (z_screen[0] > z_buffer[0]) & pixel_mask[0];
	z_mask[1] = (z_screen[1] > z_buffer[1]) & pixel_mask[1];
	z_mask[2] = (z_screen[2] > z_buffer[2]) & pixel_mask[2];
	z_mask[3] = (z_screen[3] > z_buffer[3]) & pixel_mask[3];


	depth_mask |= store_mask(z_mask[0]) << 0;
	depth_mask |= store_mask(z_mask[1]) << 4;
	depth_mask |= store_mask(z_mask[2]) << 8;
	depth_mask |= store_mask(z_mask[3]) << 12;


	__m128 z_write[4];
	z_write[0] = blend(z_screen[0], z_buffer[0], z_mask[0]);
	z_write[1] = blend(z_screen[1], z_buffer[1], z_mask[1]);
	z_write[2] = blend(z_screen[2], z_buffer[2], z_mask[2]);
	z_write[3] = blend(z_screen[3], z_buffer[3], z_mask[3]);

	{
		__m128 z_max;
		z_max = z_write[0];
		z_max = min_vec(z_write[1], z_max);
		z_max = min_vec(z_write[2], z_max);
		z_max = min_vec(z_write[3], z_max);

		__m128 z_out = z_max;
		z_max = rotate_left(z_max);
		z_out = min_vec(z_max, z_out);
		z_max = rotate_left(z_max);
		z_out = min_vec(z_max, z_out);
		z_max = rotate_left(z_max);
		z_out = min_vec(z_max, z_out);

		shader_input.z_max = store_s(z_out);
	}


	store(z_write[0], shader_input.depth_buffer + i_buffer + 0);
	store(z_write[1], shader_input.depth_buffer + i_buffer + 4);
	store(z_write[2], shader_input.depth_buffer + i_buffer + 8);
	store(z_write[3], shader_input.depth_buffer + i_buffer + 12);


	if (depth_mask == 0x0) {
		return;
	}


	__m128 screen_barry[2][4];
	screen_barry[0][0] = (w_screen[0][0] * shader_input.barycentric[0][X]) + (w_screen[1][0] * shader_input.barycentric[0][Y]) + shader_input.barycentric[0][Z];
	screen_barry[0][1] = (w_screen[0][1] * shader_input.barycentric[0][X]) + (w_screen[1][1] * shader_input.barycentric[0][Y]) + shader_input.barycentric[0][Z];
	screen_barry[0][2] = (w_screen[0][2] * shader_input.barycentric[0][X]) + (w_screen[1][2] * shader_input.barycentric[0][Y]) + shader_input.barycentric[0][Z];
	screen_barry[0][3] = (w_screen[0][3] * shader_input.barycentric[0][X]) + (w_screen[1][3] * shader_input.barycentric[0][Y]) + shader_input.barycentric[0][Z];

	screen_barry[1][0] = (w_screen[0][0] * shader_input.barycentric[1][X]) + (w_screen[1][0] * shader_input.barycentric[1][Y]) + shader_input.barycentric[1][Z];
	screen_barry[1][1] = (w_screen[0][1] * shader_input.barycentric[1][X]) + (w_screen[1][1] * shader_input.barycentric[1][Y]) + shader_input.barycentric[1][Z];
	screen_barry[1][2] = (w_screen[0][2] * shader_input.barycentric[1][X]) + (w_screen[1][2] * shader_input.barycentric[1][Y]) + shader_input.barycentric[1][Z];
	screen_barry[1][3] = (w_screen[0][3] * shader_input.barycentric[1][X]) + (w_screen[1][3] * shader_input.barycentric[1][Y]) + shader_input.barycentric[1][Z];

	__m128 r_depth[4];
	r_depth[0] = reciprocal(z_screen[0]);
	r_depth[1] = reciprocal(z_screen[1]);
	r_depth[2] = reciprocal(z_screen[2]);
	r_depth[3] = reciprocal(z_screen[3]);

	__m128 w_clip[2][4];
	w_clip[0][0] = screen_barry[0][0] * r_depth[0];
	w_clip[0][1] = screen_barry[0][1] * r_depth[1];
	w_clip[0][2] = screen_barry[0][2] * r_depth[2];
	w_clip[0][3] = screen_barry[0][3] * r_depth[3];

	w_clip[1][0] = screen_barry[1][0] * r_depth[0];
	w_clip[1][1] = screen_barry[1][1] * r_depth[1];
	w_clip[1][2] = screen_barry[1][2] * r_depth[2];
	w_clip[1][3] = screen_barry[1][3] * r_depth[3];

	__m128i colour_out[4];
	{
		const vertex4_* gradients = shader_input.gradients[ATTRIBUTE_COLOUR];

		__m128 red_float[4];
		red_float[0] = (gradients[R].x * w_clip[0][0]) + (gradients[R].y * w_clip[1][0]) + gradients[R].z;
		red_float[1] = (gradients[R].x * w_clip[0][1]) + (gradients[R].y * w_clip[1][1]) + gradients[R].z;
		red_float[2] = (gradients[R].x * w_clip[0][2]) + (gradients[R].y * w_clip[1][2]) + gradients[R].z;
		red_float[3] = (gradients[R].x * w_clip[0][3]) + (gradients[R].y * w_clip[1][3]) + gradients[R].z;

		__m128 green_float[4];
		green_float[0] = (gradients[G].x * w_clip[0][0]) + (gradients[G].y * w_clip[1][0]) + gradients[G].z;
		green_float[1] = (gradients[G].x * w_clip[0][1]) + (gradients[G].y * w_clip[1][1]) + gradients[G].z;
		green_float[2] = (gradients[G].x * w_clip[0][2]) + (gradients[G].y * w_clip[1][2]) + gradients[G].z;
		green_float[3] = (gradients[G].x * w_clip[0][3]) + (gradients[G].y * w_clip[1][3]) + gradients[G].z;

		__m128 blue_float[4];
		blue_float[0] = (gradients[B].x * w_clip[0][0]) + (gradients[B].y * w_clip[1][0]) + gradients[B].z;
		blue_float[1] = (gradients[B].x * w_clip[0][1]) + (gradients[B].y * w_clip[1][1]) + gradients[B].z;
		blue_float[2] = (gradients[B].x * w_clip[0][2]) + (gradients[B].y * w_clip[1][2]) + gradients[B].z;
		blue_float[3] = (gradients[B].x * w_clip[0][3]) + (gradients[B].y * w_clip[1][3]) + gradients[B].z;

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

		colour_out[0] = red_int[0] | (green_int[0] << 8) | (blue_int[0] << 16);
		colour_out[1] = red_int[1] | (green_int[1] << 8) | (blue_int[1] << 16);
		colour_out[2] = red_int[2] | (green_int[2] << 8) | (blue_int[2] << 16);
		colour_out[3] = red_int[3] | (green_int[3] << 8) | (blue_int[3] << 16);
	}

	float4_ u_table[4];
	float4_ v_table[4];


	{
		const vertex4_* gradients = shader_input.gradients[ATTRIBUTE_TEXCOORD];

		__m128 u_axis[4];
		u_axis[0] = (gradients[U].x * w_clip[0][0]) + (gradients[U].y * w_clip[1][0]) + gradients[U].z;
		u_axis[1] = (gradients[U].x * w_clip[0][1]) + (gradients[U].y * w_clip[1][1]) + gradients[U].z;
		u_axis[2] = (gradients[U].x * w_clip[0][2]) + (gradients[U].y * w_clip[1][2]) + gradients[U].z;
		u_axis[3] = (gradients[U].x * w_clip[0][3]) + (gradients[U].y * w_clip[1][3]) + gradients[U].z;

		__m128 v_axis[4];
		v_axis[0] = (gradients[V].x * w_clip[0][0]) + (gradients[V].y * w_clip[1][0]) + gradients[V].z;
		v_axis[1] = (gradients[V].x * w_clip[0][1]) + (gradients[V].y * w_clip[1][1]) + gradients[V].z;
		v_axis[2] = (gradients[V].x * w_clip[0][2]) + (gradients[V].y * w_clip[1][2]) + gradients[V].z;
		v_axis[3] = (gradients[V].x * w_clip[0][3]) + (gradients[V].y * w_clip[1][3]) + gradients[V].z;

		store_u(u_axis[0], u_table[0].f);
		store_u(u_axis[1], u_table[1].f);
		store_u(u_axis[2], u_table[2].f);
		store_u(u_axis[3], u_table[3].f);

		store_u(v_axis[0], v_table[0].f);
		store_u(v_axis[1], v_table[1].f);
		store_u(v_axis[2], v_table[2].f);
		store_u(v_axis[3], v_table[3].f);
	}

	const texture_handler_& texture_handler = *shader_input.texture_handler;

	float2_ du;
	du.x = (u_table[0].f[3] - u_table[0].f[0]) * (float)texture_handler.width;
	du.y = (u_table[3].f[0] - u_table[0].f[0]) * (float)texture_handler.width;

	float2_ dv;
	dv.x = (v_table[0].f[3] - v_table[0].f[0]) * (float)texture_handler.height;
	dv.y = (v_table[3].f[0] - v_table[0].f[0]) * (float)texture_handler.height;

	float area = abs((du.x * dv.y) - (du.y * dv.x))  * shader_input.mip_level_bias;
	unsigned long area_int = 1 + (unsigned long)(area + 0.5f);
	__int32 i_mip_floor;
	_BitScanReverse((unsigned long*)&i_mip_floor, area_int);

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

	__m128i tex_out[4];
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

		tex_out[0] = load_u(texels_out[0]);
		tex_out[1] = load_u(texels_out[1]);
		tex_out[2] = load_u(texels_out[2]);
		tex_out[3] = load_u(texels_out[3]);
	}

	__m128i colour_buffer[4];
	colour_buffer[0] = load(shader_input.colour_buffer + i_buffer + 0);
	colour_buffer[1] = load(shader_input.colour_buffer + i_buffer + 4);
	colour_buffer[2] = load(shader_input.colour_buffer + i_buffer + 8);
	colour_buffer[3] = load(shader_input.colour_buffer + i_buffer + 12);

	colour_buffer[0] = _mm_andnot_si128(z_mask[0], colour_buffer[0]);
	colour_buffer[1] = _mm_andnot_si128(z_mask[1], colour_buffer[1]);
	colour_buffer[2] = _mm_andnot_si128(z_mask[2], colour_buffer[2]);
	colour_buffer[3] = _mm_andnot_si128(z_mask[3], colour_buffer[3]);

	colour_buffer[0] = add_uint8_saturate(colour_buffer[0], colour_out[0] & z_mask[0]);
	colour_buffer[1] = add_uint8_saturate(colour_buffer[1], colour_out[1] & z_mask[1]);
	colour_buffer[2] = add_uint8_saturate(colour_buffer[2], colour_out[2] & z_mask[2]);
	colour_buffer[3] = add_uint8_saturate(colour_buffer[3], colour_out[3] & z_mask[3]);

	colour_buffer[0] = add_uint8_saturate(colour_buffer[0], tex_out[0] & z_mask[0]);
	colour_buffer[1] = add_uint8_saturate(colour_buffer[1], tex_out[1] & z_mask[1]);
	colour_buffer[2] = add_uint8_saturate(colour_buffer[2], tex_out[2] & z_mask[2]);
	colour_buffer[3] = add_uint8_saturate(colour_buffer[3], tex_out[3] & z_mask[3]);

	store(colour_buffer[0], shader_input.colour_buffer + i_buffer + 0);
	store(colour_buffer[1], shader_input.colour_buffer + i_buffer + 4);
	store(colour_buffer[2], shader_input.colour_buffer + i_buffer + 8);
	store(colour_buffer[3], shader_input.colour_buffer + i_buffer + 12);
}

/*
==================
==================
*/
void Process_Fragment_4x4(

	__int32 w_seed[2],
	__int32 i_tile_in,
	__int32 i_buffer_in,
	const unsigned __int32 coverage_mask,
	raster_output_& raster_output,
	shader_input_& shader_input
) {

	const __int32 i_buffer = i_buffer_in + (i_tile_in * 4 * 4);

	__m128i bazza[3][4];

	for (__int32 i_edge = 0; i_edge < 2; i_edge++) {
		__m128i w_row = set_all(w_seed[i_edge]);
		bazza[i_edge][0] = w_row + load_u(raster_output.reject_table[0][i_edge][0]);
		bazza[i_edge][1] = w_row + load_u(raster_output.reject_table[0][i_edge][1]);
		bazza[i_edge][2] = w_row + load_u(raster_output.reject_table[0][i_edge][2]);
		bazza[i_edge][3] = w_row + load_u(raster_output.reject_table[0][i_edge][3]);
	}

	pixel_shader(i_buffer, coverage_mask, bazza, shader_input);

	const __int32 i_buffer_depth_4x4 = i_buffer / (4 * 4);
	const __int32 i_buffer_depth_16x16 = i_buffer / (16 * 16);
	const __int32 i_buffer_depth_64x64 = i_buffer / (64 * 64);
	shader_input.depth_tiles_4x4[i_buffer_depth_4x4] = shader_input.z_max;
	shader_input.tile_mask_16x16 |= one_bit_64 << i_buffer_depth_16x16;
	shader_input.tile_mask_64x64 |= one_bit_64 << i_buffer_depth_64x64;
}

/*
==================
==================
*/
void Process_Fragment_16x16(

	__int32 w_seed[2],
	__int32 i_tile_in,
	__int32 i_buffer_in,
	const unsigned __int32 coverage_mask,
	raster_output_& raster_output,
	shader_input_& shader_input
) {

	const __int32 i_buffer = i_buffer_in + (i_tile_in * 16 * 16);

	__int32 w_table[2][4 * 4];
	for (__int32 i_edge = 0; i_edge < 2; i_edge++) {
		__m128i temp[4];

		__m128i w_row = set_all(w_seed[i_edge]);
		temp[0] = w_row + load_u(raster_output.reject_table[1][i_edge][0]);
		temp[1] = w_row + load_u(raster_output.reject_table[1][i_edge][1]);
		temp[2] = w_row + load_u(raster_output.reject_table[1][i_edge][2]);
		temp[3] = w_row + load_u(raster_output.reject_table[1][i_edge][3]);

		store_u(temp[0], w_table[i_edge] + (0 << 2));
		store_u(temp[1], w_table[i_edge] + (1 << 2));
		store_u(temp[2], w_table[i_edge] + (2 << 2));
		store_u(temp[3], w_table[i_edge] + (3 << 2));
	}

	for (__int32 i_tile = 0; i_tile < 16; i_tile++) {

		__int32 w_tile[2];
		w_tile[0] = w_table[0][i_tile];
		w_tile[1] = w_table[1][i_tile];

		Process_Fragment_4x4(w_tile, i_tile, i_buffer, coverage_mask, raster_output, shader_input);
	}
}

/*
==================
==================
*/
void Process_Fragment_64x64(

	__int32 w_seed[2],
	__int32 i_buffer_in,
	const unsigned __int32 coverage_mask,
	raster_output_& raster_output,
	shader_input_& shader_input
) {

	__int32 w_table[2][4 * 4];
	for (__int32 i_edge = 0; i_edge < 2; i_edge++) {
		__m128i temp[4];

		__m128i w_row = set_all(w_seed[i_edge]);
		temp[0] = w_row + load_u(raster_output.reject_table[2][i_edge][0]);
		temp[1] = w_row + load_u(raster_output.reject_table[2][i_edge][1]);
		temp[2] = w_row + load_u(raster_output.reject_table[2][i_edge][2]);
		temp[3] = w_row + load_u(raster_output.reject_table[2][i_edge][3]);

		store_u(temp[0], w_table[i_edge] + (0 << 2));
		store_u(temp[1], w_table[i_edge] + (1 << 2));
		store_u(temp[2], w_table[i_edge] + (2 << 2));
		store_u(temp[3], w_table[i_edge] + (3 << 2));
	}

	for (__int32 i_tile = 0; i_tile < 16; i_tile++) {

		__int32 w_tile[2];
		w_tile[0] = w_table[0][i_tile];
		w_tile[1] = w_table[1][i_tile];

		Process_Fragment_16x16(

			w_tile,
			i_tile,
			i_buffer_in,
			coverage_mask,
			raster_output,
			shader_input
		);
	}
}



/*
==================
==================
*/
void Process_Fragments(

	raster_output_& raster_output,
	shader_input_& shader_input
) {

	const __m128 zero = set_all(0.0f);

	shader_input.tile_mask_16x16 = 0x0;
	shader_input.tile_mask_64x64 = 0x0;

	//===============================================================================================

	{
		const __int32 n_fragments = raster_output.n_fragments[raster_output_::TRIVIAL_ACCEPT_64x64];
		for (__int32 i_fragment = 0; i_fragment < n_fragments; i_fragment++) {

			raster_fragment_& raster_fragment = raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT_64x64][i_fragment];

			const __int32 i_buffer = raster_fragment.buffer_mask_packed >> 16;
			const unsigned __int32 coverage_mask = raster_fragment.buffer_mask_packed & 0xffff;

			Process_Fragment_64x64(

				raster_fragment.w,
				i_buffer,
				coverage_mask,
				raster_output,
				shader_input
			);
		}
	}
	//===============================================================================================
	{
		const __int32 n_fragments = raster_output.n_fragments[raster_output_::TRIVIAL_ACCEPT_16x16];
		for (__int32 i_fragment = 0; i_fragment < n_fragments; i_fragment++) {

			raster_fragment_& raster_fragment = raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT_16x16][i_fragment];

			const __int32 i_buffer = raster_fragment.buffer_mask_packed >> 16;
			const unsigned __int32 coverage_mask = raster_fragment.buffer_mask_packed & 0xffff;

			Process_Fragment_16x16(

				raster_fragment.w,
				0,
				i_buffer,
				coverage_mask,
				raster_output,
				shader_input
			);
		}
	}
	//===============================================================================================
	{

		const __int32 n_fragments = raster_output.n_fragments[raster_output_::TRIVIAL_ACCEPT_4x4];
		for (__int32 i_fragment = 0; i_fragment < n_fragments; i_fragment++) {

			raster_fragment_& raster_fragment = raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT_4x4][i_fragment];
			const __int32 i_buffer = raster_fragment.buffer_mask_packed >> 16;
			const unsigned __int32 coverage_mask = raster_fragment.buffer_mask_packed & 0xffff;
			Process_Fragment_4x4(raster_fragment.w, 0, i_buffer, coverage_mask, raster_output, shader_input);
		}
	}
	//===============================================================================================
	{
		//const __int32 start = raster_output_::MAX_FRAGMENTS - 1;
		//const __int32 end = raster_output.n_fragments[raster_output_::PARTIAL_ACCEPT_4x4];
		//for (__int32 i_fragment = start; i_fragment > end; i_fragment--) {


		//	raster_fragment_& raster_fragment = raster_output.raster_fragment[raster_output_::PARTIAL_ACCEPT_4x4][i_fragment];
		//	const __int32 i_buffer = raster_fragment.buffer_mask_packed >> 16;
		//	const unsigned __int32 coverage_mask = raster_fragment.buffer_mask_packed & 0xffff;
		//	Process_Fragment_4x4(raster_fragment.w, 0, i_buffer, coverage_mask, raster_output, shader_input);
		//}
	}
	//===============================================================================================
	{
		const __int32 n_fragments = raster_output.n_fragments_COMPLETE;
		__int32 n_depth_fragments = 0;
		for (__int32 i_fragment = 0; i_fragment < n_fragments; i_fragment++) {

			raster_fragment_complete_& raster_fragment = raster_output.raster_fragment_complete[i_fragment];
			const __int32 i_buffer = raster_fragment.buffer_mask_packed >> 16;
			const unsigned __int32 coverage_mask = raster_fragment.buffer_mask_packed & 0xffff;

			pixel_shader(i_buffer, coverage_mask, raster_fragment.bazza, shader_input);

			const __int32 i_buffer_depth_4x4 = i_buffer / (4 * 4);
			const __int32 i_buffer_depth_16x16 = i_buffer / (16 * 16);
			const __int32 i_buffer_depth_64x64 = i_buffer / (64 * 64);
			shader_input.depth_tiles_4x4[i_buffer_depth_4x4] = shader_input.z_max;
			shader_input.tile_mask_16x16 |= one_bit_64 << i_buffer_depth_16x16;
			shader_input.tile_mask_64x64 |= one_bit_64 << i_buffer_depth_64x64;
		}
	}
	//===============================================================================================
	{
		//printf_s(" %llu ", shader_input.tile_mask_16x16);

		__int64 n_tiles = _mm_popcnt_u64(shader_input.tile_mask_16x16);

		for (__int32 i_bit = 0; i_bit < n_tiles; i_bit++) {

			unsigned long i_tile_16x16;
			_BitScanForward64(&i_tile_16x16, shader_input.tile_mask_16x16);
			shader_input.tile_mask_16x16 ^= one_bit_64 << i_tile_16x16;

			const __int32 i_tile_4x4 = i_tile_16x16 * (4 * 4);

			__m128 depth_4x4[4];
			depth_4x4[0] = load_u(shader_input.depth_tiles_4x4 + i_tile_4x4 + (0 * 4));
			depth_4x4[1] = load_u(shader_input.depth_tiles_4x4 + i_tile_4x4 + (1 * 4));
			depth_4x4[2] = load_u(shader_input.depth_tiles_4x4 + i_tile_4x4 + (2 * 4));
			depth_4x4[3] = load_u(shader_input.depth_tiles_4x4 + i_tile_4x4 + (3 * 4));

			__m128 z_max;
			z_max = depth_4x4[0];
			z_max = min_vec(depth_4x4[1], z_max);
			z_max = min_vec(depth_4x4[2], z_max);
			z_max = min_vec(depth_4x4[3], z_max);

			__m128 z_out = z_max;
			z_max = rotate_left(z_max);
			z_out = min_vec(z_max, z_out);
			z_max = rotate_left(z_max);
			z_out = min_vec(z_max, z_out);
			z_max = rotate_left(z_max);
			z_out = min_vec(z_max, z_out);

			shader_input.depth_tiles_16x16[i_tile_16x16] = store_s(z_out);
		}
	}
	{
		__int64 n_tiles = _mm_popcnt_u64(shader_input.tile_mask_64x64);

		//printf_s(" %llu ", n_tiles);

		for (__int32 i_bit = 0; i_bit < n_tiles; i_bit++) {

			unsigned long i_tile_64x64;
			_BitScanForward64(&i_tile_64x64, shader_input.tile_mask_64x64);
			shader_input.tile_mask_64x64 ^= one_bit_64 << i_tile_64x64;

			const __int32 i_tile_16x16 = i_tile_64x64 * (4 * 4);

			__m128 depth_16x16[4];
			depth_16x16[0] = load_u(shader_input.depth_tiles_16x16 + i_tile_16x16 + (0 * 4));
			depth_16x16[1] = load_u(shader_input.depth_tiles_16x16 + i_tile_16x16 + (1 * 4));
			depth_16x16[2] = load_u(shader_input.depth_tiles_16x16 + i_tile_16x16 + (2 * 4));
			depth_16x16[3] = load_u(shader_input.depth_tiles_16x16 + i_tile_16x16 + (3 * 4));

			__m128 z_max;
			z_max = depth_16x16[0];
			z_max = min_vec(depth_16x16[1], z_max);
			z_max = min_vec(depth_16x16[2], z_max);
			z_max = min_vec(depth_16x16[3], z_max);

			__m128 z_out = z_max;
			z_max = rotate_left(z_max);
			z_out = min_vec(z_max, z_out);
			z_max = rotate_left(z_max);
			z_out = min_vec(z_max, z_out);
			z_max = rotate_left(z_max);
			z_out = min_vec(z_max, z_out);

			shader_input.depth_tiles_64x64[i_tile_64x64] = store_s(z_out);
		}
	}
}


/*
==================
==================
*/
void Vertex_Lighting_NULL(

	const vertex_light_manager_& vertex_light_manager,
	const float3_ position[3],
	float4_ colour[3]

) {

}

/*
==================
==================
*/
void Vertex_Lighting_PLAYER(

	const vertex_light_manager_& vertex_light_manager,
	const float3_ position[3],
	float4_ colour[3]

) {

	static const float r_screen_scale_x = 1.0f / screen_scale_x;
	static const float r_screen_scale_y = 1.0f / screen_scale_y;
	const float attenuation_factor = 800.0f;
	const float specular_scale = 100.0f;
	const float diffuse_scale = 20.0f;

	float3_ position_clip[3];

	for (__int32 i_vertex = 0; i_vertex < 3; i_vertex++) {

		float depth = 1.0f / position[i_vertex].z;
		position_clip[i_vertex].x = ((position[i_vertex].x - screen_shift_x) * r_screen_scale_x) * depth;
		position_clip[i_vertex].y = ((position[i_vertex].y - screen_shift_y) * r_screen_scale_y) * depth;
		position_clip[i_vertex].z = depth;
	}

	float3_ a;
	a.x = position_clip[1].x - position_clip[0].x;
	a.y = position_clip[1].y - position_clip[0].y;
	a.z = position_clip[1].z - position_clip[0].z;

	float3_ b;
	b.x = position_clip[2].x - position_clip[0].x;
	b.y = position_clip[2].y - position_clip[0].y;
	b.z = position_clip[2].z - position_clip[0].z;

	float3_ normal;
	normal.x = (a.y * b.z) - (a.z * b.y);
	normal.y = (a.z * b.x) - (a.x * b.z);
	normal.z = (a.x * b.y) - (a.y * b.x);

	float d = (normal.x * normal.x) + (normal.y * normal.y) + (normal.z * normal.z);
	float mag = store_s(_mm_rsqrt_ss(load_s(d)));
	normal.x *= mag;
	normal.y *= mag;
	normal.z *= mag;

	float3_ light_position = { 0.0f, 0.0f, 0.0f };
	float3_ light_colour = { 100.0f, 100.0f, 10.0f };

	for (__int32 i_vertex = 0; i_vertex < 3; i_vertex++) {

		float3_ light_ray = position_clip[i_vertex] - light_position;

		float d = (light_ray.x * light_ray.x) + (light_ray.y * light_ray.y) + (light_ray.z * light_ray.z);
		float mag = store_s(_mm_rsqrt_ss(load_s(d)));
		light_ray.x *= mag;
		light_ray.y *= mag;
		light_ray.z *= mag;

		float dot = (normal.x * light_ray.x) + (normal.y * light_ray.y) + (normal.z * light_ray.z);
		dot = dot > 0.0f ? dot : 0.0f;
		dot = (dot * dot) * mag;

		colour[i_vertex].x += dot * specular_scale * light_colour.r;
		colour[i_vertex].y += dot * specular_scale * light_colour.g;
		colour[i_vertex].z += dot * specular_scale * light_colour.b;

		colour[i_vertex].x += mag * diffuse_scale * light_colour.r;
		colour[i_vertex].y += mag * diffuse_scale * light_colour.g;
		colour[i_vertex].z += mag * diffuse_scale * light_colour.b;
	}
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

	const command_buffer_& render_buffer,
	const draw_call_& draw_call,
	const __int32 i_model,
	float4_ vertices[3]

) {

	for (__int32 i_vertex = 0; i_vertex < NUM_VERTICES; i_vertex++) {

		vertices[i_vertex].x += draw_call.colour[i_model].r;
		vertices[i_vertex].y += draw_call.colour[i_model].g;
		vertices[i_vertex].z += draw_call.colour[i_model].b;
	}
}

/*
==================
==================
*/
void Shade_Vertex_Colour(

	const command_buffer_& render_buffer,
	const draw_call_& draw_call,
	const __int32 i_model,
	float4_ vertices[3]

) {

	static const __m128 max = set_all(255.0f);
	static const __m128 min = set_all(0.0f);

	for (__int32 i_vertex = 0; i_vertex < NUM_VERTICES; i_vertex++) {
		Vector_X_Matrix(vertices[i_vertex], draw_call.m_vertex_colour[i_model], vertices[i_vertex]);

		float3_ colour = draw_call.colour[i_model];

		vertices[i_vertex].x = colour.r * vertices[i_vertex].z;
		vertices[i_vertex].y = colour.g * vertices[i_vertex].z;
		vertices[i_vertex].z = colour.b * vertices[i_vertex].z;

		//__m128 clamp = load_u(vertices[i_vertex].f);
		//clamp = min_vec(max_vec(clamp, min), max);
		//store_u(clamp, vertices[i_vertex].f);
	}
	

	
}
/*
==================
==================
*/
void Shade_Vertex_Texture(

	const command_buffer_& render_buffer,
	const draw_call_& draw_call,
	const __int32 i_model,
	float4_ vertices[3]

) {

	for (__int32 i_vertex = 0; i_vertex < NUM_VERTICES; i_vertex++) {
		Vector_X_Matrix(vertices[i_vertex], draw_call.m_vertex_texture[i_model], vertices[i_vertex]);
	}
}
