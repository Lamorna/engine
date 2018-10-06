

#include "quake.h"
#include "vector.h"
#include "texture.h"
#include "model.h"
#include "memory.h"
#include "string.h"


struct uchar3 {

	unsigned char channel[3];
};

uchar3 colour_map[] = {

{  0, 0, 0}, { 15,  15,  15 }, { 31,  31,  31 }, { 47,  47,  47 },
{ 63,  63,  63 }, { 75,  75,  75 }, { 91,  91,  91 }, { 107, 107, 107 },
{ 123, 123, 123 }, { 139, 139, 139 }, { 155, 155, 155 }, { 171, 171, 171 },
{ 187, 187, 187 }, { 203, 203, 203 }, { 219, 219, 219 }, { 235, 235, 235 },
{ 15,  11,   7 }, { 23,  15,  11 }, { 31,  23,  11 }, { 39,  27,  15 },
{ 47,  35,  19 }, { 55,  43,  23 }, { 63,  47,  23 }, { 75,  55,  27 },
{ 83,  59,  27 }, { 91,  67,  31 }, { 99,  75,  31 }, { 107,  83,  31 },
{ 115,  87,  31 }, { 123,  95,  35 }, { 131, 103,  35 }, { 143, 111,  35 },
{ 11,  11,  15 }, { 19,  19,  27 }, { 27,  27,  39 }, { 39,  39,  51 },
{ 47,  47,  63 }, { 55,  55,  75 }, { 63,  63,  87 }, { 71,  71, 103 },
{ 79,  79, 115 }, { 91,  91, 127 }, { 99,  99, 139 }, { 107, 107, 151 },
{ 115, 115, 163 }, { 123, 123, 175 }, { 131, 131, 187 }, { 139, 139, 203 },
{ 0,   0,   0 }, { 7,   7,   0 }, { 11,  11,   0 }, { 19,  19,   0 },
{ 27,  27,   0 }, { 35,  35,   0 }, { 43,  43,   7 }, { 47,  47,   7 },
{ 55,  55,   7 }, { 63,  63,   7 }, { 71,  71,   7 }, { 75,  75,  11 },
{ 83,  83,  11 }, { 91,  91,  11 }, { 99,  99,  11 }, { 107, 107,  15 },
{ 7,   0,   0 }, { 15,   0,   0 }, { 23,   0,   0 }, { 31,   0,   0 },
{ 39,   0,   0 }, { 47,   0,   0 }, { 55,   0,   0 }, { 63,   0,   0 },
{ 71,   0,   0 }, { 79,   0,   0 }, { 87,   0,   0 }, { 95,   0,   0 },
{ 103,   0,   0 }, { 111,   0,   0 }, { 119,   0,   0 }, { 127,   0,   0 },
{ 19,  19,   0 }, { 27,  27,   0 }, { 35,  35,   0 }, { 47,  43,   0 },
{ 55,  47,   0 }, { 67,  55,   0 }, { 75,  59,   7 }, { 87,  67,   7 },
{ 95,  71,   7 }, { 107,  75,  11 }, { 119,  83,  15 }, { 131,  87,  19 },
{ 139,  91,  19 }, { 151,  95,  27 }, { 163,  99,  31 }, { 175, 103,  35 },
{ 35,  19,   7 }, { 47,  23,  11 }, { 59,  31,  15 }, { 75,  35,  19 },
{ 87,  43,  23 }, { 99,  47,  31 }, { 115,  55,  35 }, { 127,  59,  43 },
{ 143,  67,  51 }, { 159,  79,  51 }, { 175,  99,  47 }, { 191, 119,  47 },
{ 207, 143,  43 }, { 223, 171,  39 }, { 239, 203,  31 }, { 255, 243,  27 },
{ 11,   7,   0 }, { 27,  19,   0 }, { 43,  35,  15 }, { 55,  43,  19 },
{ 71,  51,  27 }, { 83,  55,  35 }, { 99,  63,  43 }, { 111,  71,  51 },
{ 127,  83,  63 }, { 139,  95,  71 }, { 155, 107,  83 }, { 167, 123,  95 },
{ 183, 135, 107 }, { 195, 147, 123 }, { 211, 163, 139 }, { 227, 179, 151 },
{ 171, 139, 163 }, { 159, 127, 151 }, { 147, 115, 135 }, { 139, 103, 123 },
{ 127,  91, 111 }, { 119,  83,  99 }, { 107,  75,  87 }, { 95,  63,  75 },
{ 87,  55,  67 }, { 75,  47,  55 }, { 67,  39,  47 }, { 55,  31,  35 },
{ 43,  23,  27 }, { 35,  19,  19 }, { 23,  11,  11 }, { 15,   7,   7 },
{ 187, 115, 159 }, { 175, 107, 143 }, { 163,  95, 131 }, { 151,  87, 119 },
{ 139,  79, 107 }, { 127,  75,  95 }, { 115,  67,  83 }, { 107,  59,  75 },
{ 95,  51,  63 }, { 83,  43,  55 }, { 71,  35,  43 }, { 59,  31,  35 },
{ 47,  23,  27 }, { 35,  19,  19 }, { 23,  11,  11 }, { 15,   7,   7 },
{ 219, 195, 187 }, { 203, 179, 167 }, { 191, 163, 155 }, { 175, 151, 139 },
{ 163, 135, 123 }, { 151, 123, 111 }, { 135, 111,  95 }, { 123,  99,  83 },
{ 107,  87,  71 }, { 95,  75,  59 }, { 83,  63,  51 }, { 67,  51,  39 },
{ 55,  43,  31 }, { 39,  31,  23 }, { 27,  19,  15 }, { 15,  11,   7 },
{ 111, 131, 123 }, { 103, 123, 111 }, { 95, 115, 103 }, { 87, 107,  95 },
{ 79,  99,  87 }, { 71,  91,  79 }, { 63,  83,  71 }, { 55,  75,  63 },
{ 47,  67,  55 }, { 43,  59,  47 }, { 35,  51,  39 }, { 31,  43,  31 },
{ 23,  35,  23 }, { 15,  27,  19 }, { 11,  19,  11 }, { 7,  11,   7 },
{ 255, 243,  27 }, { 239, 223,  23 }, { 219, 203,  19 }, { 203, 183,  15 },
{ 187, 167,  15 }, { 171, 151,  11 }, { 155, 131,   7 }, { 139, 115,   7 },
{ 123,  99,   7 }, { 107,  83,   0 }, { 91,  71,   0 }, { 75,  55,   0 },
{ 59,  43,   0 }, { 43,  31,   0 }, { 27,  15,   0 }, { 11,   7,   0 },
{ 0,   0, 255 }, { 11,  11, 239 }, { 19,  19, 223 }, { 27,  27, 207 },
{ 35,  35, 191 }, { 43,  43, 175 }, { 47,  47, 159 }, { 47,  47, 143 },
{ 47,  47, 127 }, { 47,  47, 111 }, { 47,  47,  95 }, { 43,  43,  79 },
{ 35,  35,  63 }, { 27,  27,  47 }, { 19,  19,  31 }, { 11,  11,  15 },
{ 43,   0,   0 }, { 59,   0,   0 }, { 75,   7,   0 }, { 95,   7,   0 },
{ 111,  15,   0 }, { 127,  23,   7 }, { 147,  31,   7 }, { 163,  39,  11 },
{ 183,  51,  15 }, { 195,  75,  27 }, { 207,  99,  43 }, { 219, 127,  59 },
{ 227, 151,  79 }, { 231, 171,  95 }, { 239, 191, 119 }, { 247, 211, 139 },
{ 167, 123,  59 }, { 183, 155,  55 }, { 199, 195,  55 }, { 231, 227,  87 },
{ 127, 191, 255 }, { 171, 231, 255 }, { 215, 255, 255 }, { 103,   0,   0 },
{ 139,   0,   0 }, { 179,   0,   0 }, { 215,   0,   0 }, { 255,   0,   0 },
{ 255, 243, 147 }, { 255, 247, 199 }, { 255, 255, 255 }, { 159,  91,  83 }

};

