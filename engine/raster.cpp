#include "raster.h"
#include "vector.h"
#include "render_front.h"
#include "setup.h"


enum {

	//NUM_BAZZAS = 2,
	NUM_EDGES = 3,

};

/*
==================
==================
*/
__int32 encode_morton(__int32 x, __int32 y) {

	x = (x | (x << 8)) & 0x00ff00ff;
	x = (x | (x << 4)) & 0x0f0f0f0f;
	x = (x | (x << 2)) & 0x33333333;

	y = (y | (y << 8)) & 0x00ff00ff;
	y = (y | (y << 4)) & 0x0f0f0f0f;
	y = (y | (y << 2)) & 0x33333333;

	return x | (y << 2);
}

/*
==================
==================
*/
__int32 encode_morton_x(__int32 x) {

	x = (x | (x << 8)) & 0x00ff00ff;
	x = (x | (x << 4)) & 0x0f0f0f0f;
	x = (x | (x << 2)) & 0x33333333;

	return x;;
}

/*
==================
==================
*/
__int32 encode_morton_y(__int32 y) {

	y = (y | (y << 8)) & 0x00ff00ff;
	y = (y | (y << 4)) & 0x0f0f0f0f;
	y = (y | (y << 2)) & 0x33333333;

	return y << 2;
}

/*
==================
==================
*/
void decode_morton(const __int32 z, __int32& x, __int32& y) {

	x = z;
	x = x & 0x33333333;
	x = (x | (x >> 2)) & 0x0f0f0f0f;
	x = (x | (x >> 4)) & 0x00ff00ff;
	x = (x | (x >> 8)) & 0x0000ffff;

	y = z >> 2;
	y = y & 0x33333333;
	y = (y | (y >> 2)) & 0x0f0f0f0f;
	y = (y | (y >> 4)) & 0x00ff00ff;
	y = (y | (y >> 8)) & 0x0000ffff;
}

