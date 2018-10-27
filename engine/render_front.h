
#pragma once

#include "master.h"
#include "vector.h"
#include "threads.h"
#include "setup.h"
#include "raster.h"

//======================================================================

struct model_;
struct tile_64_;
struct display_;
struct draw_call_;
struct command_buffer_;
struct command_buffer_handler_;
struct screen_bin_;
struct draw_command_;
struct component_data_;
struct component_buffer_;
struct raster_output_;
struct shader_data_;
struct archetype_data_;
struct bin_triangle_data_;
struct model_manager_;
struct texture_handler_;

enum {

	PLANE_LEFT,
	PLANE_RIGHT,
	PLANE_TOP,
	PLANE_BOTTOM,
	PLANE_NEAR,
	PLANE_FAR,

	NUM_PLANES,

	PLANE_DEGENERATE_W = PLANE_FAR + 1,
};

enum {

	INTERPOLANT_COLOUR,
	INTERPOLANT_TEXCOORDS,
	INTERPOLANT_BLEND_WEIGHT,

	MAX_INTERPOLANTS = 6,

	MAX_VERTEX_COMPONENTS = 4,
	NUM_VERTICES = 3,

};



struct bin_triangle_data_ {

	__int32 i_triangle;
	__int32 i_model;
	__int32 draw_id;
};

struct light_source_ {

	__int32 id;
	float3_ position;
	float3_ colour;
	float intensity;
};

struct vertex_light_manager_ {

	enum {

		MAX_LIGHTS = 1 << 8,
		NUM_BINS = 10,
	};

	static float bin_interval;
	light_source_ light_sources[MAX_LIGHTS];
	__int32 i_light[MAX_LIGHTS];
	__int32 i_bin[MAX_LIGHTS];

	struct bin_ {

		__int32 n_lights;
		__int32 i_start;
	};

	bin_ bin[NUM_BINS];
};


struct draw_call_ {

	struct id_ {

		enum {

			MAP, SHAMBLER = MAP + grid_::NUM_NODES + 1, PLAYER, SCRAG, BOUNCE_PAD, TELEPORTER, PLATFORMS, DOOR, ITEMS,
			PARTICLES, PROJECTILES, PATROL_POINTS, TRAP_DOOR, BUTTON, PLATE, CLOUDS, LAVA, SKYBOX, COUNT,
		};
	};

	enum {

		MAX_PIXEL_SHADER_FUNCTIONS = 4,
	};

	enum {

		MAX_MODELS = 1024,

	};

	struct animation_data_ {

		__int32 i_frames[2];
		float frame_interval;
		float3_ origin;
	};

	struct attribute_stream_ {

		struct stride_ {

			enum {

				COLOUR_VERTEX = 3,
				TEXTURE_VERTEX = 3,
			};
		};

		struct id_ {

			enum {

				COLOUR_VERTEX,
				TEXTURE_VERTEX,
			};
		};

		__int32 id;
		__int32 stride;
		//__int32* i_vertices;
		//float* vertices;
		//__int32* i_textures;
		//texture_handler_* texture_handlers;


		void(*vertex_shader)(

			const command_buffer_&,
			const draw_call_&,
			const __int32,
			float4_[3]

			);

		//void(*pixel_shader)(shader_input_&);
	};

	__int32 n_models;
	//__int32 entity_id[MAX_MODELS];
	__int32 model_id[MAX_MODELS];
	int3_ position[MAX_MODELS];
	float3_ scale[MAX_MODELS];
	float3_ colour[MAX_MODELS];
	matrix_ m_vertex_colour[MAX_MODELS];
	matrix_ m_vertex_texture[MAX_MODELS];
	matrix_ m_rotate[MAX_MODELS];
	animation_data_ animation_data[MAX_MODELS];
	//float blend_interval[MAX_MODELS];
	__int32 i_texture_offset[MAX_MODELS];

	__int32 i_thread;
	__int32 n_attributes;
	float mip_level_bias;
	//__int32 const* i_vertices;
	//float3_ const* vertices;

	__int32 n_additional_pixel_shaders;
	void(*additional_pixel_shaders[NUM_VERTEX_ATTRIBUTES])(shader_input_&);

	attribute_stream_ attribute_streams[NUM_VERTEX_ATTRIBUTES];

	void(*lighting_function)(

		const vertex_light_manager_& vertex_light_manager,
		const float3_ position[3],
		float4_ colour[3]
	);

	void(*renderer_function)(

		const __int32,
		const __int32,
		const model_manager_&,
		command_buffer_&,
		display_&
		);
};





struct bin_triangle_{

	float3_ position[3];
	float3_ barycentric[3];
};



struct screen_bin_ {

	enum {
		MAX_DRAW_CALLS = draw_call_::id_::COUNT,
		MAX_TRIANGLES_PER_BIN = 1 << 12,

	};

	__int32 n_draw_calls;
	__int32 draw_id[MAX_DRAW_CALLS];
	__int32 n_tris[MAX_DRAW_CALLS];

	__int32 n_triangles;
	bin_triangle_data_ bin_triangle_data[MAX_TRIANGLES_PER_BIN];
	bin_triangle_ bin_triangle[MAX_TRIANGLES_PER_BIN];

};

