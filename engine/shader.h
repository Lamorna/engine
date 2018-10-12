
#pragma once

#include "vector.h"


//======================================================================

struct texture_manager_;
struct display_;
struct raster_output_;
struct shader_input_;
struct draw_call_;
struct command_buffer_;
struct bin_triangle_data_;
struct vertex_light_manager_;
union int2_;
union float3_;
union float4_;

struct draw_command_ {

	__int32 i_index;
	__int32 x_start;
	__int32 y_start;
};

//======================================================================

void pixel_shader(

	const __int32,
	const raster_output_&,
	const draw_call_&,
	shader_input_&,
	display_&
);

void Shade_Vertex_Colour_Simple(

	const __int32,
	const command_buffer_&,
	const draw_call_&,
	const bin_triangle_data_[4],
	float4_[4][3]
);
void Shade_Vertex_Colour(

	const __int32,
	const command_buffer_&,
	const draw_call_&,
	const bin_triangle_data_[4],
	float4_[4][3]
);
void Shade_Vertex_Texture(

	const __int32,
	const command_buffer_&,
	const draw_call_&,
	const bin_triangle_data_[4],
	float4_[4][3]
);

void Vertex_Lighting(

	const __int32,
	const vertex_light_manager_&,
	const float4_[4][3],
	float4_[4][3]
);

void Vertex_Lighting_NULL(

	const __int32,
	const vertex_light_manager_&,
	const float4_[4][3],
	float4_[4][3]
);



void Process_Fragments(

	raster_output_&,
	shader_input_&
);

