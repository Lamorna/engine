
#include "model.h"
#include "vector.h"
#include "setup.h"
#include "texture.h"
#include "memory.h"
#include "quake.h"
#include "string.h"

/*
==================
==================
*/
__int32 Return_Model_ID(

	const char model_name[],
	const model_manager_& model_manager
) {

	const char* extension = ".mdl";
	char file_name[128];
	string_concatenate(model_name, extension, file_name);

	__int32 model_id = INVALID_RESULT;
	for (__int32 i_model = 0; i_model < model_manager.n_models; i_model++) {

		bool is_match = string_compare(model_manager.model[i_model].name, file_name) != 0;
		model_id = is_match ? i_model : model_id;
	}

	assert(model_id != INVALID_RESULT);

	return model_id;
}


/*
==================
==================
*/
void Create_Map_Model(

	const model_token_& map_token,
	const model_& model_cube,
	const float3_& colour,
	model_& model_map,
	memory_chunk_& memory
) {

	memcpy(&model_map, &model_cube, sizeof(model_));


	{
		const __int32 i_frame = 0;
		model_map.n_frames = 1;
		model_map.vertices_frame = (float3_**)memory.chunk_ptr;
		memory.chunk_ptr = model_map.vertices_frame + model_map.n_frames;
		model_map.vertices_frame[i_frame] = (float3_*)memory.chunk_ptr;
		model_map.n_vertices = 0;

		for (__int32 i_model = 0; i_model < map_token.n_models; i_model++) {

			for (__int32 i_vertex = 0; i_vertex < model_cube.n_vertices; i_vertex++) {

				for (__int32 i_axis = X; i_axis < W; i_axis++) {
					float centre = (float)(map_token.centre[i_model].i[i_axis]) * r_fixed_scale_real;
					float extent = (float)(map_token.extent[i_model].i[i_axis]) * r_fixed_scale_real;
					model_map.vertices_frame[i_frame][model_map.n_vertices].f[i_axis] = (model_cube.vertices_frame[i_frame][i_vertex].f[i_axis] * extent) + centre;
				}
				model_map.n_vertices++;
			}
		}
		memory.chunk_ptr = model_map.vertices_frame[i_frame] + model_map.n_vertices;
	}
	// ---------------------------------------------------------------------------
	{
		model_map.attribute_vertices[model_::ATTRIBUTE_COLOUR] = (float3_*)memory.chunk_ptr;
		model_map.n_colour_vertices = 0;

		float3_ random_colours[] = {

		{ 10.0f, 0.0f, 0.0f },
		{ 0.0f, 10.0f, 0.0f },
		{ 0.0f, 0.0f, 10.0f },
		{ 10.0f, 0.0f, 10.0f },
		{ 0.0f, 10.0f, 10.0f },
		{ 10.0f, 10.0f, 0.0f },
		//{ 26.0f, 26.0f, 0.0f },
		//{ 13.0f, 26.0f, 38.0f },
		//{ 0.0f, 0.0f, 77.0f },
		//{ 51.0f, 0.0f, 102.0f },
		//{ 0.0f, 38.0f, 77.0f },
		};
		const __int32 n_colours = sizeof(random_colours) / sizeof(random_colours[0]);

		for (__int32 i_model = 0; i_model < map_token.n_models; i_model++) {

			float3_ increment = { 0.0f, 0.0f, 0.0f };
			for (__int32 i_vertex = 0; i_vertex < model_cube.n_colour_vertices; i_vertex++) {

				model_map.attribute_vertices[model_::ATTRIBUTE_COLOUR][model_map.n_colour_vertices] = random_colours[i_model % n_colours] + increment;
				//model_map.attribute_vertices[model_::ATTRIBUTE_COLOUR][model_map.n_colour_vertices] = random_colours[i_vertex % n_colours] + increment;
				model_map.attribute_vertices[model_::ATTRIBUTE_COLOUR][model_map.n_colour_vertices] += i_vertex < (model_cube.n_colour_vertices / 2) ? 80.0f : 0.0f;
				model_map.n_colour_vertices++;
				//increment += 10.0f;
			}
		}
		memory.chunk_ptr = model_map.attribute_vertices[model_::ATTRIBUTE_COLOUR] + model_map.n_colour_vertices;
	}
	// ---------------------------------------------------------------------------
	{
		model_map.n_texture_layers = 2;
		model_map.n_texture_vertices = model_cube.n_texture_vertices * map_token.n_models;
		//model_map.texture_vertices = (float2_**)memory.chunk_ptr;
		//memory.chunk_ptr = model_map.texture_vertices + model_map.n_texture_layers;
		model_map.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY] = (float3_*)memory.chunk_ptr;
		memory.chunk_ptr = model_map.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY] + model_map.n_texture_vertices;
		model_map.attribute_vertices[model_::ATTRIBUTE_TEXTURE_SECONDARY] = (float3_*)memory.chunk_ptr;
		memory.chunk_ptr = model_map.attribute_vertices[model_::ATTRIBUTE_TEXTURE_SECONDARY] + model_map.n_texture_vertices;

		__int32 i_write = 0;
		for (__int32 i_model = 0; i_model < map_token.n_models; i_model++) {

			for (__int32 i_vertex = 0; i_vertex < model_cube.n_texture_vertices; i_vertex++) {

				model_map.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_write] = model_cube.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_vertex];
				i_write++;
			}
		}

		{
			//	0	z = x y = 0x3
			//	1	x = y z = 0x6
			//	2	z = x y = 0x3
			//	3	x = y z = 0x6
			//	4	y = x z = 0x5
			//	5	y = x z = 0x5

			const float scale = 10.0f;
			const __int32 n_faces = 6;
			__int32 i_write = 0;
			float2_ texture_size;
			texture_size.x = 256;
			texture_size.y = 256;
			for (__int32 i_model = 0; i_model < map_token.n_models; i_model++) {

				float3_ extent;
				extent.x = (float)(map_token.extent[i_model].x) * r_fixed_scale_real;
				extent.y = (float)(map_token.extent[i_model].y) * r_fixed_scale_real;
				extent.z = (float)(map_token.extent[i_model].z) * r_fixed_scale_real;

				extent *= scale;

				float2_ temp[6];
				temp[0].x = extent.x / texture_size.x;
				temp[0].y = extent.y / texture_size.y;

				temp[1].x = extent.z / texture_size.x;
				temp[1].y = extent.y / texture_size.y;

				temp[2].x = extent.x / texture_size.x;
				temp[2].y = extent.y / texture_size.y;

				temp[3].x = extent.z / texture_size.x;
				temp[3].y = extent.y / texture_size.y;

				temp[4].x = extent.x / texture_size.x;
				temp[4].y = extent.z / texture_size.y;

				temp[5].x = extent.x / texture_size.x;
				temp[5].y = extent.z / texture_size.y;

				__int32 i_vertex_read = 0;
				for (__int32 i_face = 0; i_face < n_faces; i_face++) {

					for (__int32 i_vertex = 0; i_vertex < 4; i_vertex++) {

						model_map.attribute_vertices[model_::ATTRIBUTE_TEXTURE_SECONDARY][i_write].x = model_cube.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_vertex_read].x * temp[i_face].x;
						model_map.attribute_vertices[model_::ATTRIBUTE_TEXTURE_SECONDARY][i_write].y = model_cube.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_vertex_read].y * temp[i_face].y;
						i_vertex_read++;
						i_write++;
					}
				}
			}
		}
	}
	// ---------------------------------------------------------------------------
	{
		model_map.i_vertices = (__int32*)memory.chunk_ptr;
		__int32 n_indices = 0;
		__int32 index_base = 0;
		for (__int32 i_model = 0; i_model < map_token.n_models; i_model++) {

			for (__int32 i_index = 0; i_index < model_cube.n_triangles * 3; i_index++) {
				model_map.i_vertices[n_indices] = model_cube.i_vertices[i_index] + index_base;
				n_indices++;
			}
			index_base += model_cube.n_vertices;
		}
		memory.chunk_ptr = model_map.i_vertices + n_indices;
	}
	// ---------------------------------------------------------------------------
	{
		model_map.i_attribute_vertices[model_::ATTRIBUTE_COLOUR] = (__int32*)memory.chunk_ptr;
		__int32 n_indices = 0;
		__int32 index_base = 0;
		for (__int32 i_model = 0; i_model < map_token.n_models; i_model++) {

			for (__int32 i_index = 0; i_index < model_cube.n_triangles * 3; i_index++) {
				model_map.i_attribute_vertices[model_::ATTRIBUTE_COLOUR][n_indices] = model_cube.i_attribute_vertices[model_::ATTRIBUTE_COLOUR][i_index] + index_base;
				n_indices++;
			}
			index_base += model_cube.n_colour_vertices;
		}
		memory.chunk_ptr = model_map.i_attribute_vertices[model_::ATTRIBUTE_COLOUR] + n_indices;
	}
	// ---------------------------------------------------------------------------
	{
		const __int32 n_indices = model_cube.n_triangles * 3 * map_token.n_models;
		//model_map.i_texture_vertices = (__int32**)memory.chunk_ptr;
		//memory.chunk_ptr = model_map.i_texture_vertices + model_map.n_texture_layers;
		model_map.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY] = (__int32*)memory.chunk_ptr;
		memory.chunk_ptr = model_map.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY] + n_indices;
		model_map.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_SECONDARY] = (__int32*)memory.chunk_ptr;
		memory.chunk_ptr = model_map.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_SECONDARY] + n_indices;

		__int32 i_write = 0;
		__int32 i_index_base = 0;
		for (__int32 i_model = 0; i_model < map_token.n_models; i_model++) {

			for (__int32 i_index = 0; i_index < model_cube.n_triangles * 3; i_index++) {
				model_map.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_write] = model_cube.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_index];
				model_map.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_SECONDARY][i_write] = model_cube.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_index] + i_index_base;
				i_write++;
			}
			i_index_base += model_cube.n_texture_vertices;
		}
	}
	// ---------------------------------------------------------------------------

	model_map.n_triangles = model_cube.n_triangles * map_token.n_models;

	// ---------------------------------------------------------------------------
	{
		model_map.i_textures = (__int32**)memory.chunk_ptr;
		memory.chunk_ptr = model_map.i_textures + model_map.n_texture_layers;
		model_map.i_textures[0] = (__int32*)memory.chunk_ptr;
		memory.chunk_ptr = model_map.i_textures[0] + model_map.n_triangles;
		model_map.i_textures[1] = (__int32*)memory.chunk_ptr;
		memory.chunk_ptr = model_map.i_textures[1] + model_map.n_triangles;

		__int32 i_triangle_write = 0;
		const __int32 i_textures[] = { 0, 1, 0 };
		const __int32 n_textures = sizeof(i_textures) / sizeof(i_textures[0]);
		for (__int32 i_block = 0; i_block < map_token.n_models; i_block++) {

			const __int32 i_texture = i_textures[i_block % n_textures];
			for (__int32 i_triangle = 0; i_triangle < model_cube.n_triangles; i_triangle++) {
				model_map.i_textures[0][i_triangle_write] = 0;
				model_map.i_textures[1][i_triangle_write] = i_texture;
				i_triangle_write++;
			}
		}
	}
}