struct display_ {

	enum {

		DISPLAY_WIDTH = 1024,
		DISPLAY_HEIGHT = 768,

		WINDOW_WIDTH = DISPLAY_WIDTH,
		WINDOW_HEIGHT = DISPLAY_HEIGHT,

		TILE_SIZE = 64,
		BIN_SIZE_SHIFT = 7,
		BIN_SIZE = 0x1 << BIN_SIZE_SHIFT,
		N_BINS_X = (DISPLAY_WIDTH / BIN_SIZE) + ((DISPLAY_WIDTH % BIN_SIZE) > 0),
		N_BINS_Y = (DISPLAY_HEIGHT / BIN_SIZE) + ((DISPLAY_HEIGHT % BIN_SIZE) > 0),
		BUFFER_WIDTH = N_BINS_X * BIN_SIZE,
		BUFFER_HEIGHT = N_BINS_Y * BIN_SIZE,
	};

	screen_bin_ screen_bin[thread_pool_::MAX_WORKER_THREADS][N_BINS_Y][N_BINS_X];

	CACHE_ALIGN unsigned __int32 colour_buffer_bin[thread_pool_::MAX_WORKER_THREADS][BIN_SIZE * BIN_SIZE];
	CACHE_ALIGN float depth_buffer_bin[thread_pool_::MAX_WORKER_THREADS][BIN_SIZE * BIN_SIZE];

	float depth_tiles_4x4[thread_pool_::MAX_WORKER_THREADS][(BIN_SIZE * BIN_SIZE) / (4 * 4)];
	float depth_tiles_16x16[thread_pool_::MAX_WORKER_THREADS][(BIN_SIZE * BIN_SIZE) / (16 * 16)];
	float depth_tiles_64x64[thread_pool_::MAX_WORKER_THREADS][(BIN_SIZE * BIN_SIZE) / (64 * 64)];

	HWND handle_window;
	IDirect3DDevice9* d3d9_device;
	IDirect3DSurface9* d3d9_surface;
	D3DLOCKED_RECT locked_rect;

	raster_output_ raster_output[thread_pool_::MAX_WORKER_THREADS];
};

struct shader_input_ {

	__int32 i_triangle;
	float mip_level_bias;
	unsigned __int32* colour_buffer;
	float* depth_buffer;
	const texture_handler_* texture_handler;
	const draw_call_* draw_call;
	__m128 r_area;
	__m128 z_delta[3];
	__m128 barycentric[2][3];
	vertex4_ gradients[NUM_VERTEX_ATTRIBUTES][4];

	__int32 x;
	__int32 y;
	unsigned __int64 tile_mask_16x16;
	unsigned __int64 tile_mask_64x64;
	float z_max;
	float* depth_tiles_4x4;
	float* depth_tiles_16x16;
	float* depth_tiles_64x64;
	//float depth_interpolants[3];
};

//======================================================================

const __int32 FIXED_POINT_SHIFT = 8;
static const float fixed_point_scale_g = 256.0f;
const __m128 FIXED_POINT_SCALE = convert_float(broadcast(load_s(1 << FIXED_POINT_SHIFT)));

static const float screen_scale_x = (float)display_::DISPLAY_WIDTH / 2.0f;
static const float screen_scale_y = -(float)display_::DISPLAY_HEIGHT / 2.0f;
static const float screen_shift_x = screen_scale_x;
static const float screen_shift_y = -screen_scale_y;

//======================================================================

struct display_;
struct thread_pool_;
struct draw_call_;

void Renderer_FRONT_END(

	const __int32,
	const __int32,
	const __int32[],
	const float3_[],
	const matrix_&,
	const __int32[][2],
	screen_bin_[display_::N_BINS_Y][display_::N_BINS_X]
);

void Lock_Back_Buffer(display_&);
void Release_Back_Buffer(display_&);

struct systems_::render_ {

	static void lock_back_buffer(void*, __int32);
	static void release_back_buffer(void*, __int32);
	static void process_screen_bins(void*, __int32);
	static void render_UI(void*, __int32);


};

struct parameters_::render_ {

	struct process_draw_call_ {

		__int32 i_draw_call;
		const model_manager_* model_manager;
		command_buffer_handler_* command_buffer_handler;
		display_* display;
	};
	struct lock_back_buffer_ {

		display_* display;
	};
	struct release_back_buffer_ {

		display_* display;
	};
	struct process_screen_bins_ {

		enum {
			NUM_JOBS = display_::N_BINS_Y * display_::N_BINS_X,
		};

		__int32 i_bin;
		__int32 n_threads;
		const command_buffer_handler_* command_buffer_handler;
		const model_manager_* model_manager;
		display_* display;
	};
	struct render_UI_ {

		const command_buffer_handler_* command_buffer_handler;
		const user_interface_* user_interface;
		display_* display;
	};

	process_draw_call_ process_draw_call[draw_call_::id_::COUNT];
	lock_back_buffer_ lock_back_buffer;
	release_back_buffer_ release_back_buffer;
	process_screen_bins_ process_screen_bins[process_screen_bins_::NUM_JOBS];
	render_UI_ render_UI;
};