/*
==================
==================
*/
void Rasteriser_4x4(

	raster_data_& raster_data,
	raster_output_& raster_output

) {
	static const __int64 half_fixed = 1 << (FIXED_POINT_SHIFT - 1);
	//static const __int64 one_fixed = 1 << FIXED_POINT_SHIFT;

	__int32 tile_min = 0 - display_::TILE_SIZE;
	__int32 tile_max = display_::TILE_SIZE - 1;

	raster_data.bb_min.x = max(raster_data.bb_min.x, tile_min);
	raster_data.bb_min.y = max(raster_data.bb_min.y, tile_min);
	raster_data.bb_max.x = min(raster_data.bb_max.x, tile_max);
	raster_data.bb_max.y = min(raster_data.bb_max.y, tile_max);

	const unsigned __int32 floor_4 = ~(4 - 1);

	int2_ bb_max_4;
	bb_max_4.x = raster_data.bb_max.x & floor_4;
	bb_max_4.y = raster_data.bb_max.y & floor_4;

	int2_ bb_min_4;
	bb_min_4.x = raster_data.bb_min.x & floor_4;
	bb_min_4.y = raster_data.bb_min.y & floor_4;

	__m128i dx_4[3];
	dx_4[0] = set_all((__int32)raster_data.dx[0]);
	dx_4[1] = set_all((__int32)raster_data.dx[1]);
	dx_4[2] = set_all((__int32)raster_data.dx[2]);

	__m128i dy_4[3];
	dy_4[0] = set_all((__int32)raster_data.dy[0]);
	dy_4[1] = set_all((__int32)raster_data.dy[1]);
	dy_4[2] = set_all((__int32)raster_data.dy[2]);

	__m128i vx_4[3];
	vx_4[0] = set_all((__int32)raster_data.v[0].x);
	vx_4[1] = set_all((__int32)raster_data.v[1].x);
	vx_4[2] = set_all((__int32)raster_data.v[2].x);

	__m128i vy_4[3];
	vy_4[0] = set_all((__int32)raster_data.v[0].y);
	vy_4[1] = set_all((__int32)raster_data.v[1].y);
	vy_4[2] = set_all((__int32)raster_data.v[2].y);

	__m128i bias_4[3];
	bias_4[0] = set_all((__int32)raster_data.bias[0]);
	bias_4[1] = set_all((__int32)raster_data.bias[1]);
	bias_4[2] = set_all((__int32)raster_data.bias[2]);

	static const __m128i x_series = set(0, 1, 2, 3);
	const unsigned __int32 mod_tile = display_::TILE_SIZE - 1;
	const __m128i one_fxp = set_all(1 << FIXED_POINT_SHIFT);
	const __m128i half_fxp = set_all((__int32)half_fixed);

	for (__int32 y_4 = bb_min_4.y; y_4 <= bb_max_4.y; y_4 += 4) {


		for (__int32 x_4 = bb_min_4.x; x_4 <= bb_max_4.x; x_4 += 4) {

			__int32& n_fragments = raster_output.n_fragments_COMPLETE;
			raster_fragment_complete_& raster_fragment_complete = raster_output.raster_fragment_complete[n_fragments];

			const __int32 y_4_mapped = y_4 + display_::TILE_SIZE;
			const __int32 x_4_mapped = x_4 + display_::TILE_SIZE;
			__int32 i_block = encode_morton(x_4_mapped & mod_tile, y_4_mapped & mod_tile);
			const __int32 x_tile = x_4_mapped / display_::TILE_SIZE;
			const __int32 y_tile = y_4_mapped / display_::TILE_SIZE;
			const __int32 i_tile = (y_tile * (display_::TILE_SIZE * display_::BIN_SIZE)) + (x_tile * display_::TILE_SIZE * display_::TILE_SIZE);

			__int64 y_fixed = (y_4 << FIXED_POINT_SHIFT) + half_fixed;
			__m128i y_fxp = set_all((__int32)y_fixed);
			__m128i x_fxp = set_all(x_4) + x_series;
			x_fxp <<= FIXED_POINT_SHIFT;
			x_fxp += half_fxp;

			unsigned __int32 draw_mask = 0x0;
			__int32 shift = 0;
			__int32 i_temp = 0;

			for (__int32 y = y_4; y < y_4 + 4; y++) {

				raster_fragment_complete.bazza[0][i_temp] = dx_4[0] * (y_fxp - vy_4[1]) - dy_4[0] * (x_fxp - vx_4[1]);
				raster_fragment_complete.bazza[1][i_temp] = dx_4[1] * (y_fxp - vy_4[2]) - dy_4[1] * (x_fxp - vx_4[2]);
				raster_fragment_complete.bazza[2][i_temp] = dx_4[2] * (y_fxp - vy_4[0]) - dy_4[2] * (x_fxp - vx_4[0]);

				raster_fragment_complete.bazza[0][i_temp] += bias_4[0];
				raster_fragment_complete.bazza[1][i_temp] += bias_4[1];
				raster_fragment_complete.bazza[2][i_temp] += bias_4[2];

				unsigned __int32 temp_mask = store_mask(raster_fragment_complete.bazza[0][i_temp]) & store_mask(raster_fragment_complete.bazza[1][i_temp]) & store_mask(raster_fragment_complete.bazza[2][i_temp]);
				draw_mask |= temp_mask << shift;

				raster_fragment_complete.bazza[0][i_temp] >>= FIXED_POINT_SHIFT;
				raster_fragment_complete.bazza[1][i_temp] >>= FIXED_POINT_SHIFT;
				raster_fragment_complete.bazza[2][i_temp] >>= FIXED_POINT_SHIFT;

				y_fxp += one_fxp;
				shift += 4;
				i_temp++;
			}

			raster_output.raster_fragment_complete[n_fragments].coverage_mask = draw_mask;
			raster_output.raster_fragment_complete[n_fragments].i_buffer = i_tile + i_block;
			//n_fragments++;
			n_fragments += draw_mask != 0x0;
		}
	}
}



/*
==================
==================
*/
//void emit_4x4(
//
//	const __int32 i_buffer_in,
//	const __int32 i_tile_in,
//	__int32 w_in[3][16],
//	__int32 reject_table[3][3][4][4],
//	raster_output_& raster_output
//
//) {
//
//	const __int32 i_buffer = i_buffer_in + (i_tile_in * 4 * 4);
//	__int32& n_fragments = raster_output.n_fragments[raster_output_::TRIVIAL_ACCEPT];
//	raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT][n_fragments].coverage_mask = 0x0;
//	raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT][n_fragments].i_buffer = i_buffer;
//	raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT][n_fragments].w[0] = w_in[0][i_tile_in];
//	raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT][n_fragments].w[1] = w_in[1][i_tile_in];
//	n_fragments++;
//}