//======================================================================


typedef float vec3_t[3];

struct mdl_header_t
{
	__int32 ident;            /* magic number: "IDPO" */
	__int32 version;          /* version: 6 */

	vec3_t scale;         /* scale factor */
	vec3_t translate;     /* translation vector */
	float boundingradius;
	vec3_t eyeposition;   /* eyes' position */

	__int32 num_skins;        /* number of textures */
	__int32 skinwidth;        /* texture width */
	__int32 skinheight;       /* texture height */

	__int32 num_verts;        /* number of vertices */
	__int32 num_tris;         /* number of triangles */
	__int32 num_frames;       /* number of frames */

	__int32 synctype;         /* 0 = synchron, 1 = random */
	__int32 flags;            /* state flag */
	float size;
};

struct mdl_skin_t
{
	__int32 group;				/* 0 = single, 1 = group */
	unsigned char *data;	/* texture data */
};

struct mdl_texcoord_t
{
	__int32 onseam;
	__int32 s;
	__int32 t;
};

struct mdl_triangle_t
{
	__int32 facesfront;  /* 0 = backface, 1 = frontface */
	__int32 vertex[3];   /* vertex indices */
};

/* Compressed vertex */
struct mdl_vertex_t
{
	unsigned char v[3];
	unsigned char normalIndex;
};

