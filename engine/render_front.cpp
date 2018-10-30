
#include "render_front.h"
#include "raster.h"
#include "shader.h"
#include "command.h"



//======================================================================

static const float guard_band_scale_g = 4.0f;

float vertex_light_manager_::bin_interval = 100.0f;


struct vertex_ {

	float4_ position;
	float4_ barycentric;
};

struct triangle_batch_ {

	enum {

		NUM_TRIANGLES_BATCH = 16,
		NUM_BATCHES = 4,
		NUM_VERTICES_PER_TRIANGLE = 3,
		BUFFER_SIZE = NUM_TRIANGLES_BATCH * NUM_BATCHES,
		BUFFER_MODULO = BUFFER_SIZE - 1,
	};

	unsigned __int32 i_write;
	unsigned __int32 i_read;

	bin_triangle_data_ triangle_data[BUFFER_SIZE];
	vertex_ buffer[BUFFER_SIZE][NUM_VERTICES_PER_TRIANGLE];
};

struct vertex_batch_ {

	enum {

		MAX_VERTICES = triangle_batch_::NUM_TRIANGLES_BATCH * triangle_batch_::NUM_VERTICES_PER_TRIANGLE,
	};

	__int32 n_triangles;
	__int32 n_vertices;
	bin_triangle_data_ triangle_data[triangle_batch_::NUM_TRIANGLES_BATCH];
	__int32 indices[triangle_batch_::NUM_TRIANGLES_BATCH][triangle_batch_::NUM_VERTICES_PER_TRIANGLE];
	__int32 vertex_id[MAX_VERTICES];
	float3_ vertices_in[MAX_VERTICES];
	float4_ vertices[MAX_VERTICES];

	unsigned __int32 view_volume_mask[MAX_VERTICES];
	unsigned __int32 guard_band_mask[MAX_VERTICES];
	unsigned __int32 clip_mask[triangle_batch_::NUM_TRIANGLES_BATCH];
	unsigned __int64 valid_triangle_mask;

};

struct clip_batch_ {

	__int32 n_triangles;
	bin_triangle_data_ triangle_data[triangle_batch_::NUM_TRIANGLES_BATCH];
	unsigned __int32 clip_mask[triangle_batch_::NUM_TRIANGLES_BATCH];
	vertex_ buffer[triangle_batch_::NUM_TRIANGLES_BATCH][triangle_batch_::NUM_VERTICES_PER_TRIANGLE];
};

//======================================================================


