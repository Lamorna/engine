
#pragma once

#include "collide.h"
#include "component.h"
#include "model.h"
#include "texture.h"

//======================================================================

struct user_interface_ {

	enum {
		NUM_ELEMENTS = 9,
	};

	__int32 frame_rate;
	texture_handler_ texture_handler[NUM_ELEMENTS];
};

struct particle_manager_ {

	enum {

		NUM_PARTICLES_X = 2,
		NUM_PARTICLES_Y = 4,
		NUM_PARTICLES_Z = 2,

		NUM_EMITTERS = 4,
		NUM_PARTICLES_PER_EMITTER = NUM_PARTICLES_X * NUM_PARTICLES_Y * NUM_PARTICLES_Z,
		NUM_PARTICLES_TOTAL = NUM_PARTICLES_PER_EMITTER * NUM_EMITTERS,

	};

	struct emitter_ {

		__int32 start_id;
		__int32 n_particles;
	};

	struct template_ {

		float3_ position[NUM_PARTICLES_PER_EMITTER];
		float3_ velocity[NUM_PARTICLES_PER_EMITTER];
		float3_ scale;
		float3_ colour;
	};

	template_ impact;
	template_ trapdoor;

	__int32 i_emitter;
	emitter_ emitter[NUM_EMITTERS];
	
};

struct way_point_manager_ {

	enum {

		MAX_PATROLS = 8,
		MAX_POINTS = 16,
	};

	__int32 n_patrols;
	__int32 n_points[MAX_PATROLS];
	__int32 point_ids[MAX_PATROLS][MAX_POINTS];
};

struct animation_data_ {

	struct id_ {

		enum {

			NULL_ = -1,
			IDLE,
			WALK,
			RUN,
			ATTACK,
			PAIN,
			DEATH,
			DEAD,

			COUNT,
		};
	};

	__int32 id;
	__int32 i_start;
	__int32 n_frames;
};

struct animation_model_ {

	struct id_ {

		enum {

			PLAYER,
			SHAMBLER,
			COUNT_NAMED,

			MAX_MODELS = 16,
		};
	};

	__int32 model_id;
	animation_data_ animation_data[animation_data_::id_::COUNT];

};

struct animation_manager_ {

	__int32 n_animated_models;
	animation_model_ animation_model[animation_model_::id_::MAX_MODELS];
};

struct map_validate {

	enum {
		MAX_BLOCKS = 1024,
		NUM_VERTICES_PER_BLOCK = 8,
		NUM_TRIANGLES_PER_BLOCK = 12,
		MAX_TEMP_VERTEX_SIZE = 1 << 13,

		MAX_MAP_BLOCKS = 1 << 12,
	};

	__int32 n_blocks;
	__int32 i_triangles[MAX_BLOCKS][NUM_TRIANGLES_PER_BLOCK];
	__m128 vertices[MAX_TEMP_VERTEX_SIZE];
};




struct impact_ {

	bool is_impact;
	float3_ position;
};

//======================================================================

struct behaviour_manager_;
struct lightmap_manager_;
struct entity_manager_;
struct component_data_;
struct model_token_manager_;
struct timer_;
struct draw_call_;
struct sound_triggers_;
struct sound_event_;
struct command_buffer_handler_;
struct model_;
struct model_manager_;

void Initialise_Systems(

	sound_event_[colliding_type_::COUNT][colliding_type_::COUNT],
	animation_manager_&,
	behaviour_manager_&,
	collision_manager_&,
	way_point_manager_&,
	lightmap_manager_&,
	model_manager_&,
	component_data_&,
	model_token_manager_&,
	timer_&,
	particle_manager_&,
	command_buffer_handler_&,
	systems_::collision_response_&,
	grid_&,
	memory_chunk_&
);

void Load_Map_Elements(

	model_token_manager_&
);

void Specialise_Cube_Template(

	model_manager_&,
	memory_chunk_&
);

void Load_External_Models(

	model_manager_&,
	memory_chunk_&
);

void Create_BVH(

	model_token_manager_&,
	lightmap_manager_&,
	model_manager_&,
	memory_chunk_&,
	grid_&

);

void Get_System_Info(thread_pool_&);
void Load_UI(user_interface_&, memory_chunk_&);