/* Simple frame */
struct mdl_simpleframe_t
{
	struct mdl_vertex_t bboxmin; /* bouding box min */
	struct mdl_vertex_t bboxmax; /* bouding box max */
	char name[16];
	struct mdl_vertex_t *verts;  /* vertex list of the frame */
};

/* Group of simple frames */
struct mdl_groupframe_t
{
	__int32 type;                         /* !0 = group */
	struct mdl_vertex_t min;          /* min pos in all simple frames */
	struct mdl_vertex_t max;          /* max pos in all simple frames */
	float *time;                      /* time duration for each frame */
	struct mdl_simpleframe_t *frames; /* simple frame list */
};

/* Model frame */
struct mdl_frame_t
{
	__int32 type;                        /* 0 = simple, !0 = group */
	struct mdl_simpleframe_t frame;  /* this program can't read models
									 composed of group frames! */
};

struct quake_mdl_ {

	enum {

		MAX_VERTICES = 1024,
		MAX_TRIANGLES = 2048,
		MAX_SKIN_SIZE = 1024 * 1024,
	};

	unsigned char colour_indices[MAX_SKIN_SIZE];
	mdl_header_t file_header;
	mdl_triangle_t triangles[MAX_TRIANGLES];
	mdl_texcoord_t texture_coordinates[MAX_VERTICES];
};


/*
==================
==================
*/
void Vertices_Max_Min(

	const float3_ vertices[],
	const __int32 n_vertices,
	float3_& max_out,
	float3_& min_out
)
{
	__m128 max[4];
	__m128 min[4];

	for (__int32 i_axis = X; i_axis < W; i_axis++) {
		max[i_axis] = min[i_axis] = set_all(vertices[0].f[i_axis]);
	}

	for (__int32 i_vertex_4 = 0; i_vertex_4 < n_vertices; i_vertex_4 += 4) {

		__int32 n = min(n_vertices - i_vertex_4, 4);

		__m128 vertex[4];
		for (__int32 i_vertex = 0; i_vertex < n; i_vertex++) {
			vertex[i_vertex] = load_u(vertices[i_vertex_4 + i_vertex].f);
		}
		Transpose(vertex);

		for (__int32 i = X; i < W; i++) {
			max[i] = max_vec(max[i], vertex[i]);
			min[i] = min_vec(min[i], vertex[i]);
		}
	}
	Transpose(max);
	Transpose(min);

	for (__int32 i = Y; i < W; i++) {
		max[X] = max_vec(max[i], max[X]);
		min[X] = min_vec(min[i], min[X]);
	}

	float temp_max[4];
	store_u(max[X], temp_max);
	float temp_min[4];
	store_u(min[X], temp_min);
	
	for (__int32 i_axis = X; i_axis < W; i_axis++) {
		max_out.f[i_axis] = temp_max[i_axis];
		min_out.f[i_axis] = temp_min[i_axis];
	}
}