/*
==================
==================
*/
void Clip_Triangle(

	const bin_triangle_data_& triangle_data,
	const vertex_ triangle_in[3],
	unsigned __int32 clip_mask,
	triangle_batch_& triangle_batch

)
{
	enum {
		MAX_CLIP_VERTICES = 8,
	};
	static const __int32 i_axes[NUM_PLANES] = {

		X, X, Y, Y, Z,
	};

	static const float plane_scale[NUM_PLANES] = {

		guard_band_scale_g, guard_band_scale_g, guard_band_scale_g, guard_band_scale_g, 0.0f,
	};
	static const float sign_modifiers[NUM_PLANES] = {

		1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
	};

	vertex_ temp[2][MAX_CLIP_VERTICES];
	__int32 i_read = 0;
	__int32 n_vertices = 3;

	for (__int32 i_vertex = 0; i_vertex < n_vertices; i_vertex++) {

		temp[i_read][i_vertex].position = triangle_in[i_vertex].position;
		temp[i_read][i_vertex].barycentric = triangle_in[i_vertex].barycentric;
	}

	const __int32 n_clip_planes = _mm_popcnt_u32(clip_mask);

	for (__int32 i_clip_plane = 0; i_clip_plane < n_clip_planes; i_clip_plane++) {

		unsigned long i_plane;
		_BitScanForward(&i_plane, clip_mask);
		clip_mask ^= 0x1 << i_plane;

		const float scaling = plane_scale[i_plane];
		const float sign_modifier = sign_modifiers[i_plane];
		const __int32 i_axis = i_axes[i_plane];

		float d[MAX_CLIP_VERTICES];
		unsigned __int32 d_mask = 0x0;
		for (__int32 i_vertex = 0; i_vertex < n_vertices; i_vertex++) {
			d[i_vertex] = (temp[i_read][i_vertex].position.w * scaling) + (temp[i_read][i_vertex].position.f[i_axis] * sign_modifier);
			d_mask |= (d[i_vertex] < 0.0f) << i_vertex;
		}

		float t_interval[MAX_CLIP_VERTICES];
		for (__int32 i_vertex = 0; i_vertex < n_vertices; i_vertex++) {

			__int32 i_vertex_next = (i_vertex + 1) % n_vertices;
			float delta = (d[i_vertex_next] - d[i_vertex]);
			t_interval[i_vertex] = (0.0f - d[i_vertex]) / delta;
			//t_interval[i_vertex] = blend(0.0f, t_interval[i_vertex], delta == 0.0f);
			t_interval[i_vertex] = delta == 0.0f ? 0.0f : t_interval[i_vertex];
		}

		const __int32 i_write = i_read ^ 1;
		__int32 n_vertices_out = 0;
		for (__int32 i_vertex = 0; i_vertex < n_vertices; i_vertex++) {

			const __int32 i_vertex_next = (i_vertex + 1) % n_vertices;
			unsigned __int32 a = (d_mask >> i_vertex) & 0x1;
			unsigned __int32 b = (d_mask >> i_vertex_next) & 0x1;

			temp[i_write][n_vertices_out].position = temp[i_read][i_vertex].position;
			temp[i_write][n_vertices_out].barycentric = temp[i_read][i_vertex].barycentric;
			n_vertices_out += (a == 0x0);

			for (__int32 i = X; i <= W; i++) {

				float delta_position = temp[i_read][i_vertex_next].position.f[i] - temp[i_read][i_vertex].position.f[i];
				float delta_barycentric = temp[i_read][i_vertex_next].barycentric.f[i] - temp[i_read][i_vertex].barycentric.f[i];
				temp[i_write][n_vertices_out].position.f[i] = temp[i_read][i_vertex].position.f[i] + (delta_position * t_interval[i_vertex]);
				temp[i_write][n_vertices_out].barycentric.f[i] = temp[i_read][i_vertex].barycentric.f[i] + (delta_barycentric * t_interval[i_vertex]);
			}

			n_vertices_out += (a ^ b);
		}
		n_vertices = n_vertices_out;
		i_read ^= 1;
	}

	for (__int32 i_vertex = 2; i_vertex < n_vertices; i_vertex++) {

		const __int32 i_triangle_write = triangle_batch.i_write & triangle_batch_::BUFFER_MODULO;

		triangle_batch.buffer[i_triangle_write][0].position = temp[i_read][0].position;
		triangle_batch.buffer[i_triangle_write][0].barycentric = temp[i_read][0].barycentric;

		triangle_batch.buffer[i_triangle_write][1].position = temp[i_read][i_vertex - 1].position;
		triangle_batch.buffer[i_triangle_write][1].barycentric = temp[i_read][i_vertex - 1].barycentric;

		triangle_batch.buffer[i_triangle_write][2].position = temp[i_read][i_vertex].position;
		triangle_batch.buffer[i_triangle_write][2].barycentric = temp[i_read][i_vertex].barycentric;

		triangle_batch.triangle_data[i_triangle_write] = triangle_data;

		triangle_batch.i_write++;
	}
}

