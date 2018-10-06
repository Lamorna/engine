#pragma once

struct memory_chunk_;


struct texture_handler_ {

	enum {
		MAX_MIP_LEVELS = 1 << 4,
	};

	__int32 width;
	__int32 height;
	__int32 width_shift;
	__int32 height_shift;
	__int32 n_mip_levels;
	unsigned __int32* texture[MAX_MIP_LEVELS];
};


void Load_Image_STB(

	const char*,
	texture_handler_&,
	memory_chunk_&
);

void Build_MIP_Map_Chain(

	texture_handler_&,
	memory_chunk_&

);