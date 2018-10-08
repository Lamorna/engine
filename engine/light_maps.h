
#pragma once

#include "vector.h"
#include "master.h"
#include "threads.h"
#include "setup.h"

//======================================================================

struct lightmap_data_ {

	__int32 i_read;
	__int32 i_lightmap[2];
	float3_ dx_lightmap;
	float3_ dy_lightmap;
	float3_ origin;
	float3_ dx_position;
	float3_ dy_position;
};

struct lightmap_manager_ {

	enum {
		N_SURFACES_PER_MODEL = 6,
		MAX_MODEL_NODES = 512,
		MAX_ACTIVE_MODELS = 128,
	};

	struct bvh_ {

		struct lit_model_data_ {

			__int32 i_model;
			__int32 i_node;
			__int32 i_start;
			__int32 n_models;
		};

		enum {

			MAX_LIT_MODELS = 16,
			MAX_LIGHT_MODELS = 256,
		};

		__int32 n_lit_models;
		lit_model_data_ lit_model_data[MAX_LIT_MODELS];
		//__int32 i_lit_models[MAX_LIT_MODELS];
		//__int32 i_lit_model_nodes[MAX_LIT_MODELS];
		//__int32 i_light_models[MAX_LIT_MODELS][MAX_LIGHT_MODELS];
		//__int32 n_light_models[MAX_LIT_MODELS];

		__int32 i_light_models[MAX_LIGHT_MODELS];
	};

	struct model_node_ {

		enum {
		};

		unsigned __int32 active_surface_mask;
		lightmap_data_ lightmap_data[N_SURFACES_PER_MODEL];
	};

	struct active_node_handler_ {

		__int32 n_active_nodes;
		__int32 i_node[MAX_ACTIVE_MODELS];
		__int32 i_model_node[MAX_ACTIVE_MODELS];
	};

	bvh_ bvh;
	grid_* grid_TEMP;

	__int32 n_model_nodes[grid_::NUM_NODES + 1];

	__int32 map_nodes_TEMP[grid_::MAX_ENTRIES];
	__int32 map_models_TEMP[grid_::MAX_ENTRIES];

	CACHE_ALIGN_PROPER model_node_ model_nodes[grid_::NUM_NODES + 1][MAX_MODEL_NODES];
	CACHE_ALIGN_PROPER active_node_handler_ active_node_handlers[thread_pool_::MAX_WORKER_THREADS];
};

struct lightmap_bin_ {

	enum {
		MAX_ENTRIES = 256,
	};

	__int32 i_node;
	__int32 i_model_node;
	__int32 n_light_sources;
	__int32 i_light_sources[MAX_ENTRIES];
};


//======================================================================

struct texture_manager_;
struct component_data_;
struct component_buffer_;
struct model_token_;
struct model_;
struct model_manager_;
struct timer_;
struct thread_pool_;
struct archetype_data_;
struct command_buffer_handler_;
struct memory_chunk_;


void Initialise_Light_Maps(

	const lightmap_manager_&,
	model_manager_&
);

void Compute_Light_Map_Gradients(

	const model_token_manager_&,
	const model_&,
	lightmap_manager_&,
	model_manager_&
);

void Fade_Light_Map(

	const timer_&,
	lightmap_manager_&,
	texture_manager_&
);

void Allocate_Memory_Lightmaps(

	const __int32,
	const model_token_&,
	lightmap_manager_&,
	model_&,
	memory_chunk_&
);

struct systems_::lightmap_ {

	static void process_BVH(void*, __int32);
	static void process_lightmaps(void*, __int32);
	static void fade_lightmaps(void*, __int32);
	static void buffer_swap(void*, __int32);
};

struct parameters_::lightmap_ {

	enum {

		MAX_BATCHES = 32,
	};

	struct process_lightmaps_ {

		__int32 i_lit_model_index;
		const command_buffer_handler_* command_buffer_handler;
		lightmap_manager_* lightmap_manager;
		model_manager_* model_manager;
		model_* model_spotlight;
	};
	struct process_BVH_ {

		const command_buffer_handler_* command_buffer_handler;
		lightmap_manager_* lightmap_manager;
		grid_* grid;
	};
	struct fade_lightmaps_ {

		timer_* timer;
		lightmap_manager_* lightmap_manager;
		model_* model_map;
	};
	struct buffer_swap_ {

		__int32 n_threads;
		lightmap_manager_* lightmap_manager;
		model_manager_* model_manager;
	};

	process_BVH_ process_BVH;
	process_lightmaps_ process_lightmaps[MAX_BATCHES];
	fade_lightmaps_ fade_lightmaps;
	buffer_swap_ buffer_swap;
};

//======================================================================