/*
==================
==================
*/
void Process_Triangle_Batch(

	const __int32 n_triangles,
	triangle_batch_& triangle_batch,
	screen_bin_ screen_bin[display_::N_BINS_Y][display_::N_BINS_X]
)
{

	//=======================================================================================================================
	{
		__m128 screen_scale[2];
		screen_scale[X] = set_all(screen_scale_x);
		screen_scale[Y] = set_all(screen_scale_y);
		__m128 screen_shift[2];
		screen_shift[X] = set_all(screen_shift_x);
		screen_shift[Y] = set_all(screen_shift_y);
		__m128 zero = set_all(0.0f);

		for (__int32 i_triangle_4 = 0; i_triangle_4 < n_triangles; i_triangle_4 += 4) {

			for (__int32 i_vertex = 0; i_vertex < NUM_VERTICES; i_vertex++) {

				__m128 position[4];
				__m128 barycentric[4];
				for (__int32 i_triangle = 0; i_triangle < 4; i_triangle++) {
					const __int32 i_triangle_read = (triangle_batch.i_read + i_triangle_4 + i_triangle) & triangle_batch_::BUFFER_MODULO;
					position[i_triangle] = load_u(triangle_batch.buffer[i_triangle_read][i_vertex].position.f);
					barycentric[i_triangle] = load_u(triangle_batch.buffer[i_triangle_read][i_vertex].barycentric.f);
				}
				Transpose(position);
				Transpose(barycentric);

				__m128 r_depth = reciprocal(position[W]);
				position[X] = (position[X] * r_depth * screen_scale[X]) + screen_shift[X];
				position[Y] = (position[Y] * r_depth * screen_scale[Y]) + screen_shift[Y];
				position[Z] = r_depth;

				barycentric[X] *= r_depth;
				barycentric[Y] *= r_depth;
				barycentric[Z] *= r_depth;
				barycentric[W] *= r_depth;

				Transpose(position);
				Transpose(barycentric);

				for (__int32 i_triangle = 0; i_triangle < 4; i_triangle++) {
					const __int32 i_triangle_read = (triangle_batch.i_read + i_triangle_4 + i_triangle) & triangle_batch_::BUFFER_MODULO;
					store_u(position[i_triangle], triangle_batch.buffer[i_triangle_read][i_vertex].position.f);
					store_u(barycentric[i_triangle], triangle_batch.buffer[i_triangle_read][i_vertex].barycentric.f);
				}
			}
		}
	}
	//=======================================================================================================================
	unsigned __int64 front_face_mask = 0x0;
	//=======================================================================================================================
	{
		for (__int32 i_triangle = 0; i_triangle < n_triangles; i_triangle++) {

			__int64 position_fxp[3][2];
			const __int32 i_triangle_read = (triangle_batch.i_read + i_triangle) & triangle_batch_::BUFFER_MODULO;
			for (__int32 i_vertex = 0; i_vertex < NUM_VERTICES; i_vertex++) {
				position_fxp[i_vertex][X] = (__int64)(triangle_batch.buffer[i_triangle_read][i_vertex].position.x * fixed_point_scale_g);
				position_fxp[i_vertex][Y] = (__int64)(triangle_batch.buffer[i_triangle_read][i_vertex].position.y * fixed_point_scale_g);
			}
			unsigned __int32 mask = 0x0;
			__int64 a[2];
			a[X] = position_fxp[2][X] - position_fxp[0][X];
			a[Y] = position_fxp[2][Y] - position_fxp[0][Y];

			__int64 b[2];
			b[X] = position_fxp[1][X] - position_fxp[0][X];
			b[Y] = position_fxp[1][Y] - position_fxp[0][Y];

			__int64 area = (a[X] * b[Y]) - (b[X] * a[Y]);
			area >>= FIXED_POINT_SHIFT + 1;
			front_face_mask |= (area > 0) << i_triangle;
		}
	}
	//=======================================================================================================================
	{
		const __m128i zero = set_zero_si128();
		const __m128i half_fxp = set_all(1 << (FIXED_POINT_SHIFT - 1));
		const __m128 fixed_point_scale = set_all(fixed_point_scale_g);
		__m128i bin_max[2];
		bin_max[X] = set_all(display_::N_BINS_X - 1);
		bin_max[Y] = set_all(display_::N_BINS_Y - 1);

		for (__int32 i_triangle_4 = 0; i_triangle_4 < n_triangles; i_triangle_4 += 4) {

			__m128i position_fxp[3][2];
			for (__int32 i_vertex = 0; i_vertex < NUM_VERTICES; i_vertex++) {

				__m128 position[4];
				for (__int32 i_triangle = 0; i_triangle < 4; i_triangle++) {
					const __int32 i_triangle_read = (triangle_batch.i_read + i_triangle) & triangle_batch_::BUFFER_MODULO;
					position[i_triangle] = load_u(triangle_batch.buffer[i_triangle_read][i_vertex].position.f);
				}
				Transpose(position);
				position_fxp[i_vertex][X] = convert_int_trunc(position[X] * fixed_point_scale);
				position_fxp[i_vertex][Y] = convert_int_trunc(position[Y] * fixed_point_scale);
			}

			__m128i bb_max[2];
			bb_max[X] = max_vec(position_fxp[2][X], max_vec(position_fxp[1][X], position_fxp[0][X]));
			bb_max[Y] = max_vec(position_fxp[2][Y], max_vec(position_fxp[1][Y], position_fxp[0][Y]));
			__m128i bb_min[2];
			bb_min[X] = min_vec(position_fxp[2][X], min_vec(position_fxp[1][X], position_fxp[0][X]));
			bb_min[Y] = min_vec(position_fxp[2][Y], min_vec(position_fxp[1][Y], position_fxp[0][Y]));

			bb_max[X] = (bb_max[X] + half_fxp) >> FIXED_POINT_SHIFT;
			bb_max[Y] = (bb_max[Y] + half_fxp) >> FIXED_POINT_SHIFT;
			bb_min[X] = (bb_min[X] + half_fxp) >> FIXED_POINT_SHIFT;
			bb_min[Y] = (bb_min[Y] + half_fxp) >> FIXED_POINT_SHIFT;

			unsigned __int32 colinear_mask = store_mask((bb_min[X] != bb_max[X]) & (bb_min[Y] != bb_max[Y]));
			unsigned __int32 facing_mask = (front_face_mask >> i_triangle_4) & 0xf;
			unsigned __int32 write_mask = colinear_mask & facing_mask;

			bb_max[X] >>= display_::BIN_SIZE_SHIFT;
			bb_max[Y] >>= display_::BIN_SIZE_SHIFT;
			bb_min[X] >>= display_::BIN_SIZE_SHIFT;
			bb_min[Y] >>= display_::BIN_SIZE_SHIFT;

			bb_min[X] = max_vec(zero, bb_min[X]);
			bb_min[Y] = max_vec(zero, bb_min[Y]);
			bb_max[X] = min_vec(bin_max[X], bb_max[X]);
			bb_max[Y] = min_vec(bin_max[Y], bb_max[Y]);

			__int32 x_extent[2][4];
			__int32 y_extent[2][4];
			store_u(bb_min[X], x_extent[0]);
			store_u(bb_min[Y], y_extent[0]);
			store_u(bb_max[X], x_extent[1]);
			store_u(bb_max[Y], y_extent[1]);

			__int32 n = min(n_triangles - i_triangle_4, 4);

			for (__int32 i_triangle = 0; i_triangle < n; i_triangle++) {

				const __int32 i_triangle_read = triangle_batch.i_read & triangle_batch_::BUFFER_MODULO;

				for (__int32 y = y_extent[0][i_triangle]; y <= y_extent[1][i_triangle]; y++) {

					for (__int32 x = x_extent[0][i_triangle]; x <= x_extent[1][i_triangle]; x++) {

						screen_bin_& bin = screen_bin[y][x];

						bin.bin_triangle_data[bin.n_triangles] = triangle_batch.triangle_data[i_triangle_read];

						for (__int32 i_vertex = 0; i_vertex < NUM_VERTICES; i_vertex++) {

							bin.bin_triangle[bin.n_triangles].position[i_vertex].x = triangle_batch.buffer[i_triangle_read][i_vertex].position.x;
							bin.bin_triangle[bin.n_triangles].position[i_vertex].y = triangle_batch.buffer[i_triangle_read][i_vertex].position.y;
							bin.bin_triangle[bin.n_triangles].position[i_vertex].z = triangle_batch.buffer[i_triangle_read][i_vertex].position.z;

							bin.bin_triangle[bin.n_triangles].barycentric[i_vertex].x = triangle_batch.buffer[i_triangle_read][i_vertex].barycentric.x;
							bin.bin_triangle[bin.n_triangles].barycentric[i_vertex].y = triangle_batch.buffer[i_triangle_read][i_vertex].barycentric.y;
							bin.bin_triangle[bin.n_triangles].barycentric[i_vertex].z = triangle_batch.buffer[i_triangle_read][i_vertex].barycentric.z;
						}

						__int32 draw_id = triangle_batch.triangle_data[i_triangle_read].draw_id;
						bin.n_draw_calls += bin.draw_id[bin.n_draw_calls] != draw_id;
						bin.draw_id[bin.n_draw_calls] = draw_id;
						__int32 increment = (write_mask >> i_triangle) & 0x1;
						bin.n_triangles += increment;
						bin.n_tris[bin.n_draw_calls] += increment;
					}
				}
				triangle_batch.i_read++;
			}
		}
	}
}

