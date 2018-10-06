
#pragma once

#include "master.h"
#include "threads.h"
#include "vector.h"


struct collision_manager_;
struct archetype_data_;
struct grid_;
struct model_token_manager_;

struct colliding_type_ {

		enum {

			NULL_ = -1,
			MAP,
			PLAYER,
			MONSTER,
			PROJECTILES,
			PARTICLES,
			DOOR,
			TELEPORTER,
			BOUNCE_PAD,
			PLATFORM,
			TRAP_DOOR,
			PLATE,
			BUTTON,
			MOB,

			COUNT,
		};
};


struct collision_response_type_ {

	enum {

		DEFLECT,
		REFLECT,
		STOP,
		PASS_THRU,
		REPULSE,

	};
};

struct parameters_::collide_ {

	enum {
		MAX_JOBS = 32,
	};

	struct collision_detection_ {

		__int32 i_begin;
		__int32 n_colliders_per_job;
		archetype_data_* archetype_data;
		collision_manager_* collision_manager;
		grid_* grid;
	};

	struct collision_BVH_ {

		archetype_data_* archetype_data;
		grid_* grid;
	};

	collision_BVH_ collision_BVH;
	collision_detection_ collision_detection[MAX_JOBS];
};

struct systems_::collide_ {

	static void collision_detection(void*, __int32);
	static void collision_BVH(void*, __int32);
};

struct collision_output_ {

	struct collider_ {

		__int32 entity_type;
		__int32 entity_id;
		__int32 i_archetype;
		__int32 i_entity;
		float3_ velocity;
	};
	struct collidee_ {

		__int32 entity_type;
		__int32 entity_id;
		__int32 i_archetype;
		__int32 i_entity;
		__int32 i_axis;
		int3_ position;
		int3_ total_extent;
		int3_ normal;
	};

	__int32 t_interval;
	int3_ collision_point;
	collider_ collider;
	collidee_ collidee;
};

struct thread_collision_ {

	enum {

		MAX_COLLISIONS = 1024,
		MAX_COLLISIONS_PER_THREAD = MAX_COLLISIONS / thread_pool_::MAX_WORKER_THREADS,
	};

	__int32 n_colliders;
	collision_output_ collision_output[MAX_COLLISIONS_PER_THREAD];

};

struct collision_manager_ {

	thread_collision_ thread_collisions[thread_pool_::MAX_WORKER_THREADS];

	__int32 i_response_type[colliding_type_::COUNT][colliding_type_::COUNT];
	bool allow_table[colliding_type_::COUNT][colliding_type_::COUNT];
	bool can_step[colliding_type_::COUNT][colliding_type_::COUNT];
	bool is_ground[colliding_type_::COUNT];
};

//======================================================================

struct component_data_;
struct component_buffer_;
struct collision_manager_;

//void Collision_Detection(
//
//	const __int32,
//	const __int32,
//	const component_data_&,
//	component_buffer_&,
//	collision_manager_&
//);


//======================================================================