/*
==================
==================
*/
void Create_Cube_Model(

	const __int32 n_textures,
	const char* filenames[],
	const model_& model_read,
	model_& model_write,
	memory_chunk_& memory
) {


	memcpy(&model_write, &model_read, sizeof(model_));

	model_write.n_textures = n_textures;
	model_write.texture_handlers = (texture_handler_*)memory.chunk_ptr;
	memory.chunk_ptr = model_write.texture_handlers + model_write.n_textures;

	model_write.n_texture_layers = n_textures;
	model_write.i_textures = (__int32**)memory.chunk_ptr;
	memory.chunk_ptr = model_write.i_textures + model_write.n_texture_layers;

	for (__int32 i_texture = 0; i_texture < n_textures; i_texture++) {

		Load_Image_STB(filenames[i_texture], model_write.texture_handlers[i_texture], memory);

		model_write.i_textures[i_texture] = (__int32*)memory.chunk_ptr;
		memory.chunk_ptr = model_write.i_textures[i_texture] + model_write.n_triangles;

		for (__int32 i_triangle = 0; i_triangle < model_write.n_triangles; i_triangle++) {
			//model_write.i_textures[i_texture][i_triangle] = Look_Up_Texture_ID(texture_manager, texture_ids[i_texture]);
			model_write.i_textures[i_texture][i_triangle] = i_texture;
		}
	}

	__int64 n_bytes = (__int8*)memory.chunk_ptr - memory.chunk;
	assert(n_bytes < memory_chunk_::NUM_BYTES);
}