/*
==================
==================
*/
void emit_16x16(

	const __int32 i_buffer_in,
	const __int32 i_tile_in,
	__int32 w_in[3][16],
	__int32 reject_table[3][3][4][4],
	raster_output_& raster_output

) {


	__int32& n_fragments = raster_output.n_fragments[raster_output_::TRIVIAL_ACCEPT_16];
	//raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT_16][n_fragments].coverage_mask = draw_mask;
	raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT_16][n_fragments].i_buffer = i_buffer_in + (i_tile_in * 16 * 16);
	raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT_16][n_fragments].w[0] = w_in[0][i_tile_in];
	raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT_16][n_fragments].w[1] = w_in[1][i_tile_in];
	n_fragments++;

	//__int32 w_table[3][4 * 4];

	//for (__int32 i_edge = 0; i_edge < NUM_EDGES; i_edge++) {
	//	__m128i w_row = set_all(w_in[i_edge][i_tile_in]);
	//	__m128i temp[4];
	//	temp[0] = w_row + load_u(reject_table[1][i_edge][0]);
	//	temp[1] = w_row + load_u(reject_table[1][i_edge][1]);
	//	temp[2] = w_row + load_u(reject_table[1][i_edge][2]);
	//	temp[3] = w_row + load_u(reject_table[1][i_edge][3]);

	//	store_u(temp[0], w_table[i_edge] + (0 << 2));
	//	store_u(temp[1], w_table[i_edge] + (1 << 2));
	//	store_u(temp[2], w_table[i_edge] + (2 << 2));
	//	store_u(temp[3], w_table[i_edge] + (3 << 2));
	//}
	//const __int32 i_buffer = i_buffer_in + (i_tile_in * 16 * 16);
	//for (__int32 i_tile = 0; i_tile < 16; i_tile++) {

	//	//emit_4x4(i_buffer, i_tile, w_table, reject_table, raster_output);

	//	__int32& n_fragments = raster_output.n_fragments[raster_output_::TRIVIAL_ACCEPT];
	//	raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT][n_fragments].coverage_mask = 0x0;
	//	raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT][n_fragments].i_buffer = i_buffer + (i_tile * 4 * 4);
	//	raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT][n_fragments].w[0] = w_table[0][i_tile];
	//	raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT][n_fragments].w[1] = w_table[1][i_tile];
	//	n_fragments++;
	//}
}

/*
==================
==================
*/
void emit_64x64(

	__int32 i_buffer_in,
	__int32 w_in[3],
	__int32 reject_table[3][3][4][4],
	raster_output_& raster_output

) {

	__int32& n_fragments = raster_output.n_fragments[raster_output_::TRIVIAL_ACCEPT_64];
	//raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT_16][n_fragments].coverage_mask = draw_mask;
	raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT_64][n_fragments].i_buffer = i_buffer_in;
	raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT_64][n_fragments].w[0] = w_in[0];
	raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT_64][n_fragments].w[1] = w_in[1];
	n_fragments++;

	//__int32 w_table[3][4 * 4];

	//for (__int32 i_edge = 0; i_edge < NUM_EDGES; i_edge++) {
	//	__m128i w_row = set_all(w_in[i_edge]);
	//	__m128i temp[4];
	//	temp[0] = w_row + load_u(reject_table[2][i_edge][0]);
	//	temp[1] = w_row + load_u(reject_table[2][i_edge][1]);
	//	temp[2] = w_row + load_u(reject_table[2][i_edge][2]);
	//	temp[3] = w_row + load_u(reject_table[2][i_edge][3]);

	//	store_u(temp[0], w_table[i_edge] + (0 << 2));
	//	store_u(temp[1], w_table[i_edge] + (1 << 2));
	//	store_u(temp[2], w_table[i_edge] + (2 << 2));
	//	store_u(temp[3], w_table[i_edge] + (3 << 2));
	//}
	//for (__int32 i_tile = 0; i_tile < 16; i_tile++) {
	//	emit_16x16(i_buffer_in, i_tile, w_table, reject_table, raster_output);
	//}
}

