
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

void shade_colour(shader_input_&);
void shade_texture_CLAMP(shader_input_&);
void shade_texture_WRAP(shader_input_&);
void shader_texture_BLEND(shader_input_&);

void Fragment_Shader(

	const __int32,
	const __int32,
	const __int32,
	const int2_&,
	const float3_[3][4],
	const float3_[3][4],
	const raster_output_&,
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


void shader_DISPATCH_PRE(

	const raster_output_&,
	shader_input_&
);
//
//void depth_NO_MASK(
//
//	const unsigned __int32,
//	const unsigned __int32,
//	const __m128i[3][4],
//	const shader_input_&
//);
//
//void depth_MASK(
//
//	const unsigned __int32,
//	const unsigned __int32,
//	const __m128i[3][4],
//	const shader_input_&
//);

void Process_Fragments(

	const raster_output_&,
	const shader_input_&
);

