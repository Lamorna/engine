#pragma once

#include "raster.h"
#include "vector.h"
#include "render_front.h"

struct int2_64_ {

	__int64 x;
	__int64 y;
};

struct raster_data_ {

	__int64 dx[3];
	__int64 dy[3];
	__int64 bias[3];
	__int64 step_x[3];
	__int64 step_y[3];
	bool is_offset[3][2];
	__int32 accept_table[2][3][4][4];
	int2_ bb_min;
	int2_ bb_max;
	int2_ tile_offset;
	int2_64_ v[3];
};

struct raster_fragment_ {

	__int32 w[2];
	__int32 i_buffer;
	unsigned __int32 coverage_mask;
};

struct raster_fragment_complete_ {

	__m128i bazza[3][4];
	__int32 i_buffer;
	unsigned __int32 coverage_mask;
};


struct raster_output_ {

	enum {

		TRIVIAL_ACCEPT,
		PARTIAL_ACCEPT,
		NUM_ACCEPT_TYPES,

		MAX_TRIANGLES = 4,
		MAX_FRAGMENTS = (display_::BIN_SIZE * display_::BIN_SIZE) / (4 * 4)
	};

	raster_fragment_ raster_fragment[NUM_ACCEPT_TYPES][MAX_FRAGMENTS];
	raster_fragment_complete_ raster_fragment_complete[4];
	__int32 n_fragments[NUM_ACCEPT_TYPES];
	__int32 n_fragments_COMPLETE;
	__int32 reject_table[3][3][4][4];

};

void Raster_Setup(

	const __int32,
	const float3_[3][4],
	const int2_&,
	raster_data_&
);

void Rasteriser(

	raster_data_&,
	raster_output_&
);

void shade_COLOUR(

	const unsigned __int32,
	const unsigned __int32,
	const __m128i[3][4],
	const raster_output_&
);

void Process_Fragments(const raster_output_&, const shader_input_&);

__int32 encode_morton(__int32, __int32);
__int32 encode_morton_x(__int32);
__int32 encode_morton_y(__int32);
void decode_morton(const __int32, __int32&, __int32&);