/*
==================
==================
*/
void process_4x4(

	const __int32 i_buffer_in,
	const __int32 i_tile_in,
	__int32 w_in[3][16],
	__int32 reject_table[3][3][4][4],
	raster_output_& raster_output

) {

	__m128i bazza[3][4];

	unsigned __int32 draw_mask = -1;

	for (__int32 i_edge = 0; i_edge < NUM_EDGES; i_edge++) {

		unsigned __int32 temp_mask = 0x0;
		__m128i w_row = set_all(w_in[i_edge][i_tile_in]);
		bazza[i_edge][0] = w_row + load_u(reject_table[0][i_edge][0]);
		bazza[i_edge][1] = w_row + load_u(reject_table[0][i_edge][1]);
		bazza[i_edge][2] = w_row + load_u(reject_table[0][i_edge][2]);
		bazza[i_edge][3] = w_row + load_u(reject_table[0][i_edge][3]);

		temp_mask |= store_mask(bazza[i_edge][0]) << (0 << 2);
		temp_mask |= store_mask(bazza[i_edge][1]) << (1 << 2);
		temp_mask |= store_mask(bazza[i_edge][2]) << (2 << 2);
		temp_mask |= store_mask(bazza[i_edge][3]) << (3 << 2);

		draw_mask &= temp_mask;
	}
	const __int32 i_buffer = i_buffer_in + (i_tile_in * 4 * 4);

	__int32& n_fragments = raster_output.n_fragments[raster_output_::PARTIAL_ACCEPT];
	raster_output.raster_fragment[raster_output_::PARTIAL_ACCEPT][n_fragments].coverage_mask = draw_mask;
	raster_output.raster_fragment[raster_output_::PARTIAL_ACCEPT][n_fragments].i_buffer = i_buffer;
	raster_output.raster_fragment[raster_output_::PARTIAL_ACCEPT][n_fragments].w[0] = w_in[0][i_tile_in];
	raster_output.raster_fragment[raster_output_::PARTIAL_ACCEPT][n_fragments].w[1] = w_in[1][i_tile_in];
	//n_fragments++;
	n_fragments += draw_mask != 0x0;
}