/*
==================
==================
*/
void Renderer_FRONT_END(

	const __int32 i_draw_call,
	const __int32 n_triangles_in,
	const __int32 index_stream[],
	const float3_ position_stream[],
	const matrix_& m_clip_space,
	const __int32 i_models[][2],
	screen_bin_ screen_bin[display_::N_BINS_Y][display_::N_BINS_X]
)
{
	enum {

		NUM_VERTICES = 3,
	};

	static const float4_ barycentric[] = {

		{ 1.0f, 0.0f, 0.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f, 1.0f },
	};
	const __m128 fixed_point_scale = set_all(fixed_point_scale_g);

	__int32 i_vertex_index = 0;
	__int32 i_model_index = 0;
	__int32 i_triangle_model = 0;

	vertex_batch_ vertex_batch;
	triangle_batch_ triangle_batch;
	triangle_batch.i_write = 0;
	triangle_batch.i_read = 0;

	for (__int32 i_triangle_batch = 0; i_triangle_batch < n_triangles_in; i_triangle_batch += triangle_batch_::NUM_TRIANGLES_BATCH) {

		__int32 d = n_triangles_in - i_triangle_batch;
		const bool is_final_batch = d <= triangle_batch_::NUM_TRIANGLES_BATCH;
		vertex_batch.n_triangles = is_final_batch ? d : triangle_batch_::NUM_TRIANGLES_BATCH;

		//=======================================================================================================================
		{
			vertex_batch.n_vertices = 0;

			for (__int32 i_triangle = 0; i_triangle < vertex_batch.n_triangles; i_triangle++) {

				vertex_batch.triangle_data[i_triangle].i_triangle = i_triangle_model;
				vertex_batch.triangle_data[i_triangle].i_model = i_models[i_model_index][0];
				vertex_batch.triangle_data[i_triangle].draw_id = i_draw_call;

				for (__int32 i_vertex = 0; i_vertex < NUM_VERTICES; i_vertex++) {

					const __int32 vertex_id = index_stream[i_vertex_index];
					const float3_ vertex = position_stream[vertex_id];

					{
						__int32 i_vertex_match = -1;
						for (__int32 i_vertex_id = 0; i_vertex_id < vertex_batch.n_vertices; i_vertex_id++) {

							bool is_match = vertex_batch.vertex_id[i_vertex_id] == vertex_id;
							i_vertex_match = is_match ? i_vertex_id : i_vertex_match;
						}
						bool is_new_vertex = i_vertex_match < 0;
						vertex_batch.vertices_in[vertex_batch.n_vertices] = vertex;
						vertex_batch.vertex_id[vertex_batch.n_vertices] = vertex_id;
						vertex_batch.indices[i_triangle][i_vertex] = is_new_vertex ? vertex_batch.n_vertices : i_vertex_match;
						vertex_batch.n_vertices += is_new_vertex;
					}
					i_vertex_index++;
				}

				i_triangle_model++;
				bool is_next_model = i_triangle_model == i_models[i_model_index][1];
				i_model_index += is_next_model;
				i_triangle_model = is_next_model ? 0 : i_triangle_model;

			}
		}
		//=======================================================================================================================
		{
			for (__int32 i_vertex = 0; i_vertex < vertex_batch.n_vertices; i_vertex++) {

				const float3_ vertex = vertex_batch.vertices_in[i_vertex];
				Vector_X_Matrix(vertex, m_clip_space, vertex_batch.vertices[i_vertex]);
			}
		}
		//=======================================================================================================================
		{
			__int32 shift_fxp = 16;
			__m128i sub_mask = set_all((1 << shift_fxp) - 1);
			__m128 scale_fxp = convert_float(set_all(1 << shift_fxp));

			for (__int32 i_vertex_4 = 0; i_vertex_4 < vertex_batch.n_vertices; i_vertex_4 += 4) {

				__m128 position[4];

				for (__int32 i_vertex = 0; i_vertex < 4; i_vertex++) {
					position[i_vertex] = load_u(vertex_batch.vertices[i_vertex_4 + i_vertex].f);
				}
				Transpose(position);

				for (__int32 i_axis = X; i_axis <= W; i_axis++) {
					__m128i position_fxp = convert_int_trunc(position[i_axis] * scale_fxp);
					position[i_axis] = convert_float(position_fxp >> FIXED_POINT_SHIFT) + (convert_float(position_fxp & sub_mask) / scale_fxp);
				}

				const __m128i bit = set_all(1);
				{
					const __m128 zero = set_zero();
					__m128 w_value = position[W];
					__m128 w_value_neg = -w_value;
					__m128i bit_mask = set_zero_si128();

					bit_mask |= (position[X] < w_value_neg) & (bit << PLANE_LEFT);
					bit_mask |= (position[X] > w_value)		& (bit << PLANE_RIGHT);
					bit_mask |= (position[Y] < w_value_neg) & (bit << PLANE_TOP);
					bit_mask |= (position[Y] > w_value)		& (bit << PLANE_BOTTOM);
					bit_mask |= (position[Z] < w_value_neg) & (bit << PLANE_NEAR);
					bit_mask |= (position[Z] > w_value)		& (bit << PLANE_FAR);
					bit_mask |= (position[W] < zero)		& (bit << PLANE_DEGENERATE_W);
					store_u(bit_mask, vertex_batch.view_volume_mask + i_vertex_4);
				}
				{
					__m128 w_value = position[W] * set_all(guard_band_scale_g);
					__m128 w_value_neg = -w_value;
					__m128i bit_mask = set_zero_si128();

					bit_mask |= (position[X] < w_value_neg) & (bit << PLANE_LEFT);
					bit_mask |= (position[X] > w_value)		& (bit << PLANE_RIGHT);
					bit_mask |= (position[Y] < w_value_neg) & (bit << PLANE_TOP);
					bit_mask |= (position[Y] > w_value)		& (bit << PLANE_BOTTOM);
					store_u(bit_mask, vertex_batch.guard_band_mask + i_vertex_4);
				}
			}
		}
		//=======================================================================================================================
		{
			vertex_batch.valid_triangle_mask = 0x0;

			for (__int32 i_triangle = 0; i_triangle < vertex_batch.n_triangles; i_triangle++) {

				unsigned __int32 view_volume_mask = 0x0;
				unsigned __int32 guard_band_mask = 0x0;
				unsigned __int32 cull_mask = ~0x0;

				for (__int32 i_vertex_index = 0; i_vertex_index < NUM_VERTICES; i_vertex_index++) {

					const __int32 i_vertex = vertex_batch.indices[i_triangle][i_vertex_index];
					cull_mask &= vertex_batch.view_volume_mask[i_vertex];
					view_volume_mask |= vertex_batch.view_volume_mask[i_vertex];
					guard_band_mask |= vertex_batch.guard_band_mask[i_vertex];
				}
				vertex_batch.valid_triangle_mask |= (cull_mask == 0x0) << i_triangle;
				vertex_batch.clip_mask[i_triangle] = guard_band_mask | (view_volume_mask & (0x1 << PLANE_NEAR));
			}
		}
		//=======================================================================================================================

		clip_batch_ clip_batch;
		//=======================================================================================================================
		{
			clip_batch.n_triangles = 0;

			for (__int32 i_triangle = 0; i_triangle < vertex_batch.n_triangles; i_triangle++) {

				const __int32 i_triangle_write = triangle_batch.i_write & triangle_batch_::BUFFER_MODULO;

				triangle_batch.triangle_data[i_triangle_write] = vertex_batch.triangle_data[i_triangle];
				clip_batch.triangle_data[clip_batch.n_triangles] = vertex_batch.triangle_data[i_triangle];
				clip_batch.clip_mask[clip_batch.n_triangles] = vertex_batch.clip_mask[i_triangle];

				for (__int32 i_vertex = 0; i_vertex < NUM_VERTICES; i_vertex++) {

					const __int32 i_vertex_index = vertex_batch.indices[i_triangle][i_vertex];

					triangle_batch.buffer[i_triangle_write][i_vertex].position = vertex_batch.vertices[i_vertex_index];
					triangle_batch.buffer[i_triangle_write][i_vertex].barycentric = barycentric[i_vertex];

					clip_batch.buffer[clip_batch.n_triangles][i_vertex].position = vertex_batch.vertices[i_vertex_index];
					clip_batch.buffer[clip_batch.n_triangles][i_vertex].barycentric = barycentric[i_vertex];
				}
				unsigned __int32 is_valid = (vertex_batch.valid_triangle_mask >> i_triangle) & 0x1;
				unsigned __int32 is_clipped = vertex_batch.clip_mask[i_triangle] != 0x0;
				triangle_batch.i_write += is_valid & (is_clipped ^ 1);
				clip_batch.n_triangles += is_valid & is_clipped;
			}
		}
		//=======================================================================================================================
		{
			for (__int32 i_triangle = 0; i_triangle < clip_batch.n_triangles; i_triangle++) {

				Clip_Triangle(clip_batch.triangle_data[i_triangle], clip_batch.buffer[i_triangle], clip_batch.clip_mask[i_triangle], triangle_batch);
			}
		}
		//=======================================================================================================================
		{
			const __int32 n_triangles_buffer = triangle_batch.i_write - triangle_batch.i_read;
			const __int32 n_batches = n_triangles_buffer / triangle_batch_::NUM_TRIANGLES_BATCH;
			const __int32 n_triangles_batched = is_final_batch ? n_triangles_buffer : n_batches * triangle_batch_::NUM_TRIANGLES_BATCH;

			for (__int32 i_triangle = 0; i_triangle < n_triangles_batched; i_triangle += triangle_batch_::NUM_TRIANGLES_BATCH) {

				const __int32 n_triangles = min(n_triangles_batched - i_triangle, triangle_batch_::NUM_TRIANGLES_BATCH);

				Process_Triangle_Batch(

					n_triangles,
					triangle_batch,
					screen_bin
				);
			}
		}
	}
}

/*
==================
==================
*/
void systems_::render_::lock_back_buffer(

	void* parameters, __int32 i_thread
) {

	parameters_::render_::lock_back_buffer_* func_parameters = (parameters_::render_::lock_back_buffer_*)parameters;
	display_& display = *func_parameters->display;

	display.d3d9_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &display.d3d9_surface);
	display.d3d9_surface->LockRect(&display.locked_rect, NULL, D3DLOCK_NOSYSLOCK);

}

/*
==================
==================
*/
void systems_::render_::release_back_buffer(

	void* parameters, __int32 i_thread
) {

	parameters_::render_::release_back_buffer_* func_parameters = (parameters_::render_::release_back_buffer_*)parameters;
	display_& display = *func_parameters->display;

	display.d3d9_surface->UnlockRect();
	display.d3d9_surface->Release();

	display.d3d9_device->Present(NULL, NULL, display.handle_window, NULL);

}








