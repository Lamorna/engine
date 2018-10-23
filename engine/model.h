#pragma once

#include "vector.h"

struct memory_chunk_;
struct texture_handler_;

struct grid_ {

	enum {
		NUM_NODES = 8,
		MAX_ENTRIES = 512,
	};

	struct type_ {

		struct id_ {

			enum {

				STATIC,
				MOVING,
				ROTATED,

				COUNT,
			};
		};
	};

	struct node_ {

		struct header_ {

			int3_ centre;
			int3_ extent;
			__int32 n_entries;
		};

		header_ map;
		header_ collide;

		__int32 indices[MAX_ENTRIES];
	};

	struct data_ {

		__int32 type_id;
		__int32 entity_id;
		__int32 entity_type;
		__int32 i_entity;
		__int32 i_archetype;
	};

	__int32 n_entries;
	__int32 n_entries_map;

	node_ nodes[NUM_NODES];

	data_ data[MAX_ENTRIES];
	int3_ centre[MAX_ENTRIES];
	int3_ extent[MAX_ENTRIES];
	int3_ bounding_box[MAX_ENTRIES];
	int3_ normals[MAX_ENTRIES][3];

};

struct model_token_ {

	struct id_ {

		enum {

			MAP,
			CAMERA = grid_::NUM_NODES + 1,
			PLATFORM,
			BOUNCE_PAD,
			TELEPORTER,
			DOOR,
			ITEM,
			SHAMBLER,
			PATROL_ONE,
			PATROL_TWO,
			SCRAG,
			SCRAG_WAYPOINT,
			TRAP_DOOR,
			PLATE,
			BUTTON,
			PROP,
			MOB,

			COUNT,
		};
	};

	__int32 n_models;
	__int32* index;
	int3_* centre;
	int3_* extent;
	float3_(*normals)[6];
};

struct model_token_manager_ {

	enum {

		MAX_BLOCKS = 1 << 12,

	};

	__int32 n_blocks;
	__int32 index[MAX_BLOCKS];
	model_token_ model_tokens[model_token_::id_::COUNT];
	int3_ centre[MAX_BLOCKS];
	int3_ extent[MAX_BLOCKS];
	float3_ normals[MAX_BLOCKS][6];
};

struct model_ {

	struct id_ {

		enum {

			CUBE,
			CUBE_BACKFACE,
			PLATFORM,
			BOUNCE_PAD,
			PLATE,
			BUTTON,
			SPOTLIGHT,
			PATROL_POINT,
			PARTICLE,
			SKY_BOX,
			TELEPORTER,
			DOOR,
			LAVA,
			CLOUD,
			MAP,

			COUNT_NAMED = MAP + grid_::NUM_NODES + 1,
		};
	};

	enum {

		ATTRIBUTE_COLOUR,
		ATTRIBUTE_TEXTURE_PRIMARY,
		ATTRIBUTE_TEXTURE_SECONDARY,
		MAX_ATTRIBUTES_TEMP,

		MAX_MODEL_NAME_CHARS = 128,
		MAX_FRAME_NAME_CHARS = 16,
	};

	__int32	n_triangles;
	__int32 n_frames;
	__int32	n_vertices;
	__int32	n_textures;
	__int32	n_colour_vertices;
	__int32 n_texture_vertices;
	__int32 n_texture_layers;
	__int32* i_vertices;
	__int32** i_textures;
	char name[MAX_MODEL_NAME_CHARS];
	float3_ bounding_origin;
	float3_ bounding_extent;
	float3_ translate;
	float3_* frame_origin;
	float3_* frame_extent;
	float3_** vertices_frame;
	texture_handler_* texture_handlers;
	char(*frame_name)[MAX_FRAME_NAME_CHARS];

	__int32* i_attribute_vertices[MAX_ATTRIBUTES_TEMP];
	float3_* attribute_vertices[MAX_ATTRIBUTES_TEMP];

};

struct model_manager_ {

	enum {

		MAX_MODELS = 128,
	};

	__int32 n_models;
	model_ model[MAX_MODELS];
};

void Create_Map_Model(

	const model_token_&,
	const model_&,
	const float3_&,
	model_&,
	memory_chunk_&
);

void Create_Cube_Model(

	const __int32,
	const char*[],
	const model_&,
	model_&,
	memory_chunk_&
);

void Hardcode_Cube_Template(

	model_&,
	model_&,
	memory_chunk_&
);

__int32 Return_Model_ID(

	const char[],
	const model_manager_&
);