/*
==================
==================
*/
void process_16x16(

	const __int32 i_buffer_in,
	const __int32 i_tile_in,
	__int32 w_in[3][16],
	__int32 reject_table[3][3][4][4],
	__int32 accept_table[3][3][4][4],
	raster_output_& raster_output

) {

	__int32 w_table[3][4 * 4];
	unsigned __int32 accept_mask = -1;
	unsigned __int32 partial_accept_mask;
	{
		unsigned __int32 reject_mask = -1;

		for (__int32 i_edge = 0; i_edge < NUM_EDGES; i_edge++) {

			unsigned __int32 reject_edge_mask = 0x0;
			unsigned __int32 accept_edge_mask = 0x0;
			__m128i w_4 = set_all(w_in[i_edge][i_tile_in]);

			for (__int32 y = 0; y < 4; y++) {

				__m128i w_reject = w_4 + load_u(reject_table[1][i_edge][y]);
				store_u(w_reject, w_table[i_edge] + (y << 2));
				reject_edge_mask |= store_mask(w_reject) << (y << 2);
				__m128i w_accept = w_reject + load_u(accept_table[0][i_edge][y]);
				accept_edge_mask |= store_mask(w_accept) << (y << 2);
			}
			reject_mask &= reject_edge_mask;
			accept_mask &= accept_edge_mask;
		}
		partial_accept_mask = reject_mask ^ accept_mask;
	}
	const __int32 i_buffer = i_buffer_in + (i_tile_in * 16 * 16);
	{
		__int32 n_tiles = _mm_popcnt_u32(accept_mask);
		for (__int32 i_bit = 0; i_bit < n_tiles; i_bit++) {

			unsigned long i_tile;
			_BitScanForward(&i_tile, accept_mask);
			accept_mask ^= 0x1 << i_tile;

			//emit_4x4(i_buffer, i_tile, w_table, reject_table, raster_output);

			__int32& n_fragments = raster_output.n_fragments[raster_output_::TRIVIAL_ACCEPT_4];
			raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT_4][n_fragments].coverage_mask = 0x0;
			raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT_4][n_fragments].i_buffer = i_buffer + (i_tile * 4 * 4);
			raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT_4][n_fragments].w[0] = w_table[0][i_tile];
			raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT_4][n_fragments].w[1] = w_table[1][i_tile];
			n_fragments++;
		}
	}
	{
		__int32 n_tiles = _mm_popcnt_u32(partial_accept_mask);
		for (__int32 i_bit = 0; i_bit < n_tiles; i_bit++) {

			unsigned long i_tile;
			_BitScanForward(&i_tile, partial_accept_mask);
			partial_accept_mask ^= 0x1 << i_tile;

			//process_4x4(i_buffer, i_tile, w_table, reject_table, raster_output);

			__m128i bazza[3][4];

			unsigned __int32 draw_mask = -1;

			for (__int32 i_edge = 0; i_edge < NUM_EDGES; i_edge++) {

				unsigned __int32 temp_mask = 0x0;
				__m128i w_row = set_all(w_table[i_edge][i_tile]);
				bazza[i_edge][0] = w_row + load_u(reject_table[0][i_edge][0]);
				bazza[i_edge][1] = w_row + load_u(reject_table[0][i_edge][1]);
				bazza[i_edge][2] = w_row + load_u(reject_table[0][i_edge][2]);
				bazza[i_edge][3] = w_row + load_u(reject_table[0][i_edge][3]);

				temp_mask |= store_mask(bazza[i_edge][0]) << (0 << 2);
				temp_mask |= store_mask(bazza[i_edge][1]) << (1 << 2);
				temp_mask |= store_mask(bazza[i_edge][2]) << (2 << 2);
				temp_mask |= store_mask(bazza[i_edge][3]) << (3 << 2);

				draw_mask &= temp_mask;
			}

			__int32& n_fragments = raster_output.n_fragments[raster_output_::PARTIAL_ACCEPT];
			raster_output.raster_fragment[raster_output_::PARTIAL_ACCEPT][n_fragments].coverage_mask = draw_mask;
			raster_output.raster_fragment[raster_output_::PARTIAL_ACCEPT][n_fragments].i_buffer = i_buffer + (i_tile * 4 * 4);
			raster_output.raster_fragment[raster_output_::PARTIAL_ACCEPT][n_fragments].w[0] = w_table[0][i_tile];
			raster_output.raster_fragment[raster_output_::PARTIAL_ACCEPT][n_fragments].w[1] = w_table[1][i_tile];
			//n_fragments++;
			n_fragments -= draw_mask != 0x0;
		}
	}
}

/*
==================
==================
*/
void process_64x64(

	const __int32 i_buffer_in,
	__int32 w_in[3],
	__int32 reject_table[3][3][4][4],
	__int32 accept_table[3][3][4][4],
	raster_output_& raster_output

) {
	__int32 w_table[3][4 * 4];
	unsigned __int32 accept_mask = -1;
	unsigned __int32 partial_accept_mask;
	{
		unsigned __int32 reject_mask = -1;

		for (__int32 i_edge = 0; i_edge < NUM_EDGES; i_edge++) {

			unsigned __int32 reject_edge_mask = 0x0;
			unsigned __int32 accept_edge_mask = 0x0;
			__m128i w_4 = set_all(w_in[i_edge]);

			for (__int32 y = 0; y < 4; y++) {

				__m128i w_reject = w_4 + load_u(reject_table[2][i_edge][y]);
				store_u(w_reject, w_table[i_edge] + (y << 2));
				reject_edge_mask |= store_mask(w_reject) << (y << 2);
				__m128i w_accept = w_reject + load_u(accept_table[1][i_edge][y]);
				accept_edge_mask |= store_mask(w_accept) << (y << 2);
			}
			reject_mask &= reject_edge_mask;
			accept_mask &= accept_edge_mask;
		}
		partial_accept_mask = reject_mask ^ accept_mask;
	}
	{
		__int32 n_tiles = _mm_popcnt_u32(accept_mask);
		for (__int32 i_bit = 0; i_bit < n_tiles; i_bit++) {

			unsigned long i_tile;
			_BitScanForward(&i_tile, accept_mask);
			accept_mask ^= 0x1 << i_tile;

			emit_16x16(i_buffer_in, i_tile, w_table, reject_table, raster_output);
		}
	}
	{
		__int32 n_tiles = _mm_popcnt_u32(partial_accept_mask);
		for (__int32 i_bit = 0; i_bit < n_tiles; i_bit++) {

			unsigned long i_tile;
			_BitScanForward(&i_tile, partial_accept_mask);
			partial_accept_mask ^= 0x1 << i_tile;

			process_16x16(i_buffer_in, i_tile, w_table, reject_table, accept_table, raster_output);
		}
	}
}