/*
==================
==================
*/
void Model_Centre_Extent(

	const float3_ vertices[],
	const __int32 n_vertices,
	float3_& centre,
	float3_& extent
)
{

	float3_ max;
	float3_ min;
	Vertices_Max_Min(vertices, n_vertices, max, min);

	const float half = 0.5f;
	for (__int32 i = X; i < W; i++) {
		centre.f[i] = (max.f[i] + min.f[i]) * half;
		extent.f[i] = (max.f[i] - min.f[i]) * half;
	}
}

/*
==================
==================
*/
void Load_Quake_Model(

	const __int8* file_name_model,
	const __int32 i_skin_select,
	model_& model,
	memory_chunk_& memory

	)
{

	quake_mdl_* quake_mdl = new quake_mdl_;

	FILE *file_handle = NULL;
	fopen_s(&file_handle, file_name_model, "rb");
	if (file_handle == NULL) {
		printf_s("FAILED to open %s \n", file_name_model);
		exit(0);
	}
	else{
		printf_s("opened %s \n", file_name_model);
	}

	// ----------------------------------------------------------------------------------------------------------
	// read file header
	fread(&quake_mdl->file_header, sizeof(mdl_header_t), 1, file_handle);

	//printf_s("ID STRING: %i \n", quake_mdl->file_header.ident);
	//printf_s("VERSION: %i \n", quake_mdl->file_header.version);
	//printf_s("NUM SKINS: %i \n", quake_mdl->file_header.num_skins);
	//printf_s("SKIN SIZE: %i \n", quake_mdl->file_header.skinheight * quake_mdl->file_header.skinwidth);
	//printf_s("NUM VERTS: %i \n", quake_mdl->file_header.num_verts);
	//printf_s("NUM TRIS: %i \n", quake_mdl->file_header.num_tris);
	//printf_s("NUM FRAMES: %i \n", quake_mdl->file_header.num_frames);
	// ----------------------------------------------------------------------------------------------------------
	{

		const __int32 skin_size = quake_mdl->file_header.skinheight * quake_mdl->file_header.skinwidth;

		__int32 num_bits_height = _mm_popcnt_u32(quake_mdl->file_header.skinheight);
		__int32 num_bits_width = _mm_popcnt_u32(quake_mdl->file_header.skinwidth);
		//assert((num_bits_height == 1) && (num_bits_width == 1));		// pow2 tex
		assert(skin_size < quake_mdl_::MAX_SKIN_SIZE);
		assert(quake_mdl->file_header.num_verts < quake_mdl_::MAX_VERTICES);
		assert(quake_mdl->file_header.num_tris < quake_mdl_::MAX_TRIANGLES);

		//texture_attribute_& texture_attribute = texture_manager.attributes[texture_manager.n_textures];
		//texture_attribute.id = texture_id;
		//texture_attribute.texture[0] = (unsigned __int32*)memory.chunk_ptr;

		model.n_textures = 2;		// allow for 2ndary textures by default
		model.texture_handlers = (texture_handler_*)memory.chunk_ptr;
		memory.chunk_ptr = model.texture_handlers + model.n_textures;

		unsigned long width_shift = 0;
		_BitScanForward(&width_shift, quake_mdl->file_header.skinwidth);
		unsigned long height_shift = 0;
		_BitScanForward(&height_shift, quake_mdl->file_header.skinheight);

		for (__int32 i_texture = 0; i_texture < model.n_textures; i_texture++) {

			model.texture_handlers[i_texture].height = quake_mdl->file_header.skinheight;
			model.texture_handlers[i_texture].width = quake_mdl->file_header.skinwidth;
			model.texture_handlers[i_texture].width_shift = width_shift;
			model.texture_handlers[i_texture].height_shift = height_shift;
		}

		model.texture_handlers[0].texture[0] = (unsigned __int32*)memory.chunk_ptr;
		model.texture_handlers[1].texture[0] = model.texture_handlers[0].texture[0];

		//printf_s("width: %i, height: %i \n", quake_mdl->file_header.skinheight, quake_mdl->file_header.skinwidth);


		for (__int32 i_skin = 0; i_skin < quake_mdl->file_header.num_skins; i_skin++){

			mdl_skin_t skin_header;
			fread(&skin_header.group, sizeof(__int32), 1, file_handle);
			fread(quake_mdl->colour_indices, 1, skin_size, file_handle);

			if (i_skin_select == i_skin) {

				for (__int32 i_index = 0; i_index < skin_size; i_index++) {

					unsigned char colour_index = quake_mdl->colour_indices[i_index];
					unsigned char r = colour_map[colour_index].channel[R];
					unsigned char g = colour_map[colour_index].channel[G];
					unsigned char b = colour_map[colour_index].channel[B];
					unsigned __int32 final_colour = b | (g << 8) | (r << 16);

					model.texture_handlers[0].texture[0][i_index] = final_colour;
				}
			}
		}

		memory.chunk_ptr = model.texture_handlers[0].texture[0] + (quake_mdl->file_header.skinheight * quake_mdl->file_header.skinwidth);

		Build_MIP_Map_Chain(model.texture_handlers[0], memory);
	}
	// ----------------------------------------------------------------------------------------------------------
	/* Texture coords */

	{

		fread(&quake_mdl->texture_coordinates, sizeof(mdl_texcoord_t), quake_mdl->file_header.num_verts, file_handle);
	}
	// ----------------------------------------------------------------------------------------------------------
	/* Triangle info */

	{

		fread(&quake_mdl->triangles, sizeof(mdl_triangle_t), quake_mdl->file_header.num_tris, file_handle);
	}
	// ----------------------------------------------------------------------------------------------------------
	{

	}
	// ----------------------------------------------------------------------------------------------------------
	{

		{
			model.frame_origin = (float3_*)memory.chunk_ptr;
			memory.chunk_ptr = model.frame_origin + quake_mdl->file_header.num_frames;

			model.frame_extent = (float3_*)memory.chunk_ptr;
			memory.chunk_ptr = model.frame_extent + quake_mdl->file_header.num_frames;
		}
		{
			model.frame_name = (char(*)[16])memory.chunk_ptr;
			memory.chunk_ptr = model.frame_name + quake_mdl->file_header.num_frames;
		}
		{
			model.vertices_frame = (float3_**)memory.chunk_ptr;
			memory.chunk_ptr = model.vertices_frame + quake_mdl->file_header.num_frames;

			for (__int32 i_frame = 0; i_frame < quake_mdl->file_header.num_frames; i_frame++) {

				model.vertices_frame[i_frame] = (float3_*)memory.chunk_ptr;
				memory.chunk_ptr = model.vertices_frame[i_frame] + quake_mdl->file_header.num_verts;
			}
		}

		model.translate.x = quake_mdl->file_header.translate[X];
		model.translate.y = quake_mdl->file_header.translate[Y];
		model.translate.z = quake_mdl->file_header.translate[Z];

		//__int32 i_vertex_write = 0;
		for (__int32 i_frame = 0; i_frame < quake_mdl->file_header.num_frames; i_frame++){

			mdl_frame_t frame_node;

			fread(&frame_node.type, sizeof(__int32), 1, file_handle);
			fread(&frame_node.frame.bboxmin, sizeof(mdl_vertex_t), 1, file_handle);
			fread(&frame_node.frame.bboxmax, sizeof(mdl_vertex_t), 1, file_handle);
			fread(&frame_node.frame.name, sizeof(char), 16, file_handle);

			string_copy(frame_node.frame.name, model.frame_name[i_frame]);

			//printf_s("FRAME: %i | NAME: %s \n", i_frame, model.frame_name[i_frame]);

			for (__int32 i_axis = X; i_axis < W; i_axis++) {

				float max = (float)(frame_node.frame.bboxmax.v[i_axis]) * quake_mdl->file_header.scale[i_axis];
				float min = (float)(frame_node.frame.bboxmin.v[i_axis]) * quake_mdl->file_header.scale[i_axis];
				model.frame_origin[i_frame].f[i_axis] = (max + min) * 0.5f;
				model.frame_extent[i_frame].f[i_axis] = (max - min) * 0.5f;
			}

			mdl_vertex_t temp_vertices[quake_mdl_::MAX_VERTICES];

			fread(&temp_vertices, sizeof(mdl_vertex_t), quake_mdl->file_header.num_verts, file_handle);

			for (__int32 i_vertex = 0; i_vertex < quake_mdl->file_header.num_verts; i_vertex++){

				float x = ((float)temp_vertices[i_vertex].v[X] * quake_mdl->file_header.scale[X]) + quake_mdl->file_header.translate[X];
				float y = ((float)temp_vertices[i_vertex].v[Y] * quake_mdl->file_header.scale[Y]) + quake_mdl->file_header.translate[Y];
				float z = ((float)temp_vertices[i_vertex].v[Z] * quake_mdl->file_header.scale[Z]) + quake_mdl->file_header.translate[Z];

				model.vertices_frame[i_frame][i_vertex].x = -x;
				model.vertices_frame[i_frame][i_vertex].y = z;
				model.vertices_frame[i_frame][i_vertex].z = y;
			}
		}
		model.n_frames = quake_mdl->file_header.num_frames;

		{


		}
	}

	model.n_triangles = quake_mdl->file_header.num_tris;
	model.n_vertices = quake_mdl->file_header.num_verts;
	//model.n_texture_vertices = quake_mdl->file_header.num_tris * 3;
	model.n_texture_vertices = quake_mdl->file_header.num_verts;
	model.n_texture_layers = 2;

	const __int32 skin_width = quake_mdl->file_header.skinwidth;
	const __int32 skin_height = quake_mdl->file_header.skinheight;

	// ----------------------------------------------------------------------------------------------------------
	{
		model.n_colour_vertices = model.n_vertices;
		model.attribute_vertices[model_::ATTRIBUTE_COLOUR] = (float3_*)memory.chunk_ptr;
		memory.chunk_ptr = model.attribute_vertices[model_::ATTRIBUTE_COLOUR] + model.n_colour_vertices;

		for (__int32 i_vertex = 0; i_vertex < model.n_colour_vertices; i_vertex++) {
			model.attribute_vertices[model_::ATTRIBUTE_COLOUR][i_vertex] = { 0.0f, 0.0f, 0.0f };
		}
	}
	// ----------------------------------------------------------------------------------------------------------
	{
		//model.texture_vertices = (float2_**)memory.chunk_ptr;
		//memory.chunk_ptr = model.texture_vertices + model.n_texture_layers;
		model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY] = (float3_*)memory.chunk_ptr;
		memory.chunk_ptr = model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY] + (quake_mdl->file_header.num_tris * 3);
		model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_SECONDARY] = model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY];
	}
	// ----------------------------------------------------------------------------------------------------------
	{
		__int32 i_vertex_write = 0;
		for (__int32 i_triangle = 0; i_triangle < quake_mdl->file_header.num_tris; i_triangle++) {

			bool is_front_face = quake_mdl->triangles[i_triangle].facesfront != 0;

			float3_ vertices_mdl[3];
			for (__int32 i_vertex = 0; i_vertex < 3; i_vertex++) {

				__int32 index = quake_mdl->triangles[i_triangle].vertex[i_vertex];
				bool is_on_seam = quake_mdl->texture_coordinates[index].onseam != 0;
				float s = (float)quake_mdl->texture_coordinates[index].s;
				float t = (float)quake_mdl->texture_coordinates[index].t;
				float add = (float)skin_width * 0.5f;
				s += blend(add, 0.0f, (!is_front_face) && is_on_seam);
				vertices_mdl[i_vertex].x = (s + 0.5f) / (float)skin_width;
				vertices_mdl[i_vertex].y = (t + 0.5f) / (float)skin_height;
			}

			model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_vertex_write + 2] = vertices_mdl[0];
			model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_vertex_write + 1] = vertices_mdl[1];
			model.attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_vertex_write + 0] = vertices_mdl[2];

			i_vertex_write += 3;
		}
	}
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 n_indices = quake_mdl->file_header.num_tris * 3;
		model.i_vertices = (__int32*)memory.chunk_ptr;
		//model.i_colour_vertices = model.i_vertices + n_indices;
		//memory.chunk_ptr = model.i_colour_vertices + n_indices;
		model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR] = model.i_vertices + n_indices;
		memory.chunk_ptr = model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR] + n_indices;

		//model.i_texture_vertices = (__int32**)memory.chunk_ptr;
		//memory.chunk_ptr = model.i_texture_vertices + model.n_texture_layers;
		//model.i_texture_vertices[0] = (__int32*)memory.chunk_ptr;
		//memory.chunk_ptr = model.i_texture_vertices[0] + n_indices;
		//model.i_texture_vertices[1] = model.i_texture_vertices[0];

		model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY] = (__int32*)memory.chunk_ptr;
		memory.chunk_ptr = model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY] + n_indices;
		model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_SECONDARY] = model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY];
	}
	// ----------------------------------------------------------------------------------------------------------
	__int32 i_index = 0;
	for (__int32 i_triangle = 0; i_triangle < quake_mdl->file_header.num_tris; i_triangle++){

		__int32 v0 = quake_mdl->triangles[i_triangle].vertex[2];
		__int32 v1 = quake_mdl->triangles[i_triangle].vertex[1];
		__int32 v2 = quake_mdl->triangles[i_triangle].vertex[0];

		model.i_vertices[i_index + 0] = v0;
		model.i_vertices[i_index + 1] = v1;
		model.i_vertices[i_index + 2] = v2;

		//model.i_colour_vertices[i_index + 0] = v0;
		//model.i_colour_vertices[i_index + 1] = v1;
		//model.i_colour_vertices[i_index + 2] = v2;
		model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR] [i_index + 0] = v0;
		model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR] [i_index + 1] = v1;
		model.i_attribute_vertices[model_::ATTRIBUTE_COLOUR] [i_index + 2] = v2;

		model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_index + 0] = i_index + 0;
		model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_index + 1] = i_index + 1;
		model.i_attribute_vertices[model_::ATTRIBUTE_TEXTURE_PRIMARY][i_index + 2] = i_index + 2;

		i_index += 3;
	}
	// ----------------------------------------------------------------------------------------------------------
	{
		model.i_textures = (__int32**)memory.chunk_ptr;
		memory.chunk_ptr = model.i_textures + model.n_texture_layers;
		model.i_textures[0] = (__int32*)memory.chunk_ptr;
		memory.chunk_ptr = model.i_textures[0] + model.n_triangles;
		model.i_textures[1] = (__int32*)memory.chunk_ptr;
		memory.chunk_ptr = model.i_textures[1] + model.n_triangles;

		for (__int32 i_triangle = 0; i_triangle < model.n_triangles; i_triangle++) {

			model.i_textures[0][i_triangle] = 0;
			model.i_textures[1][i_triangle] = 1;
		}
	}
	// ----------------------------------------------------------------------------------------------------------

	fclose(file_handle);

	// ----------------------------------------------------------------------------------------------------------
	// animation origin offset
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 n_vertices = model.n_vertices;
		float3_ centre;
		float3_ extent;

		Model_Centre_Extent(model.vertices_frame[0], n_vertices, centre, extent);

		model.bounding_origin = centre; // +quake_model.translate;
		model.bounding_extent = extent;
	}

	delete quake_mdl;
}