/*
==================
==================
*/
void Hardcode_Cube_Template(

	model_& cube,
	model_& cube_backface,
	memory_chunk_& memory

) {

	/*
	3-2			   [5]			0 = front
	| |			   [2]			1 = right
	0-1			[3][4][1]		2 = back
	[0]			3 = left
	7-6							4 = top
	| |							5 = bottom
	4-5

	0	z	= x y	= 0x3
	1	x	= y z	= 0x6
	2	z	= x y	= 0x3
	3	x	= y z	= 0x6
	4	y	= x z	= 0x5
	5	y	= x z	= 0x5

	{ 4, 5, 1, 0 }
	{ 6 2 1 5 }
	{ 6, 7, 3, 2 }
	{ 4 0 3 7 }
	{ 0, 1, 2, 3 }
	{ 7, 6, 5, 4 }

	*/

	float3_ positions[] = {

		{-1.0f, 1.0f, 1.0f},		// 0
		{1.0f, 1.0f, 1.0f },		// 1
		{1.0f, 1.0f, -1.0f},		// 2
		{-1.0f, 1.0f, -1.0f},		// 3

		{-1.0f, -1.0f, 1.0f},		// 4
		{1.0f, -1.0f, 1.0f},		// 5
		{1.0f, -1.0f, -1.0f},		// 6
		{-1.0f, -1.0f, -1.0f},		// 7
	};
	float2_ texture_coordinates[] = {

		{0.0f, 1.0f},
		{1.0f, 1.0f},
		{1.0f, 0.0f},
		{0.0f, 0.0f},
							   
		{0.0f, 1.0f},
		{1.0f, 1.0f},
		{1.0f, 0.0f},
		{0.0f, 0.0f},
							   
		{0.0f, 1.0f},
		{1.0f, 1.0f},
		{1.0f, 0.0f},
		{0.0f, 0.0f},
							   
		{0.0f, 1.0f},
		{1.0f, 1.0f},
		{1.0f, 0.0f},
		{0.0f, 0.0f},
							   
		{0.0f, 1.0f},
		{1.0f, 1.0f},
		{1.0f, 0.0f},
		{0.0f, 0.0f},
							   
		{0.0f, 1.0f},
		{1.0f, 1.0f},
		{1.0f, 0.0f},
		{0.0f, 0.0f},
	};
	__int32 position_indices[][4] = {

	{ 4, 5, 1, 0 },
	{ 5, 6, 2, 1 },
	{ 6, 7, 3, 2 },
	{ 7, 4, 0, 3 },
	{ 0, 1, 2, 3 },
	{ 7, 6, 5, 4 }
	};
	__int32 texture_coordinate_indices[][4] = {

		{ 0, 1, 2, 3 },
	{ 4, 5, 6, 7 },
	{ 8, 9, 10, 11 },
	{ 12, 13, 14, 15 },
	{ 16, 17, 18, 19 },
	{ 20, 21, 22, 23 }
	};

	__int32 triangle_vertex_indices[] = {
		0, 1, 2, 0, 2, 3
	};

	//=======================================================================================================================
	{
		const __int32 n_vertices = sizeof(positions) / sizeof(positions[0]);
		const __int32 n_texture_coordinates = sizeof(texture_coordinates) / sizeof(texture_coordinates[0]);
		const __int32 n_faces = sizeof(position_indices) / sizeof(position_indices[0]);
		const __int32 n_indices_per_face = sizeof(triangle_vertex_indices) / sizeof(triangle_vertex_indices[0]);
		const __int32 n_indices = n_faces * n_indices_per_face;
		const __int32 n_triangles = n_indices / 3;
		{
			cube.n_vertices = n_vertices;
			cube.n_colour_vertices = n_vertices;
			cube.n_texture_vertices = n_texture_coordinates;
			cube.n_triangles = n_triangles;
			cube.n_texture_layers = 1;
		}
		{
			cube.n_frames = 1;
			cube.vertices_frame = (float3_**)memory.chunk_ptr;
			memory.chunk_ptr = cube.vertices_frame + cube.n_frames;

			cube.vertices_frame[0] = (float3_*)memory.chunk_ptr;
			memory.chunk_ptr = cube.vertices_frame[0] + n_vertices;

			for (__int32 i_vertex = 0; i_vertex < cube.n_vertices; i_vertex++) {
				cube.vertices_frame[0][i_vertex] = positions[i_vertex];
			}
		}
		{
			cube.attribute_vertices[model_::ATTRIBUTE_COLOUR] = (float3_*)memory.chunk_ptr;
			memory.chunk_ptr = cube.attribute_vertices[model_::ATTRIBUTE_COLOUR] + cube.n_colour_vertices;

			for (__int32 i_vertex = 0; i_vertex < cube.n_colour_vertices; i_vertex++) {
				cube.attribute_vertices[model_::ATTRIBUTE_COLOUR][i_vertex] = { 0.0f, 0.0f, 0.0f };
			}
		}
		{
			//cube.texture_vertices = (float2_**)memory.chunk_ptr;
			//memory.chunk_ptr = cube.texture_vertices + cube.n_texture_layers;
			cube.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY] = (float3_*)memory.chunk_ptr;
			memory.chunk_ptr = cube.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY] + n_texture_coordinates;
			cube.attribute_vertices[model_::ATTRIBUTE_TEXTURE_SECONDARY] = cube.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY];

			for (__int32 i_vertex = 0; i_vertex < cube.n_texture_vertices; i_vertex++) {

				cube.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_vertex].x = texture_coordinates[i_vertex].x;
				cube.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_vertex].y = texture_coordinates[i_vertex].y;
			}
		}
		{
			cube.i_vertices = (__int32*)memory.chunk_ptr;
			memory.chunk_ptr = cube.i_vertices + n_indices;

			cube.i_attribute_vertices[model_::ATTRIBUTE_COLOUR] = (__int32*)memory.chunk_ptr;
			memory.chunk_ptr = cube.i_attribute_vertices[model_::ATTRIBUTE_COLOUR] + n_indices;

			//cube.i_texture_vertices = (__int32**)memory.chunk_ptr;
			//memory.chunk_ptr = cube.i_texture_vertices + cube.n_texture_layers;
			cube.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY] = (__int32*)memory.chunk_ptr;
			memory.chunk_ptr = cube.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY] + n_indices;
			cube.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_SECONDARY] = cube.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY];

			__int32 i_current = 0;
			for (__int32 i_face = 0; i_face < n_faces; i_face++) {
				for (__int32 index = 0; index < n_indices_per_face; index++) {
					cube.i_vertices[i_current] = position_indices[i_face][triangle_vertex_indices[index]];
					cube.i_attribute_vertices[model_::ATTRIBUTE_COLOUR][i_current] = position_indices[i_face][triangle_vertex_indices[index]];
					cube.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_current] = texture_coordinate_indices[i_face][triangle_vertex_indices[index]];
					i_current++;
				}
			}
		}
		{
			cube.n_textures = 1;
			cube.texture_handlers = (texture_handler_*)memory.chunk_ptr;
			memory.chunk_ptr = cube.texture_handlers + cube.n_textures;

			Load_Image_STB("textures/floor.jpg", cube.texture_handlers[0], memory);
			//Load_Image_STB("textures/stone.png", cube.texture_handlers[0], memory);

			cube.i_textures = (__int32**)memory.chunk_ptr;
			memory.chunk_ptr = cube.i_textures + cube.n_texture_layers;
			cube.i_textures[0] = (__int32*)memory.chunk_ptr;
			memory.chunk_ptr = cube.i_textures[0] + cube.n_triangles;

			for (__int32 i_triangle = 0; i_triangle < cube.n_triangles; i_triangle++) {
				cube.i_textures[0][i_triangle] = 0;
			}
		}
	}
	//=======================================================================================================================
	{
		cube_backface.n_vertices = cube.n_vertices;
		cube_backface.n_colour_vertices = cube.n_colour_vertices;
		cube_backface.n_texture_vertices = cube.n_texture_vertices;
		cube_backface.n_triangles = cube.n_triangles;
		cube_backface.n_texture_layers = cube.n_texture_layers;

		const __int32 n_indices = cube_backface.n_triangles * 3;
		{
			cube_backface.n_frames = cube.n_frames;
			cube_backface.vertices_frame = (float3_**)memory.chunk_ptr;
			memory.chunk_ptr = cube_backface.vertices_frame + cube_backface.n_frames;

			cube_backface.vertices_frame[0] = (float3_*)memory.chunk_ptr;
			memory.chunk_ptr = cube_backface.vertices_frame[0] + cube_backface.n_vertices;

			for (__int32 i_vertex = 0; i_vertex < cube_backface.n_vertices; i_vertex++) {
				cube_backface.vertices_frame[0][i_vertex] = cube.vertices_frame[0][i_vertex];
			}
		}
		{
			cube_backface.attribute_vertices[model_::ATTRIBUTE_COLOUR] = (float3_*)memory.chunk_ptr;
			memory.chunk_ptr = cube_backface.attribute_vertices[model_::ATTRIBUTE_COLOUR] + cube_backface.n_colour_vertices;

			for (__int32 i_vertex = 0; i_vertex < cube_backface.n_colour_vertices; i_vertex++) {
				cube_backface.attribute_vertices[model_::ATTRIBUTE_COLOUR][i_vertex] = cube.attribute_vertices[model_::ATTRIBUTE_COLOUR][i_vertex];
			}
		}
		{
			//cube_backface.texture_vertices = (float2_**)memory.chunk_ptr;
			//memory.chunk_ptr = cube_backface.texture_vertices + cube_backface.n_texture_layers;
			cube_backface.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY] = (float3_*)memory.chunk_ptr;
			memory.chunk_ptr = cube_backface.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY] + cube_backface.n_texture_vertices;

			for (__int32 i_vertex = 0; i_vertex < cube_backface.n_texture_vertices; i_vertex++) {
				cube_backface.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_vertex] = cube.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_vertex];
			}
		}
		{
			cube_backface.i_vertices = (__int32*)memory.chunk_ptr;
			memory.chunk_ptr = cube_backface.i_vertices + n_indices;

			cube_backface.i_attribute_vertices[model_::ATTRIBUTE_COLOUR] = (__int32*)memory.chunk_ptr;
			memory.chunk_ptr = cube_backface.i_attribute_vertices[model_::ATTRIBUTE_COLOUR] + n_indices;

			//cube_backface.i_texture_vertices = (__int32**)memory.chunk_ptr;
			//memory.chunk_ptr = cube_backface.i_texture_vertices + cube_backface.n_texture_layers;
			cube_backface.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY] = (__int32*)memory.chunk_ptr;
			memory.chunk_ptr = cube_backface.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY] + n_indices;

			__int32 i_index = 0;
			for (__int32 i_triangle = 0; i_triangle < cube_backface.n_triangles; i_triangle++) {

				cube_backface.i_vertices[i_index + 0] = cube.i_vertices[i_index + 2];
				cube_backface.i_vertices[i_index + 1] = cube.i_vertices[i_index + 1];
				cube_backface.i_vertices[i_index + 2] = cube.i_vertices[i_index + 0];

				cube_backface.i_attribute_vertices[model_::ATTRIBUTE_COLOUR][i_index + 0] = cube.i_attribute_vertices[model_::ATTRIBUTE_COLOUR][i_index + 2];
				cube_backface.i_attribute_vertices[model_::ATTRIBUTE_COLOUR][i_index + 1] = cube.i_attribute_vertices[model_::ATTRIBUTE_COLOUR][i_index + 1];
				cube_backface.i_attribute_vertices[model_::ATTRIBUTE_COLOUR][i_index + 2] = cube.i_attribute_vertices[model_::ATTRIBUTE_COLOUR][i_index + 0];

				cube_backface.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_index + 0] = cube.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_index + 2];
				cube_backface.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_index + 1] = cube.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_index + 1];
				cube_backface.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_index + 2] = cube.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_index + 0];

				i_index += 3;
			}
		}
		{
			cube_backface.n_textures = 1;
			cube_backface.texture_handlers = (texture_handler_*)memory.chunk_ptr;
			memory.chunk_ptr = cube_backface.texture_handlers + cube_backface.n_textures;

			Load_Image_STB("textures/floor.jpg", cube_backface.texture_handlers[0], memory);

			cube_backface.i_textures = (__int32**)memory.chunk_ptr;
			memory.chunk_ptr = cube_backface.i_textures + cube_backface.n_texture_layers;
			cube_backface.i_textures[0] = (__int32*)memory.chunk_ptr;
			memory.chunk_ptr = cube_backface.i_textures[0] + cube_backface.n_triangles;

			for (__int32 i_triangle = 0; i_triangle < cube_backface.n_triangles; i_triangle++) {
				cube_backface.i_textures[0][i_triangle] = 0;
			}
		}
	}
}