/*
==================
==================
*/
void Raster_Setup(

	const float3_ position[3],
	const int2_& bin_origin,
	raster_data_& raster_data

) {

	__int64 origin_x = bin_origin.x * (1 << FIXED_POINT_SHIFT);
	__int64 origin_y = bin_origin.y * (1 << FIXED_POINT_SHIFT);

	raster_data.v[0].x = __int64(position[2].x * fixed_point_scale_g) - origin_x;
	raster_data.v[0].y = __int64(position[2].y * fixed_point_scale_g) - origin_y;

	raster_data.v[1].x = __int64(position[1].x * fixed_point_scale_g) - origin_x;
	raster_data.v[1].y = __int64(position[1].y * fixed_point_scale_g) - origin_y;

	raster_data.v[2].x = __int64(position[0].x * fixed_point_scale_g) - origin_x;
	raster_data.v[2].y = __int64(position[0].y * fixed_point_scale_g) - origin_y;

	raster_data.dx[0] = raster_data.v[1].x - raster_data.v[2].x;
	raster_data.dx[1] = raster_data.v[2].x - raster_data.v[0].x;
	raster_data.dx[2] = raster_data.v[0].x - raster_data.v[1].x;

	raster_data.dy[0] = raster_data.v[1].y - raster_data.v[2].y;
	raster_data.dy[1] = raster_data.v[2].y - raster_data.v[0].y;
	raster_data.dy[2] = raster_data.v[0].y - raster_data.v[1].y;

	raster_data.step_x[0] = -raster_data.dy[0];
	raster_data.step_x[1] = -raster_data.dy[1];
	raster_data.step_x[2] = -raster_data.dy[2];

	raster_data.step_y[0] = raster_data.dx[0];
	raster_data.step_y[1] = raster_data.dx[1];
	raster_data.step_y[2] = raster_data.dx[2];

	raster_data.bias[0] = ((raster_data.dy[0] == 0) && (raster_data.dx[0] < 0)) || (raster_data.dy[0] > 0) ? 0 : -1;
	raster_data.bias[1] = ((raster_data.dy[1] == 0) && (raster_data.dx[1] < 0)) || (raster_data.dy[1] > 0) ? 0 : -1;
	raster_data.bias[2] = ((raster_data.dy[2] == 0) && (raster_data.dx[2] < 0)) || (raster_data.dy[2] > 0) ? 0 : -1;

	int2_64_ max_fixed;
	int2_64_ min_fixed;

	max_fixed.x = min_fixed.x = raster_data.v[0].x;
	max_fixed.y = min_fixed.y = raster_data.v[0].y;

	max_fixed.x = max(max_fixed.x, raster_data.v[1].x);
	max_fixed.y = max(max_fixed.y, raster_data.v[1].y);
	min_fixed.x = min(min_fixed.x, raster_data.v[1].x);
	min_fixed.y = min(min_fixed.y, raster_data.v[1].y);

	max_fixed.x = max(max_fixed.x, raster_data.v[2].x);
	max_fixed.y = max(max_fixed.y, raster_data.v[2].y);
	min_fixed.x = min(min_fixed.x, raster_data.v[2].x);
	min_fixed.y = min(min_fixed.y, raster_data.v[2].y);

	// NOT SURE WHAT THIS WAS FOR???!
	//const __int64 sub_step = 16;
	//const __int64 sub_mask = sub_step - 1;
	//max_fixed.x = (max_fixed.x + sub_mask) & (~sub_mask);
	//max_fixed.y = (max_fixed.y + sub_mask) & (~sub_mask);

	raster_data.bb_max.x = __int32(max_fixed.x >> FIXED_POINT_SHIFT);
	raster_data.bb_max.y = __int32(max_fixed.y >> FIXED_POINT_SHIFT);
	raster_data.bb_min.x = __int32(min_fixed.x >> FIXED_POINT_SHIFT);
	raster_data.bb_min.y = __int32(min_fixed.y >> FIXED_POINT_SHIFT);
}

/*
==================
==================
*/
void Rasterise_Tile(

	const __int32 i_buffer_in,
	const int2_& tile_origin,
	raster_data_& raster_data,
	raster_output_& raster_output

) {

	int2_64_ reject_corner[3];
	int2_64_ accept_step[3];
	for (__int32 i_edge = 0; i_edge < 3; i_edge++) {

		accept_step[i_edge].x = raster_data.is_offset[i_edge][X] ? -display_::TILE_SIZE : display_::TILE_SIZE;
		accept_step[i_edge].y = raster_data.is_offset[i_edge][Y] ? -display_::TILE_SIZE : display_::TILE_SIZE;

		reject_corner[i_edge].x = tile_origin.x << FIXED_POINT_SHIFT;
		reject_corner[i_edge].y = tile_origin.y << FIXED_POINT_SHIFT;
		reject_corner[i_edge].x += raster_data.is_offset[i_edge][X] ? (display_::TILE_SIZE << FIXED_POINT_SHIFT) : 0;
		reject_corner[i_edge].y += raster_data.is_offset[i_edge][Y] ? (display_::TILE_SIZE << FIXED_POINT_SHIFT) : 0;
	}

	__int64 w_reject[3];
	w_reject[0] = raster_data.dx[0] * (reject_corner[0].y - raster_data.v[1].y) - raster_data.dy[0] * (reject_corner[0].x - raster_data.v[1].x);
	w_reject[1] = raster_data.dx[1] * (reject_corner[1].y - raster_data.v[2].y) - raster_data.dy[1] * (reject_corner[1].x - raster_data.v[2].x);
	w_reject[2] = raster_data.dx[2] * (reject_corner[2].y - raster_data.v[0].y) - raster_data.dy[2] * (reject_corner[2].x - raster_data.v[0].x);

	w_reject[0] >>= FIXED_POINT_SHIFT;
	w_reject[1] >>= FIXED_POINT_SHIFT;
	w_reject[2] >>= FIXED_POINT_SHIFT;

	unsigned __int32 reject_mask = (w_reject[0] < 0) & (w_reject[1] < 0) & (w_reject[2] < 0);

	if (reject_mask == 0x0) {

		return;
	}

	__int64 w_accept[3];
	w_accept[0] = w_reject[0] + (raster_data.step_x[0] * accept_step[0].x) + (raster_data.step_y[0] * accept_step[0].y);
	w_accept[1] = w_reject[1] + (raster_data.step_x[1] * accept_step[1].x) + (raster_data.step_y[1] * accept_step[1].y);
	w_accept[2] = w_reject[2] + (raster_data.step_x[2] * accept_step[2].x) + (raster_data.step_y[2] * accept_step[2].y);

	unsigned __int32 edge_accept_mask = 0x7;
	edge_accept_mask ^= ((w_accept[0] < 0) << 0);
	edge_accept_mask ^= ((w_accept[1] < 0) << 1);
	edge_accept_mask ^= ((w_accept[2] < 0) << 2);

	__int32 w_reject_32[3];
	w_reject_32[0] = (__int32)w_reject[0];
	w_reject_32[1] = (__int32)w_reject[1];
	w_reject_32[2] = (__int32)w_reject[2];


	if (edge_accept_mask == 0x0) {

		emit_64x64(i_buffer_in, w_reject_32, raster_output.reject_table, raster_output);

		return;
	}

	process_64x64(

		i_buffer_in,
		w_reject_32,
		raster_output.reject_table,
		raster_data.accept_table,
		raster_output
	);
}

/*
==================
==================
*/
void Rasteriser(

	raster_data_& raster_data,
	raster_output_& raster_output

) {

	raster_output.n_fragments[raster_output_::PARTIAL_ACCEPT] = raster_output_::MAX_FRAGMENTS - 1;
	raster_output.n_fragments[raster_output_::TRIVIAL_ACCEPT_4] = 0;
	raster_output.n_fragments[raster_output_::TRIVIAL_ACCEPT_16] = 0;
	raster_output.n_fragments[raster_output_::TRIVIAL_ACCEPT_64] = 0;
	raster_output.n_fragments_COMPLETE = 0;

	raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT_64] = raster_output.base_fragments;
	raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT_16] = raster_output.base_fragments + 4;
	raster_output.raster_fragment[raster_output_::TRIVIAL_ACCEPT_4] = raster_output.base_fragments + 4 + 64;
	//raster_output.raster_fragment[raster_output_::PARTIAL_ACCEPT] = raster_output.base_fragments + 4 + 64 + 256;
	raster_output.raster_fragment[raster_output_::PARTIAL_ACCEPT] = raster_output.base_fragments;

	bool is_small_tris = 
		((raster_data.bb_max.x - raster_data.bb_min.x) < 4) 
		&& ((raster_data.bb_max.y - raster_data.bb_min.y) < 4);

	if (is_small_tris) {

		Rasteriser_4x4(

			raster_data,
			raster_output
		);
		return;
	}

	static const __int32 x_series[2][4] = {
	{ 0, 1, 2, 3 },
	{ -3, -2, -1, 0 },
	};
	__m128i one = set_one_si128();

	for (__int32 i_edge = 0; i_edge < NUM_EDGES; i_edge++) {

		raster_data.is_offset[i_edge][X] = raster_data.dy[i_edge] > 0;
		raster_data.is_offset[i_edge][Y] = raster_data.dx[i_edge] < 0;

		__int32 y_base = raster_data.is_offset[i_edge][Y] ? -3 : 0;
		__int32 y_step = raster_data.is_offset[i_edge][Y] ? -1 : 1;
		__int32 x_index = raster_data.is_offset[i_edge][X] ? 1 : 0;
		__int32 x_step = raster_data.is_offset[i_edge][X] ? -1 : 1;

		__int32 temp = ((__int32)raster_data.step_x[i_edge] * x_step) + ((__int32)raster_data.step_y[i_edge] * y_step);
		__m128i accept_step = set_all(temp);

		__m128i step_x4 = set_all((__int32)raster_data.step_x[i_edge]);
		__m128i step_y4 = set_all((__int32)raster_data.step_y[i_edge]);
		__m128i x_reject = load_u(x_series[x_index]);
		__m128i y_reject = set_all(y_base);
		__m128i bias_4 = set_all((__int32)raster_data.bias[i_edge]);

		for (__int32 y = 0; y < 4; y++) {

			__m128i reject_step = (step_x4 * x_reject) + (step_y4 * y_reject);

			store_u(accept_step << 4, raster_data.accept_table[1][i_edge][y]);
			store_u(accept_step << 2, raster_data.accept_table[0][i_edge][y]);

			store_u(reject_step << 4, raster_output.reject_table[2][i_edge][y]);
			store_u(reject_step << 2, raster_output.reject_table[1][i_edge][y]);
			__m128i pixel_step = ((reject_step << 1) + accept_step) >> 1;
			pixel_step += bias_4;
			store_u(pixel_step, raster_output.reject_table[0][i_edge][y]);

			y_reject += one;
		}
	}

	__int32 i_tile = 0;
	for (__int32 y_tile = 0; y_tile < display_::BIN_SIZE; y_tile += display_::TILE_SIZE) {
		for (__int32 x_tile = 0; x_tile < display_::BIN_SIZE; x_tile += display_::TILE_SIZE) {

			int2_ tile_origin;
			tile_origin.x = x_tile - display_::TILE_SIZE;
			tile_origin.y = y_tile - display_::TILE_SIZE;

			Rasterise_Tile(

				i_tile,
				tile_origin,
				raster_data,
				raster_output

			);

			i_tile += (display_::TILE_SIZE * display_::TILE_SIZE);
		}
	}
}

